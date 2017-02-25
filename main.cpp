#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include "ReceiveITCH.h"
#include "Distributor.h"

#include "ParseFeed.h"
#include "SaveToDB.h"
#include "FillMsgStructs.h"

#include "main.h"
#include  "Settings.h"
#include "NQTVDlg.h"
#include "NQTV.h"

static pthread_mutex_t mtxQueue = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtxTick 	= PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t condMap 	= PTHREAD_COND_INITIALIZER;
static bool bReady = false;

//////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    using namespace std;

    Logger::instance().log("Starting Server", Logger::Debug);
    PrimeSettings();

    g_bSettingsLoaded = false;

    int iRet = 0;
    m_request.tv_sec = 0;
    m_request.tv_nsec = 100000000;   // 1/10 of a second


    g_SThreadData.iTotalThreads = 0;
    pthread_mutex_lock(&mtxQueue);

    for (uint ii=0;  ii < NUMBER_OF_ROLES; ii++ )
    {
        if (!theApp.SSettings.iarrRole[ii]) { // option is turned off.... = 0 or bool false
            arrThreadInfo[ii].eState = TS_INACTIVE;
            continue;
        }
        g_SThreadData.idx = ii;
//      g_SThreadData.pVoid = pCQuantQueue;  // only needed for the first Thread.... to construct the Queue
        arrThreadInfo[ii].iThread_num = ii ;
        iRet = pthread_create(&arrThreadInfo[ii].thread_id, NULL, func_ptr[ii], &g_SThreadData);
//        sleep(1);
        nanosleep (&m_request, NULL);  // sleep a 1/10 of a second
        g_SThreadData.iTotalThreads++;
        arrThreadInfo[ii].eState = TS_ALIVE;
        if(iRet)
        {
            Logger::instance().log("Error pthread_create", Logger::Error);
            exit(EXIT_FAILURE);
        }
        while (!bReady)
            pthread_cond_wait(&condMap, &mtxQueue); // Wait for the Queue to init first
    };
    pthread_mutex_unlock(&mtxQueue);

    int jj = 0;

    while (theApp.SSettings.iStatus != STOPPED) {
        jj++;
        sleep(3);

//       if (jj > 200)  // jj* 3 =  seconds
        if (jj > 10)  // jj* 3 =  seconds
//        if (jj > 300)  // jj* 3 =  seconds
            theApp.SSettings.iStatus = STOPPED;
    };

    int iJoined = 0;
    string strExitMessage;

    while (iJoined < g_SThreadData.iTotalThreads) {
        // keep on checking for all terminated threads every three seconds

        for (uint ii = 0;  ii < NUMBER_OF_ROLES; ii++ ) {
            if ((arrThreadInfo[ii].eState == TS_INACTIVE)|| (arrThreadInfo[ii].eState == TS_JOINED)|| (arrThreadInfo[ii].eState == TS_ALIVE)|| (arrThreadInfo[ii].eState == TS_STARTED))
                continue;
            if (arrThreadInfo[ii].eState == TS_TERMINATED) {
                pthread_join(arrThreadInfo[ii].thread_id, NULL);
                arrThreadInfo[ii].eState = TS_JOINED;

                strExitMessage.clear();
                strExitMessage = ThreadMessage[ii];
                strExitMessage += " Joined";
                Logger::instance().log(strExitMessage, Logger::Debug);
                iJoined++;
            }
        } // for loop
        sleep(3);
    } // while loop

    pthread_cond_destroy(&condMap);
    pthread_mutex_destroy(&mtxTick);
    pthread_mutex_destroy(&mtxQueue);

    Logger::instance().log("Destroyed conditional variable", Logger::Debug);
    Logger::instance().log("Normal Termination", Logger::Debug);
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
void* Settings(void* pArg)
{
    // 	open Settings file to fill the Settings structure
    //  cout << "Calling Settings Begin" << endl;
    pthread_mutex_lock(&mtxTick);
    THREAD_DATA SThreadData;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCQSettings = NULL;
    pCQSettings= new CQSettings();

    if (!pCQSettings) {
        Logger::instance().log("Error Getting instance of Settings Object", Logger::Error);
    }

    if (pCQSettings->GetError() > 0) {
        Logger::instance().log("Error Initializing Settings Object", Logger::Error);
        exit(EXIT_FAILURE);  //  for the calling process if any
    }
    // Do NOT call after starting the client....comment out or delete
    pCQSettings->LoadSettings(); // Do NOT call after starting the client....comment out or delete
    // Do NOT call after starting the client....comment out or delete
    g_bSettingsLoaded = true;

    while (pCQSettings->GetSettings().iStatus == RUNNING) {
        sleep(1);
        if (theApp.SSettings.iStatus == STOPPED)  //  Remove After the client is up
            break;
    }

    theApp.SSettings.iStatus = STOPPED;   // Will have to keep it here even after the client is up

    delete pCQSettings;
    pCQSettings = NULL;

    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return  NULL;

}
//////////////////////////////////////////////////////////////////////////////////////////
void  PrimeSettings()
{

    pCQSettings = NULL;
    pCQSettings= new CQSettings();

    if (!pCQSettings) {
        Logger::instance().log("Error Getting instance of Settings Object", Logger::Error);
    }
    if (pCQSettings->GetError() > 0) {
        Logger::instance().log("Error Initializing Settings Object", Logger::Error);
        exit(EXIT_FAILURE);  //  for the calling process if any
    }

    // Do NOT call after starting the client....comment out or delete
    theApp.SSettings = pCQSettings->LoadSettings(); // Do NOT call after starting the client....comment out or delete
    // Do NOT call after starting the client....comment out or delete

    theApp.SSettings.iStatus = RUNNING;

    delete pCQSettings;
    pCQSettings = NULL;

}
//////////////////////////////////////////////////////////////////////////////////////////
void*  MainQueue(void* pArg)
{

    pthread_mutex_lock(&mtxTick);
    THREAD_DATA SThreadData;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;


    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCQuantQueue = NULL;
    pCQuantQueue = CQuantQueue::Instance();


    if (!pCQuantQueue->bConstructed) {  // No need to continue
        Logger::instance().log("Error Constructing Queue...Aborting", Logger::Error);
//        delete pCQuantQueue;
//        pCQuantQueue = NULL;
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        exit(EXIT_FAILURE);
    }

    g_SThreadData.g_pCQuantQueue = pCQuantQueue;

    bReady = true;
    pthread_cond_signal(&condMap);  // so Build book to check on pCOrdersMap and decide to return

    while (theApp.SSettings.iStatus == RUNNING) { //  the last thread to terminate...check with the for loop within the while loop in main
        sleep(5);
    }

    pCQuantQueue->ListQStats();

    delete pCQuantQueue;
    pCQuantQueue = NULL;

    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return  NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////
void* OrdersMap(void* pArg)  // only if buid book is checked
{

    pthread_mutex_lock(&mtxTick);

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue)) ;

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCOrdersMap = NULL;
    pCOrdersMap =  COrdersMap::instance();

    if (!pCOrdersMap) {
        Logger::instance().log("Error Creating Orders Map Object", Logger::Error);
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }

    if (pCOrdersMap->GetError()) {
        delete pCOrdersMap;
        pCOrdersMap = NULL;
        Logger::instance().log("Error in Constructing Orders Map", Logger::Error);
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;  //  can't build a book w/o Memory Mappings
    }

    arrThreadInfo[idx].eState = TS_ALIVE;
    pCOrdersMap->InitQueue(pQueue);
    /*/    while (theApp.iStatus != STOPPED) {
            if (pCOrdersMap->GetError() > 0)
                break;
            pCOrdersMap->FillMemoryMappedFile();
        };
    */  // The above loop has been moved to the funtion below...with the same condition (theApp.iStatus != STOPPED)
    uint64_t ui64NumberOfOrders =  pCOrdersMap->FillMemoryMappedFile();

    pCOrdersMap->iNInstance--;
    Logger::instance().log("Waiting for last instance of the Orders Map", Logger::Info);
    while (pCOrdersMap->iNInstance > 0) {
        sleep(3);    //wait for last instance
    }
    Logger::instance().log("Deleting instance of the Orders Map", Logger::Info);
    delete pCOrdersMap;
    pCOrdersMap = NULL;

    Logger::instance().log("Orders Map destructed", Logger::Info);
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
//////////////////////////////////////////////////////////////////////////////////////////
void* TickDataMap(void* pArg)
{

    pthread_mutex_lock(&mtxTick);

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue)) ;

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCTickDataMap = new CTickDataMap;

    if (!pCTickDataMap) {
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

    arrThreadInfo[idx].eState = TS_ALIVE;
    pCTickDataMap->InitQueue(pQueue);

    /*    while (theApp.iStatus != STOPPED) {
            if (pCTickDataMap->GetError() > 0)
                break;
            pCTickDataMap->FillMemoryMappedFile();
        };
    */
    uint64_t ui64NumberOfTicks =  pCTickDataMap->FillTickFile();

    delete pCTickDataMap;
    pCTickDataMap = NULL;
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
//////////////////////////////////////////////////////////////////////////////////////////
void* BuildBook(void* pArg)
{

    pthread_mutex_lock(&mtxTick);

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue)) ;

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCBuildBook = NULL;
    pCBuildBook = new CBuildBook();
    if (!pCBuildBook) {
        // log error
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }
    arrThreadInfo[idx].eState = TS_ALIVE;

    if (pCBuildBook->m_iError) {
        delete pCBuildBook;
        pCBuildBook = NULL;
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

    while (theApp.SSettings.iStatus != STOPPED) {
        pCBuildBook->BuildBookFromMemoryMappedFile();
    }

    pCBuildBook->ListBook("MSFT    ");
    pCBuildBook->ListBook("INTC    ");
    pCBuildBook->ListBook("GOOG    ");


    delete pCBuildBook;
    pCBuildBook = NULL;

    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* NasdTestFile(void* pArg)
{

    pthread_mutex_lock(&mtxTick);

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue));

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCReceiveITCH = NULL;
    pCReceiveITCH = new CReceiveITCH(pQueue);

    if (!pCReceiveITCH) {
        Logger::instance().log("Error Creating ReceiveITCH Object", Logger::Error);
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }
    if (pCReceiveITCH->GetError() >= 100) {
        Logger::instance().log("Error Constructing ReceiveITCH Object", Logger::Error);
        delete pCReceiveITCH;
        pCReceiveITCH = NULL;
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }

    if (!pCReceiveITCH->ReadFromTestFile(theApp.SSettings.szTestFileName)) {
        Logger::instance().log("Error Reading From Test File", Logger::Error);
    }

    if (pCReceiveITCH) {
        delete pCReceiveITCH;
        pCReceiveITCH = NULL;
    }

    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
///////////////////////////////////////////////////////////////
void *DisplayBook(void* pArg)
{

    pthread_mutex_lock(&mtxTick);

    THREAD_DATA SThreadData;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;


    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

//    pCBuildBook
    pCDisplayBook = NULL;
    pCDisplayBook = new CDisplayBook(pCBuildBook);

    if (!pCDisplayBook) {
        Logger::instance().log("Error Creating DisplayBook Object", Logger::Error);
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }
    if (pCDisplayBook->GetError() >= 100) {
        Logger::instance().log("Error Constructing DisplayBook Object", Logger::Error);
        delete pCDisplayBook;
        pCDisplayBook = NULL;
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }

    theApp.SSettings.iSaveApply = OK;
    while (theApp.SSettings.iStatus != STOPPED) {
        if ((theApp.SSettings.iSaveApply == OK) || (theApp.SSettings.iSaveApply == APPLY)) { // Check for change in status
            pCDisplayBook->DisplaySelected(); // Display the selected by clicking the bool in arrbActive[NUMBER_OF_SYMBOLS ];
            theApp.SSettings.iSaveApply = CANCEL;
        }
        sleep(1);
    }

    if (pCDisplayBook) {
        delete pCDisplayBook;
        pCDisplayBook = NULL;
    }

    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* StatsPerSec(void* pArg)
{

    pthread_mutex_lock(&mtxTick);

    THREAD_DATA SThreadData;


    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCStatsPerSec = NULL;
    pCStatsPerSec = new CStatsPerSec();

    if (!pCStatsPerSec) {
        Logger::instance().log("Error Creating StatsPerSec Object", Logger::Error);
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }
    if (pCStatsPerSec->GetError() >= 100) {
        Logger::instance().log("Error Constructing StatsPerSec Object", Logger::Error);
        delete pCStatsPerSec;
        pCStatsPerSec = NULL;
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }

    while (theApp.SSettings.iStatus != STOPPED) {
        sleep(1);
    }

    if (pCStatsPerSec) {
        delete pCStatsPerSec;
        pCStatsPerSec = NULL;
    }

    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
///////////////////////////////////////////////////////////////
void *Distributor(void* pArg)
{

    pthread_mutex_lock(&mtxTick);
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue));

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    // Calls to functions and threads go here
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* ReceiveFeed(void* pArg)
{
    pthread_mutex_lock(&mtxTick);
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    // Calls to functions and threads go here
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
//////////////////////////////////////////////////////////////////////////////////////////
void* ParseFeed(void* pArg)
{
    pthread_mutex_lock(&mtxTick);

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);    // Calls to functions and threads go here
    pthread_mutex_unlock(&mtxTick);


    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);
    return pArg;
}
//////////////////////////////////////////////////////////////////////////////////////////
void* SaveToDB(void* pArg)
{
    pthread_mutex_lock(&mtxTick);
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCSaveToDB = NULL;
    pCSaveToDB = new CSaveToDB();

    // Call the worker thread here and check for !STOPPED
    // The worker thread has to check for the availability of the Memory Mapped Files ....Check here before you continue
    if (pCSaveToDB == NULL) {
        pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
        pthread_mutex_unlock(&mtxTick);
        return NULL;
    }

    // Call member functions here

    delete pCSaveToDB;
    pCSaveToDB = NULL;
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* PlayBack(void* pArg)
{

    pthread_mutex_lock(&mtxTick);
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);    //
    pthread_mutex_unlock(&mtxTick);

    // Call Object Methods here
    //
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);
    return pArg;
}
///////////////////////////////////////////////////////////////
void* SaveToDisk(void* pArg)
{
    pthread_mutex_lock(&mtxTick);
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    /*   pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue));



       pCSaveToDisk = NULL;

       pCSaveToDisk = new  CSaveToDisk();
       // check for class errors then log and exit
       // Calls to functions and threads go here
       while(theApp.iStatus != STOPPED) { // ::TODO  check from UI
           pCSaveToDisk->WriteFeedToFile();
       }

       delete pCSaveToDisk;
       pCSaveToDisk = NULL;

       pthread_mutex_lock(&mtxTick);
       TermThreadLog(idx);
       pthread_mutex_unlock(&mtxTick);
    */
    return pArg;
}
///////////////////////////////////////////////////////////////
void InitThreadLog(int idx)
{
    string  strLogMessage = ThreadMessage[idx];
    strLogMessage += " Starting";

    Logger::instance().log(strLogMessage, Logger::Info);
    arrThreadInfo[idx].eState = TS_STARTED;

}
///////////////////////////////////////////////////////////////
void TermThreadLog(int idx)
{
    string  strLogMessage = ThreadMessage[idx];
    strLogMessage += " Finished";

    Logger::instance().log(strLogMessage, Logger::Info);
    arrThreadInfo[idx].eState = TS_TERMINATED;
}
///////////////////////////////////////////////////////////////
static void *NQTVFunction(void* ptr)
{
    /*
      CNQTVDlg* pCNQTVDlg  = NULL;
      pCNQTVDlg =  new CNQTVDlg(); // All the data needed by this object is Global in "theApp" Object....sorry...Inheritted from the windows code

      //int iRet = pCNQTVDlg->ConnectMySql();  // check for return erro code before continuing
      conn = pCNQTVDlg->ConnectMySql(HOST_NAME, USER_NAME, PASSWORD, DB_NAME, PORT_NUM, SOCKET_NAME, 0);
      if (conn == NULL)
      {
    	  printf("Connection Failed\n");
    	  theApp.g_bConnected = false;
    	  if (pCNQTVDlg)
    	  {
    		  delete pCNQTVDlg;
    	  }
    	  return NULL;
      }
      else
      {
    	  printf("Connection: Success!\n");
    	  theApp.g_bConnected = true;
      }
    //  pCNQTVDlg->m_bTestFeed = true;

    //  pCNQTVDlg->FeedStart();   // Entry point to processing

      pCNQTVDlg->Disconnect1MySql();

        if (pCNQTVDlg)
          delete (pCNQTVDlg);
     */
    return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////
// int GUI()
// {
//     /* FOR REFERENCE: http://stackoverflow.com/questions/1519885/defining-own-main-functions-arguments-argc-and-argv?rq=1 */
//     char arg0[] = "QuantServer";
//     //char arg1[] = "arg";
//     //char arg2[] = "another arg";
//     char* argv[] = { &arg0[0]/*, &arg1[0], &arg2[0]*/, NULL };
//     int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
//
//     QApplication a(argc, &argv[0]);
//     QtxGui app;
//     app.show();
//     return a.exec();
// }

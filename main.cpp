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

    // open Settings file to fill the Settings structure
//  cout << "Calling Settings Begin" << endl;
    LoadSettings();
    theApp.iStatus = RUNNING;  // ::TODO throw away after the GUI
//    cout << "Calling Settings End" << endl;
    int iRet = 0;
    g_SThreadData.iTotalThreads = 0;
    pthread_mutex_lock(&mtxQueue);

    for (uint ii=0;  ii < NUMBER_OF_ROLES; ii++ )
    {
        if (!theApp.SSettings.iarrRole[ii]) { // option is turned off.... = 0 or bool false
            arrThreadInfo[ii].eState = TS_INACTIVE;
            continue;
        }
        g_SThreadData.idx = ii;
//        g_SThreadData.pVoid = pCQuantQueue;  // only needed for the first Thread.... to construct the Queue
        arrThreadInfo[ii].iThread_num = ii ;
        iRet = pthread_create(&arrThreadInfo[ii].thread_id, NULL, func_ptr[ii], &g_SThreadData);
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

    while (theApp.iStatus != STOPPED) {
        jj++;
        sleep(3);
        if (jj > 250)  // jj* 3 =  seconds
            theApp.iStatus = STOPPED;
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
void*  MainQueue(void* pArg)
{
    THREAD_DATA SThreadData;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;

    pthread_mutex_lock(&mtxTick);
    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);
    
    pCQuantQueue = NULL;
    pCQuantQueue = CQuantQueue::Instance();


    if (!pCQuantQueue->bConstructed) {  // No need to continue
        Logger::instance().log("Error Constructing Queue...Aborting", Logger::Error);
        delete pCQuantQueue;
        pCQuantQueue = NULL;
	pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
	pthread_mutex_unlock(&mtxTick);
        exit(EXIT_FAILURE);
    }

    g_SThreadData.g_pCQuantQueue = pCQuantQueue;

    bReady = true;
    pthread_cond_signal(&condMap);  // so Build book to check on pCOrdersMap and decide to return

    while (theApp.iStatus == RUNNING) { //  the last thread to terminate...check with the for loop within the while loop in main
        sleep(5);
    }

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
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue)) ;

    pthread_mutex_lock(&mtxTick);
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
    while (theApp.iStatus != STOPPED) {
        if (pCOrdersMap->GetError() > 0)
            break;
        pCOrdersMap->FillMemoryMappedFile();
    };

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

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue)) ;

    pthread_mutex_lock(&mtxTick);
    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);
    
    pCTickDataMap = new CTickDataMap;

    if (!pCTickDataMap) {
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

    arrThreadInfo[idx].eState = TS_ALIVE;
    pCTickDataMap->InitQueue(pQueue);

    while (theApp.iStatus != STOPPED) {
        if (pCTickDataMap->GetError() > 0)
            break;
        pCTickDataMap->FillMemoryMappedFile();
    };

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

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue)) ;

    pthread_mutex_lock(&mtxTick);
    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    pCBuildBook = NULL;
    pCBuildBook = new CBuildBook();
    if (!pCBuildBook) {
//        pthread_mutex_unlock(&mtxMap);
        // log error
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }
    arrThreadInfo[idx].eState = TS_ALIVE;

//  pthread_mutex_unlock(&mtxMap);

    if (pCBuildBook->m_iError) {
        delete pCBuildBook;
        pCBuildBook = NULL;
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

    while (theApp.iStatus != STOPPED) {
        pCBuildBook->BuildBookFromMemoryMappedFile();
    }
    
    pCBuildBook->ListBook("MSFT    ");
    

    delete pCBuildBook;
    pCBuildBook = NULL;
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
//////////////////////////////////////////////////////////////////////////////////////////
void* ReceiveFeed(void* pArg)
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    pthread_mutex_lock(&mtxTick);
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
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    pthread_mutex_lock(&mtxTick);
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

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    pthread_mutex_lock(&mtxTick);
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
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    pthread_mutex_lock(&mtxTick);
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
void* NasdTestFile(void* pArg)
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue));

    pthread_mutex_lock(&mtxTick);
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
    if (pCReceiveITCH->GetError() == 100) {
        Logger::instance().log("Error Constructing ReceiveITCH Object", Logger::Error);
        delete pCReceiveITCH;
        pCReceiveITCH = NULL;
	pthread_mutex_lock(&mtxTick);
        TermThreadLog(idx);
	pthread_mutex_unlock(&mtxTick);
        return NULL;
    }

    if (!pCReceiveITCH->ReadFromTestFile(theApp.SSettings.strTestFileName.c_str())) {
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
void *Distributor(void* pArg)
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue));


    pthread_mutex_lock(&mtxTick);
    InitThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);
    
    // Calls to functions and threads go here
    pthread_mutex_lock(&mtxTick);
    TermThreadLog(idx);
    pthread_mutex_unlock(&mtxTick);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* SaveToDisk(void* pArg)
{

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    /*   pQueue = ((CQuantQueue*)(SThreadData.g_pCQuantQueue));

       InitThreadLog(idx);

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
int LoadSettings()
{
    SETTINGS  SSettings;

    Logger::instance().log("Loading Settings", Logger::Info);

    //SSettings.start_stop_pause = 1;
    SSettings.strServerName = "Main Server";
    // strcpy(SSettings.szServerName, "Main Server");

    SSettings.bMemberOfFarm = true;  		//  bool		bMemberOfFarm;	// Y/N
    SSettings.strFarmName = "Quanticks Ticker Farm 01"; 	//  std::string	strFarmName;
    SSettings.uiFarmPort =  8796; 		//  uint		uiListenOnPort;  // for incoming commands in case Member of bMemberOfFarm = Y  (range  5000...65000)

    // Please figure out the mutual exclusive cases ... i.e
    //  Role; array element
//	0       		1             2                  3                      4                   5
//{"Receive Feed Thread", "Parse Thread", "Orders Map Thread", "Build Book Thread", "Tick Data Thread", "Save To DB Thread",
//       6			7			8		9
// "Play Back Thread", "NasdTestFile Thread", "Distributor Thread", "SaveToDisk Thread"};

    SSettings.iarrRole[0] = 1;   		//  0= Main Queue
    SSettings.iarrRole[1] = 0;   		//  0= Receive Feed
    SSettings.iarrRole[2] = 0;   		//  1= Parse
    SSettings.iarrRole[3] = 1;   		//  2= Orders Map
    SSettings.iarrRole[4] = 1;   		//  3= Build Book
    SSettings.iarrRole[5] = 0;   		//  4= Tick Data
    SSettings.iarrRole[6] = 0;   		//  5= Save to DB
    SSettings.iarrRole[7] = 0;   		//  6= Play back

    SSettings.iarrRole[8] = 1;   		//  7= Test File
    SSettings.iarrRole[9] = 0;   		//  8= Distributor
    SSettings.iarrRole[10]= 0;   		//  9= Save to Disk

    SSettings.uiDistListenOnPort = 9874;   		//  uint 	uiListenPort;  // Case Y above....listen on which Port? (range  5000...65000)

    SSettings.bPartitionActive = true;  		//  bool  bPartitionActive;  // Y/N to  process....can keep the partition info but in an inactive state
    /* Range for all Ex AAPL
        SSettings.iBeginRange = 'A';   		//  int 	iBeginRange;  // e.g 'A'  or 'G'
        SSettings.iEndRange = 'Z';  		//   int 	iEndRange;
        *SSettings.strInclude = '\0';		//	char  strInclude[5];  // include from another range that was excluded from another partition
        strcpy(SSettings.strExclude, "AAPL");  		// char  strExclude[5]; 	// Exclude to be included in another partition
    */
// Range for Apple only
    SSettings.cBeginRange = '\0';   		//  int 	iBeginRange;  // e.g 'A'  or 'G'
    SSettings.cEndRange = '\0';  		//   int 	iEndRange;
    strcpy(SSettings.strInclude, "AAPL");		//	char  strInclude[5];  // include from another range that was excluded from another partition
    strcpy(SSettings.strExclude, "");  		// char  strExclude[5]; 	// Exclude to be included in another partition

    strcpy(SSettings.szUserName, "UserName");   		//  char		szUserName[SIZE_OF_NAME];
    strcpy(SSettings.szPassword, "Password");	// char		szPassword[SIZE_OF_PASSWORD];

    SSettings.ulIPAddress 	=  1234567;  	//  unsigned long	ulIPAddress;
    SSettings.uiPort 		=  8743 ;		//  uint 		uiPort;

    SSettings.ulIPAddress1	=  85236;   	//  unsigned long	ulIPAddress1;
    SSettings.uiPort1 		= 8521;  		//  uint		uiPort1;

    SSettings.dwBufferSize 	= 1000000;  		//  unsigned long	dwBufferSize;

// ODBC connection parameters in case option 3
    SSettings.strConnName = "MySqlConnection";    	//  std::string  	strConnName;  // from ODBC
    strcpy(SSettings.szDBUserName, "MySqlUserName");  	//  char		szDBUserName[SIZE_OF_NAME];
    strcpy(SSettings.szDBPassword, "MySqlPass");     	//  char		szDBPassword[SIZE_OF_PASSWORD];

//    SSettings.strTestFileName = "/home/amro/workspace/QuantServer/NasdTestFiles/08022014.NASDAQ_ITCH50"; // Test file name ...pick from UI dialog
    SSettings.strTestFileName = "/home/amro/workspace/QuantServer/NasdTestFiles/02022015.NASDAQ_ITCH50"; // Test file name ...pick from UI dialog
    //"/home/gen/itch_data/08022014.NASDAQ_ITCH50";
    SSettings.strPlayBackFileName = "Play Back File Name";  // Test file name ...pick from UI dialog


    SSettings.uiDelay = 50;    		//  uint	  uiDelay;   // range...percent = 0..99       --  0 = full speed  50 = 1/2 speed  75 = 1/4 speed    // or suggest

    SSettings.bLog = true; 		//  ushort 	usLoggingLevel;  // 0: Info   1: Errors  2: Warnings  3:All
    SSettings.strLogFileName = "Log File Name";  	// std::string  	strLogFileName;  // Entry field

    SSettings.uiNumberOfIssues = 9000; 		//    uint		uiNumberOfIssues; // Max number of issues approx...9000 = default. Will reserve an entry for each issue in a hash table.
    SSettings.ui64SizeOfOrdersMappedFile = 10; // !0 Gig           // in Giga BYtes  u_int64_t 	ui64SizeOfMemoryMappedFile; // Will set Default later...
    SSettings.ui64SizeOfTickDataMappedFile = 10;  // !0 Gig
    SSettings.uiQueueSize = 10000000;  // 10 Million elements

    theApp.SSettings = SSettings;
    return 0;
}
///////////////////////////////////////////////////////////////
int SaveSettings()
{
    int	iNumberOfBytes = 0;
    int iHandle = open("QTSrvSettings.ini", O_RDWR );
    if (iHandle)
    {
        iNumberOfBytes = write( iHandle, &theApp.SSettings, sizeof(SETTINGS));
        close(iHandle);
    }
    return 0;
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

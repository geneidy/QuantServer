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
//static pthread_mutex_t mtxTick 	= PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t condMap 	= PTHREAD_COND_INITIALIZER;
static bool bReady = false;

int main(int argc, char **argv)
{
    using namespace std;

    Logger::instance().log("Starting Server", Logger::Debug);
    cout << "Hit 'S' or 's' to stop" << endl;

// open Settings file to fill the Settings structure
//    cout << "Calling Settings Begin" << endl;
    LoadSettings();
//    cout << "Calling Settings End" << endl;
    int iRet = 0;

    pthread_mutex_lock(&mtxQueue);

    THREAD_DATA SThreadData;

    for (uint ii=0;  ii < NUMBER_OF_ROLES; ii++ )
    {
        if (!theApp.SSettings.iarrRole[ii]) // option is turned off.... = 0 or bool false
            continue;
        SThreadData.idx = ii;
        SThreadData.pVoid = pCQuantQueue;  // only needed for the first Thread.... to construct the Queue

        arrThreadInfo[ii].iThread_num = ii ;
        iRet = pthread_create(&arrThreadInfo[ii].thread_id, NULL, func_ptr[ii], &SThreadData);
        arrThreadInfo[ii].eState = TS_ALIVE;
        if(iRet)
        {
            Logger::instance().log("Error pthread_create", Logger::Error);
//            fprintf(stderr,"Error - pthread_create() return code: %d\n", iRet);
            exit(EXIT_FAILURE);
        }
        while (!bReady)
            pthread_cond_wait(&condMap, &mtxQueue); // Wait for the Queue to init first
    };
    pthread_mutex_unlock(&mtxQueue);
////////////////////////////////////////////////////////////////////////////////////////
    char cResp = 'A';
    while ( cResp != 'q')
    {
        std::cin >> cResp;
        if ((cResp == 's') || (cResp == 'S'))
        {
            theApp.g_bReceiving = false;
            theApp.iStatus = STOPPED;
            break;
        }
        if ((cResp == 'p') || (cResp == 'P'))
        {
            theApp.g_bReceiving = PAUSSED;
            theApp.iStatus 	    = PAUSSED;
            continue;
        }
    }; // while ( cResp != 'q')
//////////////////////////////////////////////////////////////////////////////////////////
    string strExitMessage;

    for (uint ii = 0;  ii < NUMBER_OF_ROLES; ii++ ) {
        if (!theApp.SSettings.iarrRole[ii]) // option is turned off.... = 0 or bool false
            continue;

        while (arrThreadInfo[ii].eState != TS_TERMINATED) {
            sleep(3);
        }
        strExitMessage.empty();
        strExitMessage = ThreadMessage[ii];
        strExitMessage += " Joined";

        pthread_join(arrThreadInfo[ii].thread_id, NULL);
        Logger::instance().log(strExitMessage, Logger::Debug);
    }
    pthread_cond_destroy(&condMap);
    return 0;
}
////////////////////////////////////////////////////////////////
void*  MainQueue(void* pArg)
{
    THREAD_DATA SThreadData;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;

    InitThreadLog(idx);

    pCQuantQueue = NULL;
    pCQuantQueue = CQuantQueue::Instance();


    if (!pCQuantQueue->bConstructed) {  // No need to continue
        Logger::instance().log("Error Constructing Queue...Aborting", Logger::Error);
        exit(EXIT_FAILURE);
    }
    bReady = true;
    pthread_cond_signal(&condMap);  // so Build book to check on pCOrdersMap and decide to return

    return (pCQuantQueue) ? pCQuantQueue: NULL;  // Global Object
}
///////////////////////////////////////////////////////////////
void* OrdersMap(void* pArg)  // only if buid book is checked
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);

    pCOrdersMap = NULL;
    pCOrdersMap =  COrdersMap::instance();

    if (!pCOrdersMap) {
        Logger::instance().log("Error Creating Orders Map Object", Logger::Error);
        return NULL;
    }

    if (pCOrdersMap->GetError()) {
        delete pCOrdersMap;
        pCOrdersMap = NULL;
        Logger::instance().log("Error in Constructing Orders Map", Logger::Error);
        return NULL;  //  can't build a book w/o Memory Mappings
    }

    arrThreadInfo[idx].eState = TS_ALIVE;

    while (theApp.iStatus != STOPPED) {
        pCOrdersMap->FillMemoryMappedFile();
    };

    delete pCOrdersMap;
    pCOrdersMap = NULL;

    TermThreadLog(idx);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* BuildBook(void* pArg)
{

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;


    InitThreadLog(idx);
    /*    pthread_mutex_lock(&mtxMap);
        // Start the Memory Mapping First


        while (!bReady)
            pthread_cond_wait(&condMap, &mtxMap);
    */
    pCBuildBook = NULL;
    pCBuildBook = new CBuildBook();
    if (!pCBuildBook) {
//        pthread_mutex_unlock(&mtxMap);
        // log error
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

//  pthread_mutex_unlock(&mtxMap);

    if (pCBuildBook->m_iError) {
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

    while (theApp.iStatus != STOPPED) {
        pCBuildBook->BuildBookFromMemoryMappedFile();
    }

    delete pCBuildBook;
    pCBuildBook = NULL;

    TermThreadLog(idx);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* TickDataMap(void* pArg)
{

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;


    InitThreadLog(idx);

    pCTickDataMap = new CTickDataMap;

    while (theApp.iStatus != STOPPED) {
        pCTickDataMap->FillMemoryMappedFile();
    };

    delete pCTickDataMap;
    pCTickDataMap = NULL;

    TermThreadLog(idx);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* ReceiveFeed(void* pArg)
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;


    InitThreadLog(idx);

    // Calls to functions and threads go here

    TermThreadLog(idx);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* ParseFeed(void* pArg)
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;


    InitThreadLog(idx);    // Calls to functions and threads go here

    TermThreadLog(idx);
    return pArg;
}
///////////////////////////////////////////////////////////////
void* SaveToDB(void* pArg)
{

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);

    pCSaveToDB = NULL;
    pCSaveToDB = new CSaveToDB();

    // Call the worker thread here and check for !STOPPED
    // The worker thread has to check for the availability of the Memory Mapped Files ....Check here before you continue
    if (pCSaveToDB == NULL) {
        TermThreadLog(idx);
        return NULL;
    }

    // Call member functions here

    delete pCSaveToDB;
    pCSaveToDB = NULL;

    TermThreadLog(idx);

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


    InitThreadLog(idx);    //
    // Call Object Methods here
    //
    TermThreadLog(idx);
    return pArg;
}
///////////////////////////////////////////////////////////////
void* NasdTestFile(void* pArg)
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);

    pCReceiveITCH = NULL;
    pCReceiveITCH = new CReceiveITCH();

    if (!pCReceiveITCH) {
        Logger::instance().log("Error Creating ReceiveITCH Object", Logger::Error);
        TermThreadLog(idx);
        return NULL;
    }
    if (pCReceiveITCH->GetError() == 100) {
        Logger::instance().log("Error Constructing ReceiveITCH Object", Logger::Error);
        delete pCReceiveITCH;
        pCReceiveITCH = NULL;
        TermThreadLog(idx);
        return NULL;
    }

    if (!pCReceiveITCH->ReadFromTestFile(theApp.SSettings.strTestFileName.c_str())) {
        Logger::instance().log("Error Reading From Test File", Logger::Error);
    }
    if (pCReceiveITCH) {
        delete pCReceiveITCH;
        pCReceiveITCH = NULL;
    }

    TermThreadLog(idx);

    return pArg;
}
///////////////////////////////////////////////////////////////
void *Distributor(void* pArg)
{
    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;


    InitThreadLog(idx);
    // Calls to functions and threads go here

    TermThreadLog(idx);

    return pArg;
}
///////////////////////////////////////////////////////////////
void* SaveToDisk(void* pArg)
{

    THREAD_DATA SThreadData;
    CQuantQueue* pQueue = NULL;

    SThreadData =  *((THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    pQueue = ((CQuantQueue*)(SThreadData.pVoid)) ;

    InitThreadLog(idx);

    pCSaveToDisk = NULL;

    pCSaveToDisk = new  CSaveToDisk();
    // check for class errors then log and exit
    // Calls to functions and threads go here
    while(theApp.g_iFeedStatus == FEED_01_RUNNING) { // ::TODO  check from UI
        pCSaveToDisk->WriteFeedToFile();
    }

    delete pCSaveToDisk;
    pCSaveToDisk = NULL;

    TermThreadLog(idx);

    return pArg;
}
///////////////////////////////////////////////////////////////
void InitThreadLog(int idx)
{

    string  strLogMessage = ThreadMessage[idx];

    strLogMessage += " Started";

    Logger::instance().log(strLogMessage, Logger::Info);
    arrThreadInfo[idx].eState = TS_ALIVE;

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
    SSettings.iarrRole[3] = 0;   		//  2= Orders Map
    SSettings.iarrRole[4] = 0;   		//  3= Build Book
    SSettings.iarrRole[5] = 0;   		//  4= Tick Data
    SSettings.iarrRole[6] = 0;   		//  5= Save to DB
    SSettings.iarrRole[7] = 0;   		//  6= Play back

    SSettings.iarrRole[8] = 1;   		//  7= Test File
    SSettings.iarrRole[9] = 0;   		//  8= Distributor
    SSettings.iarrRole[10] = 0;   		//  9= Save to Disk

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

    SSettings.strTestFileName = "/home/amro/workspace/QuantServer/NasdTestFiles/08022014.NASDAQ_ITCH50";  // Test file name ...pick from UI dialog
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


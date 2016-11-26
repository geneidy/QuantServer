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

static pthread_mutex_t mtxMap = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtxTick = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t condMap = PTHREAD_COND_INITIALIZER;
static bool bReady = false;

int main(int argc, char **argv)
{
    using namespace std;
//    int i = 0;

    Logger::instance().log("Starting Server", Logger::Debug);
    cout << "Hit 'S' or 's' to stop" << endl;

// open Settings file to fill the Settings structure
//    cout << "Calling Settings Begin" << endl;
    LoadSettings();
//    cout << "Calling Settings End" << endl;
    int iRet = 0;

    for (uint ii=0;  ii < NUMBER_OF_ROLES; ii++ )
    {

        if (!theApp.SSettings.iarrRole[ii]) // option is turned off.... = 0 or bool false
            continue;

        arrThreadInfo[ii].iThread_num = ii + 1;
        iRet = pthread_create(&arrThreadInfo[ii].thread_id, NULL, func_ptr[ii], &ii);
        arrThreadInfo[ii].eState = TS_ALIVE;
        if(iRet)
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n", iRet);
            exit(EXIT_FAILURE);
        }
    };

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

    for (uint ii=0;  ii < NUMBER_OF_ROLES; ii++ ) {
        if (arrThreadInfo[ii].eState == TS_TERMINATED)
            pthread_join(arrThreadInfo[ii].thread_id, NULL);
    }

    pthread_cond_destroy(&condMap);
    return 0;
}
///////////////////////////////////////////////////////////////
void* OrdersMap(void* pArg)  // only if buid book is checked
{
    int idx = *((int*) pArg);

    if (!theApp.SSettings.iarrRole[2])  { // Map only if buid book is checked
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

    std::string  strLogMessage = " Memory Map Thead Started";
    bReady = false;
    Logger::instance().log(strLogMessage, Logger::Debug);

    pthread_mutex_lock(&mtxMap);

    pCOrdersMap = NULL;
    pCOrdersMap =  COrdersMap::instance();


    if (!pCOrdersMap) {
        pthread_mutex_unlock(&mtxMap);
        return NULL;
    }

    if (pCOrdersMap->GetError()) {
        delete pCOrdersMap;
        pCOrdersMap = NULL;
        pthread_mutex_unlock(&mtxMap);
        pthread_cond_signal(&condMap);  // so Build book to check on pCOrdersMap and decide to return
        return NULL;  //  can't build a book w/o Memory Mappings
    }
    bReady = true;
    pthread_mutex_unlock(&mtxMap);
    pthread_cond_signal(&condMap);

    arrThreadInfo[idx].eState = TS_ALIVE;

    while (theApp.iStatus != STOPPED) {
        pCOrdersMap->FillMemoryMappedFile();

    };

    delete pCOrdersMap;
    pCOrdersMap = NULL;

    //strLogMessage = strInMessage +  " Finished";
    //Logger::instance().log(strLogMessage, Logger::Debug);
    arrThreadInfo[idx].eState = TS_TERMINATED;

    return pArg;
}
///////////////////////////////////////////////////////////////
void* BuildBook(void* pArg)
{

    string  strLogMessage =  "Build Book Started";
    int idx = *((int*) pArg);

    /*/  sleep(1);
      Logger::instance().log(strLogMessage, Logger::Debug);
      if (!pCOrdersMap){
          Logger::instance().log("Can't Build the Book without Memory Map", Logger::Debug);
          arrThreadInfo[idx].eState = TS_TERMINATED;
          return NULL;
      }

    */  pthread_mutex_lock(&mtxMap);
    // Start the Memory Mapping First
    arrThreadInfo[idx].eState = TS_ALIVE;

    while (!bReady)
        pthread_cond_wait(&condMap, &mtxMap);
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
    strLogMessage = "Build Book Finished";
//  Logger::instance().log(strLogMessage, Logger::Debug);
    arrThreadInfo[idx].eState = TS_TERMINATED;

    return pArg;
}
///////////////////////////////////////////////////////////////
void* TickDataMap(void* pArg)
{

    std::string  strLogMessage = "Tick Data Thread Started";
    Logger::instance().log(strLogMessage, Logger::Debug);

    int idx = *((int*) pArg);
    bReady = false;
    Logger::instance().log(strLogMessage, Logger::Debug);

    pCTickDataMap = new CTickDataMap;
    
    while (!bReady) {
      sleep(1);
      if (theApp.iStatus == STOPPED)
	break;
    }
     while (theApp.iStatus != STOPPED) {
        pCTickDataMap->FillMemoryMappedFile();
    };

    delete pCTickDataMap;
    pCTickDataMap = NULL;

    //strLogMessage = strInMessage +  " Finished";
    //Logger::instance().log(strLogMessage, Logger::Debug);
    arrThreadInfo[idx].eState = TS_TERMINATED;
    strLogMessage = "Tick Data Thread Finished";
    Logger::instance().log(strLogMessage, Logger::Debug);

    arrThreadInfo[idx].eState = TS_TERMINATED;
    return pArg;
}
///////////////////////////////////////////////////////////////
void* ReceiveFeed(void* pArg)
{
    int idx = *((int*) pArg);

    std::string  strLogMessage = "Parse Feed Thread Started";
    Logger::instance().log(strLogMessage, Logger::Debug);

    // Calls to functions and threads go here

    strLogMessage = "Parse Feed Thread Finished";
    Logger::instance().log(strLogMessage, Logger::Debug);

    arrThreadInfo[idx].eState = TS_TERMINATED;

    return pArg;
}
///////////////////////////////////////////////////////////////
void* ParseFeed(void* pArg)
{
    int idx = *((int*) pArg);

    std::string  strLogMessage = "Parse Feed Thread Started";

    Logger::instance().log(strLogMessage, Logger::Debug);

    // Calls to functions and threads go here


    strLogMessage =  " Parse Feed Thread Finished";
    Logger::instance().log(strLogMessage, Logger::Debug);


    arrThreadInfo[idx].eState = TS_TERMINATED;
    return pArg;
}
///////////////////////////////////////////////////////////////
void* SaveToDB(void* pArg)
{

    std::string strInMessage = (char*) pArg;
    std::string  strLogMessage = strInMessage +  " Started";

    int idx = *((int*) pArg);
//  Logger::instance().log(strLogMessage, Logger::Debug);

    pCSaveToDB = NULL;
    pCSaveToDB = new CSaveToDB();
    if (pCSaveToDB == NULL) {
        arrThreadInfo[idx].eState = TS_TERMINATED;
        return NULL;
    }

    // Call member functions here

    delete pCSaveToDB;
    //
    //
    strLogMessage = strInMessage +  " Finished";
//  Logger::instance().log(strLogMessage, Logger::Debug);
    arrThreadInfo[idx].eState = TS_TERMINATED;
    return pArg;
}
///////////////////////////////////////////////////////////////
void* PlayBack(void* pArg)
{
    std::string strInMessage = (char*) pArg;
    std::string  strLogMessage = strInMessage +  " Started";

    int idx = *((int*) pArg);
//  Logger::instance().log(strLogMessage, Logger::Debug);

    /*
      if ((iHandle = open(theApp.strFeedFileName.c_str(), O_RDWR )==  -1))
      {
          std::cout << "Invalid File Name Entered" << std::endl;
          return 1;
      }
    */

    strLogMessage = strInMessage +  " Finished";
//  Logger::instance().log(strLogMessage, Logger::Debug);

    arrThreadInfo[idx].eState = TS_TERMINATED;
    return pArg;
}
///////////////////////////////////////////////////////////////
void* NasdTestFile(void* pArg)
{

//    int idx = *((int*) pArg);
    return pArg;
}
///////////////////////////////////////////////////////////////
void *Distributor(void* pArg)
{

    string  strLogMessage = "Distributor thread Started";

    int idx = *((int*) pArg);

//  Logger::instance().log(strLogMessage, Logger::Debug);

    // Calls to functions and threads go here

    strLogMessage =  "Distributor thread Finished";
//  Logger::instance().log(strLogMessage, Logger::Debug);

    arrThreadInfo[idx].eState = TS_TERMINATED;
    return pArg;
}
///////////////////////////////////////////////////////////////
void* SaveToDisk(void* pArg)
{

    std::string strInMessage = (char*) pArg;
    std::string  strLogMessage = strInMessage +  " Started";

    Logger::instance().log(strLogMessage, Logger::Debug);
    int idx = *((int*) pArg);

    pCSaveToDisk = NULL;

    pCSaveToDisk = new  CSaveToDisk();
    // check for class errors then log and exit
    // Calls to functions and threads go here
    while(theApp.g_iFeedStatus == FEED_01_RUNNING) { // ::TODO  check from UI
        pCSaveToDisk->WriteFeedToFile();
    }

    strLogMessage = strInMessage +  " Finished";
    Logger::instance().log(strLogMessage, Logger::Debug);

    delete pCSaveToDisk;

    arrThreadInfo[idx].eState = TS_TERMINATED;

    return pArg;
}
///////////////////////////////////////////////////////////////
int LoadSettings()
{

//  cout << "In Settings Before Log" << endl;
    SETTINGS  SSettings;
//  cout << "instance of Settings created" << endl;

    Logger::instance().log("Loading Settings", Logger::Debug);
//  memset(&SSettings, '\0', sizeof(SETTINGS));
//  cout << "In Settings After Log" << endl;

    //SSettings.start_stop_pause = 1;
    SSettings.strServerName = "Main Server";
//  cout << "In Settings After strServerName" << endl;
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
    
    SSettings.iarrRole[0] = 0;   		//  0= Receive Feed
    SSettings.iarrRole[1] = 0;   		//  1= Parse
    SSettings.iarrRole[2] = 0;   		//  2= Orders Map 
    SSettings.iarrRole[3] = 0;   		//  3= Build Book
    SSettings.iarrRole[4] = 0;   		//  4= Tick Data
    SSettings.iarrRole[5] = 0;   		//  5= Save to DB
    SSettings.iarrRole[6] = 0;   		//  6= Play back
    
    SSettings.iarrRole[7] = 0;   		//  7= Test File
    SSettings.iarrRole[8] = 0;   		//  8= Distributor
    SSettings.iarrRole[9] = 0;   		//  9= Save to Disk
    
    SSettings.uiDistListenOnPort = 9874;   		//  uint 	uiListenPort;  // Case Y above....listen on which Port? (range  5000...65000)

    SSettings.bPartitionActive = true;  		//  bool  bPartitionActive;  // Y/N to  process....can keep the partition info but in an inactive state

    SSettings.iBeginRange = 'A';   		//  int 	iBeginRange;  // e.g 'A'  or 'G'
    SSettings.iEndRange = 'Z';  		//   int 	iEndRange;
    *SSettings.strInclude = '\0';		//	char  strInclude[5];  // include from another range that was excluded from another partition
    strcpy(SSettings.strExclude, "AAPL");  		// char  strExclude[5]; 	// Exclude to be included in another partition

    strcpy(SSettings.szUserName, "UserName");   		//  char		szUserName[SIZE_OF_NAME];
    strcpy(SSettings.szPassword, "Password");	// char		szPassword[SIZE_OF_PASSWORD];

    SSettings.ulIPAddress =  1234567;  	//  unsigned long	ulIPAddress;
    SSettings.uiPort 	=  8743 ;		//  uint 		uiPort;

    SSettings.ulIPAddress1=  85236;   	//  unsigned long	ulIPAddress1;
    SSettings.uiPort1 	= 8521;  		//  uint		uiPort1;

    SSettings.dwBufferSize = 100;  		//  unsigned long	dwBufferSize;

// ODBC connection parameters in case option 3
    SSettings.strConnName = "MySqlConnection";    	//  std::string  	strConnName;  // from ODBC
    strcpy(SSettings.szDBUserName, "MySqlUserName");  	//  char		szDBUserName[SIZE_OF_NAME];
    strcpy(SSettings.szDBPassword, "MySqlPass");     	//  char		szDBPassword[SIZE_OF_PASSWORD];

    SSettings.strTestFileName = "Test File Name";  // Test file name ...pick from UI dialog
    SSettings.strPlayBackFileName = "Play Back File Name";  // Test file name ...pick from UI dialog


    SSettings.uiDelay = 50;    		//  uint	  uiDelay;   // range...percent = 0..99       --  0 = full speed  50 = 1/2 speed  75 = 1/4 speed    // or suggest

    SSettings.bLog = true; 		//  ushort 	usLoggingLevel;  // 0: Info   1: Errors  2: Warnings  3:All
    SSettings.strLogFileName = "Log File Name";  	// std::string  	strLogFileName;  // Entry field

    SSettings.uiNumberOfIssues = 9000; 		//    uint		uiNumberOfIssues; // Max number of issues approx...9000 = default. Will reserve an entry for each issue in a hash table.
    SSettings.ui64SizeOfOrdersMappedFile = 10; // !0 Gig           // in Giga BYtes  u_int64_t 	ui64SizeOfMemoryMappedFile; // Will set Default later...
    SSettings.ui64SizeOfTickDataMappedFile = 10;  // !0 Gig 
    SSettings.uiQueueSize = 10000000;  // 10 Million elements

//  memset(&theApp.SSettings, '\0', sizeof(SETTINGS));
//  cout << "In Settings Before Assigning to theApp" << endl;
    theApp.SSettings = SSettings;
//  cout << "In Settings END" << endl;
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


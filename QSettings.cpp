#include "QSettings.h"
#include "NQTV.h"


CQSettings::CQSettings()
{
    m_iError = 0;
    m_iMessage = 0;

    m_uiSizeOfSettingsRecord  = sizeof(SETTINGS);

    m_Util = NULL;
    m_Util = new CUtil();

    struct stat st = {0};

    if (stat("../QSettings", &st) == -1) {
        int iRet = mkdir("../QSettings/", 0700);
        if (iRet == -1)
            m_iError = 1000;
    }
    else {

        string strSettingsFile;
        strSettingsFile.empty();

        strSettingsFile = "../QSettings/";
        strSettingsFile += "QSettingsIni.Qtx";

        m_fd = open(strSettingsFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);

        if (m_fd == -1) {
            Logger::instance().log("Settings open File Mapping Error", Logger::Debug);
            m_iError = 100;
            // Set error code and exit
        }
        if (!m_iError) {
            if (fstat64(m_fd, &m_sb) == -1) {
                Logger::instance().log("Settings Error fstat", Logger::Debug);
                m_iError = 110;
                // Set error code and exit
            }
            else   {
                if (!InitMemoryMappedFile()) {
                    Logger::instance().log("Settings Error Initializing", Logger::Error);
                    close(m_fd);
                    m_iError = 120;
                    // Set error code and exit
                }
                else {
                    m_addr = mmap(NULL, m_sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);

                    if (m_addr == MAP_FAILED) {
                        Logger::instance().log("Settings Error Mapping Failed", Logger::Error);
                        m_iError = 130;
                        // Set error code and exit
                    }
                    m_pSettings = (SETTINGS*) m_addr;  //  cast in Settings...now you have a Strut in memeory and on Disk
                }
            }
        } //   if (!m_iError) {
    } //     if (stat("../QSettings", &st) == -1) {
    m_request.tv_sec = 0;
    m_request.tv_nsec = 100000000;   // 1/10 of a second
}
/////////////////////////////////////////////////////////////////////////////////
int CQSettings::InitMemoryMappedFile()
{
    memset(&m_ssettings, '\0', sizeof(SETTINGS));

    if (m_sb.st_size <  (sizeof(SETTINGS)) ) { // Fresh file
        Logger::instance().log("Initializing Settings Mapped File", Logger::Debug);
        write(m_fd, &m_ssettings, sizeof(SETTINGS));
        Logger::instance().log("Finished Initializing Settings Mapped File", Logger::Debug);
    }
    fstat64(m_fd, &m_sb);
    if (m_sb.st_size <  sizeof(SETTINGS)) {
        Logger::instance().log("Error Initializing Settings Mapped File", Logger::Debug);
        m_iError = 200; // enum later
        return false;
    }
    m_iError = 0; // enum later
    return true;
}
/////////////////////////////////////////////////////////////////////////////////
int CQSettings::GetError()
{
    return m_iError;
}
/////////////////////////////////////////////////////////////////////////////////
CQSettings::~CQSettings()
{
    msync(m_addr, m_sb.st_size, MS_ASYNC);
    munmap(m_addr, m_sb.st_size);

    m_pSettings = NULL;
    if (m_fd)
        close(m_fd);
}
////////////////////////////////////////////////////////////////////////////////
SETTINGS CQSettings::GetSettings()
{
    if (m_pSettings)
        return *m_pSettings;

    return m_ssettings;  // for lack of anything else

}
////////////////////////////////////////////////////////////////////////////////
//
//  CQSettings::LoadSettings to be used for testing only until mapping is shared with the client
//
/////////////////////////////////////////////////////////////////////////////////
SETTINGS CQSettings::LoadSettings()
{   // the usual old way....do NOT call after activation with the client...Call ::GetSettings() instead
    // For testing puposes ...Call this method (LoadSettings) and verify that it asserts with the return pointer from ::GetSettings

    SETTINGS  SSettings;

    Logger::instance().log("Loading Settings From CQSettings Object", Logger::Info);
    memset(&SSettings, '\0', sizeof(SETTINGS));

    //SSettings.start_stop_pause = 1;

    strcpy(SSettings.szServerName, "Main Server");

    SSettings.bMemberOfFarm = true;  		//  bool		bMemberOfFarm;	// Y/N
    strcpy(SSettings.szFarmName,  "Quanticks Ticker Farm 01"); 	//  std::string	strFarmName;


    SSettings.uiFarmPort =  8796; 		//  uint		uiListenOnPort;  // for incoming commands in case Member of bMemberOfFarm = Y  (range  5000...65000)

    // Please figure out the mutual exclusive cases ... i.e
    //  Role; array element
    /*
    {MainQueue, Settings, ReceiveFeed, ParseFeed, OrdersMap, BuildBook,
    TickDataMap, SaveToDB, PlayBack, NasdTestFile, Distributor, SaveToDisk};  // All Roles for the server functions are here
    */

    /*  "Main Queue Thread", "Settings Thread",  "Receive Feed Thread", "Parse Thread",
        "Orders Map Thread", "Build Book Thread", "Tick Data Thread",\
        "Save To DB Thread", "Play Back Thread", "Nasd Test File Thread", "Distributor Thread", "SaveToDisk Thread"
    */
    SSettings.iarrRole[0] = 1;   		//  0= Main Queue
    SSettings.iarrRole[1] = 1;   		//  1= QSettings

    SSettings.iarrRole[2] = 0;   		//  2= Receive Feed
    SSettings.iarrRole[3] = 0;   		//  3= Parse
    SSettings.iarrRole[4] = 0;   		//  4= Orders Map
    SSettings.iarrRole[5] = 0;   		//  5= Build Book
    SSettings.iarrRole[6] = 0;   		//  6= Tick Data
    SSettings.iarrRole[7] = 0;   		//  7= Save to DB
    SSettings.iarrRole[8] = 0;   		//  8= Play back

    SSettings.iarrRole[9] = 0;   		//  9= Test File
    SSettings.iarrRole[10]= 1;   		//  10= Stats Per Second
    SSettings.iarrRole[11]= 0;   		//  11= Display Book
    SSettings.iarrRole[12]= 0;   		//  12= Distributor
    SSettings.iarrRole[13]= 0;   		//  13= Save to Disk

    SSettings.uiDistListenOnPort = 9874;   		//  uint 	uiListenPort;  // Case Y above....listen on which Port? (range  5000...65000)

    SSettings.bPartitionActive = true;  		//  bool  bPartitionActive;  // Y/N to  process....can keep the partition info but in an inactive state
    // Range for all Ex AAPL
    SSettings.cBeginRange = 'A';   		//  int 	iBeginRange;  // e.g 'A'  or 'G'
    SSettings.cEndRange = 'Z';  		//   int 	iEndRange;
    *SSettings.szInclude = '\0';		//	char  strInclude[5];  // include from another range that was excluded from another partition
    strcpy(SSettings.szExclude, "AAPL    ");  		// char  strExclude[5]; 	// Exclude to be included in another partition

// Range for Apple only
    SSettings.cBeginRange = '\0';   		//  int 	iBeginRange;  // e.g 'A'  or 'G'
    SSettings.cEndRange = '\0';  		//   int 	iEndRange;
    strcpy(SSettings.szInclude, "AAPL    ");		//	char  strInclude[5];  // include from another range that was excluded from another partition
    strcpy(SSettings.szExclude, "");  		// char  strExclude[5]; 	// Exclude to be included in another partition

    
// For the first release only for 5 stocks...All values to be set by the client....
    // Values for testing the server only
    SSettings.iBookLevels = 20; 
    strcpy(SSettings.szActiveSymbols[0], "AAPL    "); 	SSettings.arrbActive[0] = true;
    strcpy(SSettings.szActiveSymbols[1], "MSFT    ");	SSettings.arrbActive[1] = true;
    strcpy(SSettings.szActiveSymbols[2], "GOOG    ");	SSettings.arrbActive[2] = true;
    strcpy(SSettings.szActiveSymbols[3], "INTC    ");	SSettings.arrbActive[3] = true;
    strcpy(SSettings.szActiveSymbols[4], "AMD     ");	SSettings.arrbActive[4] = true;
    // Values for testing the server only
// For the first release only for 5 stocks    
    
    
    strcpy(SSettings.szUserName, "UserName");   		//  char		szUserName[SIZE_OF_NAME];
    strcpy(SSettings.szPassword, "Password");	// char		szPassword[SIZE_OF_PASSWORD];

    SSettings.ulIPAddress 	=  1234567;  	//  unsigned long	ulIPAddress;
    SSettings.uiPort 		=  8743 ;		//  uint 		uiPort;

    SSettings.ulIPAddress1	=  85236;   	//  unsigned long	ulIPAddress1;
    SSettings.uiPort1 		=  8521;  		//  uint		uiPort1;

    SSettings.dwBufferSize 	= 10000000;  		//  unsigned long	dwBufferSize;

// ODBC connection parameters in case option 3
    strcpy(SSettings.szConnName, "MySqlConnection");    	//  std::string  	strConnName;  // from ODBC
    strcpy(SSettings.szDBUserName, "MySqlUserName");  	//  char		szDBUserName[SIZE_OF_NAME];
    strcpy(SSettings.szDBPassword, "MySqlPass");     	//  char		szDBPassword[SIZE_OF_PASSWORD];

//    SSettings.strTestFileName = "/home/amro/workspace/QuantServer/NasdTestFiles/08022014.NASDAQ_ITCH50"; // Test file name ...pick from UI dialog
    strcpy(SSettings.szTestFileName , "/home/amro/workspace/QuantServer/NasdTestFiles/02022015.NASDAQ_ITCH50"); // The big file!!!!

    strcpy(SSettings.szPlayBackFileName, "Play Back File Name");  // Test file name ...pick from UI dialog


    SSettings.uiDelay = 50;    		//  uint	  uiDelay;   // range...percent = 0..99       --  0 = full speed  50 = 1/2 speed  75 = 1/4 speed    // or suggest

    SSettings.bLog = true; 		//  ushort 	usLoggingLevel;  // 0: Info   1: Errors  2: Warnings  3:All
    strcpy(SSettings.szLogFileName,  "Log File Name");  	// std::string  	strLogFileName;  // Entry field

    SSettings.uiNumberOfIssues = 9000; 		//    uint		uiNumberOfIssues; // Max number of issues approx...9000 = default. Will reserve an entry for each issue in a hash table.
    SSettings.ui64SizeOfOrdersMappedFile = 10; // !0 Gig           // in Giga BYtes  u_int64_t 	ui64SizeOfMemoryMappedFile; // Will set Default later...
    SSettings.ui64SizeOfTickDataMappedFile = 10;  // !0 Gig
    SSettings.uiQueueSize = 50000000;  // 25 Million elements

    // ::TODO throw away after the GUI
    SSettings.iStatus = RUNNING;  // ::TODO throw away after the GUI
    // ::TODO throw away after the GUI

//    theApp.SSettings = SSettings;

    m_pSettings = &SSettings;  // Just to verify for testing....that the Mapping is working
    
    msync(m_addr, m_sb.st_size, MS_ASYNC);
    
    return SSettings;
}
/////////////////////////////////////////////////////////////////////////////////

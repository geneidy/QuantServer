#include "StatsPerSec.h"
#include "NQTV.h"

bool  CStatsPerSec::m_bReplay = true;  // to reset values
// timer_t  CStatsPerSec::firstTimerID = NULL;

unsigned long CStatsPerSec::m_arrMessagesPerSec[MAX_MESSAGE_TYPES];

SMESSAGESPERSEC  CStatsPerSec::m_SMessagesPerSec= {0};
SMESSAGESPERSEC* CStatsPerSec::m_pMessagesPerSec = NULL;


CStatsPerSec::CStatsPerSec()
{
    m_iError = 0;

//    firstTimerID = NULL;

//    m_uiSizeOfSettingsRecord  = sizeof(SETTINGS);

    m_Util = NULL;
    m_Util = new CUtil();

    struct stat st = {0};

    if (stat("../QSettings", &st) == -1) {
        int iRet = mkdir("../QStatsPerSec/", 0700);
        if (iRet == -1)
            m_iError = 1000;
    }
    else {

        string strSettingsFile;
        strSettingsFile.empty();

        strSettingsFile = "../QSettings/";
        strSettingsFile += "StatsPerSec.Qtx";

        m_fd = open(strSettingsFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);

        if (m_fd == -1) {
            Logger::instance().log("Stats Per second open File Mapping Error", Logger::Debug);
            m_iError = 100;
            // Set error code and exit
        }
        if (!m_iError) {
            if (fstat(m_fd, &m_sb) == -1) {
                Logger::instance().log("Stats Per second Error fstat", Logger::Debug);
                m_iError = 110;
                // Set error code and exit
            }
            else   {
                if (!InitMemoryMappedFile()) {
                    Logger::instance().log("Stats Per second Error Initializing", Logger::Error);
                    close(m_fd);
                    m_iError = 120;
                    // Set error code and exit
                }
                else {
                    m_addr = mmap(NULL, m_sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);

                    if (m_addr == MAP_FAILED) {
                        Logger::instance().log("Stats Per second Error Mapping Failed", Logger::Error);
                        m_iError = 130;
                        // Set error code and exit
                    }
                    m_pMessagesPerSec = (SMESSAGESPERSEC*) m_addr;  //  cast in COMMON_ORDER_MESSAGE...now you have an array in memory of common orders
                }
            }
        } //   if (!m_iError) {
    } //     if (stat("../QSettings", &st) == -1) {
    m_request.tv_sec = 0;
    m_request.tv_nsec = 100000000;   // 1/10 of a second

    int rc = 0;

    memset(m_arrMessagesPerSec, 0, sizeof(m_arrMessagesPerSec));

    ResetPerSec();
/*  // Disable for now....will call from the main thread
    char strFirstTimer[] = "First Timer";

    rc = makeTimer(strFirstTimer, &firstTimerID, 1000, 1000);
*/// Disable for now....will call from the main thread
}
/////////////////////////////////////////////////////////////////////////////////
int CStatsPerSec::InitMemoryMappedFile()
{
    memset(&m_SMessagesPerSec, '\0', sizeof(SMESSAGESPERSEC));

    if (m_sb.st_size <  (sizeof(SMESSAGESPERSEC)) ) { // Fresh file
        Logger::instance().log("Initializing Stats Per second Mapped File", Logger::Debug);
        write(m_fd, &m_SMessagesPerSec, sizeof(SMESSAGESPERSEC));
        Logger::instance().log("Finished Initializing Stats Per second Mapped File", Logger::Debug);
    }
    fstat(m_fd, &m_sb);
    if (m_sb.st_size <  sizeof(SMESSAGESPERSEC)) {
        Logger::instance().log("Error Initializing Stats Per second Mapped File", Logger::Error);
        m_iError = 200; // enum later
        return false;
    }
    m_iError = 0; // enum later
    return true;
}
/////////////////////////////////////////////////////////////////////////////////
int CStatsPerSec::GetError()
{
    return m_iError;
}
/////////////////////////////////////////////////////////////////////////////////
CStatsPerSec::~CStatsPerSec()
{
//    timer_delete(firstTimerID);
    msync(m_pMessagesPerSec, m_sb.st_size, MS_ASYNC);
    munmap(m_addr, m_sb.st_size);

    m_pMessagesPerSec = NULL;
    if (m_fd)
        close(m_fd);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CStatsPerSec::ResetPerSec()
{
    memset(&m_SMessagesPerSec.arrMessagesPerSec, 0, sizeof(m_SMessagesPerSec.arrMessagesPerSec));
    m_pMessagesPerSec =  &m_SMessagesPerSec;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CStatsPerSec::SetPerSec()
{
    static unsigned long arrTotalMessages[MAX_MESSAGE_TYPES];

    if (m_bReplay) {
        memset(arrTotalMessages, 0, sizeof(arrTotalMessages));
        memset(m_SMessagesPerSec.arrMaxMessagesPerSec, 0, sizeof(m_SMessagesPerSec.arrMaxMessagesPerSec));
        m_bReplay = false;
    }

    for (int ii = 0; ii < MAX_MESSAGE_TYPES; ii++)
        m_SMessagesPerSec.arrMessagesPerSec[ii]   =  theApp.g_arrTotalMessages[ii]  -  arrTotalMessages[ii];

    memmove(arrTotalMessages, theApp.g_arrTotalMessages, sizeof(arrTotalMessages));   // Move total messages to local total
//  memmove(m_pMessagesPerSec->arrTotalMessages, theApp.g_arrTotalMessages, sizeof(arrTotalMessages));
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CStatsPerSec::SetMaxPerSec()
{

    for (int ii = 0; ii < MAX_MESSAGE_TYPES; ii++) {
       if (m_SMessagesPerSec.arrMessagesPerSec[ii] > m_SMessagesPerSec.arrMaxMessagesPerSec[ii])
	  m_SMessagesPerSec.arrMaxMessagesPerSec[ii]   = m_SMessagesPerSec.arrMessagesPerSec[ii];      
    }
    //memmove(arrMessagesPerSec, theApp.g_arrMessagesPerSec, sizeof(arrMessagesPerSec));
//    memmove(m_pMessagesPerSec->arrMaxMessagesPerSec, m_SMessagesPerSec.arrMaxMessagesPerSec, sizeof(m_arrMessagesPerSec));
    
}
//////////////////////////////////////////////////////////////////////////////////////////////
int CStatsPerSec::makeTimer( char *name, timer_t *timerID, int expireMS, int intervalMS )
{
  /*
    struct sigevent         te;
    struct itimerspec       its;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;

    /* Set up signal handler. 
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1)
    {
        Logger::instance().log("Error Failed to setup signal handling for Message Per Second Mapped File", Logger::Debug);
        //fprintf(stderr, "%s: Failed to setup signal handling for %s.\n", PROG, name);
        return(-1);
    }

    //   Set and enable alarm 
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timerID;
    timer_create(CLOCK_REALTIME, &te, timerID);

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = intervalMS * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = expireMS * 1000000;
    timer_settime(*timerID, 0, &its, NULL);
*/
    return(0);
}
///////////////////////////////////////////////////////////////////////////////////
void CStatsPerSec::timerHandler( int sig, siginfo_t *si, void *uc )
{
  /*
    timer_t tidp;
    tidp = si->si_value.sival_ptr;

    if (tidp == firstTimerID) // possible to have more than one timer ....just in case...
    {
        SetPerSec();
        SetMaxPerSec();
        m_SMessagesPerSec = *m_pMessagesPerSec;  //  Update the memory mapped file of messages
    }
*/
  /*
     else if ( *tidp == secondTimerID )
         //secondCB(sig, si, uc);
     else if ( *tidp == thirdTimerID )
        // thirdCB(sig, si, uc);
  */
}
///////////////////////////////////////////////////////////////////////////////////

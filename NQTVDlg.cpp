
// QtSpryDlg.cpp : implementation file
//

#define MAIN_MODULE

#include "memory.h"
//#include <signal.h>

#include <string.h>
#include <sys/time.h>
#include <iostream>

#include "NQTV.h"
#include "NQTVDlg.h"
#include "Util.h"
#include "ITCHMessages.h"

//#include "ConProperties.h"
//#include "NQTVOptions.h"

#undef MAIN_MODULE

CReceiveITCH  *CNQTVDlg::m_pCQtReceiveITCH;

FEED_MESSAGE_STATS CNQTVDlg::m_MessageStats;
bool CNQTVDlg::m_bTestFeed = false;
bool CNQTVDlg::m_bReplay = false;
static unsigned long arrMessagesPerSec[MAX_MESSAGE_TYPES];
timer_t firstTimerID = NULL;
CUtil myUtil2;
///////////////////////////////////////////////////////////////////////////////////
// CNQTVDlg dialog
///////////////////////////////////////////////////////////////////////////////////
// CNQTVDlg message handlers
CNQTVDlg::CNQTVDlg()
{
//	_T(szMessages[ii]));	// 0  per second, 1 max per second,  2 total

    memset(theApp.g_arrMessagesPerSec, 0, sizeof(theApp.g_arrMessagesPerSec));
    memset(theApp.g_arrMaxMessagesPerSec, 0, sizeof(theApp.g_arrMaxMessagesPerSec));
    memset(theApp.g_arrTotalMessages, 0, sizeof(theApp.g_arrTotalMessages));

    int rc = 0;
    char strFirstTimer[] = "First Timer";

    rc = makeTimer(strFirstTimer, &firstTimerID, 1000, 1000);


//   return rc;   // Raise a flag...cannot return from a constructor

}
/////////////////////////////////////////////////////////////////////////////////////////////
MYSQL* CNQTVDlg::ConnectMySql(char* host_name, char* user_name, char* password, char* db_name,
                              unsigned int port_num, char* socket_name, unsigned int flags)
{

    /*
      theApp.conn = mysql_init(NULL); // allocate, init connection handler

      if (theApp.conn == NULL) {
    	myUtil2.print_error(NULL, "mysql_init() failed (probably out of memory)");
        return(NULL);
      }

      if (mysql_real_connect(theApp.conn, host_name, user_name, password,
    			db_name, port_num, socket_name, flags) == NULL)
      {
        myUtil2.print_error(theApp.conn, "mysql_real_connect() failed");
        return(NULL);
      }
    */
//    return(theApp.conn);	// connection is established
    return NULL;	// connection is established
}
/////////////////////////////////////////////////////////////////////////////////////////////
int CNQTVDlg::InsertRow()
{
    return 1;  // enum error codes to return
}
/////////////////////////////////////////////////////////////////////////////////////////////
//int
void CNQTVDlg::Disconnect1MySql()
{
//	mysql_close(theApp.conn); //TODO: make return code conditional on success or failure
    //return 1;  // enum error codes to return
}
/////////////////////////////////////////////////////////////////////////////////////////////
CNQTVDlg::~CNQTVDlg()
{
    timer_delete(firstTimerID);

}
/*/////////////////////////////////////////////////////////////////////////////////////////////
void CNQTVDlg::timer_handler (int signum)
{

	SetPerSec();
	SetMaxPerSec();
// Display is diabled in this version in Linux....only command line....so all output will be from main function..via stdio

	for (int ii = 0; ii < MAX_MESSAGE_TYPES; ii++)
	{
		m_ctrlListStats.SetItem(ii, 1 , LVIF_TEXT,   CUtil.UnsingedToCString(theApp.g_arrMessagesPerSec[ii]) , 0, 0, 0, 0);
		m_ctrlListStats.SetItem(ii, 2 , LVIF_TEXT,   CUtil.UnsingedToCString(theApp.g_arrMaxMessagesPerSec[ii]) , 0, 0, 0, 0);
		m_ctrlListStats.SetItem(ii, 3 , LVIF_TEXT,   CUtil.UnsingedToCString(theApp.g_arrTotalMessages[ii]) , 0, 0, 0, 0);
	}

}
////////////////////////////////////////////////////////////////*/
void CNQTVDlg::FeedPause()  // Feed Pause
{

    theApp.g_iFlag = theApp.g_iFeedStatus = PAUSSING;
    //init the pause flag


    theApp.g_iFlag = theApp.g_iFeedStatus = PAUSSED;

}
///////////////////////////////////////////////////////////////////////////////////
void CNQTVDlg::FeedStop()  // Feed Stop
{

    theApp.g_iFlag = theApp.g_iFeedStatus = STOPPING;
    theApp.SSettings.g_bReceiving = false;

    //theApp.g_bConnected = false;
    //terminate the thread
    //init the Stop flag
    //thread will terminate through completion

    theApp.g_iFlag = theApp.g_iFeedStatus = STOPPED;

    /** Timer will retire every second anyway
     	if (m_uTimer)
    		KillTimer(m_uTimer);

    **/
    ResetPerSec();
//	SetMaxPerSec();

}
///////////////////////////////////////////////////////////////////////////////////
int CNQTVDlg::FeedStart()  // Incomming Feed  thread....
{
 /*   memset(theApp.g_arrMessagesPerSec,	0, sizeof(theApp.g_arrMessagesPerSec));
    memset(theApp.g_arrMaxMessagesPerSec,	0, sizeof(theApp.g_arrMaxMessagesPerSec));
    memset(theApp.g_arrTotalMessages,		0, sizeof(theApp.g_arrTotalMessages));

    ResetPerSec();

    theApp.g_iFlag = theApp.g_iFeedStatus = STARTING;

    theApp.g_iFlag = theApp.g_iFeedStatus = STARTING;
    CReceiveITCH*  m_pCQtReceiveITCH = NULL;

    //:: TODO get the last sequence number and pass to QTtCom
    if (m_bTestFeed)
    {
        int  SocketTV = -1;     //  = INVALID_socket;  // useless...just for compliance
        m_pCQtReceiveITCH = new CReceiveITCH();

        if (!m_pCQtReceiveITCH)
            return -1;
        //::Amro  Actual Grinding goes here
        theApp.g_bReceiving = true;

        m_pCQtReceiveITCH->ReadFromTestFile((const char*)theApp.strFeedFileName.c_str());   // this is the main loop

    }
    else
    {
        return 0;
    }
    //log info
    // Thread termination through completion
    if (m_pCQtReceiveITCH)
    {
        delete m_pCQtReceiveITCH;
        m_pCQtReceiveITCH = NULL;
    }
   
    m_ctrlListStats.InsertItem(MAX_MESSAGE_TYPES + 1, _T("End Time"));
    m_ctrlListStats.SetItem(MAX_MESSAGE_TYPES+ 1, 1, LVIF_TEXT, CString(theApp.g_Stats.strEndTime), 0,0,0,0);
    */
    return 1;// TODO enum the error codes
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CNQTVDlg::SetPerSec()
{
    static unsigned long arrTotalMessages[MAX_MESSAGE_TYPES];

    if (m_bReplay)
    {
        memset(arrTotalMessages, 0, sizeof(arrTotalMessages));
        memset(theApp.g_arrMaxMessagesPerSec, 0, sizeof(theApp.g_arrMaxMessagesPerSec));

        m_bReplay = false;
    }

    for (int ii = 0; ii < MAX_MESSAGE_TYPES; ii++)
        theApp.g_arrMessagesPerSec[ii]   =  theApp.g_arrTotalMessages[ii]  -  arrTotalMessages[ii];

    memmove(arrTotalMessages, theApp.g_arrTotalMessages, sizeof(arrTotalMessages));   // Move total messages to local total
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CNQTVDlg::ResetPerSec()
{
    memset(theApp.g_arrMessagesPerSec, 0, sizeof(theApp.g_arrMessagesPerSec));
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CNQTVDlg::SetMaxPerSec()
{

    for (int ii = 0; ii < MAX_MESSAGE_TYPES; ii++)
        theApp.g_arrMaxMessagesPerSec[ii]   = (theApp.g_arrMaxMessagesPerSec[ii] > arrMessagesPerSec[ii]) ? theApp.g_arrMaxMessagesPerSec[ii] : arrMessagesPerSec[ii];

    memmove(arrMessagesPerSec, theApp.g_arrMessagesPerSec, sizeof(arrMessagesPerSec));
}
//////////////////////////////////////////////////////////////////////////////////////////////
int CNQTVDlg::makeTimer( char *name, timer_t *timerID, int expireMS, int intervalMS )
{
    struct sigevent         te;
    struct itimerspec       its;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;

    /* Set up signal handler. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1)
    {
        //fprintf(stderr, "%s: Failed to setup signal handling for %s.\n", PROG, name);
        return(-1);
    }

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timerID;
    timer_create(CLOCK_REALTIME, &te, timerID);

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = intervalMS * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = expireMS * 1000000;
    timer_settime(*timerID, 0, &its, NULL);

    return(0);
}

///////////////////////////////////////////////////////////////////////////////////
void CNQTVDlg::timerHandler( int sig, siginfo_t *si, void *uc )
{
    timer_t tidp;
    tidp = si->si_value.sival_ptr;

    if (tidp == firstTimerID)
    {
        SetPerSec();
        SetMaxPerSec();
    }

    /* else if ( *tidp == secondTimerID )
         //secondCB(sig, si, uc);
     else if ( *tidp == thirdTimerID )
        // thirdCB(sig, si, uc);
     */
}

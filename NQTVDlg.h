
// QtSpryDlg.h : header file
//
#pragma once
#include <signal.h>
#include <time.h>
#include <mysql/mysql.h>
 
#include "ReceiveITCH.h"

// CNQTVDlg dialog
class CNQTVDlg 
{
// Construction
public:
	CNQTVDlg();	// standard constructor
	~CNQTVDlg();

// Dialog Data

	protected:

	int ProcessFeedThread(void*);
	int FeedThread(void*);
	

// Implementation
protected:
	// Generated message map functions
public:
	 int InsertRow();
	 void FeedPause();
	 void FeedStop();
	 int FeedStart();
	 
	 void OnBnClickedButtonProcessPause();
	 void OnBnClickedButtonProcessStop();
	 void OnBnClickedButtonProcessStart();
	 void OnBnClickedButtonFlushAll();

	 void OnBnClickedButtonFlushSingle();

	static bool m_bTestFeed; 
	
	MYSQL* ConnectMySql(char* host_name, char* user_name, char* password, char* db_name,
						unsigned int port_num, char* socket_name, unsigned int flags);
	void /*int*/ Disconnect1MySql(MYSQL* conn);
private:
	void timer_handler (int signum);

	static   CReceiveITCH*  m_pCQtReceiveITCH;

	static FEED_MESSAGE_STATS m_MessageStats;

	static bool	m_bReplay;

	static void ResetPerSec();
	static void SetMaxPerSec();
	static void SetPerSec();


	
	int  makeTimer( char *name, timer_t *timerID, int expireMS, int intervalMS );
	static void timerHandler( int sig, siginfo_t *si, void *uc );
	int srtSchedule( void );
	

};

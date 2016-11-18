
// NQTV.h : main header file for the PROJECT_NAME application
// QtSpry.h : main header file for the PROJECT_NAME application
//
#pragma once
#include <string>

#include <mysql/mysql.h>
#include "Settings.h"

#include "ItchIncludes.h"
  
enum feedStatus
{
	OK = 0,
	STARTING,
	RUNNING,
	PAUSSING,
	PAUSSED,
	RESUMING,
	RESUMED,
	STOPPING,
	STOPPED,
	STARTCOMERROR,
	PROCESSERROR
};

// CSpryApp:
// See QtSpry.cpp for the implementation of this class
//

#define MAX_MESSAGE_TYPES    23

class CQuantApp
{
public:
	CQuantApp();
	~CQuantApp();
	int		g_iFlag;
	int		g_iFeedStatus;
	int		g_iProcessMessageStatus;

	bool		g_bConnected;
	bool		g_bReceiving;
	int		iStatus;
	
	MYSQL* conn; // pointer to connection handler

	FEED_MESSAGE_STATS	g_Stats;
	LOCALFILEDATA  		SMemoryMappedFileSSize;
	
	std::string	strFeedFileName;
	DWORD		dwBufferSize;

	LOB_VIEW_CLT_OPTIONS	m_SOptions;
	unsigned long 	g_arrTotalMessages [MAX_MESSAGE_TYPES];
	unsigned long 	g_arrMessagesPerSec [MAX_MESSAGE_TYPES];
	unsigned long 	g_arrMaxMessagesPerSec [MAX_MESSAGE_TYPES];
	SETTINGS  	SSettings;

};

extern CQuantApp theApp;
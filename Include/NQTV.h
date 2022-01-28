/* 
 * This file is part of the QuantServer (https://github.com/geneidy/QuantServer).
 * Copyright (c) 2017 geneidy.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
	APPLY,
	CANCEL,
	STARTING,
	RUNNING,
	PAUSSING,
	PAUSSED,
	RESUMING,
	RESUMED,
	STOPPING,
	STOPPED, // 10
	CONSTRUCTED,
	DESTRUCTED,
	TERMINATE,
	STARTCOMERROR,
	PROCESSERROR,
	TERMINATE_QUEUE
};

// CSpryApp:
// See QtSpry.cpp for the implementation of this class
//

#define MAX_MESSAGE_TYPES 23

class CQuantApp
{
public:
	CQuantApp();
	~CQuantApp();
	int g_iFlag;
	int g_iFeedStatus;
	int g_iProcessMessageStatus;
	//	MYSQL* conn; // pointer to connection handler

	FEED_MESSAGE_STATS g_Stats;

	std::string strFeedFileName;

	unsigned long g_arrTotalMessages[MAX_MESSAGE_TYPES];
	unsigned long g_arrMessagesPerSec[MAX_MESSAGE_TYPES];
	unsigned long g_arrMaxMessagesPerSec[MAX_MESSAGE_TYPES];
	SETTINGS SSettings;
};

extern CQuantApp theApp;
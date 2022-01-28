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
#pragma once

#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "string.h"
//#include "Logger.h"
#include <vector>
#include "Settings.h"

#include "QSettings.h"

#include "SaveToDisk.h"
#include "SaveToDB.h"
#include "BuildBookLL.h"
//#include "BuildBookSTL.h"
#include "OrdersMap.h"
#include "TickDataMap.h"
#include "ReceiveITCH.h"
#include "QuantQueue.h"
#include "StatsPerSec.h"

#include "SaveOrdersToDisc.h"

enum tstate
{
    TS_INACTIVE,
    TS_STARTING,
    TS_STARTED,
    TS_ALIVE,
    TS_TERMINATED,
    TS_JOINED
};

typedef struct ThreadData
{
    int idx;
    void *pVoid;
    int iTotalThreads;
    CQuantQueue *g_pCQuantQueue;
} THREAD_DATA;

THREAD_DATA g_SThreadData;

bool g_bSettingsLoaded;

typedef struct thread_info
{                        /* Used as argument to thread_start() */
    pthread_t thread_id; /* ID returned by pthread_create() */
    enum tstate eState;
    int iThread_num;      /* Application-defined thread # */
    char *thread_message; /* Saying Hello */
} THREAD_INFO;

THREAD_INFO arrThreadInfo[NUMBER_OF_ROLES];
int PrimeSettings();
//////////////////////////////////////
void *Settings(void *);
void *MainQueue(void *);
//////////////////////////////////////

void *ReceiveFeed(void *);
void *ParseFeed(void *);
void *BuildBook(void *);
void *SaveToDB(void *);
void *PlayBack(void *);

void *NasdTestFile(void *);
void *StatsPerSec(void *);

void *Distributor(void *);
void *SaveToDisk(void *);
void *OrdersMap(void *);
void *TickDataMap(void *);
void *SaveOrdersToDisc(void *);
void InitThreadLog(int);
void TermThreadLog(int);

void *(*func_ptr[NUMBER_OF_ROLES])(void *) =
    {MainQueue, Settings, ReceiveFeed, ParseFeed, OrdersMap,
     BuildBook, TickDataMap, SaveToDB, PlayBack, NasdTestFile,
     StatsPerSec, SaveOrdersToDisc, Distributor, SaveToDisk}; // All Roles for the server functions are here

const char *ThreadMessage[NUMBER_OF_ROLES + 1] =
    {"Main Queue Thread", "Settings Thread", "Receive Feed Thread", "Parse Thread", "Orders Map Thread",
     "Build Book Thread", "Tick Data Thread", "Save To DB Thread", "Play Back Thread", "Nasd Test File Thread",
     "Stats Per Second Thread", "Save Orders To Disc", "Distributor Thread", "Save To Disk Thread"};

struct timespec m_request, m_remain;

#ifdef __cplusplus
extern "C"
{
#endif
    CSaveToDisk *pCSaveToDisk;
    CSaveToDB *pCSaveToDB;
    //CBuildBookSTL* 	pCBuildBook;
    CBuildBook *pCBuildBook;
    COrdersMap *pCOrdersMap;
    CTickDataMap *pCTickDataMap;
    CReceiveITCH *pCReceiveITCH;
    CQuantQueue *pCQuantQueue;

    CQSettings *pCQSettings;
    CStatsPerSec *pCStatsPerSec;
    //CDisplayBook* pCDisplayBook;
    CSaveOrdersToDisc *pCSaveOrdersToDisc;

#ifdef __cplusplus
} // extern "C"
#endif

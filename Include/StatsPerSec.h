/* 
 * TThis file is part of the QuantServer (https://github.com/geneidy/QuantServer).
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
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <string>
#include "string.h"
#include "Logger.h"
#include "Util.h"
#include "Includes.h"
#include "ITCHMessages.h"
#include "Settings.h"

// #define _POSIX_C_SOURCE 199309

using namespace std;

typedef struct SMessagesPerSec
{
  unsigned long arrMessagesPerSec[MAX_MESSAGE_TYPES];
  unsigned long arrMaxMessagesPerSec[MAX_MESSAGE_TYPES];
  unsigned long arrTotalMessages[MAX_MESSAGE_TYPES];
} SMESSAGESPERSEC;

class CStatsPerSec
{
private:
  void *m_addr;
  int m_fd;
  struct stat m_sb;

  int InitMemoryMappedFile();

  int m_iError;
  static bool m_bReplay;

  struct timespec m_request, m_remain;
  static timer_t firstTimerID;

  static SMESSAGESPERSEC m_SMessagesPerSec;
  static SMESSAGESPERSEC *m_pMessagesPerSec;

  CUtil *m_Util;

  static void ResetPerSec();

  int makeTimer(char *name, timer_t *timerID, int expireMS, int intervalMS);
  static void timerHandler(int sig, siginfo_t *si, void *uc);
  static unsigned long m_arrMessagesPerSec[MAX_MESSAGE_TYPES];

public:
  CStatsPerSec();
  ~CStatsPerSec();
  int GetError();
  static void SetMaxPerSec();
  static void SetPerSec();
};

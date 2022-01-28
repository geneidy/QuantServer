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
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string>
#include "string.h"
#include "Logger.h"
#include "Util.h"
#include "Includes.h"
#include "Settings.h"

// #define _POSIX_C_SOURCE 199309

using namespace std;

class CQSettings
{
private:
  void *m_addr;
  int m_fd;
  struct stat m_sb;

  int InitMemoryMappedFile();

  int m_iError;

  int m_iMessage;
  int m_uiSizeOfSettingsRecord;

  struct timespec m_request, m_remain;

  CUtil *m_Util;

  SETTINGS *m_pSettings;
  SETTINGS m_ssettings;

public:
  CQSettings();
  ~CQSettings();
  int GetError();
  SETTINGS GetSettings();
  SETTINGS LoadSettings();
};

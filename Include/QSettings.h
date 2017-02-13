#pragma once
#include  <sys/mman.h>
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
  void* m_addr;
  int m_fd;
  struct stat64 m_sb;
  
  int InitMemoryMappedFile();

  int m_iError;
  
  int m_iMessage;
  int   m_uiSizeOfSettingsRecord;

  struct timespec m_request, m_remain;

  CUtil*  m_Util;

  SETTINGS* m_pSettings;
  SETTINGS m_ssettings;

 
public:
   CQSettings();
  ~CQSettings();
  int GetError();
  SETTINGS GetSettings();
  SETTINGS LoadSettings();
};

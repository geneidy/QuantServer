#pragma once
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <string>

#include "ITCHMessages.h"
using namespace std;

class CDistributor
{
public:
  
  CDistributor();
  ~CDistributor();
  
   int WriteFeedToFile(char cMessageType, ITCH_MESSAGES_UNION MessageToWrite);
   uint64_t GetLastSequence();
   QT_ITCH_MESSAGE GetMessageBySequence(uint64_t);
   
   uint64_t GetNumberOfMessagesInFile();
   
   static uint64_t  m_ui64WriteSequence;
   QT_ITCH_MESSAGE  m_DistMessage;
   int m_iSizeOfMessage;
   int m_iHandle;

  int m_iError;
  std::string m_strError;
  int GetError();
  string GetErrorString();
  
};
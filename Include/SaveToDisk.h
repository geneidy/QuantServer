/*
 * Copyright 2016 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef CSAVETODISK_H
#define CSAVETODISK_H

#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <string>
#include "QuantQueue.h"
#include "ITCHMessages.h"

class CSaveToDisk
{
  CQuantQueue *m_pQuantQueue;
  ITCH_MESSAGES_UNION *pItchMessageUnion;

public:
  CSaveToDisk();
  CSaveToDisk(const CSaveToDisk &other);
  ~CSaveToDisk();
  CSaveToDisk &operator=(const CSaveToDisk &other);
  bool operator==(const CSaveToDisk &other) const;

  // int WriteFeedToFile(char cMessageType, ITCH_MESSAGES_UNION MessageToWrite);
  int WriteFeedToFile();

  uint64_t GetLastSequence();
  QT_ITCH_MESSAGE GetMessageBySequence(uint64_t);

  uint64_t GetNumberOfMessagesInFile();

  static uint64_t m_ui64WriteSequence;
  QT_ITCH_MESSAGE m_DiskMessage;
  int m_iSizeOfMessage;
  int m_iHandle;

  int m_iError;
  std::string m_strError;
  int GetError();
  string GetErrorString();
};

#endif // CSAVETODISK_H

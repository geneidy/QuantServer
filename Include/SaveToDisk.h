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

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
#include <map>
#include <atomic>
#include <pthread.h>
#include <mutex>

#include "Util.h"

#include "time.h"

#include "ITCHMessages.h"
#include "OrdersMap.h"

class CSaveOrdersToDisc //: public COrdersMap
{
private: // by default
    int m_ifd;

    uint64_t m_ui64NumRequest;

    ITCH_MESSAGES_UNION *m_pItchMessageUnion;
    COrdersMap *m_pCOrdersMap;

    COMMON_ORDER_MESSAGE *m_pCommonOrder;
    uint64_t m_uiNextOrder;

    timespec m_request;
    timespec m_remain;
    CUtil *m_pcUtil;

public:
    int m_iError;
    int ReadFromOrdersMap();
    CSaveOrdersToDisc();
    ~CSaveOrdersToDisc();

    int GetError();
};

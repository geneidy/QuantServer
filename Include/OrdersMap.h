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

#include <map>
#include <unordered_map>

#include <atomic>
#include <mutex>
#include "Util.h"
#include "QuantQueue.h"

//#define _POSIX_C_SOURCE 199309

#include "ITCHMessages.h"

typedef struct
{
  uint64_t uiNumberOfMessagesToHold;
  uint64_t ui64NumOfOrders;
  uint64_t SymbolMapSize;
  uint64_t SymbolMapMaxSize;

} SOrdersDataStat;

typedef unordered_map<uint64_t, uint64_t> OrdersSequenceUnOrderedMap;     // <  Order Ref Number  ,   Location in File   >
typedef unordered_map<uint64_t, COMMON_ORDER_MESSAGE> OrdersUnOrderedMap; // <  Order Ref Number  ,   Location in File   >

class COrdersMap
{
private:
  int m_ifd;
  OrdersUnOrderedMap::const_iterator m_itSymbolMap;

  OrdersUnOrderedMap::const_iterator m_itAuxSymbolMap;
  OrdersUnOrderedMap::const_iterator m_itRefAuxSymbolMap;

  CQuantQueue *m_pQuantQueue;

  COMMON_ORDER_MESSAGE *m_pCommonOrder;
  COMMON_ORDER_MESSAGE m_CommonOrder;

  static __thread COMMON_ORDER_MESSAGE *m_pReturnCommonOrder;

  COMMON_ORDER_MESSAGE *m_pRefCommonOrder;

  static __thread COMMON_ORDER_MESSAGE *m_pTempCommonOrder;

  ITCH_MESSAGES_UNION *pItchMessageUnion;

  ADD_ORDER_NO_MPID_MESSAGE m_Add_Order_No_Mpid;
  ADD_ORDER_MPID_MESSAGE m_Add_Order_Mpid;
  ORDER_REPLACE_MESSAGE m_OrderReplace;

  ORDER_EXECUTED_MESSAGE m_OrderExecuted;                     // 'E'
  ORDER_EXECUTED_WITH_PRICE_MESSAGE m_OrderExecutedWithPrice; // 'c'
  ORDER_CANCEL_MESSAGE m_OrderCancel;                         // 'X'

  int InitMemoryMappedFile();

  int m_iError;

  int m_iMessage;
  uint64_t m_uiSizeOfCommonOrderRecord;

  uint64_t m_uiNumberOfMessagesToHold;

  struct timespec m_request, m_remain;

  struct timespec tspec;
  uint64_t m_ui64OrderTime;

  CUtil *m_Util;

  //  static mutex m_;

  static std::mutex MapMutex;
  static std::mutex FindMapMutex;

  COMMON_ORDER_MESSAGE *GetMappedOrder(uint64_t uiOrderRefNumber);

  COrdersMap();
  pair<OrdersUnOrderedMap::iterator, bool> RetPair;
  pair<OrdersSequenceUnOrderedMap::iterator, bool> RetPair1;

protected:
  static COrdersMap *pInstance;

public:
  static OrdersUnOrderedMap m_SymbolMap;
  static OrdersUnOrderedMap::iterator m_itBookRefSymbolMap;

  static OrdersSequenceUnOrderedMap m_SequenceMap;
  static OrdersSequenceUnOrderedMap::const_iterator m_itSequenceMap;

  /*static*/ uint64_t m_ui64NumOfOrders;
  /*static*/ uint64_t m_ui64OrderSequence;
  ~COrdersMap();
  static COrdersMap *instance();
  static uint iNInstance;
  uint64_t FillMemoryMap(ITCH_MESSAGES_UNION *, int);
  uint64_t GetNumberOfOrders();

  COMMON_ORDER_MESSAGE *GetMemoryMappedOrder(uint64_t);

  SOrdersDataStat GetOrdersDataStat();
  SOrdersDataStat m_SOrdersDataStat;
  //  COMMON_ORDER_MESSAGE* GetOrder(uint64_t uiOrderRefNumber);
  int GetError();
  uint64_t GetMapSize();
  void InitQueue(CQuantQueue *pQueue);
};

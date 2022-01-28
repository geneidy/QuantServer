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
#include "OrdersMap.h"
#include <map>
#include "Util.h"

#include "ITCHMessages.h"
#include "QuantQueue.h"

typedef struct _IndexRecord
{

  char szSymbol[SIZE_OF_SYMBOL];
  double dLast;
  double dOpen;
  double dClose;
  double dHigh;
  double dLow;
  uint32_t uiVolume;
  int cTick; // '+'  '-'  '='

  double dVWAP;
  uint64_t uiNumOfTradesWithPrice;
  uint64_t uiNumOfTradesNoPrice;
  uint64_t uiTotalVolume;
  uint64_t uiTotalNumOfTrades;

  timespec tOpen;
  timespec tLastUpdate;

} SFUNDAMENTAL_RECORD;

typedef struct
{
  uint64_t uiNumberOfMessagesToHold;
  uint64_t ui64NumOfTickData;
  uint64_t uiFundamemtalMapSize;
  uint64_t uiTickMapSize;
  uint64_t uiFundamemtalMapMaxSize;
  uint64_t uiTickMapMaxSize;
} STickDataStat;

//typedef unordered_map<string , SFUNDAMENTAL_RECORD> FundamentalMap;  // <Stock Symbol  SFUNDAMENTAL_RECORD>>
//typedef multimap< string , uint64_t> TickMap;  // <Stock Symbol  COMMON_TRADE_MESSAGE>>

class CTickDataMap
{
private:
  void *m_addr;
  int m_fd;
  struct stat64 m_sb;

  //  FundamentalMap 	 	m_FundamentalMap;
  //  FundamentalMap::iterator   	m_itFundamentalMap;

  //  TickMap 	 		m_TickMap;
  //  TickMap::iterator   		m_itTickMap;

  CQuantQueue *m_pQuantQueue;

  COMMON_TRADE_MESSAGE m_CommonTrade;

  COMMON_ORDER_MESSAGE *m_pCommonOrder;

  ITCH_MESSAGES_UNION *pItchMessageUnion;
  ITCH_MESSAGES_UNION m_ItchMessageUnion;

  ADD_ORDER_NO_MPID_MESSAGE m_Add_Order_No_Mpid;
  ADD_ORDER_MPID_MESSAGE m_Add_Order_Mpid;
  ORDER_REPLACE_MESSAGE m_OrderReplace;

  ORDER_EXECUTED_MESSAGE m_OrderExecuted;                     // 'E'
  ORDER_EXECUTED_WITH_PRICE_MESSAGE m_OrderExecutedWithPrice; // 'c'
  ORDER_CANCEL_MESSAGE m_OrderCancel;                         // 'X'

  SYSTEM_EVENT_MESSAGE m_pSystemEvent;
  SFUNDAMENTAL_RECORD m_SFundamentalRecord;

  //  int InitMemoryMappedFile();

  inline void InitFundamentalRecord();

  int m_iError;

  int m_iMessage;
  int m_iSizeOfCommonTradeRecord;
  int m_iSizeOfCommonOrderRecord;
  int m_iSizeOfFundamentalRecord;

  uint64_t m_uiNumberOfMessagesToHold;
  uint64_t m_ui64NumOfTickData;

  timespec m_request, m_remain;

  //  std::pair<TickMap::iterator, bool>  RetPairT;
  //  std::pair<FundamentalMap::iterator, bool>  RetPairF;
  STickDataStat m_STickDataStat;

  CUtil *m_pcUtil;

  int m_ifd;

public:
  CTickDataMap();
  ~CTickDataMap();

  COrdersMap *m_pCOrdersMap;

  uint64_t ReadFromOrdersMap();
  //  void InitQueue(CQuantQueue* pQueue);
  uint64_t FillTickFile();
  STickDataStat GetTickDataStat();

  COMMON_TRADE_MESSAGE GetMappedRecord(uint64_t uiPosition);
  int GetError();
  COMMON_TRADE_MESSAGE *GetTradeTicks(char *szStock);
  SFUNDAMENTAL_RECORD *GetFundamentalRecord(char *szStock);
};

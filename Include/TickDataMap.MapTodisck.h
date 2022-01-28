#pragma once
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "stdio.h"
#include "stdlib.h"

#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#include <time.h>
#include "OrdersMap.h"
#include <map>
//#include <unordered_map>

#include <atomic>
#include <mutex>

#define BILLION_INT 1000000000L

#define BILLION_DOUBLE 1E9

#include "ITCHMessages.h"
#include "QuantQueue.h"

//struct timespec

typedef struct _IndexRecord
{

  char szSymbol[5];
  double dLast;
  double dOpen;
  double dClose;
  double dHigh;
  double dLow;
  char cTick; // '+'  '-'  '='

  double dVWAP;
  uint64_t uiNumOfTradesWithPrice;
  uint64_t uiNumOfTradesNoPrice;
  uint64_t uiTotalVolume;
  uint64_t uiTotalNumOfTrades;

  struct timespec tOpen;
  struct timespec tLastUpdate;

  uint64_t uiFirstLocation;
  uint64_t uiLastLocation;

} SINDEX_RECORD;

// typedef multimap<char* , uint64_t> SymbolMap;  // <Stock Symbol  COMMON_ORDER_MESSAGE>>
typedef unordered_map<char *, SINDEX_RECORD> SymbolMap; // <Stock Symbol  COMMON_ORDER_MESSAGE>>

class CTickDataMap
{
private:
  void *m_addr;
  int m_fdData;
  int m_fdIndex;
  struct stat m_sb;

  SymbolMap m_SymbolMap;
  SymbolMap::iterator m_itSymbolMap;

  CQuantQueue *m_pQuantQueue;

  COMMON_TRADE_MESSAGE m_SCommonTrade;
  COMMON_TRADE_MESSAGE m_STempCommonTrade;
  SINDEX_RECORD m_SIndexRecord;

  COrdersMap *m_pCOrdersMap;
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

  int InitMemoryMappedFile();

  int m_iError;

  int m_iMessage;
  int m_iSizeOfCommonTradeRecord;
  int m_iSizeOfIndexRecord;

  uint64_t m_ui64TickRecordLocation;

  struct timespec m_request, m_remain;

  CTickDataMap();
  inline void InitIndexRecord();

public:
  ~CTickDataMap();
  uint64_t FillTickDataFile();
  uint64_t GetNumberOfTickData();

  COMMON_TRADE_MESSAGE GetMappedRecord(uint64_t uiPosition);
  int GetError();
  uint64_t GetMapSize();
  uint64_t GetMapMaxSize();
};

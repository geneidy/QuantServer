#pragma once
#include  <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <time.h>

#include <map>
#include <unordered_map>


#include <atomic>
#include <mutex>
#include "Util.h"

// #define _POSIX_C_SOURCE 199309

#include "ITCHMessages.h"
#include "QuantQueue.h"

  typedef struct {
  uint64_t 	uiNumberOfMessagesToHold;
  uint64_t	ui64NumOfOrders;
  uint64_t      SymbolMapSize; 
  uint64_t      SymbolMapMaxSize; 
  
  }SOrdersDataStat;


typedef unordered_map<uint64_t , uint64_t> OrdersUnOrderedMap;  // <  Order Ref Number  ,   Location in File   >

class COrdersMap 
{
private:
  void* m_addr;
  int m_fd;
  struct stat64 m_sb;
  
  OrdersUnOrderedMap 	 	m_SymbolMap;
  OrdersUnOrderedMap::iterator		m_itSymbolMap;
  CQuantQueue*		m_pQuantQueue;
  
  COMMON_ORDER_MESSAGE* m_pCommonOrder;
  ITCH_MESSAGES_UNION* pItchMessageUnion;  
  
  ADD_ORDER_NO_MPID_MESSAGE  		m_Add_Order_No_Mpid;
  ADD_ORDER_MPID_MESSAGE  		m_Add_Order_Mpid;
  ORDER_REPLACE_MESSAGE 		m_OrderReplace;

  ORDER_EXECUTED_MESSAGE		m_OrderExecuted;  // 'E'
  ORDER_EXECUTED_WITH_PRICE_MESSAGE	m_OrderExecutedWithPrice; // 'c'
  ORDER_CANCEL_MESSAGE			m_OrderCancel;  // 'X'
  
  int InitMemoryMappedFile();

  int m_iError;
  
  int m_iMessage;
  uint64_t   m_uiSizeOfCommonOrderRecord;
  uint64_t	m_ui64NumOfOrders;
  
  uint64_t m_uiNumberOfMessagesToHold;
  
  struct timespec m_request, m_remain;

  CUtil*  m_Util;
  
//  static mutex m_;
  
  static std::mutex MapMutex;

   COrdersMap();
   pair<OrdersUnOrderedMap::iterator, bool>  RetPair;
protected:
	static COrdersMap *pInstance;  
	friend class Cleanup;
	class Cleanup
	{
	public:
		~Cleanup();
	};
 
public:

  ~COrdersMap();
  static COrdersMap* instance();
  uint64_t FillMemoryMappedFile();
  uint64_t GetNumberOfOrders();
  
  SOrdersDataStat GetOrdersDataStat();
  SOrdersDataStat  m_SOrdersDataStat;
  COMMON_ORDER_MESSAGE* GetMappedOrder(uint64_t uiOrderRefNumber);
  int GetError();
  uint64_t GetMapSize();
  

};

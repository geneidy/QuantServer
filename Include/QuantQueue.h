#pragma once

#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
// #include <sys/timeb.h>
#include "string.h"


#include <atomic>
#include <mutex>
using namespace std;

#include "ITCHMessages.h"
#include "Logger.h"
#include "NQTV.h"




#define   MESSAGE_TYPE_SYSTEM_EVENT  		0
#define   MESSAGE_TYPE_STOCK_DIRECTORY  		1
#define   MESSAGE_TYPE_STOCK_TRADING_ACTION 		2
#define   MESSAGE_TYPE_REG_SHO_RESTRICTION 		3
#define   MESSAGE_TYPE_MP_POSITION			4
#define   MESSAGE_TYPE_MWCBDLM			5
#define   MESSAGE_TYPE_MWCBDBM			6
#define   MESSAGE_TYPE_IPO_QUOTATION_PERIOD_UPDATE 	7
#define   MESSAGE_TYPE_ADD_ORDER_NO_MPID		8
#define   MESSAGE_TYPE_ADD_ORDER_MPID		9
#define   MESSAGE_TYPE_ORDER_EXECUTED		10
#define   MESSAGE_TYPE_ORDER_EXECUTED_WITH_PRICE	11
#define   MESSAGE_TYPE_ORDER_CANCEL			12
#define   MESSAGE_TYPE_ORDER_DELETE			13
#define   MESSAGE_TYPE_ORDER_REPLACE			14
#define   MESSAGE_TYPE_TRADE_NON_CROSS			15
#define   MESSAGE_TYPE_NOII				16
#define   MESSAGE_TYPE_RPII				17


#define  POSITION_TOP			0
#define  POSTITON_CURRENT		1

#define   E_READER_EQ_WRITER		100
#define   E_INVALID_MESSAGE_TYPE  	200


#pragma pack(push,old_pack,1)

   typedef struct {
        unsigned int uiEnqQSystemEvent		;
        unsigned int uiEnqQStockDirectory	;
        unsigned int uiEnqQStockTradingAction	;
        unsigned int uiEnqQRegSho		;
        unsigned int uiEnqQMpPosition		;
        unsigned int uiEnqQMWCBDLM 		;
        unsigned int uiEnqQMWCBDBM		;
        unsigned int uiEnqQIPOQutationUpdate	;
        unsigned int uiEnqQAddOrderNoMPID	;
        unsigned int uiEnqQAddOrderMPID      	;
        unsigned int uiEnqQOrderExecuted     	;
        unsigned int uiEnqQOrderExecutedWithPrice;
        unsigned int uiEnqQOrderCancel		;
        unsigned int uiEnqQOrderDelete       	;
        unsigned int uiEnqQOrderReplace      	;
        unsigned int uiEnqQTradeNonCross     	;
        unsigned int uiEnqQNOII 		;
        unsigned int uiEnqQRPPI         	;
	unsigned int uiEnqQUnknown         	;
	

	unsigned int uiDeqQSystemEvent		;
        unsigned int uiDeqQStockDirectory	;
        unsigned int uiDeqQStockTradingAction	;
        unsigned int uiDeqQRegSho		;
        unsigned int uiDeqQMpPosition		;
        unsigned int uiDeqQMWCBDLM 		;
        unsigned int uiDeqQMWCBDBM		;
        unsigned int uiDeqQIPOQutationUpdate	;
        unsigned int uiDeqQAddOrderNoMPID	;
        unsigned int uiDeqQAddOrderMPID      	;
        unsigned int uiDeqQOrderExecuted     	;
        unsigned int uiDeqQOrderExecutedWithPrice;
        unsigned int uiDeqQOrderCancel		;
        unsigned int uiDeqQOrderDelete       	;
        unsigned int uiDeqQOrderReplace      	;
        unsigned int uiDeqQTradeNonCross     	;
        unsigned int uiDeqQNOII 		;
        unsigned int uiDeqQRPPI         	;
        unsigned int uiDeqQUnknown         	;
	
	unsigned int uiQWrap;
	unsigned int uiReadWrap;
    }QSTAT;



typedef struct _SControl   // move it to server includes
{
    int iOpCode;
    int iParam[10];
} CONTROL;

typedef struct _AMGQueue
{
    u_int64_t		i64NodeNumber;
    int			iMessagetype;  // to determine how to cast the void pointer
    time_t		dwTickCount;
    long		lFeedTime;
    struct timeval	lTime;
    int  		FeedID;  // in case multiple feeds are carried in the Queue
    ITCH_MESSAGES_UNION QMessage;
} QUANT_QUEUE, *LPQUANTQUEUE;


typedef struct _AMGQueueMessage
{
    void* SpMessage;  // decode according to message type
    int   iMessageType;
} AMGQUEUEMESSAGE, *LPQUANTQUEUEMESSAGE;


class   CQuantQueue
{
private:
    LPQUANTQUEUE	m_pHead;
    LPQUANTQUEUE	m_pWriter;
    LPQUANTQUEUE	m_pReader;
    int			m_iErrorCode;
    u_int64_t		m_i64QueueSize;
    u_int64_t		m_i64QueueElement;

    CQuantQueue(const CQuantQueue& other);   // copy consructor
    CQuantQueue& operator=(const CQuantQueue& other);
    bool operator==(const CQuantQueue& other) const;
    int iPosition;

    static __thread u_int64_t m_Threadi64LastIndex ;
    static __thread u_int64_t m_Threadi64LastRead;
    u_int64_t m_i64LastWrite;
    static __thread int m_ThreadiStatus;
    

public:
  
    void ListQStats();
    static bool	bConstructed;

    virtual ~CQuantQueue(void);

    void* Dequeue(int *iMessagetype);
    int Enqueue(ITCH_MESSAGES_UNION* pQueueElement, int iMessageType);

    void InitReader(int iPosition);
    int  QueueEfficiency();

    u_int64_t m_i64FreeElements;
    u_int64_t m_ulTotalElements;
    // return total elements
    u_int64_t GetTotalElements(void);
    u_int64_t GetTotalFreeElements(void);

    int GetErrorCode();
    u_int64_t	GetNumberQueueElements(void);

    //pointer to arrays that will hold the elements, pointed to by Queue nodes...
    SYSTEM_EVENT_MESSAGE  		*m_SystemEvent;
    STOCK_DIRECTORY_MESSAGE  		*m_StockDirectory;
    STOCK_TRADING_ACTION_MESSAGE 		*m_StockTradingAction;
    REG_SHO_RESTRICTION_MESSAGE 		*m_RegSho;
    MP_POSITION_MESSAGE			*m_MpPosition;
    MWCBDLM_MESSAGE			*m_MWCBDLM;
    MWCBDBM_MESSAGE			*m_MWCBDBM;
    IPO_QUOTATION_PERIOD_UPDATE_MESSAGE 	*m_IPOQutationUpdate;
    ADD_ORDER_NO_MPID_MESSAGE		*m_AddOrderNoMPID;
    ADD_ORDER_MPID_MESSAGE		*m_AddOrderMPID;
    ORDER_EXECUTED_MESSAGE		*m_OrderExecuted;
    ORDER_EXECUTED_WITH_PRICE_MESSAGE 	*m_OrderExecutedWithPrice;
    ORDER_CANCEL_MESSAGE			*m_OrderCancel;
    ORDER_DELETE_MESSAGE			*m_OrderDelete;
    ORDER_REPLACE_MESSAGE			*m_OrderReplace;
    TRADE_NON_CROSS_MESSAGE		*m_TradeNonCross;
    NOII_MESSAGE				*m_NOII;
    RPII_MESSAGE				*m_RPPI;
    LPQUANTQUEUE				m_pPrev;

    static CQuantQueue* Instance();
private:
//	  void init(SETTINGS sSettings);
    void init();
    CQuantQueue(); // private consructor will call the init() method to initialize the Queue
//    unsigned int				m_ulWrap;

    COMMON_ORDER_MESSAGE	m_CommonOrder;
    
    QSTAT  m_QStat;

    static atomic<CQuantQueue*> pinstance;
    static mutex m_;

}; // end class declaration

#pragma pack(pop,old_pack)

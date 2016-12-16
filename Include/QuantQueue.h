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





// Define each Queue size in number of elements in millions
#define MILLION					1000000
#define QSIZEFUNDAMENTAL		1*MILLION
#define QSIZEMMAKER				1*MILLION
#define	QSIZEQUOTE				1*MILLION
#define QSIZEDEPTHQUOTE			1*MILLION
#define QSIZETRADE				1*MILLION
#define QSIZECONTROL			100


// REVISE ALL THE FOLLOWING NUMBERS according TO NASD Stats
#define THOUSAND					1000

#define   QSIZE_SYSTEM_EVENT  			100
#define   QSIZE_STOCK_DIRECTORY  		10*THOUSAND
#define   QSIZE_STOCK_TRADING_ACTION 		2*THOUSAND
#define   QSIZE_REG_SHO_RESTRICTION 		3*THOUSAND
#define   QSIZE_MP_POSITION			4*THOUSAND
#define   QSIZE_MWCBDLM				5*THOUSAND
#define   QSIZE_MWCBDBM				6*THOUSAND
#define   QSIZE_IPO_QUOTATION_PERIOD_UPDATE 	7*THOUSAND

#define   QSIZE_ADD_ORDER_NO_MPID		1*MILLION
#define   QSIZE_ADD_ORDER_MPID			1*MILLION
#define   QSIZE_ORDER_EXECUTED			1*MILLION
#define   QSIZE_ORDER_EXECUTED_WITH_PRICE	1*MILLION
#define   QSIZE_ORDER_CANCEL			1*MILLION
#define   QSIZE_ORDER_DELETE			1*MILLION
#define   QSIZE_ORDER_REPLACE			1*MILLION
#define   QSIZE_TRADE_NON_CROSS			1*MILLION

#define   QSIZE_NOII				1*THOUSAND
#define   QSIZE_RPII				1*THOUSAND


#define  POSITION_TOP		0
#define  POSTITON_CURRENT	1

#define   E_READER_EQ_WRITER	100
#define   E_INVALID_MESSAGE_TYPE  200


#pragma pack(push,old_pack,1)

   typedef struct {
        unsigned int uiQWrapSystemEvent		;
        unsigned int uiQWrapStockDirectory	;
        unsigned int uiQWrapStockTradingAction	;
        unsigned int uiQWrapRegSho		;
        unsigned int uiQWrapMpPosition		;
        unsigned int uiQWrapMWCBDLM 		;
        unsigned int uiQWrapMWCBDBM		;
        unsigned int uiQWrapIPOQutationUpdate	;
        unsigned int uiQWrapAddOrderNoMPID	;
        unsigned int uiQWrapAddOrderMPID      	;
        unsigned int uiQWrapOrderExecuted     	;
        unsigned int uiQWrapOrderExecutedWithPrice;
        unsigned int uiQWrapOrderCancel		;
        unsigned int uiQWrapOrderDelete       	;
        unsigned int uiQWrapOrderReplace      	;
        unsigned int uiQWrapTradeNonCross     	;
        unsigned int uiQWrapNOII 		;
        unsigned int uiQWrapRPPI         	;
	unsigned int uiQWrap;
    }SSTAT;

typedef struct _SControl   // move it to server includes
{
    int iOpCode;
    int iParam[10];
} CONTROL;

typedef struct _AMGQueue
{
    u_int64_t	i64NodeNumber;
    u_int64_t	i64Index;  // will point to the proper array structure according to message type
    int		iMessagetype;  // to determine how to cast the void pointer
    time_t		dwTickCount;
    long		lFeedTime;

    struct timeval	lTime;
    int  		FeedID;  // in case multiple feeds are carried in the Queue
    // _AMGQueue	*pNext;
    // _AMGQueue	*pPrevious;
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
    static bool m_EnqueueCriticalSection;
    static bool m_DeQueueCriticalSection;
    static __thread u_int64_t m_Threadi64LastIndex ;
    static __thread u_int64_t m_Threadi64LastNode;
    static __thread int m_ThreadiStatus;

public:

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

    unsigned int GetRecycleTimes() {
        return m_sStat.uiQWrap;
    };
    inline  u_int64_t GetLastNodeNumber()  {
        return m_Threadi64LastNode;
    };
    static CQuantQueue* Instance();
private:
//	  void init(SETTINGS sSettings);
    void init();
    CQuantQueue(); // private consructor will call the init() method to initialize the Queue
//    unsigned int				m_ulWrap;

    COMMON_ORDER_MESSAGE	m_CommonOrder;

    static atomic<CQuantQueue*> pinstance;
    static mutex m_;
////////////////////////////////////
//// No Need for static...it's a singleton class.... just in case we changed the singleton status
    static u_int64_t i64Idx_SystemEvent		;
    static u_int64_t i64Idx_StockDirectory		;
    static u_int64_t i64Idx_StockTradingAction	;
    static u_int64_t i64Idx_RegSho			;
    static u_int64_t i64Idx_MpPosition		;
    static u_int64_t i64Idx_MWCBDLM 		;
    static u_int64_t i64Idx_MWCBDBM			;
    static u_int64_t i64Idx_IPOQutationUpdate	;
    static u_int64_t i64Idx_AddOrderNoMPID		;
    static u_int64_t i64Idx_AddOrderMPID      	;
    static u_int64_t i64Idx_OrderExecuted     	;
    static u_int64_t i64Idx_OrderExecutedWithPrice 	;
    static u_int64_t i64Idx_OrderCancel		;
    static u_int64_t i64Idx_OrderDelete       	;
    static u_int64_t i64Idx_OrderReplace      	;
    static u_int64_t i64Idx_TradeNonCross     	;
    static u_int64_t i64Idx_NOII 			;
    static u_int64_t i64Idx_RPPI         		;
    SSTAT 	m_sStat;

////////////////////////////////////

}; // end class declaration


/* This is how to call
	CQuantQueue* pQuantQ1 = CQuantQueue::Instance();
	CQuantQueue* pQuantQ2 = CQuantQueue::Instance();
*/


#pragma pack(pop,old_pack)

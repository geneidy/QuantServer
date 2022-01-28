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

//#include "OrdersMap.h"

#define MESSAGE_TYPE_SYSTEM_EVENT 0
#define MESSAGE_TYPE_STOCK_DIRECTORY 1
#define MESSAGE_TYPE_STOCK_TRADING_ACTION 2
#define MESSAGE_TYPE_REG_SHO_RESTRICTION 3
#define MESSAGE_TYPE_MP_POSITION 4
#define MESSAGE_TYPE_MWCBDLM 5
#define MESSAGE_TYPE_MWCBDBM 6
#define MESSAGE_TYPE_IPO_QUOTATION_PERIOD_UPDATE 7
#define MESSAGE_TYPE_ADD_ORDER_NO_MPID 8
#define MESSAGE_TYPE_ADD_ORDER_MPID 9
#define MESSAGE_TYPE_ORDER_EXECUTED 10
#define MESSAGE_TYPE_ORDER_EXECUTED_WITH_PRICE 11
#define MESSAGE_TYPE_ORDER_CANCEL 12
#define MESSAGE_TYPE_ORDER_DELETE 13
#define MESSAGE_TYPE_ORDER_REPLACE 14
#define MESSAGE_TYPE_TRADE_NON_CROSS 15
#define MESSAGE_TYPE_NOII 16
#define MESSAGE_TYPE_RPII 17

#define POSITION_TOP 0
#define POSTITON_CURRENT 1

#define E_READER_EQ_WRITER 100
#define E_INVALID_MESSAGE_TYPE 200

#pragma pack(push, old_pack, 1)

typedef struct
{
    u_int64_t uiEnqQSystemEvent;
    u_int64_t uiEnqQStockDirectory;
    u_int64_t uiEnqQStockTradingAction;
    u_int64_t uiEnqQRegSho;
    u_int64_t uiEnqQMpPosition;
    u_int64_t uiEnqQMWCBDLM;
    u_int64_t uiEnqQMWCBDBM;
    u_int64_t uiEnqQIPOQutationUpdate;
    u_int64_t uiEnqQAddOrderNoMPID;
    u_int64_t uiEnqQAddOrderMPID;
    u_int64_t uiEnqQOrderExecuted;
    u_int64_t uiEnqQOrderExecutedWithPrice;
    u_int64_t uiEnqQOrderCancel;
    u_int64_t uiEnqQOrderDelete;
    u_int64_t uiEnqQOrderReplace;
    u_int64_t uiEnqQTradeNonCross;
    u_int64_t uiEnqQNOII;
    u_int64_t uiEnqQRPPI;
    u_int64_t uiEnqQUnknown;

    u_int64_t uiDeqQSystemEvent;
    u_int64_t uiDeqQStockDirectory;
    u_int64_t uiDeqQStockTradingAction;
    u_int64_t uiDeqQRegSho;
    u_int64_t uiDeqQMpPosition;
    u_int64_t uiDeqQMWCBDLM;
    u_int64_t uiDeqQMWCBDBM;
    u_int64_t uiDeqQIPOQutationUpdate;
    u_int64_t uiDeqQAddOrderNoMPID;
    u_int64_t uiDeqQAddOrderMPID;
    u_int64_t uiDeqQOrderExecuted;
    u_int64_t uiDeqQOrderExecutedWithPrice;
    u_int64_t uiDeqQOrderCancel;
    u_int64_t uiDeqQOrderDelete;
    u_int64_t uiDeqQOrderReplace;
    u_int64_t uiDeqQTradeNonCross;
    u_int64_t uiDeqQNOII;
    u_int64_t uiDeqQRPPI;
    u_int64_t uiDeqQUnknown;

    u_int64_t uiQWrap;
    u_int64_t uiReadWrap;
} QSTAT;

typedef struct _SControl // move it to server includes
{
    int iOpCode;
    int iParam[10];
} CONTROL;

typedef struct _AMGQueue
{
    u_int64_t i64NodeNumber;
    int iMessagetype; // to determine how to cast the void pointer
    time_t dwTickCount;
    long lFeedTime;
    struct timeval lTime;
    int FeedID; // in case multiple feeds are carried in the Queue
    ITCH_MESSAGES_UNION QMessage;
} QUANT_QUEUE, *LPQUANTQUEUE;

typedef struct _AMGQueueMessage
{
    void *SpMessage; // decode according to message type
    int iMessageType;
} AMGQUEUEMESSAGE, *LPQUANTQUEUEMESSAGE;

class CQuantQueue
{
private:
    LPQUANTQUEUE m_pHead;
    LPQUANTQUEUE m_pWriter;
    LPQUANTQUEUE m_pReader;
    int m_iErrorCode;
    u_int64_t m_i64QueueSize;
    u_int64_t m_i64QueueElement;

    CQuantQueue(const CQuantQueue &other); // copy consructor
    CQuantQueue &operator=(const CQuantQueue &other);
    bool operator==(const CQuantQueue &other) const;
    int iPosition;

    static __thread u_int64_t m_Threadi64LastIndex;
    static __thread u_int64_t m_Threadi64LastRead;
    static u_int64_t m_i64LastRead;

    u_int64_t m_i64LastWrite;
    static __thread int m_ThreadiStatus;

public:
    void ListQStats();
    static bool bConstructed;

    virtual ~CQuantQueue(void);

    void *Dequeue(int *iMessagetype);
    int Enqueue(ITCH_MESSAGES_UNION *pQueueElement, int iMessageType);

    void InitReader(int iPosition);
    int QueueEfficiency();

    u_int64_t m_i64FreeElements;
    u_int64_t m_ulTotalElements;
    // return total elements
    u_int64_t GetTotalElements(void);
    u_int64_t GetTotalFreeElements(void);

    int GetErrorCode();
    u_int64_t GetNumberQueueElements(void);

    //pointer to arrays that will hold the elements, pointed to by Queue nodes...
    SYSTEM_EVENT_MESSAGE *m_SystemEvent;
    STOCK_DIRECTORY_MESSAGE *m_StockDirectory;
    STOCK_TRADING_ACTION_MESSAGE *m_StockTradingAction;
    REG_SHO_RESTRICTION_MESSAGE *m_RegSho;
    MP_POSITION_MESSAGE *m_MpPosition;
    MWCBDLM_MESSAGE *m_MWCBDLM;
    MWCBDBM_MESSAGE *m_MWCBDBM;
    IPO_QUOTATION_PERIOD_UPDATE_MESSAGE *m_IPOQutationUpdate;
    ADD_ORDER_NO_MPID_MESSAGE *m_AddOrderNoMPID;
    ADD_ORDER_MPID_MESSAGE *m_AddOrderMPID;
    ORDER_EXECUTED_MESSAGE *m_OrderExecuted;
    ORDER_EXECUTED_WITH_PRICE_MESSAGE *m_OrderExecutedWithPrice;
    ORDER_CANCEL_MESSAGE *m_OrderCancel;
    ORDER_DELETE_MESSAGE *m_OrderDelete;
    ORDER_REPLACE_MESSAGE *m_OrderReplace;
    TRADE_NON_CROSS_MESSAGE *m_TradeNonCross;
    NOII_MESSAGE *m_NOII;
    RPII_MESSAGE *m_RPPI;
    LPQUANTQUEUE m_pPrev;

    static CQuantQueue *Instance();

private:
    //	  void init(SETTINGS sSettings);
    void init();
    CQuantQueue(); // private consructor will call the init() method to initialize the Queue
                   //    unsigned int				m_ulWrap;

    COMMON_ORDER_MESSAGE m_CommonOrder;

    QSTAT m_QStat;

    static atomic<CQuantQueue *> pinstance;
    static mutex m_;

}; // end class declaration

#pragma pack(pop, old_pack)

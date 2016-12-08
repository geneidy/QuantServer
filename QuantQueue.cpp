
#include "QuantQueue.h"
// Does NOT need to be static in this version of the Queue ..... Revise
u_int64_t  CQuantQueue::i64Idx_SystemEvent		= 0;
u_int64_t  CQuantQueue::i64Idx_StockDirectory		= 0;
u_int64_t  CQuantQueue::i64Idx_StockTradingAction	= 0;
u_int64_t  CQuantQueue::i64Idx_RegSho			= 0;
u_int64_t  CQuantQueue::i64Idx_MpPosition		= 0;
u_int64_t  CQuantQueue::i64Idx_MWCBDLM 		 	= 0;
u_int64_t  CQuantQueue::i64Idx_MWCBDBM			= 0;
u_int64_t  CQuantQueue::i64Idx_IPOQutationUpdate	= 0;
u_int64_t  CQuantQueue::i64Idx_AddOrderNoMPID		= 0;
u_int64_t  CQuantQueue::i64Idx_AddOrderMPID      	= 0;
u_int64_t  CQuantQueue::i64Idx_OrderExecuted     	= 0;
u_int64_t  CQuantQueue::i64Idx_OrderExecutedWithPrice 	= 0;
u_int64_t  CQuantQueue::i64Idx_OrderCancel		= 0;
u_int64_t  CQuantQueue::i64Idx_OrderDelete       	= 0;
u_int64_t  CQuantQueue::i64Idx_OrderReplace      	= 0;
u_int64_t  CQuantQueue::i64Idx_TradeNonCross     	= 0;
u_int64_t  CQuantQueue::i64Idx_NOII 			= 0;
u_int64_t  CQuantQueue::i64Idx_RPPI         		= 0;


__thread u_int64_t CQuantQueue::m_Threadi64LastNode = 0;
__thread u_int64_t CQuantQueue::m_Threadi64LastIndex = 0;
__thread int CQuantQueue::m_ThreadiStatus = 0;

bool CQuantQueue::m_EnqueueCriticalSection = false;   	// Critical sections
bool CQuantQueue::m_DeQueueCriticalSection = false;	// Critical sections
bool CQuantQueue::bConstructed = false;		// Critical sections

atomic<CQuantQueue*> CQuantQueue::pinstance { nullptr };
std::mutex CQuantQueue::m_;


//#include "Settings.h"
CQuantQueue* CQuantQueue::Instance() {
    if(pinstance == nullptr) {
        lock_guard<mutex> lock(m_);
        if(pinstance == nullptr) {
            pinstance = new CQuantQueue();
        }
    }
    return pinstance;
}
////////////////////////////////////////////////////////////////////////////////
CQuantQueue::CQuantQueue()
{
    init();
}
////////////////////////////////////////////////////////////////////////////////
CQuantQueue::CQuantQueue(const CQuantQueue& other)
{

}
////////////////////////////////////////////////////////////////////////////////
CQuantQueue& CQuantQueue::operator=(const CQuantQueue& other)
{
    return *this;  // for now
}
////////////////////////////////////////////////////////////////////////////////
bool CQuantQueue::operator==(const CQuantQueue& other) const
{
    return true;  // for now
}
//////////////////////////////////////////////////////////////////
void CQuantQueue::init()
{
//  Logger::instance().log("initialized the Queue", Logger::kLogLevelDebug);
    time_t ltime = 0;
    time( &ltime );

    m_i64FreeElements = 0;
    m_ulTotalElements = 0;

    m_i64FreeElements = theApp.SSettings.uiQueueSize;
    m_i64QueueSize = theApp.SSettings.uiQueueSize;

    m_pHead = NULL;
    m_pHead	= new QUANT_QUEUE[ m_i64QueueSize ];

    if (!m_pHead)
    {
        //log error
        return;
    }
    memset(m_pHead, '\0', sizeof(QUANT_QUEUE)* m_i64QueueSize);

    for (u_int64_t ii = 0; ii < m_i64QueueSize; ii++)
    {
        // message for tracing
        // m_pHead[ii].lTime'    Gett the time from aboe
        gettimeofday(&m_pHead[ii].lTime, NULL);
        m_pHead[ii].i64NodeNumber	= ii;
    }

    m_pWriter	= m_pHead;
    m_pReader = m_pWriter;

    // ::TODO Initialize critical sections here
    m_iErrorCode = ERR_CLEAR;

    m_SystemEvent                  	= new  	SYSTEM_EVENT_MESSAGE  			[ QSIZE_SYSTEM_EVENT ];
    m_StockDirectory               	= new   STOCK_DIRECTORY_MESSAGE  		[ QSIZE_STOCK_DIRECTORY ];
    m_StockTradingAction		= new   STOCK_TRADING_ACTION_MESSAGE  		[ QSIZE_STOCK_TRADING_ACTION ];
    m_RegSho               		= new   REG_SHO_RESTRICTION_MESSAGE 		[ QSIZE_REG_SHO_RESTRICTION ];
    m_MpPosition                	= new   MP_POSITION_MESSAGE  			[ QSIZE_MP_POSITION ];
    m_MWCBDLM                		= new  	MWCBDLM_MESSAGE 			[ QSIZE_MWCBDLM ];
    m_MWCBDBM                		= new   MWCBDBM_MESSAGE 			[ QSIZE_MWCBDBM ];
    m_IPOQutationUpdate             	= new   IPO_QUOTATION_PERIOD_UPDATE_MESSAGE	[ QSIZE_IPO_QUOTATION_PERIOD_UPDATE ];
    m_AddOrderNoMPID                	= new   ADD_ORDER_NO_MPID_MESSAGE 		[ QSIZE_ADD_ORDER_NO_MPID ];
    m_AddOrderMPID                	= new   ADD_ORDER_MPID_MESSAGE 			[ QSIZE_ADD_ORDER_MPID ];
    m_OrderExecuted                	= new   ORDER_EXECUTED_MESSAGE 			[ QSIZE_ORDER_EXECUTED ];
    m_OrderExecutedWithPrice        	= new   ORDER_EXECUTED_WITH_PRICE_MESSAGE 	[ QSIZE_ORDER_EXECUTED_WITH_PRICE  ];
    m_OrderCancel                	= new   ORDER_CANCEL_MESSAGE 			[ QSIZE_ORDER_CANCEL ];
    m_OrderDelete                	= new   ORDER_DELETE_MESSAGE 			[ QSIZE_ORDER_DELETE ];
    m_OrderReplace                	= new   ORDER_REPLACE_MESSAGE 			[ QSIZE_ORDER_REPLACE ];
    m_TradeNonCross                	= new   TRADE_NON_CROSS_MESSAGE 		[ QSIZE_TRADE_NON_CROSS ];
    m_NOII                		= new   NOII_MESSAGE 				[ QSIZE_NOII ];
    m_RPPI                		= new   RPII_MESSAGE 				[ QSIZE_RPII ];
    if	((!m_SystemEvent )		||
            (!m_StockDirectory)		||
            (!m_StockTradingAction)	||
            (!m_RegSho)			||
            (!m_MpPosition)         	||
            (!m_MWCBDLM )		||
            (!m_MWCBDBM)		||
            (!m_IPOQutationUpdate)	||
            (!m_AddOrderNoMPID)		||
            (!m_AddOrderMPID)       	||
            (!m_OrderExecuted )     	||
            (!m_OrderExecutedWithPrice) ||
            (!m_OrderCancel )      	||
            (! m_OrderDelete)       	||
            (!m_OrderReplace)      	||
            (!m_TradeNonCross)     	||
            (!m_NOII)         		||
            (!m_RPPI))
    {
        m_ThreadiStatus = ERR_QUEUE_ALLOCATION;
        //log
        return;  // it is going to return anyway...just formalties
    }

    memset(m_SystemEvent, '\0', sizeof(SYSTEM_EVENT_MESSAGE )*QSIZE_SYSTEM_EVENT  );
    memset(m_StockDirectory , '\0', sizeof(STOCK_DIRECTORY_MESSAGE )*QSIZE_STOCK_DIRECTORY  );
    memset(m_StockTradingAction , '\0', sizeof(STOCK_TRADING_ACTION_MESSAGE )* QSIZE_STOCK_TRADING_ACTION );
    memset(m_RegSho , '\0', sizeof(REG_SHO_RESTRICTION_MESSAGE )* QSIZE_REG_SHO_RESTRICTION );
    memset(m_MpPosition , '\0', sizeof(MP_POSITION_MESSAGE )* QSIZE_MP_POSITION );
    memset(m_MWCBDLM , '\0', sizeof(MWCBDLM_MESSAGE )* QSIZE_MWCBDLM );
    memset(m_MWCBDBM , '\0', sizeof(MWCBDBM_MESSAGE )* QSIZE_MWCBDBM );
    memset(m_IPOQutationUpdate , '\0', sizeof(IPO_QUOTATION_PERIOD_UPDATE_MESSAGE )* QSIZE_IPO_QUOTATION_PERIOD_UPDATE );
    memset(m_AddOrderNoMPID , '\0', sizeof(ADD_ORDER_NO_MPID_MESSAGE )* QSIZE_ADD_ORDER_NO_MPID );
    memset(m_AddOrderMPID , '\0', sizeof(ADD_ORDER_MPID_MESSAGE )* QSIZE_ADD_ORDER_MPID );
    memset(m_OrderExecuted , '\0', sizeof(ORDER_EXECUTED_MESSAGE )* QSIZE_ORDER_EXECUTED );
    memset(m_OrderExecutedWithPrice , '\0', sizeof(ORDER_EXECUTED_WITH_PRICE_MESSAGE )* QSIZE_ORDER_EXECUTED_WITH_PRICE );
    memset(m_OrderCancel , '\0', sizeof(ORDER_CANCEL_MESSAGE )* QSIZE_ORDER_CANCEL );
    memset(m_OrderDelete , '\0', sizeof(ORDER_DELETE_MESSAGE)* QSIZE_ORDER_DELETE );
    memset(m_OrderReplace , '\0', sizeof(ORDER_REPLACE_MESSAGE )* QSIZE_ORDER_REPLACE );
    memset(m_TradeNonCross , '\0', sizeof(TRADE_NON_CROSS_MESSAGE )* QSIZE_TRADE_NON_CROSS  );
    memset(m_NOII , '\0', sizeof(NOII_MESSAGE )*  QSIZE_NOII);
    memset(m_RPPI , '\0', sizeof(RPII_MESSAGE )* QSIZE_RPII );

    m_ulWrap = 0;
    bConstructed = true;
    // log
}
////////////////////////////////////////////////////////////////////////////
int CQuantQueue::GetErrorCode()
{
    return m_ThreadiStatus;
}
////////////////////////////////////////////////////////////////////////////
// A MUTEX HAS TO BE UTILIZED BEFORE ANY CALL
////////////////////////////////////////////////////////////////////////////
void CQuantQueue::InitReader(int iPosition)
{   // one writer....multiple readers

    if ((iPosition != POSITION_TOP) && (iPosition != POSTITON_CURRENT))
        iPosition = POSITION_TOP;

    if (iPosition == POSITION_TOP) {
        if (m_ulWrap == 0) {  // did not wrap around yet....first run of the queue...poin to top of the array
//    m_pReader = &m_pWriter[0];
            m_Threadi64LastIndex = 0;       // Index Zero
        } //
        else { // point to the last written element...possible bug...the last written was the last element in the Queue array
//    m_pReader = &m_pWriter[ m_i64QueueElement + 1 ];  // has to be fast enough or will get run over by the writer
            m_Threadi64LastIndex =  m_i64QueueElement + 1; 	// redundant
        }
    } //   if (iPosition == POSITION_TOP)

    if (iPosition == POSTITON_CURRENT) {
        if (m_i64QueueElement > 0) {
//    m_pReader = &m_pWriter[ m_i64QueueElement - 1 ];-

            m_Threadi64LastIndex =  m_i64QueueElement -1 ;	// redundant
        }
        else { // m_i64QueueElement = 0  ...not started yet
//    m_pReader = &m_pWriter[0 ];
            m_Threadi64LastIndex = 0;
        }
    }// if (iPosition == POSTITON_CURRENT)
    m_pReader = m_pWriter;
}
////////////////////////////////////////////////////////////////////////////
// A MUTEX HAS "NOT" TO BE UTILIZED BEFORE ANY CALL....or better yet....check on the last node number compared to the last you got
////////////////////////////////////////////////////////////////////////////
void* CQuantQueue::Dequeue( /*out*/int *iMessgaType)  // pass the same pReader initialized in the InitReader Method
{   // according to message type.....cast the returned void* to the appropriate message
    // If it returns a NULL ... Get error code and act accordingly

    if (m_Threadi64LastIndex >=  m_i64QueueElement)
        m_Threadi64LastIndex = 0;

    // Reader catches up with Writer....can't advance any more...
    if (m_pReader[m_Threadi64LastIndex].i64NodeNumber == m_pWriter->i64NodeNumber)
    {
        m_ThreadiStatus = E_READER_EQ_WRITER;  // Calling thread should sleep
        *iMessgaType = E_READER_EQ_WRITER;
        return NULL; // Get error code and act accordingly
    }

    iMessgaType = &m_pReader[m_Threadi64LastIndex].iMessagetype;
    switch (m_pReader->iMessagetype)
    {
    case 'S':
        return  &m_SystemEvent[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;  // formality
    case 'R' :
        return  &m_StockDirectory[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'H' :
        return  &m_StockTradingAction[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'Y':
        return  &m_RegSho[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'L':
        return  &m_MpPosition[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'V':
        return  &m_MWCBDLM[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'W':
        return  &m_MWCBDBM[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'K':
        return  &m_IPOQutationUpdate[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'A':
        return  &m_AddOrderNoMPID[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'F':
        return  &m_AddOrderMPID[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'E':
        return  &m_OrderExecuted[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'c':
        return  &m_OrderExecutedWithPrice[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'X':
        return  &m_OrderCancel[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'D':
        return  &m_OrderDelete[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'U':
        return  &m_OrderReplace[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'P':
        return  &m_TradeNonCross[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'I':
        return  &m_NOII[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'N':
        return  &m_RPPI[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    default:
        m_ThreadiStatus = E_INVALID_MESSAGE_TYPE;
        *iMessgaType = E_READER_EQ_WRITER;
        return NULL;
        break;
    }
    return NULL;// the caller has to sleep, or the queue will be running crazy
}
////////////////////////////////////////////////////////////////////////////
int CQuantQueue::Enqueue(ITCH_MESSAGES_UNION* pQueueElement, int iMessageType)
{

//	EnterCriticalSection(&m_EnqueueCriticalSection);

    switch (iMessageType)
    {
    case 'S' :
        if (i64Idx_SystemEvent >= QSIZE_SYSTEM_EVENT) {
            i64Idx_SystemEvent = 0;
            memset(m_SystemEvent, '\0', sizeof(SYSTEM_EVENT_MESSAGE )*QSIZE_SYSTEM_EVENT  );
        }
        m_SystemEvent[ i64Idx_SystemEvent] = pQueueElement->SystemEvent;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_SystemEvent++;
        break;
    case 'R' :
        if (i64Idx_StockDirectory >= QSIZE_STOCK_DIRECTORY) {
            i64Idx_StockDirectory = 0;
            memset(m_StockDirectory , '\0', sizeof(STOCK_DIRECTORY_MESSAGE )*QSIZE_STOCK_DIRECTORY  );
        }
        m_StockDirectory[ i64Idx_StockDirectory] = pQueueElement->StockDirectory;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_StockDirectory++;
        break;

    case 'H' :
        if (i64Idx_StockTradingAction >= QSIZE_STOCK_TRADING_ACTION ) {
            i64Idx_StockTradingAction = 0;
            memset(m_StockTradingAction , '\0', sizeof(STOCK_TRADING_ACTION_MESSAGE )* QSIZE_STOCK_TRADING_ACTION );
        }
        m_StockTradingAction[ i64Idx_StockTradingAction] = pQueueElement->StockTradingAction;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_StockTradingAction++;
        break;

    case 'Y':
        if (i64Idx_RegSho >= QSIZE_REG_SHO_RESTRICTION) {
            i64Idx_RegSho = 0;
            memset(m_RegSho , '\0', sizeof(REG_SHO_RESTRICTION_MESSAGE )* QSIZE_REG_SHO_RESTRICTION );
        }
        m_RegSho[ i64Idx_RegSho] = pQueueElement->RegSho;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_RegSho++;
        break;

    case 'L':
        if (i64Idx_MpPosition >= QSIZE_MP_POSITION) {
            i64Idx_MpPosition = 0;
            memset(m_MpPosition , '\0', sizeof(MP_POSITION_MESSAGE )* QSIZE_MP_POSITION );
        }
        m_MpPosition[ i64Idx_MpPosition] = pQueueElement->MpPosition;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_MpPosition++;
        break;

    case 'V':
        if (i64Idx_MWCBDLM >= QSIZE_MWCBDLM) {
            i64Idx_MWCBDLM = 0;
            memset(m_MWCBDLM , '\0', sizeof(MWCBDLM_MESSAGE )* QSIZE_MWCBDLM );
        }
        m_MWCBDLM[ i64Idx_MWCBDLM] = pQueueElement->MWCBDLM;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_MWCBDLM++;
        break;

    case 'W':
        if (i64Idx_MWCBDBM >= QSIZE_MWCBDBM) {
            i64Idx_MWCBDBM = 0;
            memset(m_MWCBDBM , '\0', sizeof(MWCBDBM_MESSAGE )* QSIZE_MWCBDBM );
        }
        m_MWCBDBM[ i64Idx_MWCBDBM] = pQueueElement->MWCBDBM;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_MWCBDBM++;
        break;

    case 'K':
        if (i64Idx_IPOQutationUpdate >= QSIZE_IPO_QUOTATION_PERIOD_UPDATE) {
            i64Idx_IPOQutationUpdate = 0;
            memset(m_IPOQutationUpdate , '\0', sizeof(IPO_QUOTATION_PERIOD_UPDATE_MESSAGE )* QSIZE_IPO_QUOTATION_PERIOD_UPDATE );
        }
        m_IPOQutationUpdate[ i64Idx_IPOQutationUpdate] = pQueueElement->IPOQutationUpdate;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_IPOQutationUpdate++;
        break;

    case 'A':
        if (i64Idx_AddOrderNoMPID >= QSIZE_ADD_ORDER_NO_MPID) {
            i64Idx_AddOrderNoMPID = 0;
            memset(m_AddOrderNoMPID , '\0', sizeof(ADD_ORDER_NO_MPID_MESSAGE )* QSIZE_ADD_ORDER_NO_MPID );
        }
        m_AddOrderNoMPID[ i64Idx_AddOrderNoMPID] = pQueueElement->AddOrderNoMPID;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_AddOrderNoMPID++;

        break;

    case 'F':
        if (i64Idx_AddOrderMPID >= QSIZE_ADD_ORDER_MPID) {
            i64Idx_AddOrderMPID = 0;
            memset(m_AddOrderMPID , '\0', sizeof(ADD_ORDER_MPID_MESSAGE )* QSIZE_ADD_ORDER_MPID );
        }
        m_AddOrderMPID[ i64Idx_AddOrderMPID] = pQueueElement->AddOrderMPID;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_AddOrderMPID++;

        break;

    case 'E':
        if (i64Idx_OrderExecuted >= QSIZE_ORDER_EXECUTED) {
            i64Idx_OrderExecuted = 0;
            memset(m_OrderExecuted , '\0', sizeof(ORDER_EXECUTED_MESSAGE )* QSIZE_ORDER_EXECUTED );
        }
        m_OrderExecuted[ i64Idx_OrderExecuted] = pQueueElement->OrderExecuted;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_OrderExecuted++;

        break;

    case 'c':
        if (i64Idx_OrderExecutedWithPrice >= QSIZE_ORDER_EXECUTED_WITH_PRICE) {
            i64Idx_OrderExecutedWithPrice = 0;
            memset(m_OrderExecutedWithPrice , '\0', sizeof(ORDER_EXECUTED_WITH_PRICE_MESSAGE )* QSIZE_ORDER_EXECUTED_WITH_PRICE );
        }
        m_OrderExecutedWithPrice[ i64Idx_OrderExecutedWithPrice] = pQueueElement->OrderExecutedWithPrice;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_OrderExecutedWithPrice++;

        break;

    case 'X':
        if (i64Idx_OrderCancel >= QSIZE_ORDER_CANCEL) {
            i64Idx_OrderCancel = 0;
            memset(m_OrderCancel , '\0', sizeof(ORDER_CANCEL_MESSAGE )* QSIZE_ORDER_CANCEL );
        }
        m_OrderCancel[ i64Idx_OrderCancel] = pQueueElement->OrderCancel;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_OrderCancel++;
        break;

    case 'D':
        if (i64Idx_OrderDelete >= QSIZE_ORDER_DELETE) {
            i64Idx_OrderDelete = 0;
            memset(m_OrderDelete , '\0', sizeof(ORDER_DELETE_MESSAGE)* QSIZE_ORDER_DELETE );
        }
        m_OrderDelete[ i64Idx_OrderDelete] = pQueueElement->OrderDelete;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_OrderDelete++;
        break;

    case 'U':
        if (i64Idx_OrderReplace >= QSIZE_ORDER_REPLACE) {
            i64Idx_OrderReplace = 0;
            memset(m_OrderReplace , '\0', sizeof(ORDER_REPLACE_MESSAGE )* QSIZE_ORDER_REPLACE );
        }
        m_OrderReplace[ i64Idx_OrderReplace] = pQueueElement->OrderReplace;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_OrderReplace++;
        break;

    case 'P':
        if (i64Idx_TradeNonCross >= QSIZE_TRADE_NON_CROSS) {
            i64Idx_TradeNonCross = 0;
            memset(m_TradeNonCross , '\0', sizeof(TRADE_NON_CROSS_MESSAGE )* QSIZE_TRADE_NON_CROSS  );
        }
        m_TradeNonCross[ i64Idx_TradeNonCross] = pQueueElement->TradeNonCross;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_TradeNonCross++;
        break;

    case 'I':
        if (i64Idx_NOII >= QSIZE_NOII) {
            i64Idx_NOII = 0;
            memset(m_NOII , '\0', sizeof(NOII_MESSAGE )*  QSIZE_NOII);
        }
        m_NOII[ i64Idx_NOII] = pQueueElement->NOII;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_NOII++;
        break;

    case 'N':
        if (i64Idx_RPPI >= QSIZE_RPII) {
            i64Idx_RPPI = 0;
            memset(m_RPPI , '\0', sizeof(RPII_MESSAGE )* QSIZE_RPII );
        }
        m_RPPI[ i64Idx_RPPI] = pQueueElement->RPPI;
        m_pWriter[ m_i64QueueElement ].i64Index = i64Idx_RPPI++;
        break;
    default:
        return 0;  // unknown message type
    }
    gettimeofday(&m_pWriter[m_i64QueueElement].lTime, NULL);
    m_Threadi64LastNode = m_pWriter[m_i64QueueElement].i64NodeNumber;   // Always keep track of the last node number

    m_pWriter[m_i64QueueElement].i64NodeNumber++; // never reset
    m_pWriter[m_i64QueueElement].iMessagetype = iMessageType;

    m_i64QueueElement++;	// advance to the next location to write


    if (m_i64QueueElement >= m_i64QueueSize)
    {
        m_i64QueueElement = 0;   // over right...start from the begining
        m_i64FreeElements = m_i64QueueSize;  // wrap around
        m_ulWrap++;
    }
    else
    {
        m_i64FreeElements--;
    }

//	LeaveCriticalSection(&m_EnqueueCriticalSection);
    return 0;
}
////////////////////////////////////////////////////////////////////////////
// return total elements
u_int64_t CQuantQueue::GetTotalElements(void)
{
    return m_ulTotalElements;
}
////////////////////////////////////////////////////////////////////////////
u_int64_t CQuantQueue::GetTotalFreeElements(void)
{
    return m_i64FreeElements;
}
////////////////////////////////////////////////////////////////////////////
int CQuantQueue::QueueEfficiency(void)
{
    return  (int)(m_i64FreeElements/m_i64QueueSize)*100;
}
//////////////////////////////////////////////////////////////////////////////
u_int64_t CQuantQueue::GetNumberQueueElements(void)
{
    return m_i64QueueElement;
}
///////////////////////////////////////////////////////////////////////////////////////
CQuantQueue::~CQuantQueue()
{
//	Logger::instance().log("Start Destructing the Queue", Logger::kLogLevelDebug);

    if (m_pHead != NULL)
    {
        delete[] m_pHead;
        m_pHead = NULL;
    }
    if (m_SystemEvent != NULL )
    {
        delete [] m_SystemEvent;
        m_SystemEvent = NULL;
    }

    if (m_StockDirectory != NULL)
    {
        delete [] m_StockDirectory;
        m_StockDirectory = NULL;
    }

    if (m_StockTradingAction != NULL)
    {
        delete [] m_StockTradingAction;
        m_StockTradingAction = NULL;
    }

    if (m_RegSho != NULL)
    {
        delete [] m_RegSho;
        m_RegSho = NULL;
    }

    if (m_MpPosition != NULL)
    {
        delete [] m_MpPosition;
        m_MpPosition = NULL;
    }

    if (m_MWCBDLM != NULL)
    {
        delete [] m_MWCBDLM;
        m_MWCBDLM = NULL;
    }
    if (m_MWCBDBM != NULL)
    {
        delete [] m_MWCBDBM;
        m_MWCBDBM = NULL;
    }
    if (m_IPOQutationUpdate != NULL)
    {
        delete [] m_IPOQutationUpdate;
        m_IPOQutationUpdate = NULL;
    }
    if (m_AddOrderNoMPID != NULL)
    {
        delete [] m_AddOrderNoMPID;
        m_AddOrderNoMPID = NULL;
    }
    if (m_AddOrderMPID != NULL)
    {
        delete [] m_AddOrderMPID;
        m_AddOrderMPID = NULL;
    }
    if (m_OrderExecuted != NULL)
    {
        delete [] m_OrderExecuted;
        m_OrderExecuted = NULL;
    }
    if (m_OrderExecutedWithPrice != NULL)
    {
        delete [] m_OrderExecutedWithPrice;
        m_OrderExecutedWithPrice = NULL;
    }
    if (m_OrderCancel != NULL)
    {
        delete [] m_OrderCancel;
        m_OrderCancel = NULL;
    }
    if (m_OrderDelete != NULL)
    {
        delete [] m_OrderDelete;
        m_OrderDelete = NULL;
    }
    if (m_OrderReplace != NULL)
    {
        delete [] m_OrderReplace;
        m_OrderReplace = NULL;
    }
    if (m_TradeNonCross != NULL)
    {
        delete [] m_TradeNonCross;
        m_TradeNonCross = NULL;
    }
    if (m_NOII != NULL)
    {
        delete [] m_NOII;
        m_NOII = NULL;
    }
    if (m_RPPI != NULL)
    {
        delete [] m_RPPI;
        m_RPPI = NULL;
    }
//	Logger::instance().log("End Destructed the Queue", Logger::kLogLevelDebug);
}
///////////////////////////////////////////////////////////////////////////////////////////

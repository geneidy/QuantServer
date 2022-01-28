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
#include "QuantQueue.h"
// Does NOT need to be static in this version of the Queue ..... Revise
u_int64_t CQuantQueue::i64Idx_SystemEvent = 0;
u_int64_t CQuantQueue::i64Idx_StockDirectory = 0;
u_int64_t CQuantQueue::i64Idx_StockTradingAction = 0;
u_int64_t CQuantQueue::i64Idx_RegSho = 0;
u_int64_t CQuantQueue::i64Idx_MpPosition = 0;
u_int64_t CQuantQueue::i64Idx_MWCBDLM = 0;
u_int64_t CQuantQueue::i64Idx_MWCBDBM = 0;
u_int64_t CQuantQueue::i64Idx_IPOQutationUpdate = 0;
u_int64_t CQuantQueue::i64Idx_AddOrderNoMPID = 0;
u_int64_t CQuantQueue::i64Idx_AddOrderMPID = 0;
u_int64_t CQuantQueue::i64Idx_OrderExecuted = 0;
u_int64_t CQuantQueue::i64Idx_OrderExecutedWithPrice = 0;
u_int64_t CQuantQueue::i64Idx_OrderCancel = 0;
u_int64_t CQuantQueue::i64Idx_OrderDelete = 0;
u_int64_t CQuantQueue::i64Idx_OrderReplace = 0;
u_int64_t CQuantQueue::i64Idx_TradeNonCross = 0;
u_int64_t CQuantQueue::i64Idx_NOII = 0;
u_int64_t CQuantQueue::i64Idx_RPPI = 0;

__thread u_int64_t CQuantQueue::m_Threadi64LastNode = 0;
__thread u_int64_t CQuantQueue::m_Threadi64LastIndex = 0;
__thread int CQuantQueue::m_ThreadiStatus = 0;

bool CQuantQueue::bConstructed = false; // Critical sections

atomic<CQuantQueue *> CQuantQueue::pinstance{nullptr};
std::mutex CQuantQueue::m_;

CQuantQueue *CQuantQueue::Instance()
{
    if (pinstance == nullptr)
    {
        lock_guard<mutex> lock(m_);
        if (pinstance == nullptr)
        {
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
CQuantQueue::CQuantQueue(const CQuantQueue &other)
{
}
////////////////////////////////////////////////////////////////////////////////
CQuantQueue &CQuantQueue::operator=(const CQuantQueue &other)
{
    return *this; // for now
}
////////////////////////////////////////////////////////////////////////////////
bool CQuantQueue::operator==(const CQuantQueue &other) const
{
    return true; // for now
}
//////////////////////////////////////////////////////////////////
void CQuantQueue::init()
{
    //  Logger::instance().log("initialized the Queue", Logger::kLogLevelDebug);
    time_t ltime = 0;
    time(&ltime);

    m_i64FreeElements = 0;
    m_ulTotalElements = 0;

    m_i64QueueElement = 0;

    m_i64FreeElements = theApp.SSettings.uiQueueSize;
    m_i64QueueSize = theApp.SSettings.uiQueueSize;

    m_pHead = NULL;
    m_pHead = new QUANT_QUEUE[m_i64QueueSize];

    if (!m_pHead)
    {
        //log error
        return;
    }
    memset(m_pHead, '\0', sizeof(QUANT_QUEUE) * m_i64QueueSize);

    for (u_int64_t ii = 0; ii < m_i64QueueSize; ii++)
    {
        // message for tracing
        // m_pHead[ii].lTime'    Gett the time from aboe
        gettimeofday(&m_pHead[ii].lTime, NULL);
        m_pHead[ii].i64NodeNumber = ii;
    }

    m_pWriter = m_pHead;
    m_pReader = m_pWriter;

    // ::TODO Initialize critical sections here
    m_iErrorCode = ERR_CLEAR;

    m_SystemEvent = new SYSTEM_EVENT_MESSAGE[QSIZE_SYSTEM_EVENT];
    m_StockDirectory = new STOCK_DIRECTORY_MESSAGE[QSIZE_STOCK_DIRECTORY];
    m_StockTradingAction = new STOCK_TRADING_ACTION_MESSAGE[QSIZE_STOCK_TRADING_ACTION];
    m_RegSho = new REG_SHO_RESTRICTION_MESSAGE[QSIZE_REG_SHO_RESTRICTION];
    m_MpPosition = new MP_POSITION_MESSAGE[QSIZE_MP_POSITION];
    m_MWCBDLM = new MWCBDLM_MESSAGE[QSIZE_MWCBDLM];
    m_MWCBDBM = new MWCBDBM_MESSAGE[QSIZE_MWCBDBM];
    m_IPOQutationUpdate = new IPO_QUOTATION_PERIOD_UPDATE_MESSAGE[QSIZE_IPO_QUOTATION_PERIOD_UPDATE];
    m_AddOrderNoMPID = new ADD_ORDER_NO_MPID_MESSAGE[QSIZE_ADD_ORDER_NO_MPID];
    m_AddOrderMPID = new ADD_ORDER_MPID_MESSAGE[QSIZE_ADD_ORDER_MPID];
    m_OrderExecuted = new ORDER_EXECUTED_MESSAGE[QSIZE_ORDER_EXECUTED];
    m_OrderExecutedWithPrice = new ORDER_EXECUTED_WITH_PRICE_MESSAGE[QSIZE_ORDER_EXECUTED_WITH_PRICE];
    m_OrderCancel = new ORDER_CANCEL_MESSAGE[QSIZE_ORDER_CANCEL];
    m_OrderDelete = new ORDER_DELETE_MESSAGE[QSIZE_ORDER_DELETE];
    m_OrderReplace = new ORDER_REPLACE_MESSAGE[QSIZE_ORDER_REPLACE];
    m_TradeNonCross = new TRADE_NON_CROSS_MESSAGE[QSIZE_TRADE_NON_CROSS];
    m_NOII = new NOII_MESSAGE[QSIZE_NOII];
    m_RPPI = new RPII_MESSAGE[QSIZE_RPII];
    if ((!m_SystemEvent) ||
        (!m_StockDirectory) ||
        (!m_StockTradingAction) ||
        (!m_RegSho) ||
        (!m_MpPosition) ||
        (!m_MWCBDLM) ||
        (!m_MWCBDBM) ||
        (!m_IPOQutationUpdate) ||
        (!m_AddOrderNoMPID) ||
        (!m_AddOrderMPID) ||
        (!m_OrderExecuted) ||
        (!m_OrderExecutedWithPrice) ||
        (!m_OrderCancel) ||
        (!m_OrderDelete) ||
        (!m_OrderReplace) ||
        (!m_TradeNonCross) ||
        (!m_NOII) ||
        (!m_RPPI))
    {
        m_ThreadiStatus = ERR_QUEUE_ALLOCATION;
        //log
        return; // it is going to return anyway...just formalties
    }

    memset(m_SystemEvent, '\0', sizeof(SYSTEM_EVENT_MESSAGE) * QSIZE_SYSTEM_EVENT);
    memset(m_StockDirectory, '\0', sizeof(STOCK_DIRECTORY_MESSAGE) * QSIZE_STOCK_DIRECTORY);
    memset(m_StockTradingAction, '\0', sizeof(STOCK_TRADING_ACTION_MESSAGE) * QSIZE_STOCK_TRADING_ACTION);
    memset(m_RegSho, '\0', sizeof(REG_SHO_RESTRICTION_MESSAGE) * QSIZE_REG_SHO_RESTRICTION);
    memset(m_MpPosition, '\0', sizeof(MP_POSITION_MESSAGE) * QSIZE_MP_POSITION);
    memset(m_MWCBDLM, '\0', sizeof(MWCBDLM_MESSAGE) * QSIZE_MWCBDLM);
    memset(m_MWCBDBM, '\0', sizeof(MWCBDBM_MESSAGE) * QSIZE_MWCBDBM);
    memset(m_IPOQutationUpdate, '\0', sizeof(IPO_QUOTATION_PERIOD_UPDATE_MESSAGE) * QSIZE_IPO_QUOTATION_PERIOD_UPDATE);
    memset(m_AddOrderNoMPID, '\0', sizeof(ADD_ORDER_NO_MPID_MESSAGE) * QSIZE_ADD_ORDER_NO_MPID);
    memset(m_AddOrderMPID, '\0', sizeof(ADD_ORDER_MPID_MESSAGE) * QSIZE_ADD_ORDER_MPID);
    memset(m_OrderExecuted, '\0', sizeof(ORDER_EXECUTED_MESSAGE) * QSIZE_ORDER_EXECUTED);
    memset(m_OrderExecutedWithPrice, '\0', sizeof(ORDER_EXECUTED_WITH_PRICE_MESSAGE) * QSIZE_ORDER_EXECUTED_WITH_PRICE);
    memset(m_OrderCancel, '\0', sizeof(ORDER_CANCEL_MESSAGE) * QSIZE_ORDER_CANCEL);
    memset(m_OrderDelete, '\0', sizeof(ORDER_DELETE_MESSAGE) * QSIZE_ORDER_DELETE);
    memset(m_OrderReplace, '\0', sizeof(ORDER_REPLACE_MESSAGE) * QSIZE_ORDER_REPLACE);
    memset(m_TradeNonCross, '\0', sizeof(TRADE_NON_CROSS_MESSAGE) * QSIZE_TRADE_NON_CROSS);
    memset(m_NOII, '\0', sizeof(NOII_MESSAGE) * QSIZE_NOII);
    memset(m_RPPI, '\0', sizeof(RPII_MESSAGE) * QSIZE_RPII);

    memset(&m_sStat, 0, sizeof(SSTAT));
    bConstructed = true;

    memset(&m_QStat, 0, sizeof(QSTAT));
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
{ // one writer....multiple readers

    if ((iPosition != POSITION_TOP) && (iPosition != POSTITON_CURRENT))
        iPosition = POSITION_TOP;

    if (iPosition == POSITION_TOP)
    {
        if (m_sStat.uiQWrap == 0)
        { // did not wrap around yet....first run of the queue...point to top of the array
            m_pReader = &m_pWriter[0];
            m_Threadi64LastIndex = 0; // Index Zero
        }                             //
        else
        {                                                 // point to the last written element...possible bug...the last written was the last element in the Queue array
                                                          //    m_pReader = &m_pWriter[ m_i64QueueElement + 1 ];  // has to be fast enough or will get run over by the writer
            m_Threadi64LastIndex = m_i64QueueElement + 1; // redundant
        }
    } //   if (iPosition == POSITION_TOP)

    if (iPosition == POSTITON_CURRENT)
    {
        if (m_i64QueueElement > 0)
        {
            m_pReader = &m_pWriter[m_i64QueueElement - 1];
            m_Threadi64LastIndex = m_i64QueueElement - 1; // redundant
        }
        else
        { // m_i64QueueElement = 0  ...not started yet
            m_pReader = &m_pWriter[0];
            m_Threadi64LastIndex = 0;
        }
    } // if (iPosition == POSTITON_CURRENT)
    m_pReader = m_pWriter;
}
////////////////////////////////////////////////////////////////////////////
// A MUTEX HAS "NOT" TO BE UTILIZED BEFORE ANY CALL....or better yet....check on the last node number compared to the last you got
////////////////////////////////////////////////////////////////////////////
void *CQuantQueue::Dequeue(/*out*/ int *iMessgaType) // pass the same pReader initialized in the InitReader Method
{                                                    // according to message type.....cast the returned void* to the appropriate message
    // If it returns a NULL ... Get error code and act accordingly

    if (m_Threadi64LastIndex >= m_i64QueueElement)
        m_Threadi64LastIndex = 0;

    // Reader catches up with Writer....can't advance any more...
    if (m_pReader[m_Threadi64LastIndex].i64NodeNumber == m_pWriter[m_i64QueueElement].i64NodeNumber)
    {
        m_ThreadiStatus = E_READER_EQ_WRITER; // Calling thread should sleep
        *iMessgaType = E_READER_EQ_WRITER;
        return NULL; // Get error code and act accordingly
    }

    *iMessgaType = m_pReader[m_Threadi64LastIndex].iMessagetype;
    switch (*iMessgaType)
    {
    case 'S':
        m_QStat.uiDeqQSystemEvent++;
        return &m_SystemEvent[m_pReader[m_Threadi64LastIndex++].i64Index];
        break; // formality
    case 'R':
        m_QStat.uiDeqQStockDirectory++;
        return &m_StockDirectory[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'H':
        m_QStat.uiDeqQStockTradingAction++;
        return &m_StockTradingAction[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'Y':
        m_QStat.uiDeqQRegSho++;
        return &m_RegSho[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'L':
        m_QStat.uiDeqQMpPosition++;
        return &m_MpPosition[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'V':
        m_QStat.uiDeqQMWCBDLM++;
        return &m_MWCBDLM[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'W':
        m_QStat.uiDeqQMWCBDBM++;
        return &m_MWCBDBM[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'K':
        m_QStat.uiDeqQIPOQutationUpdate++;
        return &m_IPOQutationUpdate[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'A':
        m_QStat.uiDeqQAddOrderNoMPID++;
        return &m_AddOrderNoMPID[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'F':
        m_QStat.uiDeqQAddOrderMPID++;
        return &m_AddOrderMPID[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'E':
        m_QStat.uiDeqQOrderExecuted++;
        return &m_OrderExecuted[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'c':
        m_QStat.uiDeqQOrderExecutedWithPrice++;
        return &m_OrderExecutedWithPrice[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'X':
        m_QStat.uiDeqQOrderCancel++;
        return &m_OrderCancel[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'D':
        m_QStat.uiDeqQOrderDelete++;
        return &m_OrderDelete[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'U':
        m_QStat.uiDeqQOrderReplace++;
        return &m_OrderReplace[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'P':
        m_QStat.uiDeqQTradeNonCross++;
        return &m_TradeNonCross[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'I':
        m_QStat.uiDeqQNOII++;
        return &m_NOII[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    case 'N':
        m_QStat.uiDeqQRPPI++;
        return &m_RPPI[m_pReader[m_Threadi64LastIndex++].i64Index];
        break;
    default:
        m_ThreadiStatus = E_INVALID_MESSAGE_TYPE;
        *iMessgaType = E_READER_EQ_WRITER;
        m_QStat.uiEnqQUnknown++;
        return NULL;
        break;
    }
    return NULL; // the caller has to sleep, or the queue will be running crazy
}
////////////////////////////////////////////////////////////////////////////
int CQuantQueue::Enqueue(ITCH_MESSAGES_UNION *pQueueElement, int iMessageType)
{

    //	EnterCriticalSection(&m_EnqueueCriticalSection);

    switch (iMessageType)
    {
    case 'S':
        if (i64Idx_SystemEvent >= QSIZE_SYSTEM_EVENT)
        {
            i64Idx_SystemEvent = 0;
            //            memset(m_SystemEvent, '\0', sizeof(SYSTEM_EVENT_MESSAGE )*QSIZE_SYSTEM_EVENT  );
            m_sStat.uiQWrapSystemEvent++;
        }
        m_SystemEvent[i64Idx_SystemEvent] = pQueueElement->SystemEvent;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_SystemEvent++;
        m_QStat.uiEnqQSystemEvent++;
        break;
    case 'R':
        if (i64Idx_StockDirectory >= QSIZE_STOCK_DIRECTORY)
        {
            i64Idx_StockDirectory = 0;
            //            memset(m_StockDirectory , '\0', sizeof(STOCK_DIRECTORY_MESSAGE )*QSIZE_STOCK_DIRECTORY  );
            m_sStat.uiQWrapStockDirectory++;
        }
        m_StockDirectory[i64Idx_StockDirectory] = pQueueElement->StockDirectory;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_StockDirectory++;
        m_QStat.uiEnqQStockDirectory++;
        break;

    case 'H':
        if (i64Idx_StockTradingAction >= QSIZE_STOCK_TRADING_ACTION)
        {
            i64Idx_StockTradingAction = 0;
            //            memset(m_StockTradingAction , '\0', sizeof(STOCK_TRADING_ACTION_MESSAGE )* QSIZE_STOCK_TRADING_ACTION );
            m_sStat.uiQWrapStockTradingAction++;
        }
        m_StockTradingAction[i64Idx_StockTradingAction] = pQueueElement->StockTradingAction;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_StockTradingAction++;
        m_QStat.uiEnqQStockTradingAction++;
        break;

    case 'Y':
        if (i64Idx_RegSho >= QSIZE_REG_SHO_RESTRICTION)
        {
            i64Idx_RegSho = 0;
            //            memset(m_RegSho , '\0', sizeof(REG_SHO_RESTRICTION_MESSAGE )* QSIZE_REG_SHO_RESTRICTION );
            m_sStat.uiQWrapRegSho++;
        }
        m_RegSho[i64Idx_RegSho] = pQueueElement->RegSho;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_RegSho++;
        m_QStat.uiEnqQRegSho++;
        break;

    case 'L':
        if (i64Idx_MpPosition >= QSIZE_MP_POSITION)
        {
            i64Idx_MpPosition = 0;
            //            memset(m_MpPosition , '\0', sizeof(MP_POSITION_MESSAGE )* QSIZE_MP_POSITION );
            m_sStat.uiQWrapMpPosition++;
        }
        m_MpPosition[i64Idx_MpPosition] = pQueueElement->MpPosition;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_MpPosition++;
        m_QStat.uiEnqQMpPosition++;
        break;

    case 'V':
        if (i64Idx_MWCBDLM >= QSIZE_MWCBDLM)
        {
            i64Idx_MWCBDLM = 0;
            //            memset(m_MWCBDLM , '\0', sizeof(MWCBDLM_MESSAGE )* QSIZE_MWCBDLM );
            m_sStat.uiQWrapMWCBDLM++;
        }
        m_MWCBDLM[i64Idx_MWCBDLM] = pQueueElement->MWCBDLM;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_MWCBDLM++;
        m_QStat.uiEnqQMWCBDLM++;
        break;

    case 'W':
        if (i64Idx_MWCBDBM >= QSIZE_MWCBDBM)
        {
            i64Idx_MWCBDBM = 0;
            //            memset(m_MWCBDBM , '\0', sizeof(MWCBDBM_MESSAGE )* QSIZE_MWCBDBM );
            m_sStat.uiQWrapMWCBDBM++;
        }
        m_MWCBDBM[i64Idx_MWCBDBM] = pQueueElement->MWCBDBM;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_MWCBDBM++;
        m_QStat.uiEnqQMWCBDBM++;
        break;

    case 'K':
        if (i64Idx_IPOQutationUpdate >= QSIZE_IPO_QUOTATION_PERIOD_UPDATE)
        {
            i64Idx_IPOQutationUpdate = 0;
            //            memset(m_IPOQutationUpdate , '\0', sizeof(IPO_QUOTATION_PERIOD_UPDATE_MESSAGE )* QSIZE_IPO_QUOTATION_PERIOD_UPDATE );
            m_sStat.uiQWrapIPOQutationUpdate++;
        }
        m_IPOQutationUpdate[i64Idx_IPOQutationUpdate] = pQueueElement->IPOQutationUpdate;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_IPOQutationUpdate++;
        m_QStat.uiEnqQIPOQutationUpdate++;
        break;

    case 'A':
        if (i64Idx_AddOrderNoMPID >= QSIZE_ADD_ORDER_NO_MPID)
        {
            i64Idx_AddOrderNoMPID = 0;
            //            memset(m_AddOrderNoMPID , '\0', sizeof(ADD_ORDER_NO_MPID_MESSAGE )* QSIZE_ADD_ORDER_NO_MPID );
            m_sStat.uiQWrapAddOrderNoMPID++;
        }
        m_AddOrderNoMPID[i64Idx_AddOrderNoMPID] = pQueueElement->AddOrderNoMPID;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_AddOrderNoMPID++;
        m_QStat.uiEnqQAddOrderNoMPID++;

        break;

    case 'F':
        if (i64Idx_AddOrderMPID >= QSIZE_ADD_ORDER_MPID)
        {
            i64Idx_AddOrderMPID = 0;
            //            memset(m_AddOrderMPID , '\0', sizeof(ADD_ORDER_MPID_MESSAGE )* QSIZE_ADD_ORDER_MPID );
            m_sStat.uiQWrapAddOrderMPID++;
        }
        m_AddOrderMPID[i64Idx_AddOrderMPID] = pQueueElement->AddOrderMPID;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_AddOrderMPID++;
        m_QStat.uiEnqQAddOrderMPID++;

        break;

    case 'E':
        if (i64Idx_OrderExecuted >= QSIZE_ORDER_EXECUTED)
        {
            i64Idx_OrderExecuted = 0;
            //            memset(m_OrderExecuted , '\0', sizeof(ORDER_EXECUTED_MESSAGE )* QSIZE_ORDER_EXECUTED );
            m_sStat.uiQWrapOrderExecuted++;
        }
        m_OrderExecuted[i64Idx_OrderExecuted] = pQueueElement->OrderExecuted;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_OrderExecuted++;
        m_QStat.uiEnqQOrderExecuted++;

        break;

    case 'c':
        if (i64Idx_OrderExecutedWithPrice >= QSIZE_ORDER_EXECUTED_WITH_PRICE)
        {
            i64Idx_OrderExecutedWithPrice = 0;
            //            memset(m_OrderExecutedWithPrice , '\0', sizeof(ORDER_EXECUTED_WITH_PRICE_MESSAGE )* QSIZE_ORDER_EXECUTED_WITH_PRICE );
            m_sStat.uiQWrapOrderExecutedWithPrice++;
        }
        m_OrderExecutedWithPrice[i64Idx_OrderExecutedWithPrice] = pQueueElement->OrderExecutedWithPrice;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_OrderExecutedWithPrice++;
        m_QStat.uiEnqQOrderExecutedWithPrice++;

        break;

    case 'X':
        if (i64Idx_OrderCancel >= QSIZE_ORDER_CANCEL)
        {
            i64Idx_OrderCancel = 0;
            //            memset(m_OrderCancel , '\0', sizeof(ORDER_CANCEL_MESSAGE )* QSIZE_ORDER_CANCEL );
            m_sStat.uiQWrapOrderCancel++;
        }
        m_OrderCancel[i64Idx_OrderCancel] = pQueueElement->OrderCancel;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_OrderCancel++;
        m_QStat.uiEnqQOrderCancel++;

        break;

    case 'D':
        if (i64Idx_OrderDelete >= QSIZE_ORDER_DELETE)
        {
            i64Idx_OrderDelete = 0;
            //            memset(m_OrderDelete , '\0', sizeof(ORDER_DELETE_MESSAGE)* QSIZE_ORDER_DELETE );
            m_sStat.uiQWrapOrderDelete++;
        }
        m_OrderDelete[i64Idx_OrderDelete] = pQueueElement->OrderDelete;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_OrderDelete++;
        m_QStat.uiEnqQOrderDelete++;

        break;

    case 'U':
        if (i64Idx_OrderReplace >= QSIZE_ORDER_REPLACE)
        {
            i64Idx_OrderReplace = 0;
            //            memset(m_OrderReplace , '\0', sizeof(ORDER_REPLACE_MESSAGE )* QSIZE_ORDER_REPLACE );
            m_sStat.uiQWrapOrderReplace++;
        }
        m_OrderReplace[i64Idx_OrderReplace] = pQueueElement->OrderReplace;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_OrderReplace++;
        m_QStat.uiEnqQOrderReplace++;

        break;

    case 'P':
        if (i64Idx_TradeNonCross >= QSIZE_TRADE_NON_CROSS)
        {
            i64Idx_TradeNonCross = 0;
            //            memset(m_TradeNonCross , '\0', sizeof(TRADE_NON_CROSS_MESSAGE )* QSIZE_TRADE_NON_CROSS  );
            m_sStat.uiQWrapTradeNonCross++;
        }
        m_TradeNonCross[i64Idx_TradeNonCross] = pQueueElement->TradeNonCross;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_TradeNonCross++;
        m_QStat.uiEnqQTradeNonCross++;
        break;

    case 'I':
        if (i64Idx_NOII >= QSIZE_NOII)
        {
            i64Idx_NOII = 0;
            //            memset(m_NOII , '\0', sizeof(NOII_MESSAGE )*  QSIZE_NOII);
            m_sStat.uiQWrapNOII++;
        }
        m_NOII[i64Idx_NOII] = pQueueElement->NOII;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_NOII++;
        m_QStat.uiEnqQNOII++;

        break;

    case 'N':
        if (i64Idx_RPPI >= QSIZE_RPII)
        {
            i64Idx_RPPI = 0;
            //            memset(m_RPPI , '\0', sizeof(RPII_MESSAGE )* QSIZE_RPII );
            m_sStat.uiQWrapRPPI++;
        }
        m_RPPI[i64Idx_RPPI] = pQueueElement->RPPI;
        m_pWriter[m_i64QueueElement].i64Index = i64Idx_RPPI++;
        m_QStat.uiEnqQRPPI++;

        break;
    default:
        m_QStat.uiEnqQUnknown++;
        return 0; // unknown message type
    }
    gettimeofday(&m_pWriter[m_i64QueueElement].lTime, NULL);

    m_Threadi64LastNode = m_pWriter[m_i64QueueElement].i64NodeNumber; // Always keep track of the last node number

    m_pWriter[m_i64QueueElement].i64NodeNumber++; // never reset
    m_pWriter[m_i64QueueElement].iMessagetype = iMessageType;

    m_i64QueueElement++; // advance to the next location to write
    m_ulTotalElements++;

    if (m_i64QueueElement >= m_i64QueueSize)
    {
        m_i64QueueElement = 0;              // overwrite...start from the begining
        m_i64FreeElements = m_i64QueueSize; // wrap around
        m_sStat.uiQWrap++;
    }
    else
    {
        m_i64FreeElements--;
    }

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
    return (int)(m_i64FreeElements / m_i64QueueSize) * 100;
}
//////////////////////////////////////////////////////////////////////////////
u_int64_t CQuantQueue::GetNumberQueueElements(void)
{
    return m_i64QueueElement;
}
//////////////////////////////////////////////////////////////////////////////
void CQuantQueue::ListQStats()
{
    string strOut;

    strOut.empty();
    strOut = "System Event Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQSystemEvent);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "System Event Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQSystemEvent);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Add Order Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQAddOrderNoMPID);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Add Order  Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQAddOrderNoMPID);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Add Order MPID Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQAddOrderMPID);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Add Order MPID Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQAddOrderMPID);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Replace Order Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQOrderReplace);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Replace Order Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQOrderReplace);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Cancel Order Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQOrderCancel);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Cancel Order Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQOrderCancel);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Delete Order Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQOrderDelete);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Delete Order Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQOrderDelete);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Executed Order Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQOrderExecuted);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Executed Order Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQOrderExecuted);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Executed Order With Price Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQOrderExecutedWithPrice);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Executed Order With Price Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQOrderExecutedWithPrice);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Unknown Enqueue: ";
    strOut += to_string(m_QStat.uiEnqQUnknown);
    Logger::instance().log(strOut, Logger::Info);

    strOut.empty();
    strOut = "Unknown Dequeue: ";
    strOut += to_string(m_QStat.uiDeqQUnknown);
    Logger::instance().log(strOut, Logger::Info);
}
///////////////////////////////////////////////////////////////////////////////////////
CQuantQueue::~CQuantQueue()
{

    Logger::instance().log("Start Destructing the Queue", Logger::Info);

    if (m_pHead != NULL)
    {
        delete[] m_pHead;
        m_pHead = NULL;
    }
    if (m_SystemEvent != NULL)
    {
        delete[] m_SystemEvent;
        m_SystemEvent = NULL;
    }

    if (m_StockDirectory != NULL)
    {
        delete[] m_StockDirectory;
        m_StockDirectory = NULL;
    }

    if (m_StockTradingAction != NULL)
    {
        delete[] m_StockTradingAction;
        m_StockTradingAction = NULL;
    }

    if (m_RegSho != NULL)
    {
        delete[] m_RegSho;
        m_RegSho = NULL;
    }

    if (m_MpPosition != NULL)
    {
        delete[] m_MpPosition;
        m_MpPosition = NULL;
    }

    if (m_MWCBDLM != NULL)
    {
        delete[] m_MWCBDLM;
        m_MWCBDLM = NULL;
    }
    if (m_MWCBDBM != NULL)
    {
        delete[] m_MWCBDBM;
        m_MWCBDBM = NULL;
    }
    if (m_IPOQutationUpdate != NULL)
    {
        delete[] m_IPOQutationUpdate;
        m_IPOQutationUpdate = NULL;
    }
    if (m_AddOrderNoMPID != NULL)
    {
        delete[] m_AddOrderNoMPID;
        m_AddOrderNoMPID = NULL;
    }
    if (m_AddOrderMPID != NULL)
    {
        delete[] m_AddOrderMPID;
        m_AddOrderMPID = NULL;
    }
    if (m_OrderExecuted != NULL)
    {
        delete[] m_OrderExecuted;
        m_OrderExecuted = NULL;
    }
    if (m_OrderExecutedWithPrice != NULL)
    {
        delete[] m_OrderExecutedWithPrice;
        m_OrderExecutedWithPrice = NULL;
    }
    if (m_OrderCancel != NULL)
    {
        delete[] m_OrderCancel;
        m_OrderCancel = NULL;
    }
    if (m_OrderDelete != NULL)
    {
        delete[] m_OrderDelete;
        m_OrderDelete = NULL;
    }
    if (m_OrderReplace != NULL)
    {
        delete[] m_OrderReplace;
        m_OrderReplace = NULL;
    }
    if (m_TradeNonCross != NULL)
    {
        delete[] m_TradeNonCross;
        m_TradeNonCross = NULL;
    }
    if (m_NOII != NULL)
    {
        delete[] m_NOII;
        m_NOII = NULL;
    }
    if (m_RPPI != NULL)
    {
        delete[] m_RPPI;
        m_RPPI = NULL;
    }
    string strLog;

    strLog.clear();
    strLog = "Queue Stats....Total Elements Inserted: ";
    strLog += to_string(m_ulTotalElements);
    Logger::instance().log(strLog, Logger::Info);

    strLog.clear();
    strLog = "Queue Stats....Total Elements Dequeued: ";
    strLog += to_string(m_Threadi64LastIndex);
    Logger::instance().log(strLog, Logger::Info);

    strLog.clear();
    strLog = "Queue Stats....Number of Wrap around: ";
    strLog += to_string(m_sStat.uiQWrap);
    Logger::instance().log(strLog, Logger::Info);

    strLog.clear();
    strLog = "Queue Stats....Number of Added Orders With MPID Wrap around: ";
    strLog += to_string(m_sStat.uiQWrapAddOrderMPID);
    Logger::instance().log(strLog, Logger::Info);

    strLog.clear();
    strLog = "Queue Stats....Number of Added Orders Without MPID Wrap around: ";
    strLog += to_string(m_sStat.uiQWrapAddOrderNoMPID);
    Logger::instance().log(strLog, Logger::Info);

    Logger::instance().log("End Destructed the Queue", Logger::Info);
}
///////////////////////////////////////////////////////////////////////////////////////////

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

__thread u_int64_t CQuantQueue::m_Threadi64LastRead = 0;
u_int64_t CQuantQueue::m_i64LastRead = 0;

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
    m_i64QueueSize = theApp.SSettings.uiQueueSize; // Number of elements in the Queue

    m_pHead = NULL;
    m_pHead = new QUANT_QUEUE[m_i64QueueSize];

    if (!m_pHead)
    {
        //log error
        return;
    }
    /*    memset(m_pHead, '\0', sizeof(QUANT_QUEUE)* m_i64QueueSize);

    for (u_int64_t ii = 0; ii < m_i64QueueSize; ii++)
    {
        // message for tracing
        // m_pHead[ii].lTime'    Gett the time from aboe
        gettimeofday(&m_pHead[ii].lTime, NULL);
        m_pHead[ii].i64NodeNumber	= ii;
    }
*/
    m_pWriter = m_pHead;
    m_pReader = m_pWriter;

    // ::TODO Initialize critical sections here
    m_iErrorCode = ERR_CLEAR;

    bConstructed = true;
    m_i64LastWrite = 0;

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
{   // one writer....multiple readers
    /*
    if ((iPosition != POSITION_TOP) && (iPosition != POSTITON_CURRENT))
        iPosition = POSITION_TOP;

    if (iPosition == POSITION_TOP) {
        if (m_QStat.uiQWrap == 0) {  // did not wrap around yet....first run of the queue...point to top of the array
            m_pReader = &m_pWriter[0];
            m_Threadi64LastIndex = 0;       // Index Zero
        } //
        else { // point to the last written element...possible bug...the last written was the last element in the Queue array
//    m_pReader = &m_pWriter[ m_i64QueueElement + 1 ];  // has to be fast enough or will get run over by the writer
            m_Threadi64LastIndex =  m_i64QueueElement + 1; 	// redundant
        }
    } //   if (iPosition == POSITION_TOP)

    if (iPosition == POSTITON_CURRENT) {
        if (m_i64QueueElement > 0) {
             m_pReader = &m_pWriter[ m_i64QueueElement - 1 ];
            m_Threadi64LastIndex =  m_i64QueueElement -1 ;	// redundant
        }
        else { // m_i64QueueElement = 0  ...not started yet
            m_pReader = &m_pWriter[0 ];
            m_Threadi64LastIndex = 0;
        }
    }// if (iPosition == POSTITON_CURRENT)
//    m_pReader = m_pWriter;
*/
    m_Threadi64LastIndex = 0;
    m_pReader = m_pHead;
}
////////////////////////////////////////////////////////////////////////////
// A MUTEX HAS "NOT" TO BE UTILIZED BEFORE ANY CALL....or better yet....check on the last node number compared to the last you got
////////////////////////////////////////////////////////////////////////////
void *CQuantQueue::Dequeue(/*out*/ int *iMessgaType) // pass the same pReader initialized in the InitReader Method
{                                                    // according to message type.....cast the returned void* to the appropriate message
    // If it returns a NULL ... Get error code and act accordingly

    // Reader catches up with Writer....can't advance any more...
    if (m_Threadi64LastRead >= m_i64LastWrite)
    {
        m_ThreadiStatus = E_READER_EQ_WRITER; // Calling thread should sleep
        *iMessgaType = E_READER_EQ_WRITER;
        return NULL; // Get error code and act accordingly
    }

    if (m_Threadi64LastIndex > m_i64QueueSize)
    { //
        m_Threadi64LastIndex = 0;
        m_QStat.uiReadWrap++;
    }

    m_Threadi64LastRead++; // never reset

    //    m_i64LastRead = m_Threadi64LastRead;

    *iMessgaType = m_pReader[m_Threadi64LastIndex].iMessagetype;

    switch (*iMessgaType)
    {
    case 'S':
        m_QStat.uiDeqQSystemEvent++;
        break; // formality
    case 'R':
        m_QStat.uiDeqQStockDirectory++;
        break;
    case 'H':
        m_QStat.uiDeqQStockTradingAction++;
        break;
    case 'Y':
        m_QStat.uiDeqQRegSho++;
        break;
    case 'L':
        m_QStat.uiDeqQMpPosition++;
        break;
    case 'V':
        m_QStat.uiDeqQMWCBDLM++;
        break;
    case 'W':
        m_QStat.uiDeqQMWCBDBM++;
        break;
    case 'K':
        m_QStat.uiDeqQIPOQutationUpdate++;
        break;
    case 'A':
        m_QStat.uiDeqQAddOrderNoMPID++;
        break;
    case 'F':
        m_QStat.uiDeqQAddOrderMPID++;
        break;
    case 'E':
        m_QStat.uiDeqQOrderExecuted++;
        break;
    case 'c':
        m_QStat.uiDeqQOrderExecutedWithPrice++;
        break;
    case 'X':
        m_QStat.uiDeqQOrderCancel++;
        break;
    case 'D':
        m_QStat.uiDeqQOrderDelete++;
        break;
    case 'U':
        m_QStat.uiDeqQOrderReplace++;
        break;
    case 'P':
        m_QStat.uiDeqQTradeNonCross++;
        break;
    case 'I':
        m_QStat.uiDeqQNOII++;
        break;
    case 'N':
        m_QStat.uiDeqQRPPI++;
        break;
    default:
        m_ThreadiStatus = E_INVALID_MESSAGE_TYPE;
        *iMessgaType = E_READER_EQ_WRITER;
        m_QStat.uiEnqQUnknown++;
        return NULL; // Unknow message
        break;
    }
    m_i64LastRead++;
    return &m_pReader[m_Threadi64LastIndex++].QMessage;
}
////////////////////////////////////////////////////////////////////////////
int CQuantQueue::Enqueue(ITCH_MESSAGES_UNION *pQueueElement, int iMessageType)
{

    gettimeofday(&m_pWriter[m_i64QueueElement].lTime, NULL);

    m_pWriter[m_i64QueueElement].QMessage = *pQueueElement;
    m_pWriter[m_i64QueueElement].i64NodeNumber++; // never reset
    m_pWriter[m_i64QueueElement].iMessagetype = iMessageType;

    m_i64LastWrite++; //never reset

    m_i64QueueElement++; // advance to the next location to write
    m_ulTotalElements++;

    if (m_i64QueueElement >= m_i64QueueSize)
    {
        m_i64QueueElement = 0;              // overwrite...start from the begining
        m_i64FreeElements = m_i64QueueSize; // wrap around
        m_QStat.uiQWrap++;
    }
    else
    {
        m_i64FreeElements--;
    }
    switch (iMessageType)
    {
    case 'S':
        m_QStat.uiEnqQSystemEvent++;
        break; // formality
    case 'R':
        m_QStat.uiEnqQStockDirectory++;
        break;
    case 'H':
        m_QStat.uiEnqQStockTradingAction++;
        break;
    case 'Y':
        m_QStat.uiEnqQRegSho++;
        break;
    case 'L':
        m_QStat.uiEnqQMpPosition++;
        break;
    case 'V':
        m_QStat.uiEnqQMWCBDLM++;
        break;
    case 'W':
        m_QStat.uiEnqQMWCBDBM++;
        break;
    case 'K':
        m_QStat.uiEnqQIPOQutationUpdate++;
        break;
    case 'A':
        m_QStat.uiEnqQAddOrderNoMPID++;
        break;
    case 'F':
        m_QStat.uiEnqQAddOrderMPID++;
        break;
    case 'E':
        m_QStat.uiEnqQOrderExecuted++;
        break;
    case 'c':
        m_QStat.uiEnqQOrderExecutedWithPrice++;
        break;
    case 'X':
        m_QStat.uiEnqQOrderCancel++;
        break;
    case 'D':
        m_QStat.uiEnqQOrderDelete++;
        break;
    case 'U':
        m_QStat.uiEnqQOrderReplace++;
        break;
    case 'P':
        m_QStat.uiEnqQTradeNonCross++;
        break;
    case 'I':
        m_QStat.uiEnqQNOII++;
        break;
    case 'N':
        m_QStat.uiEnqQRPPI++;
        break;
    default:
        m_QStat.uiEnqQUnknown++;
        break;
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

    string strLog;

    strLog.clear();
    strLog = "Queue Stats....Total Elements Queued: ";
    strLog += to_string(m_i64LastWrite); // never
    Logger::instance().log(strLog, Logger::Info);

    strLog.clear();
    strLog = "Queue Stats....Total Elements Dequeued: ";
    strLog += to_string(m_i64LastRead);
    Logger::instance().log(strLog, Logger::Info);

    strLog.clear();
    strLog = "Queue Stats....Number of Write Wrap around: ";
    strLog += to_string(m_QStat.uiQWrap);
    Logger::instance().log(strLog, Logger::Info);

    strLog.clear();
    strLog = "Queue Stats....Number of Read Wrap around: ";
    strLog += to_string(m_QStat.uiReadWrap);
    Logger::instance().log(strLog, Logger::Info);

    Logger::instance().log("End Destructed the Queue", Logger::Info);
}
///////////////////////////////////////////////////////////////////////////////////////////

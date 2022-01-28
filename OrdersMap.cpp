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
#include "OrdersMap.h"

COrdersMap *COrdersMap::pInstance = nullptr;
mutex COrdersMap::MapMutex;

static pthread_mutex_t mtxFindMap = PTHREAD_MUTEX_INITIALIZER;

OrdersUnOrderedMap::iterator COrdersMap::m_itBookRefSymbolMap;
OrdersUnOrderedMap COrdersMap::m_SymbolMap;

OrdersSequenceUnOrderedMap COrdersMap::m_SequenceMap;
OrdersSequenceUnOrderedMap::const_iterator COrdersMap::m_itSequenceMap;

uint COrdersMap::iNInstance = 0;
// uint64_t   COrdersMap::m_ui64NumOfOrders 	= 0;
__thread COMMON_ORDER_MESSAGE *COrdersMap::m_pReturnCommonOrder = nullptr;
__thread COMMON_ORDER_MESSAGE *COrdersMap::m_pTempCommonOrder = nullptr;

COMMON_ORDER_MESSAGE thrSOrder;

__thread uint64_t thrSequence;

////////////////////////////////////////////////////////////////////
COrdersMap *COrdersMap::instance()
{
    lock_guard<mutex> guard(COrdersMap::MapMutex);
    if (pInstance == nullptr)
    {
        pInstance = new COrdersMap();
    }
    iNInstance++;

    return pInstance;
}
//////////////////////////////////////////////////////////////////////////////////
COrdersMap::COrdersMap()
{
    string strMessage;

    m_iError = 0;
    m_iMessage = 0;

    m_SymbolMap.clear();
    m_SequenceMap.clear();

    m_uiSizeOfCommonOrderRecord = sizeof(COMMON_ORDER_MESSAGE);
    m_uiNumberOfMessagesToHold = m_SymbolMap.max_size(); // don't bank on it per documentation
    strMessage = "Orders Map...Max Orders to hold in Map: ";
    strMessage += to_string(m_uiNumberOfMessagesToHold);
    Logger::instance().log(strMessage, Logger::Info);

    /*
            m_Util = nullptr;
            m_Util = new CUtil(theApp.SSettings.szActiveSymbols, theApp.SSettings.arrbActive);
    */
    /*
            struct stat64 st = {0};

            if (stat64("../Orders/", &st) == -1) {
                mkdir("../Orders/", 0700);
            }

            m_SymbolMap.clear();

            string strOrdersFile;
            strOrdersFile.empty();

            strOrdersFile = "../Orders/";
            strOrdersFile += m_Util->GetFormatedDate();
            strOrdersFile += "QuanticksOrders.qtx";

            m_fd = open64(strOrdersFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);

            if (m_fd == -1) {
                Logger::instance().log("Orders Map: Orders open File Mapping Error", Logger::Debug);
                m_iError = 100;
                // Set error code and exit
            }
            if (!m_iError) {
                if (fstat64(m_fd, &m_sb) == -1) {
                    Logger::instance().log("Orders Map: Error fstat", Logger::Debug);
                    m_iError = 110;
                    // Set error code and exit
                }
                else   {
                    if (!InitMemoryMappedFile()) {
                        Logger::instance().log("Orders Map: Error Initializing", Logger::Error);
                        close(m_fd);
                        m_iError = 120;
                        // Set error code and exit
                    }
                    else {
                        m_addr = mmap(nullptr, m_sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);

                        if (m_addr == MAP_FAILED) {
                            Logger::instance().log("Orders Map: Error Mapping Failed", Logger::Error);
                            m_iError = 130;
                            // Set error code and exit
                        }
                        m_pCommonOrder = (COMMON_ORDER_MESSAGE*) m_addr;  //  cast in COMMON_ORDER_MESSAGE...now you have an array in memory of common orders
                        m_pReturnCommonOrder 	= m_pCommonOrder;
                        m_pRefCommonOrder 	= m_pCommonOrder;
                    }
                    m_request.tv_sec = 0;
                    m_request.tv_nsec = 100000000;   // 1/10 of a second
                }
            } //   if (!m_iError) {*/

    m_ui64OrderSequence = 700000000; // seven hundred million
}
//////////////////////////////////////////////////////////////////////////////////
void COrdersMap::InitQueue(CQuantQueue *pQueue)
{
    m_pQuantQueue = pQueue;
    // Init the Queue
    m_pQuantQueue->InitReader(POSITION_TOP);
    Logger::instance().log("Orders Map: Queue initialized in Orders Data Map file", Logger::Info);
}
//////////////////////////////////////////////////////////////////////////////////
COrdersMap::~COrdersMap()
{
    string strMessage;

    strMessage = "Orders Map...Number of Orders Inserted: ";
    strMessage += to_string(m_SymbolMap.size());
    Logger::instance().log(strMessage, Logger::Info);

    strMessage = "Sequence Map...Number of Orders Inserted: ";
    strMessage += to_string(m_SequenceMap.size());
    Logger::instance().log(strMessage, Logger::Info);

    strMessage = "Orders Map...Map (Utilization) Efficiency: ";
    double dEffic = (m_SymbolMap.size() / m_SymbolMap.max_size()) * 100;
    strMessage += to_string(dEffic);
    Logger::instance().log(strMessage, Logger::Info);

    Logger::instance().log("Orders Map: Start...Clearing Orders Map", Logger::Info);
    m_SymbolMap.clear();
    m_SequenceMap.clear();
    Logger::instance().log("Orders Map: End...Clearing Orders Map", Logger::Info);

    pthread_mutex_destroy(&mtxFindMap);

    if (m_Util)
    {
        delete m_Util;
        m_Util = nullptr;
    }
}
//////////////////////////////////////////////////////////////////////////////////
int COrdersMap::GetError()
{
    return m_iError;
}
//////////////////////////////////////////////////////////////////////////////////
int COrdersMap::InitMemoryMappedFile()
{ /*
       COMMON_ORDER_MESSAGE SCommonOrder;
       m_uiNumberOfMessagesToHold = 0;

       memset(&SCommonOrder, '\0', m_uiSizeOfCommonOrderRecord );

       m_uiNumberOfMessagesToHold = (theApp.SSettings.ui64SizeOfOrdersMappedFile* 1000000000) / m_uiSizeOfCommonOrderRecord;

       if (m_sb.st_size < (theApp.SSettings.ui64SizeOfOrdersMappedFile* 1000000000)) { // Fresh file
           Logger::instance().log("Orders Map: Initializing Mapped File...This will take few minutes", Logger::Debug);
           for (uint64_t ii = 0; ii < (m_uiNumberOfMessagesToHold + 1); ii++) {
               write(m_fd, &SCommonOrder, m_uiSizeOfCommonOrderRecord);  // init with nullptr
           }
           Logger::instance().log("Orders Map: Finished Initializing Orders Mapped File", Logger::Debug);
       }

       fstat64(m_fd, &m_sb);
       if (m_sb.st_size < (theApp.SSettings.ui64SizeOfOrdersMappedFile* 1000000000)) { // Fresh file
           Logger::instance().log("Orders Map: Error Initializing Mapped File", Logger::Debug);
           m_iError = 200; // enum later
           return false;
       }
       m_iError = 0; // enum later
    */
    return true;
}
//////////////////////////////////////////////////////////////////////////////////
SOrdersDataStat COrdersMap::GetOrdersDataStat() // Total Trade records inserted
{

    m_SOrdersDataStat.uiNumberOfMessagesToHold = m_uiNumberOfMessagesToHold;
    m_SOrdersDataStat.ui64NumOfOrders = m_ui64NumOfOrders;
    m_SOrdersDataStat.SymbolMapSize = m_SymbolMap.size();
    m_SOrdersDataStat.SymbolMapMaxSize = m_SymbolMap.max_size();

    return m_SOrdersDataStat;
}
//////////////////////////////////////////////////////////////////////////////////
COMMON_ORDER_MESSAGE *COrdersMap::GetMemoryMappedOrder(uint64_t thrSequence)
{
    pthread_mutex_lock(&mtxFindMap);
    if (thrSequence >= m_ui64NumOfOrders)
    { // not yet inserted
        pthread_mutex_unlock(&mtxFindMap);
        return nullptr;
    }

    m_itSequenceMap = m_SequenceMap.find(thrSequence);
    if (m_itSequenceMap == m_SequenceMap.end())
    {
        pthread_mutex_unlock(&mtxFindMap);
        return nullptr;
    }

    m_itBookRefSymbolMap = m_SymbolMap.find(m_itSequenceMap->second);

    if (m_itBookRefSymbolMap == m_SymbolMap.end())
    {
        pthread_mutex_unlock(&mtxFindMap);
        return nullptr;
    }

    pthread_mutex_unlock(&mtxFindMap);
    return (&m_itBookRefSymbolMap->second);
}
//////////////////////////////////////////////////////////////////////////////////
COMMON_ORDER_MESSAGE *COrdersMap::GetMappedOrder(uint64_t uiOrderRefNumber) // called internally....only checks the Orders Map (m_SymbolMap)
{

    pthread_mutex_lock(&mtxFindMap);
    memset(&thrSOrder, '\0', sizeof(COMMON_ORDER_MESSAGE)); // change thread local storage to regular

    m_itAuxSymbolMap = m_SymbolMap.find(uiOrderRefNumber);

    if (m_itAuxSymbolMap == m_SymbolMap.end())
    {
        pthread_mutex_unlock(&mtxFindMap);
        return nullptr;
    }
    else
    {
        thrSOrder = m_itAuxSymbolMap->second;
        pthread_mutex_unlock(&mtxFindMap);
        return &thrSOrder;
    }
    return nullptr; // unreachable code
}
//////////////////////////////////////////////////////////////////////////////////
uint64_t COrdersMap::FillMemoryMap(ITCH_MESSAGES_UNION *pItchMessageUnion, int m_iMessage)
{
    string strMessage;
    /*
             pItchMessageUnion = (ITCH_MESSAGES_UNION*) m_pQuantQueue->Dequeue(&m_iMessage);   //  from the old system

     	clock_gettime(CLOCK_REALTIME, &tspec);
             m_ui64OrderTime = (tspec.tv_sec * 1000000000) + tspec.tv_nsec;
    */
    if (pItchMessageUnion == nullptr)
    {
        nanosleep(&m_request, nullptr); // sleep a 1/10 of a second
        // return m_ui64NumOfOrders;
    }

    if (m_ui64NumOfOrders > m_uiNumberOfMessagesToHold)
    {
        Logger::instance().log("Orders Map: Orders Map Exceeded Number of Orders to be Inserted in the Memory Mapped File...Inserted: ", Logger::Error);
        Logger::instance().log(to_string(m_ui64NumOfOrders), Logger::Error);
        m_iError = 300; // Enum later
        return m_ui64NumOfOrders;
    }

    RetPair.second = true; // not all the switch statements cases do an insert....
    memset(&m_CommonOrder, '\0', m_uiSizeOfCommonOrderRecord);

    switch (m_iMessage)
    {
    case 'A': // Add Order NO MPID
        m_CommonOrder.cBuySell = pItchMessageUnion->AddOrderNoMPID.cBuySell;
        m_CommonOrder.cMessageType = pItchMessageUnion->AddOrderNoMPID.cMessageType;
        m_CommonOrder.dPrice = pItchMessageUnion->AddOrderNoMPID.dPrice;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->AddOrderNoMPID.iOrderRefNumber;
        m_CommonOrder.iShares = pItchMessageUnion->AddOrderNoMPID.iShares;
        m_CommonOrder.iTimeStamp = pItchMessageUnion->AddOrderNoMPID.iTimeStamp;
        // No attribution...will attrib it to Nasdaq
        strcpy(m_CommonOrder.szMPID, "NSDQ");
        strcpy(m_CommonOrder.szStock, pItchMessageUnion->AddOrderNoMPID.szStock);

        pthread_mutex_lock(&mtxFindMap);
        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_CommonOrder.iOrderRefNumber, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_CommonOrder.iOrderRefNumber)); // Will cause an error
        pthread_mutex_unlock(&mtxFindMap);

        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;

            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map: Error inserting in 'Map' in Orders Mapped File (Add Order No MPID) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
        }

        break;

    case 'F': // Add Order with MPID
        m_CommonOrder.cBuySell = pItchMessageUnion->AddOrderMPID.cBuySell;
        m_CommonOrder.cMessageType = pItchMessageUnion->AddOrderMPID.cMessageType;
        m_CommonOrder.dPrice = pItchMessageUnion->AddOrderMPID.dPrice;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->AddOrderMPID.iOrderRefNumber;
        m_CommonOrder.iShares = pItchMessageUnion->AddOrderMPID.iShares;
        m_CommonOrder.iTimeStamp = pItchMessageUnion->AddOrderMPID.iTimeStamp;

        strcpy(m_CommonOrder.szMPID, pItchMessageUnion->AddOrderMPID.szMPID);
        strcpy(m_CommonOrder.szStock, pItchMessageUnion->AddOrderMPID.szStock);

        pthread_mutex_lock(&mtxFindMap);
        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_CommonOrder.iOrderRefNumber, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_CommonOrder.iOrderRefNumber));

        pthread_mutex_unlock(&mtxFindMap);
        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;

            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map: Error inserting in 'Map' in Orders Mapped File (Add Order With MPID) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
        }

        break;

    case 'U': // Replace Order
        m_pTempCommonOrder = nullptr;
        m_pTempCommonOrder = GetMappedOrder(pItchMessageUnion->OrderReplace.iOldOrderRefNumber);
        if (!m_pTempCommonOrder)
            break;

        strcpy(m_CommonOrder.szStock, m_pTempCommonOrder->szStock);
        strcpy(m_CommonOrder.szMPID, m_pTempCommonOrder->szMPID);

        m_CommonOrder.cBuySell = m_pTempCommonOrder->cBuySell;
        m_CommonOrder.iPrevShares = m_pTempCommonOrder->iShares;
        m_CommonOrder.dPrevPrice = m_pTempCommonOrder->dPrice;

        // Add the new one
        m_CommonOrder.cMessageType = pItchMessageUnion->OrderReplace.cMessageType;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->OrderReplace.iNewOrderRefNumber;
        m_CommonOrder.iPrevOrderRefNumber = pItchMessageUnion->OrderReplace.iOldOrderRefNumber;
        m_CommonOrder.iShares = pItchMessageUnion->OrderReplace.iShares; // new Qty
        m_CommonOrder.dPrice = pItchMessageUnion->OrderReplace.dPrice;   // new Price
        m_CommonOrder.iTimeStamp = pItchMessageUnion->OrderReplace.iTimeStamp;
        m_CommonOrder.TrackingNumber = pItchMessageUnion->OrderReplace.TrackingNumber;

        pthread_mutex_lock(&mtxFindMap);

        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_CommonOrder.iOrderRefNumber, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_CommonOrder.iOrderRefNumber)); // Will cause an error

        pthread_mutex_unlock(&mtxFindMap);
        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;

            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map: Error inserting in 'Map' in Orders Mapped File (Replace Order) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
        }

        break;

    case 'E': // Executed Order  // Tick Data
        m_pTempCommonOrder = nullptr;
        m_pTempCommonOrder = GetMappedOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
        if (!m_pTempCommonOrder)
            break;

        strcpy(m_CommonOrder.szStock, m_pTempCommonOrder->szStock);
        strcpy(m_CommonOrder.szMPID, m_pTempCommonOrder->szMPID);
        m_CommonOrder.dPrice = m_pTempCommonOrder->dPrice;
        m_CommonOrder.cBuySell = m_pTempCommonOrder->cBuySell;

        m_CommonOrder.cMessageType = pItchMessageUnion->OrderExecuted.cMessageType;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->OrderExecuted.iOrderRefNumber;
        // Hack....put the order match number in Prev Order
        m_CommonOrder.iPrevOrderRefNumber = pItchMessageUnion->OrderExecuted.iOrderMatchNumber;
        m_CommonOrder.iShares = pItchMessageUnion->OrderExecuted.iShares;
        m_CommonOrder.iTimeStamp = pItchMessageUnion->OrderExecuted.iTimeStamp;
        m_CommonOrder.TrackingNumber = pItchMessageUnion->OrderExecuted.TrackingNumber;

        pthread_mutex_lock(&mtxFindMap);

        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_ui64OrderSequence, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_ui64OrderSequence)); // Will cause an error
        pthread_mutex_unlock(&mtxFindMap);
        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;
            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map...Error inserting in 'Map' in Orders Mapped File (Executed Order) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
            m_ui64OrderSequence++;
        }
        break;

    case 'c': // Executed with price   // Tick Data
        m_pTempCommonOrder = nullptr;
        m_pTempCommonOrder = GetMappedOrder(pItchMessageUnion->OrderExecutedWithPrice.iOrderRefNumber);
        if (!m_pTempCommonOrder)
            break;

        strcpy(m_CommonOrder.szStock, m_pTempCommonOrder->szStock);
        strcpy(m_CommonOrder.szMPID, m_pTempCommonOrder->szMPID);
        m_CommonOrder.cBuySell = m_pTempCommonOrder->cBuySell;
        //	    m_CommonOrder.dPrice =  m_pTempCommonOrder->dPrice;

        m_CommonOrder.cMessageType = pItchMessageUnion->OrderExecutedWithPrice.cMessageType;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->OrderExecutedWithPrice.iOrderRefNumber;
        // Hack....put the order match number in Prev Order
        m_CommonOrder.iPrevOrderRefNumber = pItchMessageUnion->OrderExecutedWithPrice.iOrderMatchNumber; // Order Match number is Unique...try use as an index in the Sequence map
        m_CommonOrder.iShares = pItchMessageUnion->OrderExecutedWithPrice.iShares;
        m_CommonOrder.dPrice = pItchMessageUnion->OrderExecutedWithPrice.dExecutionPrice;
        m_CommonOrder.iTimeStamp = pItchMessageUnion->OrderExecutedWithPrice.iTimeStamp;
        m_CommonOrder.TrackingNumber = pItchMessageUnion->OrderExecutedWithPrice.TrackingNumber;

        pthread_mutex_lock(&mtxFindMap);
        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_ui64OrderSequence, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_ui64OrderSequence)); // Will cause an error
        pthread_mutex_unlock(&mtxFindMap);
        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;

            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map: Error inserting in 'Map' in Orders Mapped File (Executed with price) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
            m_ui64OrderSequence++;
        }
        break;

    case 'X': // Cancel Order
        m_pTempCommonOrder = nullptr;
        m_pTempCommonOrder = GetMappedOrder(pItchMessageUnion->OrderCancel.iOrderRefNumber);
        if (!m_pTempCommonOrder)
            break;

        strcpy(m_CommonOrder.szStock, m_pTempCommonOrder->szStock);
        strcpy(m_CommonOrder.szMPID, m_pTempCommonOrder->szMPID);

        m_CommonOrder.cBuySell = m_pTempCommonOrder->cBuySell;
        m_CommonOrder.dPrice = m_pTempCommonOrder->dPrice;

        m_CommonOrder.cMessageType = pItchMessageUnion->OrderCancel.cMessageType;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->OrderCancel.iOrderRefNumber;

        //Canceled Shares	The number of shares being removed from the display size of the order as the result of a cancellation.
        m_CommonOrder.iShares = pItchMessageUnion->OrderCancel.iShares;

        m_CommonOrder.iTimeStamp = pItchMessageUnion->OrderCancel.iTimeStamp;
        m_CommonOrder.TrackingNumber = pItchMessageUnion->OrderCancel.TrackingNumber;

        pthread_mutex_lock(&mtxFindMap);
        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_ui64OrderSequence, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_ui64OrderSequence));
        pthread_mutex_unlock(&mtxFindMap);

        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;

            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map...Error inserting in 'Map' in Orders Mapped File (Cancel Order) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
            m_ui64OrderSequence++;
        }

        break;

    case 'D': // Delete Order
        m_pTempCommonOrder = nullptr;
        m_pTempCommonOrder = GetMappedOrder(pItchMessageUnion->OrderDelete.iOrderRefNumber);
        if (!m_pTempCommonOrder)
            break;

        strcpy(m_CommonOrder.szStock, m_pTempCommonOrder->szStock);
        strcpy(m_CommonOrder.szMPID, m_pTempCommonOrder->szMPID);

        m_CommonOrder.iShares = m_pTempCommonOrder->iShares;
        m_CommonOrder.cBuySell = m_pTempCommonOrder->cBuySell;
        m_CommonOrder.dPrice = m_pTempCommonOrder->dPrice;

        m_CommonOrder.cMessageType = pItchMessageUnion->OrderDelete.cMessageType;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->OrderDelete.iOrderRefNumber;
        m_CommonOrder.iTimeStamp = pItchMessageUnion->OrderDelete.iTimeStamp;
        m_CommonOrder.TrackingNumber = pItchMessageUnion->OrderDelete.TrackingNumber;

        pthread_mutex_lock(&mtxFindMap);

        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_ui64OrderSequence, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_ui64OrderSequence)); // Will cause an error
        pthread_mutex_unlock(&mtxFindMap);
        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;

            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map...Error inserting in 'Map' in Orders Mapped File (Deleted Order) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
            m_ui64OrderSequence++;
        }
    case 'P': // Trade non-cross
        m_pTempCommonOrder = nullptr;
        m_pTempCommonOrder = GetMappedOrder(pItchMessageUnion->TradeNonCross.iOrderRefNumber);
        if (!m_pTempCommonOrder)
            break;

        strcpy(m_CommonOrder.szStock, m_pTempCommonOrder->szStock);
        strcpy(m_CommonOrder.szMPID, m_pTempCommonOrder->szMPID);

        m_CommonOrder.iShares = m_pTempCommonOrder->iShares;
        m_CommonOrder.cBuySell = m_pTempCommonOrder->cBuySell;
        m_CommonOrder.dPrice = m_pTempCommonOrder->dPrice;

        m_CommonOrder.cMessageType = pItchMessageUnion->TradeNonCross.cMessageType;
        m_CommonOrder.iOrderRefNumber = pItchMessageUnion->TradeNonCross.iOrderRefNumber;
        m_CommonOrder.iTimeStamp = pItchMessageUnion->TradeNonCross.iTimeStamp;
        m_CommonOrder.TrackingNumber = pItchMessageUnion->TradeNonCross.TrackingNumber;

        pthread_mutex_lock(&mtxFindMap);

        RetPair = m_SymbolMap.insert(pair<uint64_t, COMMON_ORDER_MESSAGE>(m_ui64OrderSequence, m_CommonOrder));
        RetPair1 = m_SequenceMap.insert(pair<uint64_t, uint64_t>(m_ui64NumOfOrders, m_ui64OrderSequence)); // Will cause an error
        pthread_mutex_unlock(&mtxFindMap);
        if ((!RetPair.second) || (!RetPair1.second))
        {
            strMessage.clear();
            strMessage = "Order Ref Number: ";
            strMessage += to_string(m_CommonOrder.iOrderRefNumber);
            strMessage += " MPID: ";
            strMessage += m_CommonOrder.szMPID;
            strMessage += " Symbol: ";
            strMessage += m_CommonOrder.szStock;

            Logger::instance().log(strMessage, Logger::Error);
            Logger::instance().log("Orders Map...Error inserting in 'Map' in Orders Mapped File (Deleted Order) ", Logger::Error);
            break;
        }
        else
        {
            m_ui64NumOfOrders++;
            m_ui64OrderSequence++;
        }

        break;

    default:
        break;
    };

    return m_ui64NumOfOrders;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

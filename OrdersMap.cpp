#include "OrdersMap.h"

//using namespace std;

COrdersMap* COrdersMap::pInstance = nullptr;
mutex 	COrdersMap::MapMutex;

static pthread_mutex_t mtxFindMap = PTHREAD_MUTEX_INITIALIZER;

uint COrdersMap::iNInstance = 0;
uint64_t   COrdersMap::m_ui64NumOfOrders = 0;
__thread  COMMON_ORDER_MESSAGE* COrdersMap::m_pReturnCommonOrder = NULL;
__thread  COMMON_ORDER_MESSAGE* COrdersMap::m_pTempCommonOrder = NULL;

OrdersUnOrderedMap COrdersMap::m_SymbolMap;

////////////////////////////////////////////////////////////////////
COrdersMap* COrdersMap::instance()
{

    lock_guard<mutex> guard(COrdersMap::MapMutex);
    if (pInstance == nullptr) {
        pInstance = new COrdersMap();
    }
    iNInstance++;

    return pInstance;
}
//////////////////////////////////////////////////////////////////////////////////
COrdersMap::COrdersMap()
{
    m_iError = 0;
    m_iMessage = 0;

    m_uiSizeOfCommonOrderRecord = sizeof(COMMON_ORDER_MESSAGE);

    m_Util = NULL;
    m_Util = new CUtil();

    /*
     *struct stat st = {0};

    if (stat("/some/directory", &st) == -1) {
        mkdir("../Orders/", 0700);
    }
     *
     */
    m_SymbolMap.clear();

    string strOrdersFile;
    strOrdersFile.empty();

    strOrdersFile = "../Orders/";
    strOrdersFile += m_Util->GetFormatedDate();
    strOrdersFile += "QuanticksOrders.Qtx";

    m_fd = open64(strOrdersFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);

    if (m_fd == -1) {
        Logger::instance().log("Orders open File Mapping Error", Logger::Debug);
        m_iError = 100;
        // Set error code and exit
    }
    if (!m_iError) {
        if (fstat64(m_fd, &m_sb) == -1) {
            Logger::instance().log("Error fstat", Logger::Debug);
            m_iError = 110;
            // Set error code and exit
        }
        else   {
            if (!InitMemoryMappedFile()) {
                Logger::instance().log("Error Initializing", Logger::Error);
                close(m_fd);
                m_iError = 120;
                // Set error code and exit
            }
            else {
                m_addr = mmap(NULL, m_sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);

                if (m_addr == MAP_FAILED) {
                    Logger::instance().log("Error Mapping Failed", Logger::Error);
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
    } //   if (!m_iError) {

}
//////////////////////////////////////////////////////////////////////////////////
void COrdersMap::InitQueue(CQuantQueue* pQueue)
{
    m_pQuantQueue = pQueue;
    // Init the Queue
    m_pQuantQueue->InitReader(POSITION_TOP);
    Logger::instance().log("Queue initialized in Orders Data Map file", Logger::Info);
}
//////////////////////////////////////////////////////////////////////////////////
COrdersMap::~COrdersMap()
{
    Logger::instance().log("Start...UnMapping Orders Data Map file", Logger::Info);
    msync(m_addr, m_sb.st_size, MS_ASYNC);
    munmap(m_addr, m_sb.st_size);
    Logger::instance().log("End...UnMapping Orders Data Map file", Logger::Info);

    Logger::instance().log("Start...Clearing Orders Map", Logger::Info);
    m_SymbolMap.clear();
    Logger::instance().log("End...Clearing Orders Map", Logger::Info);

   if (m_fd)
      close(m_fd);

   pthread_mutex_destroy(&mtxFindMap);

    if (m_Util) {
        delete m_Util;
        m_Util = NULL;
    }
}
//////////////////////////////////////////////////////////////////////////////////
int COrdersMap::GetError()
{
    return m_iError;
}
//////////////////////////////////////////////////////////////////////////////////
int COrdersMap::InitMemoryMappedFile()
{
    COMMON_ORDER_MESSAGE SCommonOrder;
    m_uiNumberOfMessagesToHold = 0;

    memset(&SCommonOrder, '\0', m_uiSizeOfCommonOrderRecord );

    m_uiNumberOfMessagesToHold = (theApp.SSettings.ui64SizeOfOrdersMappedFile* 1000000000) / m_uiSizeOfCommonOrderRecord;

    if (m_sb.st_size < (theApp.SSettings.ui64SizeOfOrdersMappedFile* 1000000000)) { // Fresh file
        Logger::instance().log("Initializing Mapped File...This will take few minutes", Logger::Debug);
        for (uint64_t ii = 0; ii < (m_uiNumberOfMessagesToHold + 1); ii++) {
            write(m_fd, &SCommonOrder, m_uiSizeOfCommonOrderRecord);  // init with NULL
        }
        Logger::instance().log("Finished Initializing Orders Mapped File", Logger::Debug);
    }

    fstat64(m_fd, &m_sb);
    if (m_sb.st_size < (theApp.SSettings.ui64SizeOfOrdersMappedFile* 1000000000)) { // Fresh file
        Logger::instance().log("Error Initializing Mapped File", Logger::Debug);
        m_iError = 200; // enum later
        return false;
    }
    m_iError = 0; // enum later
    return true;
}
//////////////////////////////////////////////////////////////////////////////////
SOrdersDataStat COrdersMap::GetOrdersDataStat() // Total Trade records inserted
{

    m_SOrdersDataStat.uiNumberOfMessagesToHold 	= m_uiNumberOfMessagesToHold;
    m_SOrdersDataStat.ui64NumOfOrders 		= m_ui64NumOfOrders;
    m_SOrdersDataStat.SymbolMapSize 		= m_SymbolMap.size();
    m_SOrdersDataStat.SymbolMapMaxSize 		= m_SymbolMap.max_size();

    return m_SOrdersDataStat;
}
//////////////////////////////////////////////////////////////////////////////////
COMMON_ORDER_MESSAGE* COrdersMap::GetMemoryMappedOrder(uint64_t ui64OrderIndex)
{

    if ((ui64OrderIndex >= (m_ui64NumOfOrders -1))  || (ui64OrderIndex >= m_uiNumberOfMessagesToHold))
        return NULL;

    return &m_pRefCommonOrder[ui64OrderIndex];

}
//////////////////////////////////////////////////////////////////////////////////
COMMON_ORDER_MESSAGE* COrdersMap::GetOrder(uint64_t uiOrderNumber) // called from other threads
{
    pthread_mutex_lock(&mtxFindMap);
    m_itRefAuxSymbolMap =  m_SymbolMap.find(uiOrderNumber);

    if (m_itRefAuxSymbolMap == m_SymbolMap.end()) {
        pthread_mutex_unlock(&mtxFindMap);
        return NULL;
    }
    else {
        uint64_t uiRecordNumber = 0;
        uiRecordNumber = m_itRefAuxSymbolMap->second;
        pthread_mutex_unlock(&mtxFindMap);
        if (m_ui64NumOfOrders < uiRecordNumber)  // found bug here....switched < >
            return &m_pReturnCommonOrder[m_itRefAuxSymbolMap->second];
        else
            return NULL;
    }
}
//////////////////////////////////////////////////////////////////////////////////
COMMON_ORDER_MESSAGE* COrdersMap::GetMappedOrder(uint64_t uiOrderRefNumber)  // called internally
{
    pthread_mutex_lock(&mtxFindMap);
    m_itAuxSymbolMap =  m_SymbolMap.find(uiOrderRefNumber);

    if (m_itAuxSymbolMap == m_SymbolMap.end()) {
        pthread_mutex_unlock(&mtxFindMap);
        return NULL;
    }
    else {
        pthread_mutex_unlock(&mtxFindMap);
        return &m_pReturnCommonOrder[m_itAuxSymbolMap->second];
    }
}
//////////////////////////////////////////////////////////////////////////////////
uint64_t COrdersMap::FillMemoryMappedFile()
{
// Get Messages from Queue....Save the order Messages and disregard others

// ::TODO Check for the size limitation of the Memory Mapped File
    while (theApp.SSettings.iStatus != STOPPED) {
        pItchMessageUnion = (ITCH_MESSAGES_UNION*) m_pQuantQueue->Dequeue(&m_iMessage);
        if (pItchMessageUnion == NULL) {
            nanosleep (&m_request, NULL);  // sleep a 1/10 of a second
            // return m_ui64NumOfOrders;
        }

        if (m_ui64NumOfOrders >= m_uiNumberOfMessagesToHold) {
            Logger::instance().log("Exceeded Number of Orders to be Inserted in the Memory Mapped File ", Logger::Error);
            m_iError = 300; // Enum later
            return m_ui64NumOfOrders;
        }

        RetPair.second = true;  // not all the switch statements cases do an insert....
        memset(&m_pCommonOrder[m_ui64NumOfOrders], '\0', m_uiSizeOfCommonOrderRecord);
        switch (m_iMessage) {
        case 'A':  // Add Order NO MPID
            m_pCommonOrder[m_ui64NumOfOrders].cBuySell 		= pItchMessageUnion->AddOrderNoMPID.cBuySell;
            m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->AddOrderNoMPID.cMessageType;
            m_pCommonOrder[m_ui64NumOfOrders].dPrice 		= pItchMessageUnion->AddOrderNoMPID.dPrice;
            m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->AddOrderNoMPID.iOrderRefNumber;
            m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->AddOrderNoMPID.iShares;
            m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->AddOrderNoMPID.iTimeStamp;
            // No attribution...will attrib it to Nasdaq
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szMPID, "NSDQ");
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, pItchMessageUnion->AddOrderNoMPID.szStock);
/*
            if (!m_Util->CheckInclude(pItchMessageUnion->AddOrderNoMPID.szStock)) // check for Range
                return 0;
*/            //     m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber= pItchMessageUnion->AddOrderNoMPID.TrackingNumber;
            pthread_mutex_lock(&mtxFindMap);
            RetPair = m_SymbolMap.insert(pair<uint64_t , uint64_t>(m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber, m_ui64NumOfOrders) );
            pthread_mutex_unlock(&mtxFindMap);
            if (!RetPair.second) {
                Logger::instance().log("Error inserting in 'Map' in Orders Mapped File (Add Order No MPID) ", Logger::Error);
                break;
            }
            m_ui64NumOfOrders++;

            break;

        case 'F':  // Add Order with MPID
            m_pCommonOrder[m_ui64NumOfOrders].cBuySell 		= pItchMessageUnion->AddOrderMPID.cBuySell;
            m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->AddOrderMPID.cMessageType;
            m_pCommonOrder[m_ui64NumOfOrders].dPrice 		= pItchMessageUnion->AddOrderMPID.dPrice;
            m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->AddOrderMPID.iOrderRefNumber;
            m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->AddOrderMPID.iShares;
            m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->AddOrderMPID.iTimeStamp;

            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szMPID, pItchMessageUnion->AddOrderMPID.szMPID);
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, pItchMessageUnion->AddOrderMPID.szStock);

            // m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber= pItchMessageUnion->AddOrderMPID.TrackingNumber;
/*	    
            if (!m_Util->CheckInclude(pItchMessageUnion->AddOrderMPID.szStock)) // check for Range
                return 0;
*/
            pthread_mutex_lock(&mtxFindMap);
            RetPair = m_SymbolMap.insert(pair<uint64_t , uint64_t>(m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber, m_ui64NumOfOrders) );
            pthread_mutex_unlock(&mtxFindMap);
            if (!RetPair.second) {
                Logger::instance().log("Error inserting in 'Map' in Orders Mapped File (Add Order With MPID) ", Logger::Error);
                break;
            }
            m_ui64NumOfOrders++;
            break;

        case 'U':   // Replace Order ... Delete OldOrderRefNumber  and Add NewOrderRefNumber >>>Copy MPID if exist
            m_pTempCommonOrder = NULL;
            m_pTempCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderReplace.iOldOrderRefNumber);
            if (!m_pTempCommonOrder)
                break;
/*
            if (!m_Util->CheckInclude(m_pTempCommonOrder->szStock)) // check for Range
                return 0;
*/
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pTempCommonOrder->szStock);
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szMPID, m_pTempCommonOrder->szMPID);

            m_pCommonOrder[m_ui64NumOfOrders].cBuySell             	=   m_pTempCommonOrder->cBuySell;
            m_pCommonOrder[m_ui64NumOfOrders].iPrevShares          	=   m_pTempCommonOrder->iShares;
            m_pCommonOrder[m_ui64NumOfOrders].dPrevPrice	       	=   m_pTempCommonOrder->dPrice;

            // Add the new one
            m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderReplace.cMessageType;
            m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber  	= pItchMessageUnion->OrderReplace.iNewOrderRefNumber;
            m_pCommonOrder[m_ui64NumOfOrders].iPrevOrderRefNumber 	= pItchMessageUnion->OrderReplace.iOldOrderRefNumber;
            m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->OrderReplace.iShares;  // new Qty
            m_pCommonOrder[m_ui64NumOfOrders].dPrice		= pItchMessageUnion->OrderReplace.dPrice;   // new Price
            m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderReplace.iTimeStamp;
//        m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderReplace.TrackingNumber;
            // find the old one
            pthread_mutex_lock(&mtxFindMap);
            m_itSymbolMap = m_SymbolMap.find(pItchMessageUnion->OrderReplace.iOldOrderRefNumber);
            if(m_itSymbolMap ==  m_SymbolMap.end()) {
                // Log Error
                pthread_mutex_unlock(&mtxFindMap);
                break;
            }

            // delete the old one
            m_SymbolMap.erase(m_itSymbolMap);
            // Insert new 0rder in Map
            RetPair = m_SymbolMap.insert(pair<uint64_t , uint64_t>(m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber, m_ui64NumOfOrders) );
            pthread_mutex_unlock(&mtxFindMap);

            if (!RetPair.second) {
                Logger::instance().log("Error inserting in 'Map' in Orders Mapped File (Replace Order) ", Logger::Error);
                break;
            }

            // ::TODO log error if any
            m_ui64NumOfOrders++;
            break;

        case 'E':  // Executed Order  // Tick Data  /// Where do I get the execution price from
            m_pTempCommonOrder = NULL;
            m_pTempCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
            if (!m_pTempCommonOrder)
                break;
/*
            if (!m_Util->CheckInclude(m_pTempCommonOrder->szStock)) // check for Range
                return 0;
*/
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pTempCommonOrder->szStock);
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szMPID, m_pTempCommonOrder->szMPID);
            m_pCommonOrder[m_ui64NumOfOrders].dPrice 		=  m_pTempCommonOrder->dPrice;
            m_pCommonOrder[m_ui64NumOfOrders].cBuySell             	=   m_pTempCommonOrder->cBuySell;

            m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderExecuted.cMessageType;
            m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->OrderExecuted.iOrderRefNumber;
            // Hack....put the order match number in Prev Order
            m_pCommonOrder[m_ui64NumOfOrders].iPrevOrderRefNumber  	= pItchMessageUnion->OrderExecuted.iOrderMatchNumber;
            m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->OrderExecuted.iShares;
            m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderExecuted.iTimeStamp;
//        m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderExecuted.TrackingNumber;
            m_ui64NumOfOrders++;

            break;

        case 'c': // Executed with price   // Tick Data
            m_pTempCommonOrder = NULL;
            m_pTempCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderExecutedWithPrice.iOrderRefNumber);
            if (!m_pTempCommonOrder)
                break;
/*
            if (!m_Util->CheckInclude(m_pTempCommonOrder->szStock)) // check for Range
                return 0;
*/	    
/* Moved to FillMsgStructs
            if (pItchMessageUnion->OrderExecutedWithPrice.cPrintable == 'N')
                return 0;
*/

            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pTempCommonOrder->szStock);
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szMPID, m_pTempCommonOrder->szMPID);
            m_pCommonOrder[m_ui64NumOfOrders].cBuySell             	=   m_pTempCommonOrder->cBuySell;
//	m_pCommonOrder[m_ui64NumOfOrders].dPrice =  m_pTempCommonOrder->dPrice;

            m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderExecutedWithPrice.cMessageType;
            m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->OrderExecutedWithPrice.iOrderRefNumber;
            // Hack....put the order match number in Prev Order
            m_pCommonOrder[m_ui64NumOfOrders].iPrevOrderRefNumber  	= pItchMessageUnion->OrderExecutedWithPrice.iOrderMatchNumber;
            m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->OrderExecutedWithPrice.iShares;
            m_pCommonOrder[m_ui64NumOfOrders].dPrice 		= pItchMessageUnion->OrderExecutedWithPrice.dExecutionPrice;
            m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderExecutedWithPrice.iTimeStamp;
//        m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderExecutedWithPrice.TrackingNumber;
            m_ui64NumOfOrders++;
            break;

        case 'X':  // Cancel Order
            m_pTempCommonOrder = NULL;
            m_pTempCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderCancel.iOrderRefNumber);
            if (!m_pTempCommonOrder)
                break;
/*
            if (!m_Util->CheckInclude(m_pTempCommonOrder->szStock)) // check for Range
                return 0;
*/
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pTempCommonOrder->szStock);
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szMPID,  m_pTempCommonOrder->szMPID);

            m_pCommonOrder[m_ui64NumOfOrders].cBuySell             =   m_pTempCommonOrder->cBuySell;
            m_pCommonOrder[m_ui64NumOfOrders].dPrice               =   m_pTempCommonOrder->dPrice;

            m_pCommonOrder[m_ui64NumOfOrders].cMessageType 	   = pItchMessageUnion->OrderCancel.cMessageType;
            m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber  	= pItchMessageUnion->OrderCancel.iOrderRefNumber;

            //Canceled Shares	The number of shares being removed from the display size of the order as the result of a cancellation.
            m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->OrderCancel.iShares;

            m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderCancel.iTimeStamp;
//	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderCancel.TrackingNumber;
            m_ui64NumOfOrders++;

            break;

        case 'D':  // Delete Order
            m_pTempCommonOrder = NULL;
            m_pTempCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderDelete.iOrderRefNumber);
            if (!m_pTempCommonOrder)
                break;
/*
            if (!m_Util->CheckInclude(m_pTempCommonOrder->szStock)) // check for Range
                return 0;
*/
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pTempCommonOrder->szStock);
            strcpy(m_pCommonOrder[m_ui64NumOfOrders].szMPID,  m_pTempCommonOrder->szMPID);

            m_pCommonOrder[m_ui64NumOfOrders].iShares 			=   m_pTempCommonOrder->iShares;
            m_pCommonOrder[m_ui64NumOfOrders].cBuySell             	=   m_pTempCommonOrder->cBuySell;
            m_pCommonOrder[m_ui64NumOfOrders].dPrice               	=   m_pTempCommonOrder->dPrice;

            m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderDelete.cMessageType;
            m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->OrderDelete.iOrderRefNumber;
            m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderDelete.iTimeStamp;
//        m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderDelete.TrackingNumber;
            m_ui64NumOfOrders++;
            break;

        default:
            break;
        };
    };

    return m_ui64NumOfOrders;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OrdersMap.h"
#include "Util.h"

//using namespace std;

COrdersMap* COrdersMap::pInstance = nullptr;
mutex 	COrdersMap::MapMutex;

////////////////////////////////////////////////////////////////////
COrdersMap* COrdersMap::instance()
{
    static Cleanup cleanup;

    lock_guard<mutex> guard(MapMutex);
    if (pInstance == nullptr)
	pInstance = new COrdersMap();
    return pInstance;
}
////////////////////////////////////////////////////////////////////
COrdersMap::Cleanup::~Cleanup()
{
	lock_guard<mutex> guard(COrdersMap::MapMutex);
	delete COrdersMap::pInstance;
	COrdersMap::pInstance = nullptr;
}
//////////////////////////////////////////////////////////////////////////////////
COrdersMap::COrdersMap()
{
    m_iError = 0;
    m_iMessage = 0;
    m_ui64NumOfOrders = 0;
    
    m_uiSizeOfCommonOrderRecord = sizeof(COMMON_ORDER_MESSAGE);
    CUtil  Util;
    
/*
 *struct stat st = {0};

if (stat("/some/directory", &st) == -1) {
    mkdir("/some/directory", 0700);
}
 *
 */    
    string strOrdersFile;
    strOrdersFile.empty();
    
    strOrdersFile = "./Orders/";
    strOrdersFile += Util.GetFormatedDate();
    strOrdersFile += "QuanticksOrders.Qtx";
    
//    m_fd = open64("./Ticks/QuanticksTickData.Qtx", O_RDWR|O_CREAT, S_IRWXU);
    m_fd = open64(strOrdersFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);
    
//    m_fd = open64("./Orders/QuanticksCOrdersMap.Qtx", O_RDWR|O_CREAT, S_IRWXU);
    if (m_fd == -1) {
        Logger::instance().log("open File Mapping Error", Logger::Debug);
        m_iError = 100;
        // Set error code and exit
    }

    if (fstat64(m_fd, &m_sb) == -1) {
        Logger::instance().log("Error fstat", Logger::Debug);
        m_iError = 110;
        // Set error code and exit
    }

    if (!InitMemoryMappedFile()) {
        Logger::instance().log("Error Initializing", Logger::Debug);
        close(m_fd);
        m_iError = 120;
        // Set error code and exit
    }

    if (!m_iError){
	m_addr = mmap(NULL, m_sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);

	if (m_addr == MAP_FAILED) {
	    Logger::instance().log("Error Mapping Failed", Logger::Debug);
	    m_iError = 130;
	    // Set error code and exit
	}
	m_pCommonOrder = (COMMON_ORDER_MESSAGE*) m_addr;  //  cast in COMMON_ORDER_MESSAGE...now you have an array in memory of common orders

    // Init the Queue
	m_pQuantQueue = NULL;
	m_pQuantQueue = CQuantQueue::Instance();   // Only one instance is allowed of this singelton class

	if (!m_pQuantQueue) {
	    Logger::instance().log("Error initialized the Queue", Logger::Debug);
	    m_iError = 140;
	}
	m_pQuantQueue->InitReader(POSITION_TOP);
    }
    m_request.tv_nsec = 100000000;   // 1/10 of a second 
}
//////////////////////////////////////////////////////////////////////////////////
COrdersMap::~COrdersMap()
{
    munmap(m_addr, m_sb.st_size);
    m_SymbolMap.clear();
    close(m_fd);
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
        Logger::instance().log("Initializing Mapped File", Logger::Debug);
        for (uint64_t ii = 0; ii < m_uiNumberOfMessagesToHold; ii++) {
            write(m_fd, &SCommonOrder, m_uiSizeOfCommonOrderRecord);  // init with NULL
	}
	Logger::instance().log("Finished Initializing Orders Mapped File", Logger::Debug);
    }

    fstat64(m_fd, &m_sb);
    if (m_sb.st_size < (theApp.SSettings.ui64SizeOfOrdersMappedFile* 1000000000)) { // Fresh file
        Logger::instance().log("Error Initializing Mapped File", Logger::Debug);
        return false;
    }
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
COMMON_ORDER_MESSAGE* COrdersMap::GetMappedOrder(uint64_t uiOrderRefNumber)
{
  m_itSymbolMap =  m_SymbolMap.find(uiOrderRefNumber);
  
  if (m_itSymbolMap == m_SymbolMap.end())
    return NULL;
  else
    return &m_pCommonOrder[m_itSymbolMap->second];
}
//////////////////////////////////////////////////////////////////////////////////
uint64_t COrdersMap::FillMemoryMappedFile()
{
// Get Messages from Queue....Save the order Messages and disregard others

// ::TODO Check for the size limitation of the Memory Mapped File

  pItchMessageUnion = (ITCH_MESSAGES_UNION*) m_pQuantQueue->Dequeue(&m_iMessage);
  if (pItchMessageUnion == NULL){
    nanosleep (&m_request, &m_remain);  // sleep a 1/10 of a second
    return m_ui64NumOfOrders;
  }
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
	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber= pItchMessageUnion->AddOrderNoMPID.TrackingNumber;
	
	RetPair = m_SymbolMap.insert(std::pair<uint64_t , uint64_t>(m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber, m_ui64NumOfOrders) );	
	
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
	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber= pItchMessageUnion->AddOrderMPID.TrackingNumber;

	RetPair = m_SymbolMap.insert(std::pair<uint64_t , uint64_t>(m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber, m_ui64NumOfOrders) );	

	m_ui64NumOfOrders++;
	
	break;
	
    case 'U':   // Replace Order ... Delete OldOrderRefNumber  and Add NewOrderRefNumber >>>Copy MPID if exist
      
      	m_pCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderReplace.iOldOrderRefNumber);
	if (!m_pCommonOrder)
	  break;
	
	strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pCommonOrder->szStock);

	// Add the new one
	m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderReplace.cMessageType;
	m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber  	= pItchMessageUnion->OrderReplace.iNewOrderRefNumber;
	m_pCommonOrder[m_ui64NumOfOrders].iPrevOrderRefNumber 	= pItchMessageUnion->OrderReplace.iOldOrderRefNumber;
	
	m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->OrderReplace.iShares;
	m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderReplace.iTimeStamp;
	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderReplace.TrackingNumber;
	
	// find the old one
	m_itSymbolMap = m_SymbolMap.find(pItchMessageUnion->OrderReplace.iOldOrderRefNumber);
	if(m_itSymbolMap ==  m_SymbolMap.end()){
	  // Log Error
	  break;
	}
	// delete the old one
	m_SymbolMap.erase(m_itSymbolMap);
	// Insert new 0rder in Map
	RetPair = m_SymbolMap.insert(std::pair<uint64_t , uint64_t>(m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber, m_ui64NumOfOrders) );	
	// ::TODO log error if any

	m_ui64NumOfOrders++;	
	
	break;

    case 'E':  // Executed Order  // Tick Data  /// Where do I get the execution price from
       	m_pCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderExecuted.iOrderMatchNumber);
	if (!m_pCommonOrder)
	  break;
	
	strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pCommonOrder->szStock);

	m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderExecuted.cMessageType;
	m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->OrderExecuted.iOrderRefNumber;		  
	// Hack....put the order match number in Prev Order
	m_pCommonOrder[m_ui64NumOfOrders].iPrevOrderRefNumber  	= pItchMessageUnion->OrderExecuted.iOrderMatchNumber;
	m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->OrderExecuted.iShares;

	m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderExecuted.iTimeStamp;
	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderExecuted.TrackingNumber;
	m_ui64NumOfOrders++;	
	
	break;
	
    case 'c': // Executed with price   // Tick Data

       	m_pCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderExecutedWithPrice.iOrderMatchNumber);
	if (!m_pCommonOrder)
	  break;
	
	strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pCommonOrder->szStock);

	m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderExecutedWithPrice.cMessageType;
	m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->OrderExecutedWithPrice.iOrderRefNumber;		  
	// Hack....put the order match number in Prev Order
	m_pCommonOrder[m_ui64NumOfOrders].iPrevOrderRefNumber  	= pItchMessageUnion->OrderExecutedWithPrice.iOrderMatchNumber;
	m_pCommonOrder[m_ui64NumOfOrders].iShares = pItchMessageUnion->OrderExecutedWithPrice.iShares;

	m_pCommonOrder[m_ui64NumOfOrders].dPrice = pItchMessageUnion->OrderExecutedWithPrice.dExecutionPrice;
	
	m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderExecutedWithPrice.iTimeStamp;
	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderExecutedWithPrice.TrackingNumber;
	m_ui64NumOfOrders++;	
	
	break;
	
    case 'X':  // Cancel Order
       	m_pCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderCancel.iOrderRefNumber);
	if (!m_pCommonOrder)
	  break;
	
	strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pCommonOrder->szStock);

      
	m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderCancel.cMessageType;
	m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber  	= pItchMessageUnion->OrderCancel.iOrderRefNumber;
	//Canceled Shares	The number of shares being removed from the display size of the order as the result of a cancellation.
	m_pCommonOrder[m_ui64NumOfOrders].iShares 		= pItchMessageUnion->OrderCancel.iShares;
	m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderCancel.iTimeStamp;
	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderCancel.TrackingNumber;
	m_ui64NumOfOrders++;	
	
	break;
	
    case 'D':  // Delete Order
      
       	m_pCommonOrder     = GetMappedOrder(pItchMessageUnion->OrderDelete.iOrderRefNumber);
	if (!m_pCommonOrder)
	  break;
	
	strcpy(m_pCommonOrder[m_ui64NumOfOrders].szStock, m_pCommonOrder->szStock);
	m_pCommonOrder[m_ui64NumOfOrders].iShares = m_pCommonOrder->iShares;
	
	m_pCommonOrder[m_ui64NumOfOrders].cMessageType 		= pItchMessageUnion->OrderDelete.cMessageType;
	m_pCommonOrder[m_ui64NumOfOrders].iOrderRefNumber 	= pItchMessageUnion->OrderDelete.iOrderRefNumber;
	m_pCommonOrder[m_ui64NumOfOrders].iTimeStamp 		= pItchMessageUnion->OrderDelete.iTimeStamp;
	m_pCommonOrder[m_ui64NumOfOrders].TrackingNumber 	= pItchMessageUnion->OrderDelete.TrackingNumber;
	m_ui64NumOfOrders++;
	
	break;
	
    default:
      break;
  };
  return m_ui64NumOfOrders;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

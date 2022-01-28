#include "TickDataMap.h"

//////////////////////////////////////////////////////////////////////////////////
CTickDataMap::CTickDataMap()
{
	m_iError = 0;
	m_iMessage = 0;
	m_ui64TickRecordLocation = 0;

	m_fdData = open("QuanticksTickData.Qtx", O_RDWR | O_CREAT); // Data File
	if (m_fdData == -1)
	{
		Logger::instance().log("open QuanticksTickData.Qtx Error", Logger::Debug);
		m_iError = 100;
		// Set error code and exit
	}

	if (fstat(m_fdData, &m_sb) == -1)
	{
		Logger::instance().log("Error fstat on Tick Data", Logger::Debug);
		m_iError = 110;
		// Set error code and exit
	}

	m_fdIndex = open("QuanticksTickDataIndex.Qtx", O_RDWR | O_CREAT); // Index File
	if (m_fdIndex == -1)
	{
		Logger::instance().log("open QuanticksTickDataIndex.Qtx Error", Logger::Debug);
		m_iError = 100;
		// Set error code and exit
	}

	if (fstat(m_fdIndex, &m_sb) == -1)
	{
		Logger::instance().log("Error fstat on Tick Data Index", Logger::Debug);
		m_iError = 110;
		// Set error code and exit
	}

	if (!m_iError)
	{

		// Init the Queue
		m_pQuantQueue = NULL;
		m_pQuantQueue = CQuantQueue::Instance(); // Only one instance is allowed of this singelton class

		if (!m_pQuantQueue)
		{
			Logger::instance().log("Error initialized the Queue", Logger::Debug);
			m_iError = 140;
		}
		m_pQuantQueue->InitReader(POSITION_TOP);
	}
	m_pCOrdersMap = NULL;
	m_pCOrdersMap = COrdersMap::instance();
	m_request.tv_nsec = 100000000; // 1/10 of a second

	m_iSizeOfIndexRecord = sizeof(SINDEX_RECORD);
	m_iSizeOfCommonTradeRecord = sizeof(COMMON_TRADE_MESSAGE);
}
//////////////////////////////////////////////////////////////////////////////////
CTickDataMap::~CTickDataMap()
{

	m_SymbolMap.clear();
	close(m_fdData);

	close(m_fdIndex);
}
//////////////////////////////////////////////////////////////////////////////////
int CTickDataMap::GetError()
{
	return m_iError;
}
//////////////////////////////////////////////////////////////////////////////////
int CTickDataMap::InitMemoryMappedFile()
{
	return true;
}
//////////////////////////////////////////////////////////////////////////////////
u_int64_t CTickDataMap::GetNumberOfTickData()
{
	return m_ui64TickRecordLocation;
}
//////////////////////////////////////////////////////////////////////////////////
COMMON_TRADE_MESSAGE CTickDataMap::GetMappedRecord(uint64_t iOrderRefNumber)
{
	COMMON_TRADE_MESSAGE x;
	return x; // lseek64();		m_SCommonTrade;
}
//////////////////////////////////////////////////////////////////////////////////
uint64_t CTickDataMap::FillTickDataFile()
{
	// Get Messages from Queue....Save the order Messages and disregard others

	// ::TODO Check for the size limitation of the Memory Mapped File

	pItchMessageUnion = (ITCH_MESSAGES_UNION *)m_pQuantQueue->Dequeue(&m_iMessage);
	if (pItchMessageUnion == NULL)
	{
		nanosleep(&m_request, &m_remain); // sleep a 1/10 of a second
		return m_ui64TickRecordLocation;
	}

	// Get Symbol from Orders Map.....Hack it for now

	memset(&m_SCommonTrade, '\0', m_iSizeOfCommonTradeRecord);
	memset(&m_SIndexRecord, '\0', m_iSizeOfIndexRecord);

	switch (m_iMessage)
	{
	case 'E': // Executed Order  // Tick Data
		m_pCommonOrder = m_pCOrdersMap->GetMappedOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
		if (!m_pCommonOrder)
			break;
		//Begin Update Data
		memset(&m_SCommonTrade, '\0', m_iSizeOfCommonTradeRecord);
		m_SCommonTrade.cMessageType = pItchMessageUnion->OrderExecuted.cMessageType;
		strcpy(m_SCommonTrade.szStock, m_pCommonOrder->szStock);
		m_SCommonTrade.iOrderRefNumber = pItchMessageUnion->OrderExecuted.iOrderMatchNumber;
		m_SCommonTrade.iShares = pItchMessageUnion->OrderExecuted.iShares;
		m_SCommonTrade.iTimeStamp = pItchMessageUnion->OrderExecuted.iTimeStamp;
		m_SCommonTrade.dPrice = m_pCommonOrder->dPrice; // ::TODO revise

		//End Update Data
		m_itSymbolMap = m_SymbolMap.find(m_SCommonTrade.szStock);
		//Begin Update Index

		InitIndexRecord();
		clock_gettime(CLOCK_REALTIME, &m_SIndexRecord.tLastUpdate);
		if (m_itSymbolMap == m_SymbolMap.end())
		{	// No Entry Yet
			// Add a new entry to the Map
			strcpy(m_SIndexRecord.szSymbol, m_SCommonTrade.szStock);
			m_SIndexRecord.dLast = m_SCommonTrade.dPrice;
			m_SIndexRecord.dOpen = m_SCommonTrade.dPrice;
			m_SIndexRecord.dHigh = m_SCommonTrade.dPrice;
			m_SIndexRecord.dLow = m_SCommonTrade.dPrice;
			m_SIndexRecord.uiTotalNumOfTrades++;
			m_SIndexRecord.uiTotalVolume = m_SCommonTrade.iShares;
			m_SIndexRecord.uiFirstLocation = m_ui64TickRecordLocation; // Location in File...use lseek to find
			m_SIndexRecord.uiLastLocation = m_ui64TickRecordLocation;

			m_SymbolMap.insert(pair<char *, SINDEX_RECORD>(m_pCommonOrder->szStock, m_SIndexRecord));
			// write data to file
			lseek64(m_fdData, m_SIndexRecord.uiFirstLocation, SEEK_SET);
			write(m_fdData, &m_SIndexRecord, m_iSizeOfIndexRecord);
		}
		else
		{ //Found it...Update the record in the file and then Update the map

			m_itSymbolMap->second.dLast = m_SCommonTrade.dPrice;
			// m_SIndexRecord.dClose = m_SCommonTrade.dPrice;
			if (m_SCommonTrade.dPrice > m_itSymbolMap->second.dHigh) // set dHigh
				m_itSymbolMap->second.dHigh = m_SCommonTrade.dPrice;
			if (m_SCommonTrade.dPrice < m_itSymbolMap->second.dLow) // set dLow
				m_itSymbolMap->second.dLow = m_SCommonTrade.dPrice;

			m_itSymbolMap->second.uiTotalNumOfTrades++;
			m_itSymbolMap->second.uiTotalVolume += m_SCommonTrade.iShares;
			//  m_itSymbolMap->second.uiLastLocation = m_ui64TickRecordLocation;  // Location in File...use lseek to find

			//Update index in the map
			lseek64(m_fdData, m_itSymbolMap->second.uiLastLocation * m_iSizeOfCommonTradeRecord, SEEK_SET);
			read(m_fdData, &m_STempCommonTrade, m_iSizeOfCommonTradeRecord);
			m_itSymbolMap->second.uiLastLocation = m_ui64TickRecordLocation;
			m_STempCommonTrade.uiNextLocation = m_ui64TickRecordLocation;
			// update the previous record
			//	  lseek64(m_fdData, - m_iSizeOfCommonTradeRecord, SEEK_CUR);
			// RESET to position after read
			lseek64(m_fdData, m_itSymbolMap->second.uiLastLocation * m_iSizeOfCommonTradeRecord, SEEK_SET);
			write(m_fdData, &m_STempCommonTrade, m_iSizeOfCommonTradeRecord);
			// write the current record at end of file
			//  lseek64(m_fdData, m_ui64TickRecordLocation* m_iSizeOfCommonTradeRecord, SEEK_SET);
			lseek64(m_fdData, m_iSizeOfCommonTradeRecord, SEEK_END);
			m_SCommonTrade.uiNextLocation = 0; // this is always the last record or is updated as above
			write(m_fdData, &m_SCommonTrade, m_iSizeOfCommonTradeRecord);
		}

		m_ui64TickRecordLocation++;

		// Get Symbol from Orders Map.....Hack it for now
		//	m_SymbolMap.insert(pair<char* , uint64_t>(m_pCommonOrder->szStock, m_ui64TickRecordLocation));
		break;

	case 'c': // Executed with price   // Tick Data
		m_pCommonOrder = m_pCOrdersMap->GetMappedOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
		if (!m_pCommonOrder)
			break;

		m_SCommonTrade.cMessageType = pItchMessageUnion->OrderExecutedWithPrice.cMessageType;
		strcpy(m_SCommonTrade.szStock, m_pCommonOrder->szStock);

		m_SCommonTrade.iOrderRefNumber = pItchMessageUnion->OrderExecutedWithPrice.iOrderMatchNumber;
		m_SCommonTrade.iShares = pItchMessageUnion->OrderExecutedWithPrice.iShares;
		m_SCommonTrade.iTimeStamp = pItchMessageUnion->OrderExecutedWithPrice.iTimeStamp;
		m_SCommonTrade.dPrice = pItchMessageUnion->OrderExecutedWithPrice.dExecutionPrice;

		//End Update Data
		m_itSymbolMap = m_SymbolMap.find(m_SCommonTrade.szStock);
		//Begin Update Index

		InitIndexRecord();
		clock_gettime(CLOCK_REALTIME, &m_SIndexRecord.tLastUpdate);
		if (m_itSymbolMap == m_SymbolMap.end())
		{	// No Entry Yet
			// Add a new entry to the Map
			strcpy(m_SIndexRecord.szSymbol, m_SCommonTrade.szStock);
			m_SIndexRecord.dLast = m_SCommonTrade.dPrice;
			m_SIndexRecord.dOpen = m_SCommonTrade.dPrice;
			m_SIndexRecord.dHigh = m_SCommonTrade.dPrice;
			m_SIndexRecord.dLow = m_SCommonTrade.dPrice;
			m_SIndexRecord.uiTotalNumOfTrades++;
			m_SIndexRecord.uiTotalVolume = m_SCommonTrade.iShares;
			m_SIndexRecord.uiFirstLocation = m_ui64TickRecordLocation; // Location in File...use lseek to find
			m_SIndexRecord.uiLastLocation = m_ui64TickRecordLocation;

			m_SymbolMap.insert(pair<char *, SINDEX_RECORD>(m_pCommonOrder->szStock, m_SIndexRecord));
			// write data to file
			lseek64(m_fdData, m_SIndexRecord.uiFirstLocation, SEEK_SET);
			write(m_fdData, &m_SIndexRecord, m_iSizeOfIndexRecord);
		}
		else
		{ //Found it...Update the record in the file and then Update the map

			m_itSymbolMap->second.dLast = m_SCommonTrade.dPrice;
			// m_SIndexRecord.dClose = m_SCommonTrade.dPrice;
			if (m_SCommonTrade.dPrice > m_itSymbolMap->second.dHigh) // set dHigh
				m_itSymbolMap->second.dHigh = m_SCommonTrade.dPrice;
			if (m_SCommonTrade.dPrice < m_itSymbolMap->second.dLow) // set dLow
				m_itSymbolMap->second.dLow = m_SCommonTrade.dPrice;

			m_itSymbolMap->second.uiTotalNumOfTrades++;
			m_itSymbolMap->second.uiTotalVolume += m_SCommonTrade.iShares;
			//  m_itSymbolMap->second.uiLastLocation = m_ui64TickRecordLocation;  // Location in File...use lseek to find

			//Update index in the map
			lseek64(m_fdData, m_itSymbolMap->second.uiLastLocation * m_iSizeOfCommonTradeRecord, SEEK_SET);
			read(m_fdData, &m_STempCommonTrade, m_iSizeOfCommonTradeRecord);
			m_itSymbolMap->second.uiLastLocation = m_ui64TickRecordLocation;
			m_STempCommonTrade.uiNextLocation = m_ui64TickRecordLocation;
			// update the previous record
			//	  lseek64(m_fdData, - m_iSizeOfCommonTradeRecord, SEEK_CUR);
			// RESET to position after read
			lseek64(m_fdData, m_itSymbolMap->second.uiLastLocation * m_iSizeOfCommonTradeRecord, SEEK_SET);
			write(m_fdData, &m_STempCommonTrade, m_iSizeOfCommonTradeRecord);
			// write the current record at end of file
			//  lseek64(m_fdData, m_ui64TickRecordLocation* m_iSizeOfCommonTradeRecord, SEEK_SET);
			lseek64(m_fdData, m_iSizeOfCommonTradeRecord, SEEK_END);
			m_SCommonTrade.uiNextLocation = 0; // this is always the last record or is updated as above
			write(m_fdData, &m_SCommonTrade, m_iSizeOfCommonTradeRecord);
		}

		m_ui64TickRecordLocation++;

	case 'P': // TRADE_NON_CROSS_MESSAGE
		m_pCommonOrder = m_pCOrdersMap->GetMappedOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
		if (!m_pCommonOrder)
			break;

		m_SCommonTrade.cMessageType = pItchMessageUnion->TradeNonCross.cMessageType;
		strcpy(m_SCommonTrade.szStock, m_pCommonOrder->szStock);

		m_SCommonTrade.iOrderRefNumber = pItchMessageUnion->TradeNonCross.iOrderRefNumber;
		m_SCommonTrade.iShares = pItchMessageUnion->TradeNonCross.iShares;
		m_SCommonTrade.iTimeStamp = pItchMessageUnion->TradeNonCross.iTimeStamp;
		m_SCommonTrade.dPrice = pItchMessageUnion->TradeNonCross.dPrice;

		//End Update Data
		m_itSymbolMap = m_SymbolMap.find(m_SCommonTrade.szStock);
		//Begin Update Index

		InitIndexRecord();
		clock_gettime(CLOCK_REALTIME, &m_SIndexRecord.tLastUpdate);
		if (m_itSymbolMap == m_SymbolMap.end())
		{	// No Entry Yet
			// Add a new entry to the Map
			strcpy(m_SIndexRecord.szSymbol, m_SCommonTrade.szStock);
			m_SIndexRecord.dLast = m_SCommonTrade.dPrice;
			m_SIndexRecord.dOpen = m_SCommonTrade.dPrice;
			m_SIndexRecord.dHigh = m_SCommonTrade.dPrice;
			m_SIndexRecord.dLow = m_SCommonTrade.dPrice;
			m_SIndexRecord.uiTotalNumOfTrades++;
			m_SIndexRecord.uiTotalVolume = m_SCommonTrade.iShares;
			m_SIndexRecord.uiFirstLocation = m_ui64TickRecordLocation; // Location in File...use lseek to find
			m_SIndexRecord.uiLastLocation = m_ui64TickRecordLocation;

			m_SymbolMap.insert(pair<char *, SINDEX_RECORD>(m_pCommonOrder->szStock, m_SIndexRecord));
			// write data to file
			lseek64(m_fdData, m_SIndexRecord.uiFirstLocation, SEEK_SET);
			write(m_fdData, &m_SIndexRecord, m_iSizeOfIndexRecord);
		}
		else
		{ //Found it...Update the record in the file and then Update the map

			m_itSymbolMap->second.dLast = m_SCommonTrade.dPrice;
			// m_SIndexRecord.dClose = m_SCommonTrade.dPrice;
			if (m_SCommonTrade.dPrice > m_itSymbolMap->second.dHigh) // set dHigh
				m_itSymbolMap->second.dHigh = m_SCommonTrade.dPrice;
			if (m_SCommonTrade.dPrice < m_itSymbolMap->second.dLow) // set dLow
				m_itSymbolMap->second.dLow = m_SCommonTrade.dPrice;

			m_itSymbolMap->second.uiTotalNumOfTrades++;
			m_itSymbolMap->second.uiTotalVolume += m_SCommonTrade.iShares;
			//  m_itSymbolMap->second.uiLastLocation = m_ui64TickRecordLocation;  // Location in File...use lseek to find

			//Update index in the map
			lseek64(m_fdData, m_itSymbolMap->second.uiLastLocation * m_iSizeOfCommonTradeRecord, SEEK_SET);
			read(m_fdData, &m_STempCommonTrade, m_iSizeOfCommonTradeRecord);
			m_itSymbolMap->second.uiLastLocation = m_ui64TickRecordLocation;
			m_STempCommonTrade.uiNextLocation = m_ui64TickRecordLocation;
			// update the previous record
			//	  lseek64(m_fdData, - m_iSizeOfCommonTradeRecord, SEEK_CUR);
			// RESET to position after read
			lseek64(m_fdData, m_itSymbolMap->second.uiLastLocation * m_iSizeOfCommonTradeRecord, SEEK_SET);
			write(m_fdData, &m_STempCommonTrade, m_iSizeOfCommonTradeRecord);
			// write the current record at end of file
			//  lseek64(m_fdData, m_ui64TickRecordLocation* m_iSizeOfCommonTradeRecord, SEEK_SET);
			lseek64(m_fdData, m_iSizeOfCommonTradeRecord, SEEK_END);
			m_SCommonTrade.uiNextLocation = 0; // this is always the last record or is updated as above
			write(m_fdData, &m_SCommonTrade, m_iSizeOfCommonTradeRecord);
		}

		m_ui64TickRecordLocation++;

		break;

	case 'S': // system event message
	case 'M': // system event message
	case 'C': // system event message
		m_pSystemEvent = pItchMessageUnion->SystemEvent;
		if (m_pSystemEvent.cEventCode == 'E')
		{
			for (m_itSymbolMap = m_SymbolMap.begin(); m_itSymbolMap != m_SymbolMap.end(); m_itSymbolMap++)
				m_itSymbolMap->second.dClose = m_itSymbolMap->second.dLast;
		};
		break;
	default:
		break;
	};
	return m_ui64TickRecordLocation; // = Number of inserted records
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t CTickDataMap::GetMapSize()
{
	return m_SymbolMap.size();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t CTickDataMap::GetMapMaxSize()
{
	return m_SymbolMap.max_size();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CTickDataMap::InitIndexRecord()
{

	m_SIndexRecord.dLast = 0;
	m_SIndexRecord.dOpen = 0;
	m_SIndexRecord.dClose = 0;
	m_SIndexRecord.dHigh = 0;
	m_SIndexRecord.dLow = 0;
	m_SIndexRecord.uiTotalNumOfTrades = 0;
	m_SIndexRecord.uiTotalVolume = 0;
}

/*
std::multimap<char,int> mymm;

  mymm.insert(std::make_pair('x',50));
  mymm.insert(std::make_pair('y',100));
  mymm.insert(std::make_pair('y',150));
  mymm.insert(std::make_pair('y',200));
  mymm.insert(std::make_pair('z',250));
  mymm.insert(std::make_pair('z',300));

  for (char c='x'; c<='z'; c++)
  {
    std::cout << "There are " << mymm.count(c) << " elements with key " << c << ":";
    std::multimap<char,int>::iterator it;
    for (it=mymm.equal_range(c).first; it!=mymm.equal_range(c).second; ++it)
      std::cout << ' ' << (*it).second;
    std::cout << '\n';
  }
*/

/*
  std::multimap<char,int> mymm;

  mymm.insert(std::pair<char,int>('a',10));
  mymm.insert(std::pair<char,int>('b',20));
  mymm.insert(std::pair<char,int>('b',30));
  mymm.insert(std::pair<char,int>('b',40));
  mymm.insert(std::pair<char,int>('c',50));
  mymm.insert(std::pair<char,int>('c',60));
  mymm.insert(std::pair<char,int>('d',60));

  std::cout << "mymm contains:\n";
  for (char ch='a'; ch<='d'; ch++)
  {
    std::pair <std::multimap<char,int>::iterator, std::multimap<char,int>::iterator> ret;
    ret = mymm.equal_range(ch);
    std::cout << ch << " =>";
    for (std::multimap<char,int>::iterator it=ret.first; it!=ret.second; ++it)
      std::cout << ' ' << it->second;
    std::cout << '\n';
  }

  return 0;
}
Edit & Run


mymm contains:
a => 10
b => 20 30 40
c => 50 60
d => 60

*/

/*
 int main ()
{
  std::multimap<char,int> mymm;

  mymm.insert(std::make_pair('x',50));
  mymm.insert(std::make_pair('y',100));
  mymm.insert(std::make_pair('y',150));
  mymm.insert(std::make_pair('y',200));
  mymm.insert(std::make_pair('z',250));
  mymm.insert(std::make_pair('z',300));

  for (char c='x'; c<='z'; c++)
  {
    std::cout << "There are " << mymm.count(c) << " elements with key " << c << ":";
    std::multimap<char,int>::iterator it;
    for (it=mymm.equal_range(c).first; it!=mymm.equal_range(c).second; ++it)
      std::cout << ' ' << (*it).second;
    std::cout << '\n';
  }

  return 0;
}
Edit & Run


Output:

There are 1 elements with key x: 50
There are 3 elements with key y: 100 150 200
There are 2 elements with key z: 250 300
 
*/
/*
timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
*/

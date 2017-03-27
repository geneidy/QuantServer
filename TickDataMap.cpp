#include "TickDataMap.h"

#define SIZE_OF_ORDERS_FILE_NAME  71
//////////////////////////////////////////////////////////////////////////////////
CTickDataMap::CTickDataMap()
{
    m_iError = 0;
    m_iMessage = 0;
    m_ui64NumOfTickData = 0;

    m_pcUtil = NULL;

    m_pcUtil = new CUtil(theApp.SSettings.szActiveSymbols, theApp.SSettings.arrbActive);

    if (!m_pcUtil) {
        Logger::instance().log("Error Initializing CUtil", Logger::Error);
        m_iError = 100;
    }

    struct stat st = {0};

    if (stat("../Ticks/", &st) == -1) {
        mkdir("../Ticks/", 0700);
    }

    string strTickFile;
    strTickFile.empty();

    strTickFile = "../Ticks/";
    strTickFile += m_pcUtil->GetFormatedDate();
    strTickFile += "QuanticksTicks.qtx";

//    m_fd = open64("./Ticks/QuanticksTickData.Qtx", O_RDWR|O_CREAT, S_IRWXU);
    m_fd = open64(strTickFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);

    if (m_fd == -1) {
        Logger::instance().log("Tick Data open File Mapping Error", Logger::Error);
        m_iError = 100;
        // Set error code and exit
    }
    if (!m_iError) {

        if (fstat64(m_fd, &m_sb) == -1) {
            Logger::instance().log("Error fstat", Logger::Error);
            m_iError = 110;
            // Set error code and exit
        }
        else {
            m_iSizeOfCommonTradeRecord = sizeof(COMMON_TRADE_MESSAGE); // Avoid computing the size million times every second

            m_pCOrdersMap = NULL;
            m_pCOrdersMap = COrdersMap::instance();
            if (m_pCOrdersMap) {
                Logger::instance().log("Order Map File instance in Tick Data", Logger::Info);
            }
            else {
                m_iError = 200;  // enum later
                Logger::instance().log("Error Getting Order Map File instance in Tick Data", Logger::Error);
            }
            m_request.tv_sec = 0;
            m_request.tv_nsec = 100000000;   // 1/10 of a second
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////
void CTickDataMap::InitQueue(CQuantQueue* pQueue)
{
    m_pQuantQueue = pQueue;
    // Init the Queue
    m_pQuantQueue->InitReader(POSITION_TOP);
    Logger::instance().log("Queue initialized in Tick Data Map file", Logger::Info);
}
//////////////////////////////////////////////////////////////////////////////////
CTickDataMap::~CTickDataMap()
{
    if (m_pCOrdersMap)
        m_pCOrdersMap->iNInstance--;


    Logger::instance().log("Start...UnMapping TickDataMap file", Logger::Info);
    msync(m_addr, m_sb.st_size, MS_ASYNC);
    munmap(m_addr, m_sb.st_size);
    Logger::instance().log("End...UnMapping TickDataMap file", Logger::Info);

    /*    Logger::instance().log("Start...Clearing TickMap", Logger::Info);
        m_TickMap.clear();
        Logger::instance().log("End...Clearing TickMap", Logger::Info);

        Logger::instance().log("Start...Clearing Fundamental Map", Logger::Info);
        m_FundamentalMap.clear();
        Logger::instance().log("End...Clearing Fundamental Map", Logger::Info);
    */    close(m_fd);

    if (m_pcUtil) {
        delete m_pcUtil;
        m_pcUtil = NULL;
    }

}
//////////////////////////////////////////////////////////////////////////////////
int CTickDataMap::GetError()
{
    return m_iError;
}
//////////////////////////////////////////////////////////////////////////////////
int CTickDataMap::InitMemoryMappedFile()
{
    COMMON_TRADE_MESSAGE SCommonTrade;
    m_uiNumberOfMessagesToHold = 0;

    memset(&SCommonTrade, '\0', m_iSizeOfCommonTradeRecord );

    m_uiNumberOfMessagesToHold = (theApp.SSettings.ui64SizeOfTickDataMappedFile* 1000000000) / m_iSizeOfCommonTradeRecord;

    int iRet = 0;
    if (m_sb.st_size < (theApp.SSettings.ui64SizeOfTickDataMappedFile* 1000000000)) { // Fresh file
        Logger::instance().log("Initializing Tick Data Mapped File", Logger::Info);
        for (uint64_t ii = 0; ii < (m_uiNumberOfMessagesToHold +1); ii++) {
            iRet = write(m_fd, &SCommonTrade, m_iSizeOfCommonTradeRecord);  // init with NULL
            // check for errors
        }
        Logger::instance().log("Finished Initializing Tick Data Mapped File", Logger::Info);
    }
    fstat64(m_fd, &m_sb);
    if (m_sb.st_size < (theApp.SSettings.ui64SizeOfTickDataMappedFile* 1000000000)) { // Fresh file
        Logger::instance().log("Error Initializing Tick Data Mapped File", Logger::Error);
        return false;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////
STickDataStat CTickDataMap::GetTickDataStat() // Total Trade records inserted
{
    m_STickDataStat.uiNumberOfMessagesToHold 	= m_uiNumberOfMessagesToHold; // Max
    m_STickDataStat.ui64NumOfTickData 		= m_ui64NumOfTickData;  // last sequential number inserted
    /*
        m_STickDataStat.uiFundamemtalMapSize 	= m_FundamentalMap.size();
        m_STickDataStat.uiFundamemtalMapMaxSize	= m_FundamentalMap.max_size();

        m_STickDataStat.uiTickMapSize        	= m_TickMap.size();
        m_STickDataStat.uiTickMapMaxSize		= m_TickMap.max_size();
    */
    return m_STickDataStat;
}
//////////////////////////////////////////////////////////////////////////////////
uint64_t CTickDataMap::FillTickFile()
{
// Get Messages from Queue....Save the order Messages and disregard others

// ::TODO Check for the size limitation of the Memory Mapped File
    while (theApp.SSettings.iStatus != STOPPED) {
        pItchMessageUnion = (ITCH_MESSAGES_UNION*) m_pQuantQueue->Dequeue(&m_iMessage);
        if (pItchMessageUnion == NULL) {
            nanosleep (&m_request, &m_remain);  // sleep a 1/10 of a second
//            return m_ui64NumOfTickData;
        }

        // Get Symbol from Orders Map...

        switch (m_iMessage) {
        case 'E':  // Executed Order  // Tick Data
            m_pCommonOrder     = m_pCOrdersMap->GetOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
            if (!m_pCommonOrder)
                break;

            if (!m_pcUtil->IsSymbolIn(m_pCommonOrder->szStock)) // check for Range
                break;

            /*
                        if (!m_pcUtil->CheckInclude(m_pCommonOrder->szStock)) // check for Range
                            return 0;
            */

            memset(&m_CommonTrade, '\0', m_iSizeOfCommonOrderRecord);

            m_CommonTrade.cMessageType 	= pItchMessageUnion->OrderExecuted.cMessageType;
            strcpy(m_CommonTrade.szStock, m_pCommonOrder->szStock);
            m_CommonTrade.iOrderRefNumber  	= pItchMessageUnion->OrderExecuted.iOrderMatchNumber;
            m_CommonTrade.iShares 		= pItchMessageUnion->OrderExecuted.iShares;
            m_CommonTrade.iTimeStamp 		= pItchMessageUnion->OrderExecuted.iTimeStamp;
            m_CommonTrade.dPrice		= m_pCommonOrder->dPrice;  // ::TODO Revise

            m_ui64NumOfTickData++;

            write(m_fd, &m_CommonTrade, m_iSizeOfCommonTradeRecord);

            /*            m_itTickMap =  m_TickMap.insert(pair< char* , uint64_t>(m_pCommonOrder->szStock, m_ui64NumOfTickData));

                        InitFundamentalRecord();
                        clock_gettime(CLOCK_REALTIME, &m_SFundamentalRecord.tLastUpdate);
                        if (m_itFundamentalMap == m_FundamentalMap.end()) { // No Entry Yet
                            // Add a new entry to the Map
                            strcpy(m_SFundamentalRecord.szSymbol, m_pCommonTrade->szStock);
                            m_SFundamentalRecord.dLast 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dOpen 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dHigh 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dLow 	= m_pCommonTrade->dPrice;

                            m_SFundamentalRecord.uiTotalNumOfTrades++;
                            m_SFundamentalRecord.uiVolume       = m_pCommonTrade->iShares;
                            m_SFundamentalRecord.uiTotalVolume 	+= m_pCommonTrade->iShares;

                            RetPairF = m_FundamentalMap.insert(pair<char*, SFUNDAMENTAL_RECORD>(m_pCommonOrder->szStock, m_SFundamentalRecord));
                        }
                        else { //Found it...  Update the map

                            if (m_itFundamentalMap->second.dHigh < m_SFundamentalRecord.dLast)  // set dHigh
                                m_itFundamentalMap->second.dHigh = m_SFundamentalRecord.dLast;
                            if (m_itFundamentalMap->second.dLow > m_SFundamentalRecord.dLast)    // set dLow
                                m_itFundamentalMap->second.dLow = m_SFundamentalRecord.dLast;

                            m_itFundamentalMap->second.uiTotalNumOfTrades++;
                            m_SFundamentalRecord.uiVolume       = m_pCommonTrade->iShares;
                            m_itFundamentalMap->second.uiTotalVolume += m_pCommonTrade->iShares;
                            // Update ticks
                            if (m_itFundamentalMap->second.dLast < m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '+';
                            if (m_itFundamentalMap->second.dLast > m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '-';
                            if (m_itFundamentalMap->second.dLast == m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '=';

                        }
            */
            break;

        case 'c': // Executed with price   // Tick Data
            m_pCommonOrder     = m_pCOrdersMap->GetOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
            if (!m_pCommonOrder)
                break;

            if (!m_pcUtil->IsSymbolIn(m_pCommonOrder->szStock)) // check for Range
                break;

            memset(&m_CommonTrade, '\0', m_iSizeOfCommonOrderRecord);

            m_CommonTrade.cMessageType 	= pItchMessageUnion->OrderExecutedWithPrice.cMessageType;
            strcpy(m_CommonTrade.szStock, m_pCommonOrder->szStock);

            m_CommonTrade.iOrderRefNumber  	= pItchMessageUnion->OrderExecutedWithPrice.iOrderMatchNumber;
            m_CommonTrade.iShares 		= pItchMessageUnion->OrderExecutedWithPrice.iShares;
            m_CommonTrade.iTimeStamp 		= pItchMessageUnion->OrderExecutedWithPrice.iTimeStamp;
            m_CommonTrade.dPrice		= pItchMessageUnion->OrderExecutedWithPrice.dExecutionPrice;

            write(m_fd, &m_CommonTrade, m_iSizeOfCommonTradeRecord);

            m_ui64NumOfTickData++;

            /*            m_itTickMap =  m_TickMap.insert(pair< char* const, uint64_t> (m_pCommonOrder->szStock, m_ui64NumOfTickData));

                        InitFundamentalRecord();
                        clock_gettime(CLOCK_REALTIME, &m_SFundamentalRecord.tLastUpdate);
                        if (m_itFundamentalMap == m_FundamentalMap.end()) { // No Entry Yet
                            // Add a new entry to the Map
                            strcpy(m_SFundamentalRecord.szSymbol, m_pCommonTrade->szStock);
                            m_SFundamentalRecord.dLast 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dOpen 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dHigh 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dLow 	= m_pCommonTrade->dPrice;

                            m_SFundamentalRecord.uiTotalNumOfTrades++;
                            m_SFundamentalRecord.uiVolume       = m_pCommonTrade->iShares;
                            m_SFundamentalRecord.uiTotalVolume 	+= m_pCommonTrade->iShares;

                            RetPairF =   m_FundamentalMap.insert(pair<char*, SFUNDAMENTAL_RECORD>(m_pCommonOrder->szStock, m_SFundamentalRecord));
                        }
                        else { //Found it...  Update the map

                            if (m_itFundamentalMap->second.dHigh < m_SFundamentalRecord.dLast)  // set dHigh
                                m_itFundamentalMap->second.dHigh = m_SFundamentalRecord.dLast;
                            if (m_itFundamentalMap->second.dLow > m_SFundamentalRecord.dLast)    // set dLow
                                m_itFundamentalMap->second.dLow = m_SFundamentalRecord.dLast;

                            m_itFundamentalMap->second.uiTotalNumOfTrades++;
                            m_SFundamentalRecord.uiVolume       = m_pCommonTrade->iShares;
                            m_itFundamentalMap->second.uiTotalVolume += m_pCommonTrade->iShares;

                            if (m_itFundamentalMap->second.dLast < m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '+';
                            if (m_itFundamentalMap->second.dLast > m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '-';
                            if (m_itFundamentalMap->second.dLast == m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '=';
                        }
            */            break;

        case 'P' :  // TRADE_NON_CROSS_MESSAGE
            m_pCommonOrder     = m_pCOrdersMap->GetOrder(pItchMessageUnion->OrderExecuted.iOrderRefNumber);
            if (!m_pCommonOrder)
                break;

            if (!m_pcUtil->IsSymbolIn(m_pCommonOrder->szStock)) // check for Range
                break;


            memset(&m_CommonTrade, '\0', m_iSizeOfCommonOrderRecord);

            m_CommonTrade.cMessageType 	= pItchMessageUnion->TradeNonCross.cMessageType;
            strcpy(m_CommonTrade.szStock, m_pCommonOrder->szStock);

            m_CommonTrade.iOrderRefNumber  	= pItchMessageUnion->TradeNonCross.iOrderRefNumber;
            m_CommonTrade.iShares 		= pItchMessageUnion->TradeNonCross.iShares;
            m_CommonTrade.iTimeStamp 		= pItchMessageUnion->TradeNonCross.iTimeStamp;
            m_CommonTrade.dPrice		= pItchMessageUnion->TradeNonCross.dPrice;

            write(m_fd, &m_CommonTrade, m_iSizeOfCommonTradeRecord);

            m_ui64NumOfTickData++;
            /*            m_itTickMap =  m_TickMap.insert(pair<char* , uint64_t>(m_pCommonOrder->szStock, m_ui64NumOfTickData));

                        InitFundamentalRecord();
                        clock_gettime(CLOCK_REALTIME, &m_SFundamentalRecord.tLastUpdate);
                        if (m_itFundamentalMap == m_FundamentalMap.end()) { // No Entry Yet
                            // Add a new entry to the Map
                            strcpy(m_SFundamentalRecord.szSymbol, m_pCommonTrade->szStock);
                            m_SFundamentalRecord.dLast 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dOpen 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dHigh 	= m_pCommonTrade->dPrice;
                            m_SFundamentalRecord.dLow 	= m_pCommonTrade->dPrice;

                            m_SFundamentalRecord.uiTotalNumOfTrades++;
                            m_SFundamentalRecord.uiVolume       = m_pCommonTrade->iShares;
                            m_SFundamentalRecord.uiTotalVolume 	+= m_pCommonTrade->iShares;

                            RetPairF =  m_FundamentalMap.insert(pair<char*, SFUNDAMENTAL_RECORD>(m_pCommonOrder->szStock, m_SFundamentalRecord));
                        }
                        else { //Found it...  Update the map

                            if (m_itFundamentalMap->second.dHigh < m_SFundamentalRecord.dLast)  // set dHigh
                                m_itFundamentalMap->second.dHigh = m_SFundamentalRecord.dLast;
                            if (m_itFundamentalMap->second.dLow > m_SFundamentalRecord.dLast)    // set dLow
                                m_itFundamentalMap->second.dLow = m_SFundamentalRecord.dLast;

                            m_itFundamentalMap->second.uiTotalNumOfTrades++;
                            m_SFundamentalRecord.uiVolume       = m_pCommonTrade->iShares;
                            m_itFundamentalMap->second.uiTotalVolume += m_pCommonTrade->iShares;

                            if (m_itFundamentalMap->second.dLast < m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '+';
                            if (m_itFundamentalMap->second.dLast > m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '-';
                            if (m_itFundamentalMap->second.dLast == m_pCommonTrade->dPrice)
                                m_itFundamentalMap->second.cTick = '=';
                        }
            */
            break;
            //System Event Messages
            //.....copy the lastest price to the close
        case 'S': //'E': End of System hours   //'M':  End of Market hours.  //'C':   End of Messages
            /*       m_pSystemEvent = pItchMessageUnion->SystemEvent;
                   if ((m_pSystemEvent.cEventCode == 'E')|| (m_pSystemEvent.cEventCode == 'M')|| (m_pSystemEvent.cEventCode == 'C')) {
                       for (m_itFundamentalMap= m_FundamentalMap.begin(); m_itFundamentalMap!= m_FundamentalMap.end(); ++m_itFundamentalMap)
                           m_itFundamentalMap->second.dClose = m_itFundamentalMap->second.dLast;
                   };
              */
            break;

        default:
            break;
        };
    };
    return m_ui64NumOfTickData;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CTickDataMap::InitFundamentalRecord()
{
    /*    m_SFundamentalRecord.dLast 	= 0;
        m_SFundamentalRecord.dOpen 	= 0;
        m_SFundamentalRecord.dClose = 0;
        m_SFundamentalRecord.dHigh 	= 0;
        m_SFundamentalRecord.dLow 	= 0;
        m_SFundamentalRecord.uiTotalNumOfTrades = 0;
        m_SFundamentalRecord.uiTotalVolume = 0;
    */
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
COMMON_TRADE_MESSAGE* CTickDataMap::GetTradeTicks(char* szStock)
{
// EXAMPLE Code to be copied in the calling Object....the return statement is the data to be used..............
// DO NOT USE THIS CODE HERE AS IS ... REFER TO statement ABOVE

    /*    TickMap::iterator  it;
        pair <TickMap::iterator, TickMap::iterator> ret;

        ret = m_TickMap.equal_range(szStock);
        if ((ret.first == m_TickMap.end()) && (ret.second == m_TickMap.end()))
            return NULL;

        for (it = ret.first; it != ret.second; ++it) {
            return &m_pCommonTrade[it->second];  //  this is the data to be plotted
        }
    */
    return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
SFUNDAMENTAL_RECORD* CTickDataMap::GetFundamentalRecord(char* szStock)
{
// EXAMPLE Code to be copied in the calling Object....the return statement is the data to be used..............
// DO NOT USE THIS CODE HERE AS IS ... REFER TO statement ABOVE

    /*    FundamentalMap::iterator it;

        it = m_FundamentalMap.find(szStock);
        if ( it != m_FundamentalMap.end())
            return &it->second;  // This is the data to be plotted
    */
    return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
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



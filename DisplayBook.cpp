
#include "DisplayBook.h"

BOOK_THREAD_INFO 	CDisplayBook::m_arrThreadInfo[NUMBER_OF_BOOKS_TO_DISPALY];
BOOK_THREAD_DATA 	CDisplayBook::m_arrBookThreadData[NUMBER_OF_BOOKS_TO_DISPALY];

CBuildBook* 	CDisplayBook::m_pcBuildBook = NULL;

///////////////////////////////////////////////////////////////////
CDisplayBook::CDisplayBook(CBuildBook*  pCBuildBook)
{
  m_pcBuildBook  = pCBuildBook;
    m_pcUtil = NULL;
//    m_pcUtil = new CUtil(theApp.SSettings.szActiveSymbols);

    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {        // spin a thread for each Stock
        m_arrThreadInfo[ii].eState == _TS_INACTIVE;
    }
}
///////////////////////////////////////////////////////////////////
CDisplayBook::~CDisplayBook()
{
    if (m_pcUtil) {
        delete m_pcUtil;
        m_pcUtil = NULL;
    }
}
///////////////////////////////////////////////////////////////////
void CDisplayBook::DisplaySelected()
{

    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {        // spin a thread for each Stock

        if (!theApp.SSettings.arrbActive[ii]) { // Non active
            m_arrThreadInfo[ii].eState = _TS_INACTIVE;
            continue;
        }

        m_arrThreadInfo[ii].iThread_num = ii ;

        m_arrBookThreadData[ii].idx = ii;			// App assigned number
        m_arrBookThreadData[ii].nLevels = theApp.SSettings.iBookLevels; // copy number of levels
        strcpy(m_arrBookThreadData[ii].szSymbol, theApp.SSettings.szActiveSymbols[ii]);  // copy the symbol

        int iRet = pthread_create(&m_arrThreadInfo[ii].thread_id, NULL, &DisplaySingleBook, &m_arrBookThreadData[ii]);
    }

}
///////////////////////////////////////////////////////////////////
void CDisplayBook::StopDisplaySelected(char* szSymbol)
{

}
///////////////////////////////////////////////////////////////////
void CDisplayBook::StopDisplayAllBooks()
{
  

}
///////////////////////////////////////////////////////////////////
void* CDisplayBook::DisplaySingleBook(void* pArg)
{

    BOOK_THREAD_DATA SThreadData;

    SThreadData =  *((BOOK_THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    int nLevels = SThreadData.nLevels;

    string strToFind(SThreadData.szSymbol);

    SBOOK_LEVELS SBookLevels;
    SBID_ASK* pTemp;

    m_arrThreadInfo[idx].eState == _TS_ALIVE;

    NLEVELS Del;


    m_pcBuildBook->m_itBookMap = m_pcBuildBook->m_BookMap.find(strToFind);

    if (m_pcBuildBook->m_itBookMap == m_pcBuildBook->m_BookMap.end())
         return  NULL;

    while (theApp.SSettings.iStatus != STOPPED) {

        SBookLevels = m_pcBuildBook->m_itBookMap->second;

        cout << "Open: " 		<< SBookLevels.m_OHLC.dOpen 	<< endl;
        cout << "Close: " 		<< SBookLevels.m_OHLC.dClose 	<< endl;
        cout << "High: " 		<< SBookLevels.m_OHLC.dHigh 	<< endl;
        cout << "Low: " 		<< SBookLevels.m_OHLC.dLow 	<< endl;
        cout << "Last: " 		<< SBookLevels.m_OHLC.dLast 	<< endl;
        cout << "Last Volume: " 	<< SBookLevels.m_OHLC.uiVolume 	<< endl;

        cout << "VWAP: " 		<< SBookLevels.m_OHLC.dVWAP 		<< endl;
        cout << "Total Volume " 	<< SBookLevels.m_OHLC.uiTotalVolume	<< endl;
        cout << "Total Trades " 	<< SBookLevels.m_OHLC.uiTotalNumOfTrades<< endl;
        cout << "Last Tick " 	<< SBookLevels.m_OHLC.cTick 		<< endl;


        cout << "======================================================================================"<< endl;
        cout << "Bid Levels for: " << strToFind << endl;
        cout << "======================================================================================"<< endl;

        while (SBookLevels.pTopBid != NULL) {  // Print the Bid Levels
            cout << SBookLevels.pTopBid->dPrice << " " << SBookLevels.pTopBid->szMPID << " " << SBookLevels.pTopBid->uiQty << " "<< SBookLevels.pTopBid->uiNumOfOrders << endl;
            pTemp = SBookLevels.pTopBid;
            SBookLevels.pTopBid = pTemp->pNextBidAsk;
        }// while (SBookLevels.pTopBid != NULL) {

        cout << "======================================================================================"<< endl;
        cout << "Ask Levels for: " << strToFind << endl;
        cout << "======================================================================================"<< endl;

        while (SBookLevels.pTopAsk != NULL) {  // Print the Ask Levels
            cout<< SBookLevels.pTopAsk->dPrice << " "  << SBookLevels.pTopAsk->szMPID << " " << SBookLevels.pTopAsk->uiQty << " "<< SBookLevels.pTopAsk->uiNumOfOrders << endl;
            pTemp = SBookLevels.pTopAsk;
            SBookLevels.pTopAsk = pTemp->pNextBidAsk;
        }// while (SBookLevels.pTopAsk != NULL) {
        cout << endl;
        cout << endl;
        cout << endl;
        cout << endl;
    }
    
    m_arrThreadInfo[idx].eState = _TS_TERMINATED;
    
    return NULL;  // log later

}
///////////////////////////////////////////////////////////////////
int CDisplayBook::GetError()
{
    return 1;
}
///////////////////////////////////////////////////////////////////
char* CDisplayBook::GetErrorDescription()
{
    return NULL;
}
///////////////////////////////////////////////////////////////////

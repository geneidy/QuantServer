#include <iostream>
#include <sstream>
#include <iomanip>

#include "BuildBook.h"
#include "NQTV.h"
#include "Logger.h"

//BookMap  CBuildBook::m_BookMap;
//BookMap::iterator	CBuildBook::m_itBookMap;

CBuildBook::CBuildBook()
{
    m_iError = 0;
    m_pCOrdersMap = nullptr;

    m_pCOrdersMap = COrdersMap::instance();

    if (!m_pCOrdersMap) {
        m_iError = 100;
        Logger::instance().log("Build Book... Obtaining File Mapping Error", Logger::Error);
    }

    m_uiNextOrder = 0;
    m_request.tv_nsec = 100000000;   // 1/10 of a micro second
    m_iSizeOfBook = sizeof( SBOOK_LEVELS);

    m_Stats.uiLevelDeleted = 0;

    m_ui64NumRequest = 0;
}
////////////////////////////////////////////////////
CBuildBook::~CBuildBook()
{

    string  strMsg;
    strMsg.empty();

    Logger::instance().log("Build Book... Destructing....Started Flushing All Books...Please Wait", Logger::Info);
    FlushAllBooks();
    Logger::instance().log("Build Book...Destructing....Ended Flushing All Books", Logger::Info);

    strMsg = "Build Book...Size of Book Map: ";
    strMsg += to_string(m_BookMap.size());
    Logger::instance().log(strMsg, Logger::Info);

    Logger::instance().log("Build Book...Destructing...Started Clearing Book", Logger::Info);
    m_BookMap.clear();
    Logger::instance().log("Build Book...Destructing...Ended Clearing Book", Logger::Info);

    m_pCOrdersMap->iNInstance--;

}
////////////////////////////////////////////////////
int CBuildBook::BuildBookFromOrdersMap()  // Entry point for processing...Called from a while loop in Main.cpp
{

    while (theApp.SSettings.iStatus != STOPPED) {  // m_pCOrdersMap
        if (!m_pCOrdersMap)
            break;

        if ((theApp.SSettings.iSystemEventCode == 'M') || (theApp.SSettings.iSystemEventCode == 'E') || (theApp.SSettings.iSystemEventCode == 'C'))   { // set close
            CloseBook();
        }

        m_pCommonOrder = m_pCOrdersMap->GetMemoryMappedOrder(m_ui64NumRequest++);

        if (m_pCommonOrder == nullptr) {
            m_ui64NumRequest--;
            nanosleep (&m_request, &m_remain);  // sleep a 1/10 of a second
            continue;
        }

        m_dPrice = m_pCommonOrder->dPrice;
        m_uiQty = m_pCommonOrder->iShares;

        if ((m_dPrice == 0) || (m_uiQty == 0))
            return 0;

        m_iMessage = m_pCommonOrder->cMessageType;

        switch (m_iMessage) {
        case 'A': 	// Add orders No MPID
        case 'F': 	// Add orders
            ProcessAdd(m_iMessage); // just to put a break point
            break;
        case 'E':  	// Order executed
        case 'c':  	// Order executed  with price
        case 'X':  	// Order Cancel
	case 'D': 	// Order deleted
            ProcessDelete(m_iMessage);
            break;
        case 'U':
            ProcessReplace(m_iMessage);
            break;
        default:
	    break;
            //return 0;
        }
    }
    return 0;
}
////////////////////////////////////////////////////
int CBuildBook::InitLevelStats()
{
    return true;
}
/*///////////////////////////////////////////////////
int CBuildBook::ListBook(const char *szSymbol , uint32_t uiMaxLevels)
{

    return true;
}
*//////////////////////////////////////////////////////////////////////////////////////
void CBuildBook::ListBookStats()
{
    string strMsg;
    uint32_t uiCount = 0;

    strMsg.empty();

    uiCount = m_BookMap.size();

    strMsg = "Build Book...Book Map Size: ";
    strMsg += to_string(uiCount);

    Logger::instance().log(strMsg, Logger::Debug);

}
///////////////////////////////////////////////////
int CBuildBook::ProcessAdd(int iMessage)
{
    bool   bFound 	= false;
    bool   bMMFound 	= false;

    bool  bBidInserted 	= false;
    bool  bBidAddedQty 	= false;

    bool  bAskInserted 	= false;
    bool  bAskAddedQty 	= false;

    m_pBook.pTopBid = nullptr;
    m_pBook.pTopAsk = nullptr;

    lpInsert  	= nullptr;
    lpCurrent 	= nullptr;
    lpPrevious 	= nullptr;

    m_itBookMap = m_BookMap.find(m_pCommonOrder->szStock);
    if (m_itBookMap != m_BookMap.end()) {  // found
        m_pBook = m_itBookMap->second;  // Fetch the book for this stock
    } // else NOT found fill the m_pBook struct
    else {

        m_pBook.m_iAskLevels 	= 0;
        m_pBook.m_iBidLevels 	= 0;
        /*      m_pBook.pTopAsk     	= nullptr;
                m_pBook.pTopBid     	= nullptr;
        */	m_pBook.bUpdated = false;
//        pthread_mutexattr_init(&m_pBook.mtxAttr);
//        pthread_mutex_init(&m_pBook.mtxBidAsk, nullptr);
        InitOHLC();
    }
    memset(m_szMPID, '\0', SIZE_OF_MM);
    strcpy(m_szMPID, m_pCommonOrder->szMPID);

//    pthread_mutex_lock(&m_pBook.mtxBidAsk);
    if (m_pCommonOrder->cBuySell == 'B')  { // bid to buy
        // Case 1: A fresh bucket with nothing in
        if (m_pBook.pTopBid == nullptr) {  // Empty list
            lpInsert = AllocateNode(m_dPrice, m_uiQty);  // check for nullptr
            bBidInserted = true;
            m_pBook.m_iBidLevels++;
            m_pBook.pTopBid = lpInsert;
        }
        else {
            // Case 2: I have at least one node (i.e.  one price level)
            lpCurrent	=	m_pBook.pTopBid;
            lpPrevious	=	lpCurrent;
            bFound = false;
            while (lpCurrent != nullptr)
            {
                if (m_dPrice == lpCurrent->dPrice)     // price match found
                {
                    bMMFound = false;
                    lpMM = lpCurrent;
                    while ((lpMM) && (lpMM->dPrice == m_dPrice )) {  	// Price found
                        if (!strcmp(lpMM->szMPID, m_szMPID)) { 		// MM found    (3)
                            lpMM->uiQty += m_uiQty;  // update volume
                            bBidAddedQty = true;
                            lpMM->uiNumOfOrders++;
                            bMMFound = true;
                            bFound = true;
                            break;
                        } // if (!strcmp(lpMM->szMPID, m_szMPID)) { // MM found
                        lpPrevMM = lpMM;
                        lpMM = lpMM->pNextBidAsk;
                    }  // while ((lpMM) && (lpMM->dPrice == m_dPrice ))
                    // MM Not found at this price level...Add a new one
                    if ((!bMMFound)&& (m_dPrice == lpCurrent->dPrice)) { //  add a new MM at this price level
                        lpInsert = AllocateNode(m_dPrice, m_uiQty);  //   		(5)
                        bBidInserted = true;
                        bFound = true;
                        lpPrevMM->pNextBidAsk = lpInsert;
                        lpInsert->pNextBidAsk = lpMM;
                        m_pBook.m_iBidLevels++;
                    }//        if (!bMMFound) { //  add a new MM at this price level
                    break;
                } // if price match found
                if (m_dPrice > lpCurrent->dPrice) {   // new price level
                    lpInsert = AllocateNode(m_dPrice, m_uiQty);
                    bBidInserted = true;
                    bFound = true;
                    m_pBook.m_iBidLevels++;
                    if (lpPrevious == lpCurrent) { // insert at the top of the list 	(1)
                        m_pBook.pTopBid	= lpInsert;
                        lpInsert->pNextBidAsk	= lpCurrent;
                    }
                    else {
                        lpPrevious->pNextBidAsk = lpInsert;   // 		(2)
                        lpInsert->pNextBidAsk = lpCurrent;
                    }
                    break;
                } // New price level on top or middle of the list
                lpPrevious = lpCurrent;
                lpCurrent = lpCurrent->pNextBidAsk;
            }//while (lpCurrent != nullptr)
            // Case 3: New price level at the bottom of the list
            if ((lpCurrent == nullptr) && (!bFound)) {    					// (4)
                lpInsert = AllocateNode(m_dPrice, m_uiQty);
                bBidInserted = true;
                m_pBook.m_iBidLevels++;
                lpPrevious->pNextBidAsk = lpInsert;
                lpInsert->pNextBidAsk = lpCurrent; // technically a nullptr
            }//		if ((lpCurrent == nullptr) && (!bFound))
        } // else ....not an empty list
        if (m_itBookMap == m_BookMap.end()) {  // A Fresh Stock just in
            m_RetPair = m_BookMap.insert(pair<string, SBOOK_LEVELS> (m_pCommonOrder->szStock, m_pBook));
            if (!m_RetPair.second) {
                Logger::instance().log("Error Inserting in Book Map", Logger::Error);
            }
        }
        else { // Update an existing one in the Map
            m_itBookMap->second = m_pBook;
        }

    } //     if (m_pCommonOrder->cBuySell == 'B')  { // bid to buy
/////////////////////////////////////////////////////////////////////////////////////////////////
    else			// ask to sell
    {
        // Case 1: A fresh bucket with nothing in
        if (m_pBook.pTopAsk == nullptr) {  // Empty list
            lpInsert = AllocateNode(m_dPrice, m_uiQty);  // check for nullptr
            bAskInserted = true;
            m_pBook.m_iAskLevels++;
            m_pBook.pTopAsk = lpInsert;
        }
        else {
            // Case 2: I have at least one node (i.e.  one price level)
            lpCurrent	=	m_pBook.pTopAsk;
            lpPrevious	=	lpCurrent;
            bFound = false;
            while (lpCurrent != nullptr)
            {
                if (m_dPrice == lpCurrent->dPrice)     // price match found
                {
                    bMMFound = false;
                    lpMM = lpCurrent;
                    while ((lpMM) && (lpMM->dPrice == m_dPrice )) {  	// Price found
                        if (!strcmp(lpMM->szMPID, m_szMPID)) { 		// MM found    (3)
                            lpMM->uiQty += m_uiQty;  // update volume
                            bAskAddedQty = true;
                            lpMM->uiNumOfOrders++;
                            bMMFound = true;
                            bFound = true;
                            break;
                        } // if (!strcmp(lpMM->szMPID, m_szMPID)) { // MM found
                        lpPrevMM = lpMM;
                        lpMM = lpMM->pNextBidAsk;
                    }  // while ((lpMM) && (lpMM->dPrice == m_dPrice ))
                    // MM Not found at this price level...Add a new one
                    if ((!bMMFound)&& (m_dPrice == lpCurrent->dPrice)) { //  add a new MM at this price level
                        lpInsert = AllocateNode(m_dPrice, m_uiQty);  //   		(5)
                        bAskInserted = true;
                        bFound = true;
                        lpPrevMM->pNextBidAsk = lpInsert;
                        lpInsert->pNextBidAsk = lpMM;
                        m_pBook.m_iAskLevels++;
                    }//        if (!bMMFound) { //  add a new MM at this price level
                    break;
                } // if price match found
                if (m_dPrice < lpCurrent->dPrice) {   // new price level
                    lpInsert = AllocateNode(m_dPrice, m_uiQty);
                    bAskInserted = true;
                    bFound = true;
                    m_pBook.m_iAskLevels++;

                    if (lpPrevious == lpCurrent) { // insert at the top of the list 	(1)
                        m_pBook.pTopAsk	= lpInsert;
                        lpInsert->pNextBidAsk	= lpCurrent;
                    }
                    else {
                        lpPrevious->pNextBidAsk = lpInsert;   // 		(2)
                        lpInsert->pNextBidAsk = lpCurrent;
                    }
                    break;
                } // New price level on top or middle of the list
                lpPrevious = lpCurrent;
                lpCurrent = lpCurrent->pNextBidAsk;
            }//while (lpCurrent != nullptr)
            // Case 3: New price level at the bottom of the list
            if ((lpCurrent == nullptr) && (!bFound)) {    					// (4)
                lpInsert = AllocateNode(m_dPrice, m_uiQty);
                bAskInserted = true;
                m_pBook.m_iAskLevels++;
                lpPrevious->pNextBidAsk = lpInsert;
                lpInsert->pNextBidAsk = lpCurrent; // technically a nullptr
            }//		if ((lpCurrent == nullptr) && (!bFound))
        } // else ....not an empty list
        if (m_itBookMap == m_BookMap.end()) {  // A Fresh Stock just in
            m_RetPair = m_BookMap.insert(pair<string, SBOOK_LEVELS> (m_pCommonOrder->szStock, m_pBook));
            if (!m_RetPair.second) {
                Logger::instance().log("Error Inserting in Book Map", Logger::Error);
            }
        }
        else { // Update an existing one in the Map
            m_itBookMap->second = m_pBook;
        }

    }// else
    m_pBook.bUpdated = true;
//    pthread_mutex_unlock(&m_pBook.mtxBidAsk);

    if ((!bBidAddedQty) && (!bBidInserted) && (!bAskAddedQty) && (!bAskInserted)) {
        int iError = true;
    }
//     ListBook("MSFT    ");
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
int CBuildBook::ProcessDelete(int iIn)
{
    lpCurrent = nullptr;
    lpPrevious = nullptr;

    bool bFound = false;

    m_strMsg.empty();

    m_itBookMap = m_BookMap.find(m_pCommonOrder->szStock);

    if (m_itBookMap != m_BookMap.end()) {  // found
        m_pBook = m_itBookMap->second;  // Fetch the book for this stock
//        pthread_mutex_lock(&m_pBook.mtxBidAsk);

        if ((m_iMessage == 'E' ) || (m_iMessage == 'c' )) {  	// Order executed
            m_pBook.m_OHLC.dLast 	= m_dPrice;
            if ((theApp.SSettings.iSystemEventCode == 'Q') && (m_pBook.m_OHLC.dOpen == 0))  {
                m_pBook.m_OHLC.dOpen 	= m_dPrice;
                gettimeofday(&m_pBook.m_OHLC.tOpen, nullptr);
            }

            if (m_pBook.m_OHLC.dHigh < m_pBook.m_OHLC.dLast) { // set dHigh
                m_pBook.m_OHLC.dHigh = m_pBook.m_OHLC.dLast;
            }
            if (m_pBook.m_OHLC.dLow > m_pBook.m_OHLC.dLast) {   // set dLow
                m_pBook.m_OHLC.dLow = m_pBook.m_OHLC.dLast;
            }

            if (m_pBook.m_OHLC.dLast < m_dPrice) { // Set the ticks
                m_pBook.m_OHLC.cTick = '+';
            }
            if (m_pBook.m_OHLC.dLast > m_dPrice) {
                m_pBook.m_OHLC.cTick = '-';
            }
            if (m_pBook.m_OHLC.dLast == m_dPrice) {
                m_pBook.m_OHLC.cTick = '=';
            }

            m_pBook.m_OHLC.uiTotalNumOfTrades++;
            m_pBook.m_OHLC.uiVolume       	= m_uiQty;
            m_pBook.m_OHLC.uiTotalVolume 	+= m_uiQty;
            m_pBook.m_OHLC.dVWAP += (m_dPrice * m_uiQty)/m_pBook.m_OHLC.uiTotalVolume;

            gettimeofday(&m_pBook.m_OHLC.tLastUpdate, nullptr);
        } // if ((m_iMessage == 'E' ) || (m_iMessage == 'c' )) {  	// Order executed
    } // found
    else { // else Error....should not happen
        m_strMsg = "Build Book...Error Deleting Order in Book..Symbol: ";
        m_strMsg += m_pCommonOrder->szStock;
        m_strMsg += " Market Maker: ";
        m_strMsg += m_pCommonOrder->szMPID;
        m_strMsg += " Price = ";
        m_strMsg += to_string(m_dPrice);
        m_strMsg += " Qty = ";
        m_strMsg += to_string(m_uiQty);
        m_strMsg += " Order Ref. Number";
        m_strMsg += m_pCommonOrder->iOrderRefNumber;
        m_strMsg += " Not Found";
        Logger::instance().log(m_strMsg, Logger::Error);
//        pthread_mutex_unlock(&m_pBook.mtxBidAsk);

        return 0;
    }

    memset(m_szMPID, '\0', SIZE_OF_MM);
    strcpy(m_szMPID, m_pCommonOrder->szMPID);

    if (m_pCommonOrder->cBuySell == 'B') { // bid to buy
        lpCurrent	=	m_pBook.pTopBid;
        lpPrevious	=	lpCurrent;
        bFound = false;
        while (lpCurrent != nullptr) {
            if ((m_dPrice == lpCurrent->dPrice) &&  (!strcmp(lpCurrent->szMPID, m_szMPID))) {  // price match found
                lpCurrent->uiQty -= m_uiQty;  // update volume
                lpCurrent->uiNumOfOrders--;
                bFound = true;
                break;
            } // if price match found

            if (m_dPrice > lpCurrent->dPrice) { // should not happen
                break;
            }
            lpPrevious = lpCurrent;
            lpCurrent = lpCurrent->pNextBidAsk;

        } // while (lpCurrent != nullptr)

        if ((lpCurrent == nullptr) && (!bFound)) {
//            pthread_mutex_unlock(&m_pBook.mtxBidAsk);
//            m_itBookMap->second = m_pBook;
            return -1; // enum later  ... Should Not Happen
        }

        if ((bFound) && (lpCurrent->uiQty <= 0)) { // Remove this node
            if (lpCurrent   ==	m_pBook.pTopBid)  { // first node
                m_pBook.pTopBid = lpCurrent->pNextBidAsk;
//                m_itBookMap->second = m_pBook;
            }
            else {
                lpPrevious->pNextBidAsk = lpCurrent->pNextBidAsk;
            }
            delete lpCurrent;
            lpCurrent = nullptr;
            m_Stats.uiLevelDeleted++;
        }

        m_pBook.bUpdated = true;
//        pthread_mutex_unlock(&m_pBook.mtxBidAsk);
        m_itBookMap->second = m_pBook;
    } // if (m_pCommonOrder->cBuySell == 'B')  // bid to buy
/////////////////////////////////////////////////////////////////////////////////
    if (m_pCommonOrder->cBuySell == 'S') {  // Ask to sell
        lpCurrent	=	m_pBook.pTopAsk;
        lpPrevious	=	lpCurrent;
        bFound = false;
        while (lpCurrent != nullptr) {
            if ((m_dPrice == lpCurrent->dPrice) &&  (!strcmp(lpCurrent->szMPID, m_szMPID))) {   // price match found
                lpCurrent->uiQty -= m_uiQty;  // update volume
                lpCurrent->uiNumOfOrders--;
                bFound = true;
                break;
            } // if price match found

            if (m_dPrice < lpCurrent->dPrice) { // should not happen
                break;
            }
            lpPrevious = lpCurrent;
            lpCurrent = lpCurrent->pNextBidAsk;


        } // while (lpCurrent != nullptr)
        if ((lpCurrent == nullptr) && (!bFound)) {

//            pthread_mutex_unlock(&m_pBook.mtxBidAsk);
//            m_itBookMap->second = m_pBook;
            return -1; // enum later
        }
        if ((bFound) && (lpCurrent->uiQty <= 0)) { // Remove this node
            if (lpCurrent   ==	m_pBook.pTopAsk)  { // first node
//                m_pBook.pTopAsk = nullptr;
                m_pBook.pTopAsk = lpCurrent->pNextBidAsk;
//                m_itBookMap->second = m_pBook;
            }
            else {
                lpPrevious->pNextBidAsk = lpCurrent->pNextBidAsk;
            }
            delete lpCurrent;
            lpCurrent = nullptr;
            m_Stats.uiLevelDeleted++;
        }
        m_pBook.bUpdated = true;
//        pthread_mutex_unlock(&m_pBook.mtxBidAsk);
        m_itBookMap->second = m_pBook;

    } // if (m_pCommonOrder->cBuySell == 'S')  // ask to sell
//    ListBook("MSFT    ");
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
int CBuildBook::ProcessReplace(int iIn)
{

    m_dPrice = m_pCommonOrder->dPrevPrice;// previous price
    m_uiQty = m_pCommonOrder->iPrevShares; // previous shares

    ProcessDelete(10);

    m_dPrice = m_pCommonOrder->dPrice;  // new price
    m_uiQty = m_pCommonOrder->iShares;  // new shares

    ProcessAdd(10);

    return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
SBID_ASK* CBuildBook::AllocateNode(double dPrice, unsigned int uiQty)
{
    SBID_ASK*		lpNewNode = nullptr;

    lpNewNode			= new(SBID_ASK);
    if (!lpNewNode) {
        Logger::instance().log("Build Book...Error Allocating Memory", Logger::Error);
        return nullptr;
    }
    lpNewNode->uiQty		= uiQty;
    lpNewNode->dPrice		= dPrice;
    strncpy(lpNewNode->szMPID, m_szMPID, SIZE_OF_MM);

    lpNewNode->uiNumOfOrders	= 1;
    lpNewNode->pNextBidAsk	= nullptr;
    /*	lpNewNode->SLevelStat.uiAttribAdd 	= 0;
    	lpNewNode->SLevelStat.uiCancelled 	= 0;
    	lpNewNode->SLevelStat.uiDeleted   	= 0;
    	lpNewNode->SLevelStat.uiExecuted  	= 0;
    	lpNewNode->SLevelStat.uiNonAttribAdd 	= 0;
    	lpNewNode->SLevelStat.uiReplaced 	= 0;
    */
    return lpNewNode;
}

///////////////////////////////////////////////////////////////////////////////////////
void CBuildBook::InitOHLC()
{
    m_pBook.m_OHLC.dLast 	= 0;
    m_pBook.m_OHLC.dOpen 	= 0;
    m_pBook.m_OHLC.dHigh 	= 0;
    m_pBook.m_OHLC.dLow 	= 0;

    m_pBook.m_OHLC.uiTotalNumOfTrades 	= 0;
    m_pBook.m_OHLC.uiVolume       	= 0;
    m_pBook.m_OHLC.uiTotalVolume 	= 0;

    m_pBook.m_OHLC.cTick = '=';  // '+'  '-'  '='
    m_pBook.m_OHLC.dVWAP = 0;

    gettimeofday(&m_pBook.m_OHLC.tOpen, nullptr);
    gettimeofday(&m_pBook.m_OHLC.tLastUpdate, nullptr);

}
///////////////////////////////////////////////////////////////////////////////////////
void CBuildBook::CloseBook()
{
    for (m_itBookMap = m_BookMap.begin(); m_itBookMap != m_BookMap.end(); ++m_itBookMap) {
        m_itBookMap->second.m_OHLC.dClose 	= m_itBookMap->second.m_OHLC.dLast;
    }
}
///////////////////////////////////////////////////////////////////////////////////////
string CBuildBook::MakeKey() // NOT needed in this version
{

    char  strOut[20];
    memset(strOut, '\0', 20);

//    std::ostringstream strstrm;
//    strstrm << setprecision(4) << m_pCommonOrder->dPrice << "+" << m_pCommonOrder->szMPID;
    string strstrm;
    strstrm.empty();

    strstrm = to_string( m_pCommonOrder->dPrice);
    strstrm +=  "+";
    strstrm += m_pCommonOrder->szMPID;

    strncpy(strOut, strstrm.c_str(), 20);  //  for debug only ... throw away code
    return strstrm;

}
//////////////////////////////////////////////////////////////////////////////////////////
NLEVELS  CBuildBook::ListBook(char* szSymbol, int nLevels )
{
    SBOOK_LEVELS SBookLevels;
    SBID_ASK* pTemp;

    string strToFind(szSymbol);

    NLEVELS SLevels;

    SLevels = {0};

    m_itBookMap = m_BookMap.find(strToFind);

    if (m_itBookMap == m_BookMap.end())
        return  SLevels;

    SBookLevels = m_itBookMap->second;
    /*
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
    */

    cout << "======================================================================================"<< endl;
    cout << "Bid Levels for: " << szSymbol << endl;
    cout << "======================================================================================"<< endl;

    while (SBookLevels.pTopBid != nullptr) {  // Print the Bid Levels
        cout << SBookLevels.pTopBid->dPrice << " " << SBookLevels.pTopBid->szMPID << " " << SBookLevels.pTopBid->uiQty << " "<< SBookLevels.pTopBid->uiNumOfOrders << endl;
        pTemp = SBookLevels.pTopBid;
        SBookLevels.pTopBid = pTemp->pNextBidAsk;
        if (++SLevels.iBidLevels > nLevels)
	  break;
    }// while (SBookLevels.pTopBid != nullptr) {

    cout << endl << endl;
    cout << "Bid Levels: " << SLevels.iBidLevels << endl;

    cout << "======================================================================================"<< endl;
    cout << "Ask Levels for: " << szSymbol << endl;
    cout << "======================================================================================"<< endl;

    SBookLevels = m_itBookMap->second;

    while (SBookLevels.pTopAsk != nullptr) {  // Print the Ask Levels
        cout<< SBookLevels.pTopAsk->dPrice << " "  << SBookLevels.pTopAsk->szMPID << " " << SBookLevels.pTopAsk->uiQty << " "<< SBookLevels.pTopAsk->uiNumOfOrders << endl;
        pTemp = SBookLevels.pTopAsk;
        SBookLevels.pTopAsk = pTemp->pNextBidAsk;
        if (++SLevels.iAskLevels > nLevels)
	  break;
    }// while (SBookLevels.pTopAsk != nullptr) {
    
    cout << endl << endl;
    cout << "Ask Levels: " << SLevels.iAskLevels << endl;
    cout << endl;
    cout << endl;
    cout << endl;

    return SLevels;  // log later
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
NLEVELS  CBuildBook::FlushBook(char* szSymbol )
{
    SBOOK_LEVELS SBookLevels;
    SBID_ASK* pTemp;

    NLEVELS Del;

    Del.iAskLevels = 0;
    Del.iBidLevels = 0;

    m_itBookMap = m_BookMap.find(szSymbol);

    if (m_itBookMap == m_BookMap.end())
        return  Del;

    SBookLevels = m_itBookMap->second;

//    pthread_mutexattr_destroy(&SBookLevels.mtxAttr);
//    pthread_mutex_destroy(&SBookLevels.mtxBidAsk);

    while (SBookLevels.pTopAsk != nullptr) {  // Clear the Ask Levels
        pTemp = SBookLevels.pTopAsk;
        SBookLevels.pTopAsk = pTemp->pNextBidAsk;
        delete pTemp;
        Del.iAskLevels++;

    }// while (SBookLevels.pTopAsk != nullptr) {

    while (SBookLevels.pTopBid != nullptr) {  // Clear the Bid Levels
        pTemp = SBookLevels.pTopBid;
        SBookLevels.pTopBid = pTemp->pNextBidAsk;
        delete pTemp;
        Del.iBidLevels++;

    }// while (SBookLevels.pTopBid != nullptr) {

    return Del;  // log later
}
///////////////////////////////////////////////////////////////////////////////////////
NLEVELS CBuildBook::FlushAllBooks()
{
    SBOOK_LEVELS SBookLevels;
    SBID_ASK* pTemp;

    NLEVELS Del;

    Del.iAskLevels = 0;
    Del.iBidLevels = 0;

    string strMsg;

    char  szStock[SIZE_OF_SYMBOL];
    memset(szStock, '\0', SIZE_OF_SYMBOL);

    for (m_itBookMap = m_BookMap.begin(); m_itBookMap != m_BookMap.end(); ++m_itBookMap)
    {
        SBookLevels = m_itBookMap->second;
//        strncpy(szStock, m_itBookMap->first.c_str(), 8);
        strcpy(szStock, m_itBookMap->first.c_str());

        while (SBookLevels.pTopAsk != nullptr) {  // Clear the Ask Levels
            pTemp = SBookLevels.pTopAsk;
            SBookLevels.pTopAsk = pTemp->pNextBidAsk;
            Del.iAskLevels++;
            delete pTemp;
        }// while (SBookLevels.pTopAsk != nullptr) {

        while (SBookLevels.pTopBid != nullptr) {  // Clear the Bid Levels
            pTemp = SBookLevels.pTopBid;
            SBookLevels.pTopBid = pTemp->pNextBidAsk;
            Del.iBidLevels++;
            delete pTemp;
        }// while (SBookLevels.pTopAsk != nullptr) {
        memset(szStock, '\0', SIZE_OF_SYMBOL);
//        pthread_mutexattr_destroy(&SBookLevels.mtxAttr);
//        pthread_mutex_destroy(&SBookLevels.mtxBidAsk);

    };

    strMsg.empty();
    strMsg = "Build Book...Deleted Bid Levels: ";
    strMsg += to_string(Del.iBidLevels);
    Logger::instance().log(strMsg, Logger::Info);

    strMsg.empty();
    strMsg = "Build Book...Deleted Ask Levels: ";
    strMsg += to_string(Del.iAskLevels);
    Logger::instance().log(strMsg, Logger::Info);

    return Del;  // log later
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

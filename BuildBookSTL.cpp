#include <iostream>
#include <sstream>
#include <iomanip>

#include "BuildBookSTL.h"
#include "NQTV.h"
#include "Logger.h"

CBuildBookSTL::CBuildBookSTL()
{

    m_iError = 0;
    m_pCOrdersMap = NULL;

    m_pCOrdersMap = COrdersMap::instance();

    if (!m_pCOrdersMap) {
        m_iError = 100;
        Logger::instance().log("Obtaining File Mapping Error in Build Book", Logger::Error);
    }
    m_ui64NumRequest = 0;
    
    m_uiNextOrder = 0;
    m_request.tv_nsec = 100000000;   // 1/10 of a second
    m_iSizeOfBook = sizeof( SBOOK_LEVELS);
    
    m_BookMap.clear();

}
////////////////////////////////////////////////////
CBuildBookSTL::~CBuildBookSTL()
{

//    ListBookStats();

    if (m_pCOrdersMap)
        m_pCOrdersMap->iNInstance--;

    Logger::instance().log("Build Book...Destructing Started Flushing All Books", Logger::Info);

    FlushAllBooks();

    Logger::instance().log("Build Book...Destructing....Ended Flushing All Books", Logger::Info);
    Logger::instance().log("Build Book...Destructing...Started Clearing Book Map", Logger::Info);

    m_BookMap.clear();
    Logger::instance().log("Build Book...Destructing...Ended Clearing Book Map", Logger::Info);

}
////////////////////////////////////////////////////
int CBuildBookSTL::BuildBookFromOrderMap()  // Entry point for processing...Called from a while loop in Main.cpp
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
string CBuildBookSTL::MakeKey()
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

    strncpy(strOut, strstrm.c_str(), 19);  //  for debug only ... throw away code

    return strstrm;

}
////////////////////////////////////////////////////
int CBuildBookSTL::ProcessAdd(int iMessage)
{

    m_dPrice = m_pCommonOrder->dPrice;
    m_uiQty = m_pCommonOrder->iShares;

    strcpy(m_SBidAsk.szMPID, m_pCommonOrder->szMPID);

    m_strPriceMM.empty();
    m_strPriceMM = MakeKey();

    if (m_pCommonOrder->cBuySell == 'B') { // bid to buy
        //Find Book Entry for this stock
        m_itBookMap = m_BookMap.find(m_pCommonOrder->szStock);
        if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
            // search for price level
            m_itPriceLevelMap = m_itBookMap->second.BidPLMap.find(m_strPriceMM);

            if ( m_itPriceLevelMap != m_itBookMap->second.BidPLMap.end()) { // Price level found....Update it
                m_itPriceLevelMap->second.uiQty += m_uiQty;
                m_itPriceLevelMap->second.uiNumOfOrders++;
/*                if (strcmp(m_SBidAsk.szMPID, "NSDQ"))
                    m_itPriceLevelMap->second.SLevelStat.uiAttribAdd++;
                else
                    m_itPriceLevelMap->second.SLevelStat.uiNonAttribAdd++;
*/            }
            else { // Price level not found...Add it
                AddPriceLevel('B');
                m_itBookMap->second.m_iBidLevels++;
            }// if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
        } // if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
        else { // first time entry in the Book
            // Make a book Entry
            m_pBookLevels.m_iBidLevels = 1;
            m_RetPairBookMap = m_BookMap.insert(pair<string , SBOOK_LEVELS>(m_pCommonOrder->szStock, m_pBookLevels) );
            // Make a level Entry
            m_itBookMap = m_RetPairBookMap.first;
            AddPriceLevel('B');

        }// else { // first time entry in the Book
    }
///////
    else { // Ask to sell
        //Find Book Entry for this stock
        m_itBookMap = m_BookMap.find(m_pCommonOrder->szStock);
        if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
            // search for price level
            m_itPriceLevelMap = m_itBookMap->second.AskPLMap.find(m_strPriceMM);

            if ( m_itPriceLevelMap != m_itBookMap->second.AskPLMap.end()) { // Price level found....Update it
                m_itPriceLevelMap->second.uiQty += m_uiQty;
                m_itPriceLevelMap->second.uiNumOfOrders++;
/*                if (strcmp(m_SBidAsk.szMPID, "NSDQ"))
                    m_itPriceLevelMap->second.SLevelStat.uiAttribAdd++;
                else
                    m_itPriceLevelMap->second.SLevelStat.uiNonAttribAdd++;
 */           }
            else { // Price level not found...Add it
                AddPriceLevel('A');
                m_itBookMap->second.m_iAskLevels++;
            }// if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
        } // if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
        else { // first time entry in the Book
            // Make a book Entry
            m_pBookLevels.m_iAskLevels = 1;
            m_RetPairBookMap = m_BookMap.insert(pair<string , SBOOK_LEVELS>(m_pCommonOrder->szStock, m_pBookLevels) );
            // Make a level Entry
            m_itBookMap = m_RetPairBookMap.first;
            AddPriceLevel('A');
        }// else { // first time entry in the Book
    } //   else { // Ask to sell
///////
    return 1;
}
////////////////////////////////////////////////////
bool CBuildBookSTL::AddPriceLevel(int iSide)
{
    InitLevelStats();

    m_SBidAsk.dPrice = m_dPrice;
    m_SBidAsk.uiQty = m_uiQty;
    m_SBidAsk.uiNumOfOrders = 1;

/*
    if (strcmp(m_SBidAsk.szMPID, "NSDQ"))
        m_SBidAsk.SLevelStat.uiAttribAdd = 1;
    else
        m_SBidAsk.SLevelStat.uiNonAttribAdd = 1;*/

    if (iSide == 'B')
        m_RetPairPriceLevelMap = m_itBookMap->second.BidPLMap.insert(pair<string /*Price+MM */, SBID_ASK >(m_strPriceMM, m_SBidAsk));
    else
        m_RetPairPriceLevelMap = m_itBookMap->second.AskPLMap.insert(pair<string /*Price+MM */, SBID_ASK >(m_strPriceMM, m_SBidAsk));

    //log error if any
    if (!m_RetPairPriceLevelMap.second) {
        Logger::instance().log("Error creating book level", Logger::Error);
        //
    }
    return m_RetPairPriceLevelMap.second;  //  a Bool on operation success or failure
}
////////////////////////////////////////////////////
int CBuildBookSTL::InitLevelStats()
{
    m_SBidAsk.uiNumOfOrders 	= 1;
    m_SBidAsk.uiQty 		= 0;
    m_SBidAsk.dPrice 		= 0;
/*
    m_SBidAsk.SLevelStat.uiAttribAdd = 	0;
    m_SBidAsk.SLevelStat.uiNonAttribAdd = 0;

    m_SBidAsk.SLevelStat.uiCancelled = 	0;
    m_SBidAsk.SLevelStat.uiDeleted = 	0;
    m_SBidAsk.SLevelStat.uiExecuted = 	0;
    m_SBidAsk.SLevelStat.uiReplaced = 	0;
*/
    return true;
}
////////////////////////////////////////////////////
int CBuildBookSTL::ProcessReplace(int iMessage)
{
    m_dPrice = m_pCommonOrder->dPrevPrice;
    m_uiQty = m_pCommonOrder->iPrevShares;

    ProcessDelete(10);

    m_dPrice = m_pCommonOrder->dPrice;
    m_uiQty = m_pCommonOrder->iShares;

    ProcessAdd(10);

//    m_itPriceLevelMap->second.SLevelStat.uiReplaced++;

    return 1;
}
////////////////////////////////////////////////////
int CBuildBookSTL::ProcessDelete(int iMessage)
{

    m_dPrice = m_pCommonOrder->dPrice;
    m_uiQty = m_pCommonOrder->iShares;

    strcpy(m_SBidAsk.szMPID, m_pCommonOrder->szMPID);

    m_strPriceMM.empty();
    m_strPriceMM = MakeKey();

    if (m_pCommonOrder->cBuySell == 'B') { // bid to buy
        //Find Book Entry for this stock
        m_itBookMap = m_BookMap.find(m_pCommonOrder->szStock);
        if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
            // search for price level
            m_itPriceLevelMap = m_itBookMap->second.BidPLMap.find(m_strPriceMM);

            if ( m_itPriceLevelMap != m_itBookMap->second.BidPLMap.end()) { // Price level found....Update it
                UpdatePriceLevel('B');
            }
            else { // Price level not found...Log error
                Logger::instance().log("Book level Not found in Delete book", Logger::Debug);
            }
        }//       if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
        else { // No book entry...should not happen
            Logger::instance().log("No Book Entry found in Delete book", Logger::Debug);
        }
    }//   if (m_pCommonOrder->cBuySell == 'B'){  // bid to buy
    else {  // ask to sell
        //Find Book Entry for this stock
        m_itBookMap = m_BookMap.find(m_pCommonOrder->szStock);
        if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
            // search for price level
            m_itPriceLevelMap = m_itBookMap->second.AskPLMap.find(m_strPriceMM);

            if ( m_itPriceLevelMap != m_itBookMap->second.AskPLMap.end()) { // Price level found....Update it
                UpdatePriceLevel('A');
            }
            else { // Price level not found...Log error
                Logger::instance().log("Book level Not found in Delete book", Logger::Debug);
            }
        }//       if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
        else { // No book entry...should not happen
            Logger::instance().log("No Book Entry found in Delete book", Logger::Debug);
        }
    }//   if (m_pCommonOrder->cBuySell == 'A'){  // bid to buy

    return 1;
}
///////////////////////////////////////////////////
bool CBuildBookSTL::UpdatePriceLevel(int iSide)
{
    m_itPriceLevelMap->second.uiQty -= m_uiQty;

    if (m_itPriceLevelMap->second.uiQty <= 0) { // Update number of levels
        if (iSide == 'B')
            m_itBookMap->second.m_iBidLevels--;
        else
            m_itBookMap->second.m_iAskLevels--;

        m_PriceLevelMap.erase(m_strPriceMM);      // remove level
        return true;
    }

    m_itPriceLevelMap->second.uiNumOfOrders--;
    /*
        switch (m_iMessage) {
        case 'E':  // Order executed
        case 'c':  // Order executed  with price
            m_itPriceLevelMap->second.SLevelStat.uiExecuted--;
            break;
        case 'X':  // Order Cancel
            m_itPriceLevelMap->second.SLevelStat.uiCancelled--;
            break;
        case 'D': // Order deleted
            m_itPriceLevelMap->second.SLevelStat.uiDeleted--;
            break;
        }
    */
    return true;
}
////////////////////////////////////////////////////
int CBuildBookSTL::ListBook(const char *szSymbol , uint32_t uiMaxLevels)
{

   BookMap::iterator	itBookMap;

    PriceLevelMap::reverse_iterator  itBidPriceLevel;
    PriceLevelMap::iterator itAskPriceLevel;

    itBookMap = m_BookMap.find(szSymbol);

    if (itBookMap == m_BookMap.end()) {
        return 0;
        // Log...no entry for such Stock
    }
    
//    string strToFind(szSymbol);

    uint32_t uiLevels = 0;
    

    cout << "======================================================================================"<< endl;
    cout << "Bid Levels for: " << szSymbol << endl;
    cout << "======================================================================================"<< endl;
    

    for ( itBidPriceLevel = itBookMap->second.BidPLMap.rbegin();  itBidPriceLevel != itBookMap->second.BidPLMap.rend(); ++ itBidPriceLevel)
    {
        if (uiLevels++ > uiMaxLevels)
            break;
	
        cout<<  itBidPriceLevel->second.dPrice << " ";
	cout<<  itBidPriceLevel->second.szMPID << " ";
        cout<<  itBidPriceLevel->second.uiQty  << " ";
        cout<<  itBidPriceLevel->second.uiNumOfOrders << endl;
/*
        cout<<  itBidPriceLevel->second.SLevelStat.uiAttribAdd;
        cout<<  itBidPriceLevel->second.SLevelStat.uiNonAttribAdd;
        cout<<  itBidPriceLevel->second.SLevelStat.uiCancelled;
        cout<<  itBidPriceLevel->second.SLevelStat.uiReplaced;
        cout<<  itBidPriceLevel->second.SLevelStat.uiDeleted;
        cout<<  itBidPriceLevel->second.SLevelStat.uiExecuted;*/

    }

    cout << endl << endl;
    cout << "Bid Levels: " << uiLevels << endl;
    
    itBookMap = m_BookMap.find(szSymbol);
    
    uiLevels = 0;

    cout << "======================================================================================"<< endl;
    cout << "Ask Levels for: " << szSymbol << endl;
    cout << "======================================================================================"<< endl;
    
    
    for (itAskPriceLevel = itBookMap->second.AskPLMap.begin(); itAskPriceLevel != itBookMap->second.AskPLMap.end(); ++itAskPriceLevel)
//    for (auto itAskPriceLevel = itBookMap->second.AskPLMap.rbegin(); itAskPriceLevel != itBookMap->second.AskPLMap.rend(); ++itAskPriceLevel)    
    {
        if (uiLevels++ > uiMaxLevels)
            break;

        cout<< itAskPriceLevel->second.dPrice << " ";
        cout<< itAskPriceLevel->second.szMPID << " ";	
        cout<< itAskPriceLevel->second.uiQty  << " ";
        cout<< itAskPriceLevel->second.uiNumOfOrders << endl;
/*
        cout<< itAskPriceLevel->second.SLevelStat.uiAttribAdd;
        cout<< itAskPriceLevel->second.SLevelStat.uiNonAttribAdd;
        cout<< itAskPriceLevel->second.SLevelStat.uiCancelled;
        cout<< itAskPriceLevel->second.SLevelStat.uiReplaced;
        cout<< itAskPriceLevel->second.SLevelStat.uiDeleted;
        cout<< itAskPriceLevel->second.SLevelStat.uiExecuted;*/
    }

    cout << endl << endl;
    cout << "Ask Levels: " << uiLevels << endl;
    
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////
uint32_t  CBuildBookSTL::FlushBook(char* szSymbol )
{

    m_itBookMap = m_BookMap.find(szSymbol);

    if (m_itBookMap == m_BookMap.end()) {
        return 0;
        // Log...no entry for such Stock
    }

    m_itBookMap->second.BidPLMap.clear();
    m_itBookMap->second.AskPLMap.clear();

    return 1; // ::TODO log
}
//////////////////////////////////////////////////////////////////////////////////////
uint64_t CBuildBookSTL::FlushAllBooks()
{

    string strMsg;
    uint32_t uiCount = 0;

    strMsg.clear();

    uiCount = m_BookMap.size();

    strMsg = "Book Map Size: ";
    strMsg += to_string(uiCount);

    Logger::instance().log(strMsg, Logger::Debug);

    strMsg.clear();
    strMsg = "Flushing All Books: ";
    Logger::instance().log(strMsg, Logger::Debug);
    
    uint64_t uiNLevels = 0;

    for (m_itBookMap = m_BookMap.begin(); m_itBookMap != m_BookMap.end(); ++m_itBookMap)
    {
        m_itBookMap->second.BidPLMap.clear();
        m_itBookMap->second.AskPLMap.clear();

        uiNLevels++;
    }
    
    strMsg.clear();
    strMsg = "Flushed: ";
    strMsg += to_string(--uiNLevels);
    strMsg += " Books";
    Logger::instance().log(strMsg, Logger::Debug);
    
    // Be carefull will cause a crach because the Bid and Ask Maps per Symbol are cleared
/*
    strMsg += "  Bid Levels: ";
    strMsg += to_string(m_itBookMap->second.BidPLMap.size());
    Logger::instance().log("--------------------------- ", Logger::Debug);
    
    strMsg.clear();
    strMsg += "  Ask Levels: ";
    strMsg += to_string(m_itBookMap->second.AskPLMap.size());
    Logger::instance().log(strMsg, Logger::Debug);
    */

    return uiNLevels;
}
//////////////////////////////////////////////////////////////////////////////////////
void CBuildBookSTL::ListBookStats()
{
    string strMsg;
    uint32_t uiCount = 0;

    strMsg.empty();

    uiCount = m_BookMap.size();

    strMsg = "Book Map Size: ";
    strMsg += to_string(uiCount);

    Logger::instance().log(strMsg, Logger::Debug);

}
///////////////////////////////////////////////////////////////////////////////////////
void CBuildBookSTL::CloseBook()
{
  /*
    for (m_itBookMap = m_BookMap.begin(); m_itBookMap != m_BookMap.end(); ++m_itBookMap) {
        m_itBookMap->second.m_OHLC.dClose 	= m_itBookMap->second.m_OHLC.dLast;
    }*/
}
///////////////////////////////////////////////////////////////////////////////////////

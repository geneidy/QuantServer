#include <iostream>
#include <sstream>
#include <iomanip>

#include "BuildBook.h"
#include "NQTV.h"
#include "Logger.h"

CBuildBook::CBuildBook()
{

    m_iError = 0;
    m_pCOrdersMap = NULL;

    m_pCOrdersMap = COrdersMap::instance();

    if (!m_pCOrdersMap) {
        m_iError = 100;
        Logger::instance().log("Obtaining File Mapping Error in Build Book", Logger::Error);
    }

    m_uiNextOrder = 0;
    m_request.tv_nsec = 100000000;   // 1/10 of a second
    m_iSizeOfBook = sizeof( SBOOK_LEVELS);
    
 }
////////////////////////////////////////////////////
CBuildBook::~CBuildBook()
{
    Logger::instance().log("Destructing....Started Flushing All Books", Logger::Info);

    FlushAllBooks();

    Logger::instance().log("Destructing....Ended Flushing All Books", Logger::Info);
    Logger::instance().log("Destructing...Started Clearing Book Map", Logger::Info);

    m_BookMap.clear();

    Logger::instance().log("Destructing...Ended Clearing Book Map", Logger::Info);
}
////////////////////////////////////////////////////
int CBuildBook::BuildBookFromMemoryMappedFile()  // Entry point for processing...Called from a while loop in Main.cpp
{

    m_pCommonOrder = m_pCOrdersMap->GetMappedOrder(m_uiNextOrder++); // start with Order Zero
    if (m_pCommonOrder == NULL) {
        m_uiNextOrder--;
        nanosleep (&m_request, &m_remain);  // sleep a 1/10 of a second
        // Log the issue here....reading is faster than writing
        return m_uiNextOrder; // ::TODO think of  a return value
    }

    m_iMessage = m_pCommonOrder->cMessageType;

    switch (m_iMessage) {
    case 'A': // Add orders No MPID
    case 'F': // Add orders
//        ProcessAdd(m_iMessage);
        break;

    case 'E':  // Order executed
    case 'c':  // Order executed  with price
    case 'X':  // Order Cancel
    case 'D': // Order deleted
        ProcessDelete(m_iMessage);
        break;
    case 'U':
        ProcessReplace(m_iMessage);
        break;

    default:
        return 0;
    }

    return 0;
}
////////////////////////////////////////////////////
string CBuildBook::MakeKey()
{
    std::ostringstream strstrm;

    strstrm << setprecision(4) << m_pCommonOrder->dPrice << "+" << m_pCommonOrder->szMPID;

    return strstrm.str();

}
////////////////////////////////////////////////////
int CBuildBook::ProcessAdd(int iMessage)
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

            if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
                m_itPriceLevelMap->second.uiQty += m_uiQty;
                m_itPriceLevelMap->second.uiNumOfOrders++;
                if (strcmp(m_SBidAsk.szMPID, "NSDQ"))
                    m_itPriceLevelMap->second.SLevelStat.uiAttribAdd++;
                else
                    m_itPriceLevelMap->second.SLevelStat.uiNonAttribAdd++;
            }
            else { // Price level not found...Add it
                AddPriceLevel('B');
                m_itBookMap->second.m_iBidLevels++;
            }// if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
        } // if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
        else { // first time entry in the Book
            // Make a book Entry
            m_pBookLevels.m_iBidLevels = 1;
            m_RetPairBookMap = m_BookMap.insert(pair<char* , SBOOK_LEVELS>(m_pCommonOrder->szStock, m_pBookLevels) );
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

            if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
                m_itPriceLevelMap->second.uiQty += m_uiQty;
                m_itPriceLevelMap->second.uiNumOfOrders++;
                if (strcmp(m_SBidAsk.szMPID, "NSDQ"))
                    m_itPriceLevelMap->second.SLevelStat.uiAttribAdd++;
                else
                    m_itPriceLevelMap->second.SLevelStat.uiNonAttribAdd++;
            }
            else { // Price level not found...Add it
                AddPriceLevel('A');
                m_itBookMap->second.m_iAskLevels++;
            }// if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
        } // if (m_itBookMap != m_BookMap.end()) {  // Book found for this stock
        else { // first time entry in the Book
            // Make a book Entry
            m_pBookLevels.m_iAskLevels = 1;
            m_RetPairBookMap = m_BookMap.insert(pair<char* , SBOOK_LEVELS>(m_pCommonOrder->szStock, m_pBookLevels) );
            // Make a level Entry
            m_itBookMap = m_RetPairBookMap.first;
            AddPriceLevel('A');
        }// else { // first time entry in the Book
    } //   else { // Ask to sell
///////
    return 1;
}
////////////////////////////////////////////////////
bool CBuildBook::AddPriceLevel(int iSide)
{
    InitLevelStats();

    m_SBidAsk.dPrice = m_dPrice;
    m_SBidAsk.uiQty = m_uiQty;
    m_SBidAsk.uiNumOfOrders = 1;


    if (strcmp(m_SBidAsk.szMPID, "NSDQ"))
        m_SBidAsk.SLevelStat.uiAttribAdd = 1;
    else
        m_SBidAsk.SLevelStat.uiNonAttribAdd = 1;

    if (iSide == 'B')
        m_RetPairPriceLevelMap = m_itBookMap->second.BidPLMap.insert(pair<string /*Price+MM */, SBID_ASK >(m_strPriceMM, m_SBidAsk));
    else
        m_RetPairPriceLevelMap = m_itBookMap->second.AskPLMap.insert(pair<string /*Price+MM */, SBID_ASK >(m_strPriceMM, m_SBidAsk));

    //log error if any
    if (!m_RetPairPriceLevelMap.second){
        Logger::instance().log("Error creating book level", Logger::Error);
	// 
    }
    return m_RetPairPriceLevelMap.second;  //  a Bool on operation success or failure
}
////////////////////////////////////////////////////
int CBuildBook::InitLevelStats()
{
    m_SBidAsk.uiNumOfOrders 	= 1;
    m_SBidAsk.uiQty 		= 0;
    m_SBidAsk.dPrice 		= 0;

    m_SBidAsk.SLevelStat.uiAttribAdd = 	0;
    m_SBidAsk.SLevelStat.uiNonAttribAdd = 0;

    m_SBidAsk.SLevelStat.uiCancelled = 	0;
    m_SBidAsk.SLevelStat.uiDeleted = 	0;
    m_SBidAsk.SLevelStat.uiExecuted = 	0;
    m_SBidAsk.SLevelStat.uiReplaced = 	0;

    return true;
}
////////////////////////////////////////////////////
int CBuildBook::ProcessReplace(int iMessage)
{
    m_dPrice = m_pCommonOrder->dPrevPrice;
    m_uiQty = m_pCommonOrder->iPrevShares;

    ProcessDelete(10);

    m_dPrice = m_pCommonOrder->dPrice;
    m_uiQty = m_pCommonOrder->iShares;

    ProcessAdd(10);

    m_itPriceLevelMap->second.SLevelStat.uiReplaced++;

    return 1;
}
////////////////////////////////////////////////////
int CBuildBook::ProcessDelete(int iMessage)
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

            if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
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

            if ( m_itPriceLevelMap != m_PriceLevelMap.end()) { // Price level found....Update it
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
bool CBuildBook::UpdatePriceLevel(int iSide)
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
int CBuildBook::ListBook(char *szSymbol , uint32_t uiMaxLevels)
{

    BookMap::iterator	itBookMap;

    PriceLevelMap 	PriceLevelMap;
    PriceLevelMap::iterator itPriceLevelMap;

    itBookMap = m_BookMap.find(szSymbol);

    if (itBookMap == m_BookMap.end()) {
        return 0;
        // Log...no entry for such Stock
    }

    uint32_t uiLevels = 0;

    for (itPriceLevelMap = itBookMap->second.BidPLMap.begin(); itPriceLevelMap != itBookMap->second.BidPLMap.end(); ++itPriceLevelMap)
    {
        if (uiLevels++ > uiMaxLevels)
            break;
        cout<< itPriceLevelMap->second.szMPID;
        cout<< itPriceLevelMap->second.dPrice;
        cout<< itPriceLevelMap->second.uiQty;
        cout<< itPriceLevelMap->second.uiNumOfOrders;

        cout<< itPriceLevelMap->second.SLevelStat.uiAttribAdd;
        cout<< itPriceLevelMap->second.SLevelStat.uiNonAttribAdd;
        cout<< itPriceLevelMap->second.SLevelStat.uiCancelled;
        cout<< itPriceLevelMap->second.SLevelStat.uiReplaced;
        cout<< itPriceLevelMap->second.SLevelStat.uiDeleted;
        cout<< itPriceLevelMap->second.SLevelStat.uiExecuted;

    }

    uiLevels = 0;

    for (itPriceLevelMap = itBookMap->second.AskPLMap.begin(); itPriceLevelMap != itBookMap->second.AskPLMap.end(); ++itPriceLevelMap)
    {
        if (uiLevels++ > uiMaxLevels)
            break;

        cout<< itPriceLevelMap->second.szMPID;
        cout<< itPriceLevelMap->second.dPrice;
        cout<< itPriceLevelMap->second.uiQty;
        cout<< itPriceLevelMap->second.uiNumOfOrders;

        cout<< itPriceLevelMap->second.SLevelStat.uiAttribAdd;
        cout<< itPriceLevelMap->second.SLevelStat.uiNonAttribAdd;
        cout<< itPriceLevelMap->second.SLevelStat.uiCancelled;
        cout<< itPriceLevelMap->second.SLevelStat.uiReplaced;
        cout<< itPriceLevelMap->second.SLevelStat.uiDeleted;
        cout<< itPriceLevelMap->second.SLevelStat.uiExecuted;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////
uint32_t  CBuildBook::FlushBook(char* szSymbol )
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
uint64_t CBuildBook::FlushAllBooks()
{

    uint64_t uiNLevels = 0;

    for (m_itBookMap = m_BookMap.begin(); m_itBookMap != m_BookMap.end(); ++m_itBookMap)
    {
        m_itBookMap->second.BidPLMap.clear();
        m_itBookMap->second.AskPLMap.clear();
        uiNLevels++;
    }

    return uiNLevels;
}
//////////////////////////////////////////////////////////////////////////////////////
/*
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

  while (SBookLevels.pTopAsk != NULL) {  // Clear the Ask Levels
    pTemp = SBookLevels.pTopAsk;
    SBookLevels.pTopAsk = pTemp->pNextBidAsk;
    delete pTemp;
    Del.iAskLevels++;

  }// while (SBookLevels.pTopAsk != NULL) {


  while (SBookLevels.pTopBid != NULL) {  // Clear the Bid Levels
    pTemp = SBookLevels.pTopBid;
    SBookLevels.pTopBid = pTemp->pNextBidAsk;
    delete pTemp;
    Del.iBidLevels++;

  }// while (SBookLevels.pTopBid != NULL) {

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


    for (m_itBookMap = m_BookMap.begin(); m_itBookMap != m_BookMap.end(); ++m_itBookMap)
    {
        SBookLevels = m_itBookMap->second;

        while (SBookLevels.pTopAsk != NULL) {  // Clear the Ask Levels
            pTemp = SBookLevels.pTopAsk;
            SBookLevels.pTopAsk = pTemp->pNextBidAsk;
            Del.iAskLevels++;
            delete pTemp;
        }// while (SBookLevels.pTopAsk != NULL) {


        while (SBookLevels.pTopBid != NULL) {  // Clear the Bid Levels
            pTemp = SBookLevels.pTopBid;
            SBookLevels.pTopBid = pTemp->pNextBidAsk;
            Del.iBidLevels++;
            delete pTemp;
        }// while (SBookLevels.pTopAsk != NULL) {

    };

    return Del;  // log later
}
//////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////
int CBuildBook::ProcessAdd(int iMessage)
{
	bool	bFound = false;

	m_pBook.pTopBid = NULL;
	m_pBook.pTopAsk = NULL;

	lpInsert  = NULL;
	lpCurrent = NULL;
	lpPrevious = NULL;


	m_itBookMap = m_BookMap.find(m_pCommonOrder->szStock);
	if (m_itBookMap != m_BookMap.end()) {  // found
	  m_pBook = m_itBookMap->second;  // Fetch the book for this stock
	} // else NOT found fill the m_pBook struct
	else {
	  memset(&m_pBook, '\0', m_iSizeOfBook);

	}

	m_dPrice = m_pCommonOrder->dPrice;
	m_uiQty = m_pCommonOrder->iShares;

	if (m_pCommonOrder->cBuySell == 'B')  // bid to buy
	{
	  // Case 1: A fresh bucket with nothing in
	    if (m_pBook.pTopBid == NULL)   // Empty list
	    {
		lpInsert = AllocateNode(m_dPrice, m_uiQty);
		m_pBook.m_iBidLevels++;

		if (lpInsert == NULL)
		{
		//:: TODO log error...should not happen
		  return 100;
		}
		m_pBook.pTopBid = lpInsert;
	    }
	    else
	    {
	   // Case 2: I have at least one node (i.e.  one price level)
		lpCurrent	=	m_pBook.pTopBid;
		lpPrevious	=	lpCurrent;
		bFound = false;
		while (lpCurrent != NULL)
		{
		    if (m_dPrice == lpCurrent->dPrice)     // price match found
		    {
			lpCurrent->uiQty += m_uiQty;  // update volume
			lpCurrent->uiNumOfOrders++;
			bFound = true;
			break;
		    }
		    if (m_dPrice > lpCurrent->dPrice)    // new price level
		    {
			lpInsert = AllocateNode(m_dPrice, m_uiQty);
			m_pBook.m_iBidLevels++;

			if (lpInsert == NULL)
			{// :: TODO log error...should not happen
			    return 100;
			}
			if (lpPrevious == lpCurrent) // insert at the top of the list
			{
			    m_pBook.pTopBid	= lpInsert;
			    lpInsert->pNextBidAsk	= lpCurrent;
//			    break;
			}
			else
			{
			    lpPrevious->pNextBidAsk = lpInsert;
			    lpInsert->pNextBidAsk = lpCurrent;
//			    break;
			}
			break;
		     }
			lpPrevious = lpCurrent;
			lpCurrent = lpCurrent->pNextBidAsk;
		}//while (lpCurrent != NULL)

		// Case 3: New price level at the bottom of the list

		if ((lpCurrent == NULL) && (!bFound))
		{
		    lpInsert = AllocateNode(m_dPrice, m_uiQty);
		    m_pBook.m_iBidLevels++;

		    if (lpInsert == NULL)
		    {
		    //::TODO log error...should not happen
			return 100;
		    }
		    lpPrevious->pNextBidAsk = lpInsert;
		    lpInsert->pNextBidAsk = lpCurrent; // technically a NULL
		}//		if ((lpCurrent == NULL) && (!bFound))
	    } // else
			lpCurrent = m_pBook.pTopBid;
	}

	else			// ask to sell
	{
  // Case 1: A fresh bucket with nothing in
	    if (m_pBook.pTopAsk == NULL)   // Empty list
	    {
		lpInsert = AllocateNode(m_dPrice, m_uiQty);
		m_pBook.m_iAskLevels++;

		if (lpInsert == NULL)
		{
		//:: TODO log error...should not happen
		  return 100;
		}
		m_pBook.pTopAsk = lpInsert;
	    }
	    else
	    {
	   // Case 2: I have at least one node (i.e.  one price level)
		lpCurrent	=	m_pBook.pTopAsk;
		lpPrevious	=	lpCurrent;
		bFound = false;
		while (lpCurrent != NULL)
		{
		    if (m_dPrice == lpCurrent->dPrice)     // price match found
		    {
			lpCurrent->uiQty += m_uiQty;  // update volume
			lpCurrent->uiNumOfOrders++;
			bFound = true;
			break;
		    }
		    if (m_dPrice < lpCurrent->dPrice)    // new price level
		    {
			lpInsert = AllocateNode(m_dPrice, m_uiQty);
			m_pBook.m_iAskLevels++;

			if (lpInsert == NULL)
			{// :: TODO log error...should not happen
			    return 100;
			}
			if (lpPrevious == lpCurrent) // insert at the top of the list
			{
			    m_pBook.pTopAsk	= lpInsert;
			    lpInsert->pNextBidAsk	= lpCurrent;
//			    break;
			}
			else
			{
			    lpPrevious->pNextBidAsk = lpInsert;
			    lpInsert->pNextBidAsk = lpCurrent;
//			    break;
			}
			break;
		     }
			lpPrevious = lpCurrent;
			lpCurrent = lpCurrent->pNextBidAsk;
		}//while (lpCurrent != NULL)

		// Case 3: New price level at the bottom of the list

		if ((lpCurrent == NULL) && (!bFound))
		{
		    lpInsert = AllocateNode(m_dPrice, m_uiQty);
		    m_pBook.m_iAskLevels++;

		    if (lpInsert == NULL)
		    {
		    //::TODO log error...should not happen
			return 100;
		    }
		    lpPrevious->pNextBidAsk = lpInsert;
		    lpInsert->pNextBidAsk = lpCurrent; // technically a NULL
		}// if ((lpCurrent == NULL) && (!bFound))
	    } // else
	    lpCurrent = m_pBook.pTopAsk;
	}// else
	if (m_itBookMap == m_BookMap.end()) {  // A Fresh Stock just in
	    m_RetPair = m_BookMap.insert(pair<char*, SBOOK_LEVELS> (m_pCommonOrder->szStock, m_pBook));
	    // :: TODO check on error
	}
	else { // Update an existing one in the Map
	  m_itBookMap->second = m_pBook;
	}

	return 0;
}
*/

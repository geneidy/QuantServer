
#include "BuildBook.h"
#include "NQTV.h"
#include "Logger.h"

CBuildBook::CBuildBook()
{

  m_iError = 0;
  m_pCOrdersMap = NULL;
    
  m_pCOrdersMap = COrdersMap::instance();
    
  if (!m_pCOrdersMap){
      m_iError = 100;
      Logger::instance().log("Obtaining File Mapping Error in Build Book", Logger::Debug);
  }

  m_uiNextOrder = 0;
  m_request.tv_nsec = 100000000;   // 1/10 of a second 
  m_iSizeOfBook = sizeof( SBOOK_LEVELS);
}
////////////////////////////////////////////////////
CBuildBook::~CBuildBook()
{
  Logger::instance().log("Destructing....Started Flushing All Books", Logger::Debug);
  FlushAllBooks();
  Logger::instance().log("Destructing....Ended Flushing All Books", Logger::Debug);
  Logger::instance().log("Destructing...Started Clearing Book Map", Logger::Debug);
  m_BookMap.clear();
  Logger::instance().log("Destructing...Ended Clearing Book Map", Logger::Debug);  
}
////////////////////////////////////////////////////
int CBuildBook::BuildBookFromMemoryMappedFile()  // Entry point for processing...Called from a while loop in Main.cpp
{
  
    m_pCommonOrder = m_pCOrdersMap->GetMappedOrder(m_uiNextOrder++); // start with Order Zero
    if (m_pCommonOrder == NULL){
      m_uiNextOrder--;
      nanosleep (&m_request, &m_remain);  // sleep a 1/10 of a second
      // Log the issue here....reading is faster than writing
      return m_uiNextOrder; // ::TODO think of  a return value
    }

    m_iMessage = m_pCommonOrder->cMessageType;

    switch (m_iMessage) {
    case 'A': // Add orders No MPID
    case 'F': // Add orders
        ProcessAdd(m_iMessage);
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
int CBuildBook::ProcessReplace(int iMessage)
{

//    m_OrderReplace = pItchMessageUnion->OrderReplace;

    return 1;
}
////////////////////////////////////////////////////
int CBuildBook::ProcessDelete(int iMessage)
{
/*   // To update the stats only here
    switch (iMessage) {
    case 'E':  // Order executed
      break;
    case 'c':  // Order executed  with price
      break;
    case 'X':  // Order Cancel
      break;
    case 'D': // Order deleted
      break;
    default:
      break;
    }
  */  
    return 1;
}
////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////
SBID_ASK* CBuildBook::AllocateNode(double dPrice, unsigned int uiQty)
{
	SBID_ASK*		lpNewNode = NULL;
	
	lpNewNode			= new(SBID_ASK);
	lpNewNode->uiQty		= uiQty;	
	lpNewNode->dPrice		= dPrice;
	lpNewNode->uiNumOfOrders	= 0;
	lpNewNode->pNextBidAsk		= NULL;
	lpNewNode->SLevelStat.uiAttribAdd 	= 0;
	lpNewNode->SLevelStat.uiCancelled 	= 0;
	lpNewNode->SLevelStat.uiDeleted   	= 0;
	lpNewNode->SLevelStat.uiExecuted  	= 0;
	lpNewNode->SLevelStat.uiNonAttribAdd 	= 0;
	lpNewNode->SLevelStat.uiReplaced 	= 0;

	return lpNewNode;
}
///////////////////////////////////////////////////////////////////////////////////////
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
NLEVELS  CBuildBook::FlushAllBooks()
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

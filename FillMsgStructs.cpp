
#include "FillMsgStructs.h"
#include "NQTV.h"
#include "Logger.h"
#include <stdio.h>
#include <time.h>
#include "memory.h"

#include "DBLayer.h"
#include "Logger.h"

char strBuff[39];
//struct _timeb timebuffer;

#define  _SP   0

////////////////////////////////////////////////////////////////////////////
CFillMsgStructs::CFillMsgStructs(void)
{
  
   	time_t ltime = 0;
	struct tm stToday;
    
    
	time( &ltime );
        localtime_r( &ltime ,  &stToday);

//	memset(&theApp.g_Stats, 0 , sizeof(FEED_MESSAGE_STATS));
//	memset(theApp.g_Stats.strStartTime, '\0', sizeof(theApp.g_Stats.strStartTime));
//	strftime(theApp.g_Stats.strStartTime, sizeof(theApp.g_Stats.strStartTime), "%Y-%m-%d- %H:%M:%S" , &stToday);

	m_pCUtil	= NULL;
	m_pCUtil	= new CUtil();

	m_pDBLayer	= NULL;
//	m_pDBLayer	= new CDBLayer();

	m_bConnected = false;
	
	m_pQuantQueue = NULL;
	m_pQuantQueue = CQuantQueue::Instance();   // Only one instance is allowed of this singelton class
	
//	if (!m_pQuantQueue)
//	    Logger::instance().log("Error initialized the Queue", Logger::kLogLevelDebug);	  
  
}
////////////////////////////////////////////////////////////////////////////
CFillMsgStructs::~CFillMsgStructs(void)
{

   	time_t ltime = 0;
	struct tm stToday;

	time( &ltime );
	localtime_r(&ltime, &stToday);

	memset(theApp.g_Stats.strEndTime, '\0', sizeof(theApp.g_Stats.strEndTime));
	strftime(theApp.g_Stats.strEndTime, sizeof(theApp.g_Stats.strEndTime), "%Y-%m-%d- %H:%M:%S" , &stToday);

	if (m_pCUtil)
	{
		delete	m_pCUtil;
		m_pCUtil = NULL;
	}

	if (m_pDBLayer)
	{
//		delete m_pDBLayer;
		m_pDBLayer = NULL;
	}
	
	
	m_bConnected = false;
}
////////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::DirectToMethod(UINT8* uiMsg)
{

	switch (uiMsg[0])
	{
	case 'S': SystemEvent(uiMsg);
 		theApp.g_arrTotalMessages[0 ]++;
 		break;
 	case 'R': StockDirectory(uiMsg);
 		theApp.g_arrTotalMessages[1]++;
 		break;
	case 'H': StockTradingAction(uiMsg);
 		theApp.g_arrTotalMessages[2]++;
 		break;
 	case 'Y': RegShoRestriction(uiMsg);
 		theApp.g_arrTotalMessages[3]++;
 		break;
 	case 'L': Market_Participant_Position(uiMsg);
 		theApp.g_arrTotalMessages[14]++;
 		break;
 	case 'V': MWCBDeclineLevelMessage(uiMsg);
 		theApp.g_arrTotalMessages[15]++;
 		break;
 	case 'W': MWCBBreachMessage(uiMsg);
 		theApp.g_arrTotalMessages[16]++;
 		break;
 	case 'K': IPOQuotingPeriodUpdate(uiMsg);
 		theApp.g_arrTotalMessages[17]++;
 		break;
 	case 'A': AddOrderNoMPIDMessage(uiMsg);
 		theApp.g_arrTotalMessages[4]++;
 		break;
	case 'F': AddOrderWithMPID(uiMsg);
		theApp.g_arrTotalMessages[5]++;
		break;
 	case 'E': OrderExecutionMessage(uiMsg);
 		theApp.g_arrTotalMessages[7]++;
 		theApp.g_arrTotalMessages[6]++;
 		break;
 	case 'c': OrderExecutionWithPriceMessage(uiMsg);
 		theApp.g_arrTotalMessages[8]++;
 		theApp.g_arrTotalMessages[6]++;
 		break;
 	case 'X': OrderCancelMessage(uiMsg);
 		theApp.g_arrTotalMessages[9]++;
 		theApp.g_arrTotalMessages[6]++;
 		break;
 	case 'D': OrderDelete(uiMsg);
 		theApp.g_arrTotalMessages[10]++;
 		theApp.g_arrTotalMessages[6]++;
 		break;
 	case 'U': OrderReplace(uiMsg);
 		theApp.g_arrTotalMessages[11]++;
 		theApp.g_arrTotalMessages[6]++;
 		break;
 	case 'P':
 		TradeMessageNonCross(uiMsg);
 		theApp.g_arrTotalMessages[12]++;
 		break;
 	case 'I': NOII(uiMsg);
 		theApp.g_arrTotalMessages[19]++;
 		break;
 	case 'N': RetailPriceImprovementIndicator(uiMsg);
 		theApp.g_arrTotalMessages[20]++;
 		break;
	default:  // we do not process this type of messages
		break;
	};
	theApp.g_arrTotalMessages[22]++;
 
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::SystemEvent(UINT8* uiMsg)
{
	memset(&m_IMUSys.SystemEvent , '\0', sizeof(SYSTEM_EVENT_MESSAGE ));

	m_IMUSys.SystemEvent.cMessageType = 'S'; 
	m_IMUSys.SystemEvent.iStockLocale = 0;   // Always Zero per documentation
	m_IMUSys.SystemEvent.iTrackingNumber = m_pCUtil->GetValueUnsignedLong(uiMsg, 3, 2);
	m_IMUSys.SystemEvent.iTimeStamp = m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	m_IMUSys.SystemEvent.cEventCode = m_pCUtil->GetValueChar(uiMsg, 11 ,1);


	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'S');
	
/****************************************************************************************
	m_pDBLayer->SystemEvent(m_SystemEvent.cMessageType,
							m_SystemEvent.iStockLocale,
							m_SystemEvent.iTrackingNumber,
							m_SystemEvent.iTimeStamp,
							m_SystemEvent.cEventCode);
****************************************************************************************/

return 0;
}
///////////////////////////////////////////////////////////////////////////*/
int  CFillMsgStructs::StockDirectory(UINT8* uiMsg)
{
	memset(&m_IMUSys.StockDirectory  , '\0', sizeof(STOCK_DIRECTORY_MESSAGE ));


	m_IMUSys.StockDirectory.cMessageType			= 'R';
	m_IMUSys.StockDirectory.iSockLocale			= m_pCUtil->GetValueUnsignedLong(uiMsg, 1, 2);
	m_IMUSys.StockDirectory.iTracking_Number		= m_pCUtil->GetValueUnsignedLong(uiMsg, 3, 2);
	m_IMUSys.StockDirectory.iTimeStamp				= m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	strcpy(m_IMUSys.StockDirectory.szStock, m_pCUtil->GetValueAlpha( uiMsg,11, 8));
	m_IMUSys.StockDirectory.eMarketCategory		= m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_IMUSys.StockDirectory.eFSI					= m_pCUtil->GetValueChar(uiMsg, 20,1);
	m_IMUSys.StockDirectory.iRoundLotSize			= m_pCUtil->GetValueUnsignedLong( uiMsg, 21, 4);
	m_IMUSys.StockDirectory.cRoundLotsOnly         =  m_pCUtil->GetValueChar(  uiMsg, 25, 1); 
	m_IMUSys.StockDirectory.cIssueClassification	= m_pCUtil->GetValueChar(uiMsg, 26, 1);
	strcpy(m_IMUSys.StockDirectory.strIssueSubType, m_pCUtil->GetValueAlpha(uiMsg, 27, 2));
	m_IMUSys.StockDirectory.cAuthenticity			=  m_pCUtil->GetValueChar(uiMsg, 29, 1);
	m_IMUSys.StockDirectory.cShortSaleThresholdInd = m_pCUtil->GetValueChar(uiMsg, 30, 1);

	m_IMUSys.StockDirectory.cIPOFlag				= m_pCUtil->GetValueChar(uiMsg, 31, 1);
	m_IMUSys.StockDirectory.cLULDRefPrice			= m_pCUtil->GetValueChar(uiMsg, 32, 1);
	m_IMUSys.StockDirectory.cETPFlag				= m_pCUtil->GetValueChar(uiMsg, 33, 1);		
	m_IMUSys.StockDirectory.iETPLeverageFactor		= m_pCUtil->GetValueChar(uiMsg, 34, 4);
	m_IMUSys.StockDirectory.cInverseFactor			= m_pCUtil->GetValueChar(uiMsg, 38, 1);	
	
	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'R');
	
	

/****************************************************************************************
	m_pDBLayer->StockDirectoryUpdate(1,
 	m_StockDirectory.cMessageType,
	m_StockDirectory.iSockLocale,
	m_StockDirectory.iTracking_Number,
	m_StockDirectory.iTimeStamp,
 	m_StockDirectory.szStock,
	m_StockDirectory.eMarketCategory,
    m_StockDirectory.eFSI,
	m_StockDirectory.iRoundLotSize,
	m_StockDirectory.cRoundLotsOnly,
    m_StockDirectory.cIssueClassification,
	m_StockDirectory.strIssueSubType,
	m_StockDirectory.cAuthenticity,
	m_StockDirectory.cShortSaleThresholdInd,
	m_StockDirectory.cIPOFlag,
    m_StockDirectory.cLULDRefPrice,
	m_StockDirectory.cETPFlag,
    m_StockDirectory.iETPLeverageFactor,
	m_StockDirectory.cInverseFactor);
************************************************************************************************/

	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
int  CFillMsgStructs::StockTradingAction(UINT8* uiMsg)
{

	memset(&m_IMUSys.StockTradingAction, '\0', sizeof(STOCK_TRADING_ACTION_MESSAGE ));

	m_IMUSys.StockTradingAction.cMessageType			= 'H';
	m_IMUSys.StockTradingAction.iStockLocate			=  m_pCUtil->GetValueUnsignedLong(uiMsg, 1, 2);
	m_IMUSys.StockTradingAction.iTrackingNumber		= m_pCUtil->GetValueUnsignedLong(uiMsg, 3, 2);
	m_IMUSys.StockTradingAction.iTimeStamp				= m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	
	strcpy(m_IMUSys.StockTradingAction.Symbol, m_pCUtil->GetValueAlpha( uiMsg,11, 8));
	m_IMUSys.StockTradingAction.cTradingState = m_pCUtil->GetValueChar(uiMsg, 19, 1);

	m_IMUSys.StockTradingAction.cReserved = m_pCUtil->GetValueChar(uiMsg, 20, 1); 
	strcpy(m_IMUSys.StockTradingAction.strReason , m_pCUtil->GetValueAlpha(uiMsg, 21, 4));
	
	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'H');
	

/****************************************************************************************
	m_pDBLayer->StockTradingAction(1,
 	m_StockTradingAction.cMessageType,
	m_StockTradingAction.iStockLocate,
	m_StockTradingAction.iTrackingNumber,
	m_StockTradingAction.iTimeStamp,
	m_StockTradingAction.Symbol,
	m_StockTradingAction.cTradingState,
	m_StockTradingAction.cReserved,
	m_StockTradingAction.strReason
	);
************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::RegShoRestriction(UINT8* uiMsg)
{
	memset(&m_IMUSys.RegSho, '\0', sizeof(REG_SHO_RESTRICTION_MESSAGE));
	
	m_IMUSys.RegSho.cMessage			= 'Y';
	m_IMUSys.RegSho.iLocateCode		= m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.RegSho.iTimeStamp		= m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	strcpy(m_IMUSys.RegSho.Symbol, m_pCUtil->GetValueAlpha( uiMsg,11, 8));
	m_IMUSys.RegSho.cRegSHOAction	= m_pCUtil->GetValueChar (uiMsg, 19, 1);
	

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'Y');
		
/****************************************************************************************
	m_pDBLayer->RegShoRestriction( 
 	m_RegShoRestricted.cMessage,
	m_RegShoRestricted.iLocateCode,
	m_RegShoRestricted.iTimeStamp,
	m_RegShoRestricted.Symbol,
	m_RegShoRestricted.cRegSHOAction);
************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
int  CFillMsgStructs::Market_Participant_Position(UINT8* uiMsg)
{
	memset(&m_IMUSys.MpPosition, '\0', sizeof(MP_POSITION_MESSAGE ));

	m_IMUSys.MpPosition.cMessageType		= 'L';
	m_IMUSys.MpPosition.iLocateCode		= m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.MpPosition.TrackingNumber		=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.MpPosition.iTimeStamp			= m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	strcpy(m_IMUSys.MpPosition.szMPID, m_pCUtil->GetValueAlpha( uiMsg, 11, 4));
	strcpy(m_IMUSys.MpPosition.szStock, m_pCUtil->GetValueAlpha( uiMsg, 15, 8));
	m_IMUSys.MpPosition.cPrimaryMM			= m_pCUtil->GetValueChar(uiMsg, 23, 1);
	m_IMUSys.MpPosition.cMMMode			= m_pCUtil->GetValueChar(uiMsg, 24, 1);
	m_IMUSys.MpPosition.cMarketParticipantState = m_pCUtil->GetValueChar(uiMsg, 25, 1);

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'L');
		
	
/************************************************************************************************
	m_pDBLayer->MarketParticipantPosition(m_MPPosition.cMessageType,
		m_MPPosition.iLocateCode,
		m_MPPosition.TrackingNumber,
		m_MPPosition.iTimeStamp,
		m_MPPosition.szMPID, 
		m_MPPosition.szStock,
		m_MPPosition.cPrimaryMM,
		m_MPPosition.cMMMode,
		m_MPPosition.cMarketParticipantState
		);
************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
int  CFillMsgStructs::MWCBDeclineLevelMessage(UINT8* uiMsg)
{
	memset(&m_IMUSys.MWCBDLM, '\0', sizeof(MWCBDLM_MESSAGE));

	m_IMUSys.MWCBDLM.cMessageType = 'V';
	m_IMUSys.MWCBDLM.iLocateCode = m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.MWCBDLM.TrackingNumber =  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.MWCBDLM.iTimeStamp	=  m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.MWCBDLM.dLevel1      =   double (m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8))/100000000;
	m_IMUSys.MWCBDLM.dLevel2      =   double (m_pCUtil->GetValueUnsignedInt64(uiMsg, 19, 8))/100000000;
	m_IMUSys.MWCBDLM.dLevel3		=  double (m_pCUtil->GetValueUnsignedInt64(uiMsg, 27, 8))/100000000;

	
	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'V');
	
/************************************************************************************************
	m_pDBLayer->MWCBDeclineLevelMessage(m_IMUSys.MWCBDLM.cMessageType,
		m_MWCBDLM.iLocateCode,
		m_MWCBDLM.TrackingNumber,
		m_MWCBDLM.iTimeStamp,
		m_MWCBDLM.dLevel1,
		m_MWCBDLM.dLevel2,
		m_MWCBDLM.dLevel3);

***********************************************************************************************/

	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::MWCBBreachMessage(UINT8* uiMsg)
{
	memset(&m_IMUSys.MWCBDBM, '\0', sizeof(MWCBDBM_MESSAGE));

	m_IMUSys.MWCBDBM.cMessageType = 'W';
	m_IMUSys.MWCBDBM.iLocateCode = m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.MWCBDBM.TrackingNumber =  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.MWCBDBM.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.MWCBDBM.cBreachLevel   = (short) m_pCUtil->GetValueChar( uiMsg, 11, 1);
	
	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'W');
	

/************************************************************************************************
	m_pDBLayer->MWCBBreachMessage(m_MWCBDBM.cMessageType,
		m_MWCBDBM.iLocateCode,
		m_MWCBDBM.TrackingNumber,
		m_MWCBDBM.iTimeStamp,
		m_MWCBDBM.cBreachLevel);

************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::IPOQuotingPeriodUpdate(UINT8* uiMsg)
{
	memset(&m_IMUSys.IPOQutationUpdate, '\0', sizeof(IPO_QUOTATION_PERIOD_UPDATE_MESSAGE));

	m_IMUSys.IPOQutationUpdate.cMessageType = 'K';
	m_IMUSys.IPOQutationUpdate.iLocateCode =  m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.IPOQutationUpdate.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.IPOQutationUpdate.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	strcpy(m_IMUSys.IPOQutationUpdate.szStock, m_pCUtil->GetValueAlpha( uiMsg, 11, 8));

	m_IMUSys.IPOQutationUpdate.iIPOQuotationReleaseTime = m_pCUtil->GetValueUnsignedLong( uiMsg, 19, 4);
	m_IMUSys.IPOQutationUpdate.cIPOQuotationReleaseQualifier = m_pCUtil->GetValueChar( uiMsg, 23, 1);
	m_IMUSys.IPOQutationUpdate.dIPOPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 24, 4))/10000;

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'K');
	
	
/************************************************************************************************
	m_pDBLayer->IPOQuotingPeriodUpdate(m_IPOQuotationPeriod.cMessageType,
		m_IPOQuotationPeriod.iLocateCode,
		m_IPOQuotationPeriod.TrackingNumber,
		m_IPOQuotationPeriod.iTimeStamp,
		m_IPOQuotationPeriod.szStock,
		m_IPOQuotationPeriod.iIPOQuotationReleaseTime,
		m_IPOQuotationPeriod.cIPOQuotationReleaseQualifier,
		m_IPOQuotationPeriod.dIPOPrice
		);
************************************************************************************************/

	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::AddOrderNoMPIDMessage(UINT8* uiMsg)
{
	memset(&m_IMUSys.AddOrderNoMPID, '\0', sizeof(ADD_ORDER_NO_MPID_MESSAGE ));

	m_IMUSys.AddOrderNoMPID.cMessageType = 'A';
	m_IMUSys.AddOrderNoMPID.iLocateCode =   m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.AddOrderNoMPID.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.AddOrderNoMPID.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.AddOrderNoMPID.iOrderRefNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_IMUSys.AddOrderNoMPID.cBuySell        = m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_IMUSys.AddOrderNoMPID.iShares		=  m_pCUtil->GetValueUnsignedLong(uiMsg, 20, 4);

	strcpy(m_IMUSys.AddOrderNoMPID.szStock, m_pCUtil->GetValueAlpha( uiMsg, 24, 8));
	m_IMUSys.AddOrderNoMPID.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;

	
	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'A');
		
/************************************************************************************************
	m_pDBLayer->AddOrderMPID(m_AddOrderNoMPID.cMessageType,
		m_AddOrderNoMPID.iLocateCode,
		m_AddOrderNoMPID.TrackingNumber,
		m_AddOrderNoMPID.iTimeStamp,
		m_AddOrderNoMPID.iOrderRefNumber, 
		m_AddOrderNoMPID.cBuySell,
		m_AddOrderNoMPID.iShares,
		m_AddOrderNoMPID.szStock,
		m_AddOrderNoMPID.dPrice,
		"NSDQ");
***********************************************************************************************/
	// The NO_MPID table has retiered...Will attribute the order to NSDQ as the MPID
	 
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::AddOrderWithMPID(UINT8* uiMsg)
{
	memset(&m_IMUSys.AddOrderMPID, '\0', sizeof(ADD_ORDER_MPID_MESSAGE ));

	m_IMUSys.AddOrderMPID.cMessageType = 'F';
	m_IMUSys.AddOrderMPID.iLocateCode =    m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.AddOrderMPID.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.AddOrderMPID.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.AddOrderMPID.iOrderRefNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_IMUSys.AddOrderMPID.cBuySell        = m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_IMUSys.AddOrderMPID.iShares		=  m_pCUtil->GetValueUnsignedLong(uiMsg, 20, 4);

	strcpy(m_IMUSys.AddOrderMPID.szStock, m_pCUtil->GetValueAlpha( uiMsg, 24, 8));
	m_IMUSys.AddOrderMPID.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;
	strcpy(m_IMUSys.AddOrderMPID.szMPID,  m_pCUtil->GetValueAlpha(uiMsg, 36, 4));

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'F');
	
/*****************************************************************
	m_pDBLayer->AddOrderMPID(m_AddOrderMPID.cMessageType,
		m_AddOrderMPID.iLocateCode,
		m_AddOrderMPID.TrackingNumber,
		m_AddOrderMPID.iTimeStamp,
		m_AddOrderMPID.iOrderRefNumber, 
		m_AddOrderMPID.cBuySell,
		m_AddOrderMPID.iShares,
		m_AddOrderMPID.szStock,
		m_AddOrderMPID.dPrice,
		m_AddOrderMPID.szMPID);
	
******************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderExecutionMessage(UINT8* uiMsg)
{
	memset(&m_IMUSys.OrderExecuted, '\0', sizeof(ORDER_EXECUTED_MESSAGE));

	m_IMUSys.OrderExecuted.cMessageType = 'E';
	m_IMUSys.OrderExecuted.iLocateCode =    m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.OrderExecuted.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.OrderExecuted.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.OrderExecuted.iOrderRefNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_IMUSys.OrderExecuted.iShares = m_pCUtil->GetValueUnsignedLong(uiMsg, 19, 4);
	m_IMUSys.OrderExecuted.iOrderMatchNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 23, 8);

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'E');
		
	
/************************************************************************************************
	m_pDBLayer->OrderExecuted(m_OrderExecuted.cMessageType,
	m_OrderExecuted.iLocateCode,
	m_OrderExecuted.TrackingNumber,
	m_OrderExecuted.iTimeStamp,
	m_OrderExecuted.iOrderRefNumber,
	m_OrderExecuted.iShares,
	m_OrderExecuted.iOrderMatchNumber);
************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderExecutionWithPriceMessage(UINT8* uiMsg)
{
	memset(&m_IMUSys.OrderExecutedWithPrice, '\0', sizeof(ORDER_EXECUTED_WITH_PRICE_MESSAGE ));

	m_IMUSys.OrderExecutedWithPrice.cMessageType = 'c';
	m_IMUSys.OrderExecutedWithPrice.iLocateCode =    m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.OrderExecutedWithPrice.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.OrderExecutedWithPrice.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.OrderExecutedWithPrice.iOrderRefNumber = m_pCUtil->GetValueUnsignedLong(uiMsg, 11, 8); 
	m_IMUSys.OrderExecutedWithPrice.iShares = m_pCUtil->GetValueUnsignedLong(uiMsg, 19, 4);
	m_IMUSys.OrderExecutedWithPrice.iOrderMatchNumber = m_pCUtil->GetValueUnsignedLong(uiMsg, 23, 8);

	m_IMUSys.OrderExecutedWithPrice.cPrintable = m_pCUtil->GetValueChar(uiMsg, 31, 1);
	m_IMUSys.OrderExecutedWithPrice.dExecutionPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'c');
	
/************************************************************************************************
	m_pDBLayer->OrderExecutedWithPrice(m_OrderExecutedWithPrice.cMessageType,
	m_OrderExecutedWithPrice.iLocateCode,
	m_OrderExecutedWithPrice.TrackingNumber,
	m_OrderExecutedWithPrice.iTimeStamp,
	m_OrderExecutedWithPrice.iOrderRefNumber,
	m_OrderExecutedWithPrice.iShares,
	m_OrderExecutedWithPrice.iOrderMatchNumber,
	m_OrderExecutedWithPrice.cPrintable,
	m_OrderExecutedWithPrice.dExecutionPrice);
************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderCancelMessage(UINT8* uiMsg)
{
	memset(&m_IMUSys.OrderCancel, '\0', sizeof(ORDER_CANCEL_MESSAGE ));

	m_IMUSys.OrderCancel.cMessageType		=	'X';
	m_IMUSys.OrderCancel.iLocateCode		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.OrderCancel.TrackingNumber	=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.OrderCancel.iTimeStamp		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.OrderCancel.iOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_IMUSys.OrderCancel.iShares			=	m_pCUtil->GetValueUnsignedLong(uiMsg, 19, 4);

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'X');

/************************************************************************************************
	m_pDBLayer->OrderCancel(m_OrderCancel.cMessageType, 
							m_OrderCancel.iLocateCode,
							m_OrderCancel.TrackingNumber,
							m_OrderCancel.iTimeStamp,
							m_OrderCancel.iOrderRefNumber,
							m_OrderCancel.iShares);

************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderDelete(UINT8* uiMsg)
{
	memset(&m_IMUSys.OrderDelete  , '\0', sizeof(ORDER_DELETE_MESSAGE ));

	m_IMUSys.OrderDelete.cMessageType		=	'D';
	m_IMUSys.OrderDelete.iLocateCode		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.OrderDelete.TrackingNumber	=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.OrderDelete.iTimeStamp		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.OrderDelete.iOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 


	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'D');
		
	
/************************************************************************************************
	m_pDBLayer->OrderDelete(m_IMUSys.OrderDelete.cMessageType,
							m_OrderDelete.iLocateCode,
							m_OrderDelete.TrackingNumber,
							m_OrderDelete.iTimeStamp,
							m_OrderDelete.iOrderRefNumber);
************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderReplace(UINT8* uiMsg)
{
	memset(&m_IMUSys.OrderReplace, '\0', sizeof(ORDER_REPLACE_MESSAGE ));

	m_IMUSys.OrderReplace.cMessageType			= 'U';
	m_IMUSys.OrderReplace.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.OrderReplace.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.OrderReplace.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.OrderReplace.iOldOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_IMUSys.OrderReplace.iNewOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 19, 8); 

	m_IMUSys.OrderReplace.iShares				=   m_pCUtil->GetValueUnsignedLong(uiMsg, 27, 4);
	m_IMUSys.OrderReplace.dPrice				=   double (m_pCUtil->GetValueUnsignedLong(uiMsg, 31, 4))/10000;


	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'U');
		
	
/************************************************************************************************
	m_pDBLayer->OrderReplace(m_OrderReplace.cMessageType,
							m_OrderReplace.iLocateCode,
							m_OrderReplace.TrackingNumber,
							m_OrderReplace.iTimeStamp,
							m_OrderReplace.iOldOrderRefNumber,
							m_OrderReplace.iNewOrderRefNumber,
							m_OrderReplace.iShares,
							m_OrderReplace.dPrice
							);
************************************************************************************************/
	return 0; // :: TODO 
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::TradeMessageNonCross(UINT8* uiMsg)
{
	memset(&m_IMUSys.TradeNonCross  , '\0', sizeof(TRADE_NON_CROSS_MESSAGE ));

	m_IMUSys.TradeNonCross.cMessageType		= 'P';
	m_IMUSys.TradeNonCross.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.TradeNonCross.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.TradeNonCross.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.TradeNonCross.iOrderRefNumber		=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 

	m_IMUSys.TradeNonCross.cBuySell = m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_IMUSys.TradeNonCross.iShares = m_pCUtil->GetValueUnsignedLong(uiMsg, 20, 4);
	strcpy(m_IMUSys.TradeNonCross.szStock,  m_pCUtil->GetValueAlpha(uiMsg, 24, 8));
	m_IMUSys.TradeNonCross.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;
	m_IMUSys.TradeNonCross.iMatchNumber  = m_pCUtil->GetValueUnsignedInt64(uiMsg, 36, 8);


	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'P');
		
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::NOII(UINT8* uiMsg)
{
	memset(&m_IMUSys.NOII  , '\0', sizeof(NOII_MESSAGE ));

	m_IMUSys.NOII.cMessageType			= 'I';
	m_IMUSys.NOII.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.NOII.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.NOII.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_IMUSys.NOII.iPairedShares		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 11, 8);
	m_IMUSys.NOII.iImbalanceShares     =  m_pCUtil->GetValueUnsignedInt64( uiMsg, 19, 8);
	m_IMUSys.NOII.iImbalanceDirection  =  m_pCUtil->GetValueChar( uiMsg, 27, 1);
	strcpy(m_IMUSys.NOII.szStock, m_pCUtil->GetValueAlpha( uiMsg, 28, 8));

	m_IMUSys.NOII.dFarPrice =  double (m_pCUtil->GetValueUnsignedLong(uiMsg, 36, 4))/10000;
	
	m_IMUSys.NOII.dNearPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 40, 4))/10000;
	m_IMUSys.NOII.dRefPrice  = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 44, 4))/10000;
	m_IMUSys.NOII.cCrossType = m_pCUtil->GetValueChar(uiMsg, 48, 1);
	m_IMUSys.NOII.cPriceVariation = m_pCUtil->GetValueChar(uiMsg, 49, 1);
	

	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'I');
	

/************************************************************************************************
	m_pDBLayer->NOII(	m_NOII.cMessageType,
						m_NOII.iLocateCode,
						m_NOII.TrackingNumber,
						m_NOII.iTimeStamp,
						m_NOII.iPairedShares,
						m_NOII.iImbalanceShares,
						m_NOII.iImbalanceDirection,
						m_NOII.szStock,
						m_NOII.dFarPrice,
						m_NOII.dNearPrice,
						m_NOII.dRefPrice,
						m_NOII.cCrossType,
						m_NOII.cPriceVariation
						);
************************************************************************************************/

	return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::RetailPriceImprovementIndicator(UINT8* uiMsg)
{
	memset(&m_IMUSys.RPPI  , '\0', sizeof(RPII_MESSAGE ));

	m_IMUSys.RPPI.cMessageType			=	'N';
	m_IMUSys.RPPI.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IMUSys.RPPI.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IMUSys.RPPI.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	strcpy(m_IMUSys.RPPI.szStock, m_pCUtil->GetValueAlpha( uiMsg, 11, 8));

	m_IMUSys.RPPI.cInterestFlag  = m_pCUtil->GetValueChar(uiMsg, 19, 1);


	if (m_pQuantQueue)
	  m_pQuantQueue->Enqueue(&m_IMUSys, 'N');
		
/************************************************************************************************
	m_pDBLayer->RPII(	m_IMUSys.RPPI.cMessageType,
						m_RPPI.iLocateCode,
						m_RPPI.TrackingNumber,
						m_RPPI.iTimeStamp,
						m_RPPI.szStock,
						m_RPPI.cInterestFlag
						);
************************************************************************************************/
	return 0;
}
///////////////////////////////////////////////////////////////////////////
FEED_MESSAGE_STATS  CFillMsgStructs::GetStats()
{
	return theApp.g_Stats;
}
///////////////////////////////////////////////////////////////////////////*/

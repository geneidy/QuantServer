
#include "FillMsgStructs.h"
#include "NQTV.h"

#include <stdio.h>
#include <time.h>
#include "memory.h"

#include "DBLayer.h"

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
	m_pDBLayer	= new CDBLayer();

	m_bConnected = false;

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
		delete m_pDBLayer;
		m_pDBLayer = NULL;
	}

	m_bConnected = false;

}
////////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::DirectToMethod(UINT8* uiMsg)
{

	switch (uiMsg[0])
	{
// 	case 'S': SystemEvent(uiMsg);
// 		theApp.g_arrTotalMessages[0 ]++;
// 		break;
// 	case 'R': StockDirectory(uiMsg);
// 		theApp.g_arrTotalMessages[1]++;
// 		break;
// 	case 'H': StockTradingAction(uiMsg);
// 		theApp.g_arrTotalMessages[2]++;
// 		break;
// 	case 'Y': RegShoRestriction(uiMsg);
// 		theApp.g_arrTotalMessages[3]++;
// 		break;
// 	case 'L': Market_Participant_Position(uiMsg);
// 		theApp.g_arrTotalMessages[14]++;
// 		break;
// 	case 'V': MWCBDeclineLevelMessage(uiMsg);
// 		theApp.g_arrTotalMessages[15]++;
// 		break;
// 	case 'W': MWCBBreachMessage(uiMsg);
// 		theApp.g_arrTotalMessages[16]++;
// 		break;
// 	case 'K': IPOQuotingPeriodUpdate(uiMsg);
// 		theApp.g_arrTotalMessages[17]++;
// 		break;
// 	case 'A': AddOrderNoMPIDMessage(uiMsg);
// 		theApp.g_arrTotalMessages[4]++;
// 		break;
	case 'F': AddOrderWithMPID(uiMsg);
		theApp.g_arrTotalMessages[5]++;
		break;
// 	case 'E': OrderExecutionMessage(uiMsg);
// 		theApp.g_arrTotalMessages[7]++;
// 		theApp.g_arrTotalMessages[6]++;
// 		break;
// 	case 'c': OrderExecutionWithPriceMessage(uiMsg);
// 		theApp.g_arrTotalMessages[8]++;
// 		theApp.g_arrTotalMessages[6]++;
// 		break;
// 	case 'X': OrderCancelMessage(uiMsg);
// 		theApp.g_arrTotalMessages[9]++;
// 		theApp.g_arrTotalMessages[6]++;
// 		break;
// 	case 'D': OrderDelete(uiMsg);
// 		theApp.g_arrTotalMessages[10]++;
// 		theApp.g_arrTotalMessages[6]++;
// 		break;
// 	case 'U': OrderReplace(uiMsg);
// 		theApp.g_arrTotalMessages[11]++;
// 		theApp.g_arrTotalMessages[6]++;
// 		break;
// 	case 'P':
// 		TradeMessageNonCross(uiMsg);
// 		theApp.g_arrTotalMessages[12]++;
// 		break;
// 	case 'I': NOII(uiMsg);
// 		theApp.g_arrTotalMessages[19]++;
// 		break;
// 	case 'N': RetailPriceImprovementIndicator(uiMsg);
// 		theApp.g_arrTotalMessages[20]++;
// 		break;
	default:  // we do not process this type of messages
		break;
	};
	theApp.g_arrTotalMessages[22]++;
 
	return 0;
}
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::SystemEvent(UINT8* uiMsg)
{
	memset(&m_SystemEvent , '\0', sizeof(SYSTEM_EVENT_MESSAGE ));

	m_SystemEvent.cMessageType = 'S'; 
	m_SystemEvent.iStockLocale = 0;   // Always Zero per documentation
	m_SystemEvent.iTrackingNumber = m_pCUtil->GetValueUnsignedLong(uiMsg, 3, 2);
	m_SystemEvent.iTimeStamp = m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	m_SystemEvent.cEventCode = m_pCUtil->GetValueChar(uiMsg, 11 ,1);

****************************************************************************************
	m_pDBLayer->SystemEvent(m_SystemEvent.cMessageType,
							m_SystemEvent.iStockLocale,
							m_SystemEvent.iTrackingNumber,
							m_SystemEvent.iTimeStamp,
							m_SystemEvent.cEventCode);
****************************************************************************************

return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::StockDirectory(UINT8* uiMsg)
{
	memset(&m_StockDirectory  , '\0', sizeof(STOCK_DIRECTORY_MESSAGE ));


	m_StockDirectory.cMessageType			= 'R';
	m_StockDirectory.iSockLocale			= m_pCUtil->GetValueUnsignedLong(uiMsg, 1, 2);
	m_StockDirectory.iTracking_Number		= m_pCUtil->GetValueUnsignedLong(uiMsg, 3, 2);
	m_StockDirectory.iTimeStamp				= m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	strcpy(m_StockDirectory.strStock, m_pCUtil->GetValueAlpha( uiMsg,11, 8));
	m_StockDirectory.eMarketCategory		= m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_StockDirectory.eFSI					= m_pCUtil->GetValueChar(uiMsg, 20,1);
	m_StockDirectory.iRoundLotSize			= m_pCUtil->GetValueUnsignedLong( uiMsg, 21, 4);
	m_StockDirectory.cRoundLotsOnly         =  m_pCUtil->GetValueChar(  uiMsg, 25, 1); 
	m_StockDirectory.cIssueClassification	= m_pCUtil->GetValueChar(uiMsg, 26, 1);
	strcpy(m_StockDirectory.strIssueSubType, m_pCUtil->GetValueAlpha(uiMsg, 27, 2));
	m_StockDirectory.cAuthenticity			=  m_pCUtil->GetValueChar(uiMsg, 29, 1);
	m_StockDirectory.cShortSaleThresholdInd = m_pCUtil->GetValueChar(uiMsg, 30, 1);

	m_StockDirectory.cIPOFlag				= m_pCUtil->GetValueChar(uiMsg, 31, 1);
	m_StockDirectory.cLULDRefPrice			= m_pCUtil->GetValueChar(uiMsg, 32, 1);
	m_StockDirectory.cETPFlag				= m_pCUtil->GetValueChar(uiMsg, 33, 1);		
	m_StockDirectory.iETPLeverageFactor		= m_pCUtil->GetValueChar(uiMsg, 34, 4);
	m_StockDirectory.cInverseFactor			= m_pCUtil->GetValueChar(uiMsg, 38, 1);	
****************************************************************************************
	m_pDBLayer->StockDirectoryUpdate(1,
 	m_StockDirectory.cMessageType,
	m_StockDirectory.iSockLocale,
	m_StockDirectory.iTracking_Number,
	m_StockDirectory.iTimeStamp,
 	m_StockDirectory.strStock,
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
************************************************************************************************

	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::StockTradingAction(UINT8* uiMsg)
{

	memset(&m_StockTradingAction, '\0', sizeof(STOCK_TRADING_ACTION_MESSAGE ));

	m_StockTradingAction.cMessageType			= 'H';
	m_StockTradingAction.iStockLocate			=  m_pCUtil->GetValueUnsignedLong(uiMsg, 1, 2);
	m_StockTradingAction.iTrackingNumber		= m_pCUtil->GetValueUnsignedLong(uiMsg, 3, 2);
	m_StockTradingAction.iTimeStamp				= m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	
	strcpy(m_StockTradingAction.Symbol, m_pCUtil->GetValueAlpha( uiMsg,11, 8));
	m_StockTradingAction.cTradingState = m_pCUtil->GetValueChar(uiMsg, 19, 1);

	m_StockTradingAction.cReserved = m_pCUtil->GetValueChar(uiMsg, 20, 1); 
	strcpy(m_StockTradingAction.strReason , m_pCUtil->GetValueAlpha(uiMsg, 21, 4));

****************************************************************************************
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
************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::RegShoRestriction(UINT8* uiMsg)
{
	memset(&m_RegShoRestricted, '\0', sizeof(REG_SHO_RESTRICTION_MESSAGE));
	
	m_RegShoRestricted.cMessage			= 'Y';
	m_RegShoRestricted.iLocateCode		= m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_RegShoRestricted.iTimeStamp		= m_pCUtil->GetValueUnsignedInt64(uiMsg, 5, 6);
	strcpy(m_RegShoRestricted.Symbol, m_pCUtil->GetValueAlpha( uiMsg,11, 8));
	m_RegShoRestricted.cRegSHOAction	= m_pCUtil->GetValueChar (uiMsg, 19, 1);

****************************************************************************************
	m_pDBLayer->RegShoRestriction( 
 	m_RegShoRestricted.cMessage,
	m_RegShoRestricted.iLocateCode,
	m_RegShoRestricted.iTimeStamp,
	m_RegShoRestricted.Symbol,
	m_RegShoRestricted.cRegSHOAction);
************************************************************************************************

	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::Market_Participant_Position(UINT8* uiMsg)
{
	memset(&m_MPPosition, '\0', sizeof(MP_POSITION_MESSAGE ));

	m_MPPosition.cMessageType		= 'Y';
	m_MPPosition.iLocateCode		= m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_MPPosition.TrackingNumber		=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_MPPosition.iTimeStamp			= m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	strcpy(m_MPPosition.strMPID, m_pCUtil->GetValueAlpha( uiMsg, 11, 4));
	strcpy(m_MPPosition.Stock, m_pCUtil->GetValueAlpha( uiMsg, 15, 8));
	m_MPPosition.cPrimaryMM			= m_pCUtil->GetValueChar(uiMsg, 23, 1);
	m_MPPosition.cMMMode			= m_pCUtil->GetValueChar(uiMsg, 24, 1);
	m_MPPosition.cMarketParticipantState = m_pCUtil->GetValueChar(uiMsg, 25, 1);

************************************************************************************************
	m_pDBLayer->MarketParticipantPosition(m_MPPosition.cMessageType,
		m_MPPosition.iLocateCode,
		m_MPPosition.TrackingNumber,
		m_MPPosition.iTimeStamp,
		m_MPPosition.strMPID, 
		m_MPPosition.Stock,
		m_MPPosition.cPrimaryMM,
		m_MPPosition.cMMMode,
		m_MPPosition.cMarketParticipantState
		);
************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::MWCBDeclineLevelMessage(UINT8* uiMsg)
{
	memset(&m_MWCBDLM, '\0', sizeof(MWCBDLM_MESSAGE));

	m_MWCBDLM.cMessageType = 'V';
	m_MWCBDLM.iLocateCode = m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_MWCBDLM.TrackingNumber =  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_MWCBDLM.iTimeStamp	=  m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_MWCBDLM.dLevel1      =   double (m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8))/100000000;
	m_MWCBDLM.dLevel2      =   double (m_pCUtil->GetValueUnsignedInt64(uiMsg, 19, 8))/100000000;
	m_MWCBDLM.dLevel3		=  double (m_pCUtil->GetValueUnsignedInt64(uiMsg, 27, 8))/100000000;

************************************************************************************************
	m_pDBLayer->MWCBDeclineLevelMessage(m_MWCBDLM.cMessageType,
		m_MWCBDLM.iLocateCode,
		m_MWCBDLM.TrackingNumber,
		m_MWCBDLM.iTimeStamp,
		m_MWCBDLM.dLevel1,
		m_MWCBDLM.dLevel2,
		m_MWCBDLM.dLevel3);

***********************************************************************************************

	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::MWCBBreachMessage(UINT8* uiMsg)
{
	memset(&m_MWCBDBM, '\0', sizeof(MWCBDBM_MESSAGE));

	m_MWCBDBM.cMessageType = 'W';
	m_MWCBDBM.iLocateCode = m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_MWCBDBM.TrackingNumber =  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_MWCBDBM.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_MWCBDBM.cBreachLevel   = (short) m_pCUtil->GetValueChar( uiMsg, 11, 1);

************************************************************************************************
	m_pDBLayer->MWCBBreachMessage(m_MWCBDBM.cMessageType,
		m_MWCBDBM.iLocateCode,
		m_MWCBDBM.TrackingNumber,
		m_MWCBDBM.iTimeStamp,
		m_MWCBDBM.cBreachLevel);

************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::IPOQuotingPeriodUpdate(UINT8* uiMsg)
{
	memset(&m_IPOQuotationPeriod, '\0', sizeof(IPO_QUOTATION_PERIOD_UPDATE_MESSAGE));

	m_IPOQuotationPeriod.cMessageType = 'K';
	m_IPOQuotationPeriod.iLocateCode =  m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_IPOQuotationPeriod.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_IPOQuotationPeriod.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	strcpy(m_IPOQuotationPeriod.Stock, m_pCUtil->GetValueAlpha( uiMsg, 11, 8));

	m_IPOQuotationPeriod.iIPOQuotationReleaseTime = m_pCUtil->GetValueUnsignedLong( uiMsg, 19, 4);
	m_IPOQuotationPeriod.cIPOQuotationReleaseQualifier = m_pCUtil->GetValueChar( uiMsg, 23, 1);
	m_IPOQuotationPeriod.dIPOPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 24, 4))/10000;

************************************************************************************************
	m_pDBLayer->IPOQuotingPeriodUpdate(m_IPOQuotationPeriod.cMessageType,
		m_IPOQuotationPeriod.iLocateCode,
		m_IPOQuotationPeriod.TrackingNumber,
		m_IPOQuotationPeriod.iTimeStamp,
		m_IPOQuotationPeriod.Stock,
		m_IPOQuotationPeriod.iIPOQuotationReleaseTime,
		m_IPOQuotationPeriod.cIPOQuotationReleaseQualifier,
		m_IPOQuotationPeriod.dIPOPrice
		);
************************************************************************************************

	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::AddOrderNoMPIDMessage(UINT8* uiMsg)
{
	memset(&m_AddOrderNoMPID, '\0', sizeof(ADD_ORDER_NO_MPID_MESSAGE ));

	m_AddOrderNoMPID.cMessageType = 'A';
	m_AddOrderNoMPID.iLocateCode =   m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_AddOrderNoMPID.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_AddOrderNoMPID.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_AddOrderNoMPID.iOrderRefNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_AddOrderNoMPID.cBuySell        = m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_AddOrderNoMPID.iShares		=  m_pCUtil->GetValueUnsignedLong(uiMsg, 20, 4);

	strcpy(m_AddOrderNoMPID.Stock, m_pCUtil->GetValueAlpha( uiMsg, 24, 8));
	m_AddOrderNoMPID.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;

************************************************************************************************
	m_pDBLayer->AddOrderMPID(m_AddOrderNoMPID.cMessageType,
		m_AddOrderNoMPID.iLocateCode,
		m_AddOrderNoMPID.TrackingNumber,
		m_AddOrderNoMPID.iTimeStamp,
		m_AddOrderNoMPID.iOrderRefNumber, 
		m_AddOrderNoMPID.cBuySell,
		m_AddOrderNoMPID.iShares,
		m_AddOrderNoMPID.Stock,
		m_AddOrderNoMPID.dPrice,
		"NSDQ");
************************************************************************************************
	// The NO_MPID table has retiered...Will attribute the order to NSDQ as the MPID
	 
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
////////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::AddOrderWithMPID(UINT8* uiMsg)
{
	memset(&m_AddOrderMPID, '\0', sizeof(ADD_ORDER_MPID_MESSAGE ));

	m_AddOrderMPID.cMessageType = 'F';
	m_AddOrderMPID.iLocateCode =    m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_AddOrderMPID.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_AddOrderMPID.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_AddOrderMPID.iOrderRefNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_AddOrderMPID.cBuySell        = m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_AddOrderMPID.iShares		=  m_pCUtil->GetValueUnsignedLong(uiMsg, 20, 4);

	strcpy(m_AddOrderMPID.Stock, m_pCUtil->GetValueAlpha( uiMsg, 24, 8));
	m_AddOrderMPID.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;
	strcpy(m_AddOrderMPID.strMPID,  m_pCUtil->GetValueAlpha(uiMsg, 36, 4));

//************************************************************************************************
	m_pDBLayer->AddOrderMPID(m_AddOrderMPID.cMessageType,
		m_AddOrderMPID.iLocateCode,
		m_AddOrderMPID.TrackingNumber,
		m_AddOrderMPID.iTimeStamp,
		m_AddOrderMPID.iOrderRefNumber, 
		m_AddOrderMPID.cBuySell,
		m_AddOrderMPID.iShares,
		m_AddOrderMPID.Stock,
		m_AddOrderMPID.dPrice,
		m_AddOrderMPID.strMPID);
//************************************************************************************************
	return 0;
}
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderExecutionMessage(UINT8* uiMsg)
{
	memset(&m_OrderExecuted, '\0', sizeof(ORDER_EXECUTED_MESSAGE));

	m_OrderExecuted.cMessageType = 'E';
	m_OrderExecuted.iLocateCode =    m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_OrderExecuted.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_OrderExecuted.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_OrderExecuted.iOrderRefNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_OrderExecuted.iShares = m_pCUtil->GetValueUnsignedLong(uiMsg, 19, 4);
	m_OrderExecuted.iOrderMatchNumber = m_pCUtil->GetValueUnsignedInt64(uiMsg, 23, 8);

************************************************************************************************
	m_pDBLayer->OrderExecuted(m_OrderExecuted.cMessageType,
	m_OrderExecuted.iLocateCode,
	m_OrderExecuted.TrackingNumber,
	m_OrderExecuted.iTimeStamp,
	m_OrderExecuted.iOrderRefNumber,
	m_OrderExecuted.iShares,
	m_OrderExecuted.iOrderMatchNumber);
************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderExecutionWithPriceMessage(UINT8* uiMsg)
{
	memset(&m_OrderExecutedWithPrice, '\0', sizeof(ORDER_EXECUTED_WITH_PRICE_MESSAGE ));

	m_OrderExecutedWithPrice.cMessageType = 'c';
	m_OrderExecutedWithPrice.iLocateCode =    m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_OrderExecutedWithPrice.TrackingNumber=  m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_OrderExecutedWithPrice.iTimeStamp     = m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_OrderExecutedWithPrice.iOrderRefNumber = m_pCUtil->GetValueUnsignedLong(uiMsg, 11, 8); 
	m_OrderExecutedWithPrice.iShares = m_pCUtil->GetValueUnsignedLong(uiMsg, 19, 4);
	m_OrderExecutedWithPrice.iOrderMatchNumber = m_pCUtil->GetValueUnsignedLong(uiMsg, 23, 8);

	m_OrderExecutedWithPrice.cPrintable = m_pCUtil->GetValueChar(uiMsg, 31, 1);
	m_OrderExecutedWithPrice.dExecutionPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;

************************************************************************************************
	m_pDBLayer->OrderExecutedWithPrice(m_OrderExecutedWithPrice.cMessageType,
	m_OrderExecutedWithPrice.iLocateCode,
	m_OrderExecutedWithPrice.TrackingNumber,
	m_OrderExecutedWithPrice.iTimeStamp,
	m_OrderExecutedWithPrice.iOrderRefNumber,
	m_OrderExecutedWithPrice.iShares,
	m_OrderExecutedWithPrice.iOrderMatchNumber,
	m_OrderExecutedWithPrice.cPrintable,
	m_OrderExecutedWithPrice.dExecutionPrice);
************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderCancelMessage(UINT8* uiMsg)
{
	memset(&m_OrderCancel, '\0', sizeof(ORDER_CANCEL_MESSAGE ));

	m_OrderCancel.cMessageType		=	'X';
	m_OrderCancel.iLocateCode		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_OrderCancel.TrackingNumber	=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_OrderCancel.iTimeStamp		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_OrderCancel.iOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_OrderCancel.iShares			=	m_pCUtil->GetValueUnsignedLong(uiMsg, 19, 4);

************************************************************************************************
	m_pDBLayer->OrderCancel(m_OrderCancel.cMessageType, 
							m_OrderCancel.iLocateCode,
							m_OrderCancel.TrackingNumber,
							m_OrderCancel.iTimeStamp,
							m_OrderCancel.iOrderRefNumber,
							m_OrderCancel.iShares);

************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderDelete(UINT8* uiMsg)
{
	memset(&m_OrderDelete  , '\0', sizeof(ORDER_DELETE_MESSAGE ));

	m_OrderDelete.cMessageType		=	'D';
	m_OrderDelete.iLocateCode		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_OrderDelete.TrackingNumber	=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_OrderDelete.iTimeStamp		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_OrderDelete.iOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 

************************************************************************************************
	m_pDBLayer->OrderDelete(m_OrderDelete.cMessageType,
							m_OrderDelete.iLocateCode,
							m_OrderDelete.TrackingNumber,
							m_OrderDelete.iTimeStamp,
							m_OrderDelete.iOrderRefNumber);
************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderReplace(UINT8* uiMsg)
{
	memset(&m_OrderReplace, '\0', sizeof(ORDER_REPLACE_MESSAGE ));

	m_OrderReplace.cMessageType			= 'U';
	m_OrderReplace.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_OrderReplace.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_OrderReplace.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_OrderReplace.iOldOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 
	m_OrderReplace.iNewOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 19, 8); 

	m_OrderReplace.iShares				=   m_pCUtil->GetValueUnsignedLong(uiMsg, 27, 4);
	m_OrderReplace.dPrice				=   double (m_pCUtil->GetValueUnsignedLong(uiMsg, 31, 4))/10000;

************************************************************************************************
	m_pDBLayer->OrderReplace(m_OrderReplace.cMessageType,
							m_OrderReplace.iLocateCode,
							m_OrderReplace.TrackingNumber,
							m_OrderReplace.iTimeStamp,
							m_OrderReplace.iOldOrderRefNumber,
							m_OrderReplace.iNewOrderRefNumber,
							m_OrderReplace.iShares,
							m_OrderReplace.dPrice
							);
************************************************************************************************
	return 0; // :: TODO 
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::TradeMessageNonCross(UINT8* uiMsg)
{
	memset(&m_TradeNonCross  , '\0', sizeof(TRADE_NON_CROSS_MESSAGE ));

	m_TradeNonCross.cMessageType		= 'P';
	m_TradeNonCross.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_TradeNonCross.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_TradeNonCross.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_TradeNonCross.iOrderRefNumber		=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8); 

	m_TradeNonCross.cBuySell = m_pCUtil->GetValueChar(uiMsg, 19, 1);
	m_TradeNonCross.iShares = m_pCUtil->GetValueUnsignedLong(uiMsg, 20, 4);
	strcpy(m_TradeNonCross.Stock,  m_pCUtil->GetValueAlpha(uiMsg, 24, 8));
	m_TradeNonCross.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;
	m_TradeNonCross.iMatchNumber  = m_pCUtil->GetValueUnsignedInt64(uiMsg, 36, 8);

	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::NOII(UINT8* uiMsg)
{
	memset(&m_NOII  , '\0', sizeof(NOII_MESSAGE ));

	m_NOII.cMessageType			= 'I';
	m_NOII.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_NOII.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_NOII.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	m_NOII.iPairedShares		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 11, 8);
	m_NOII.iImbalanceShares     =  m_pCUtil->GetValueUnsignedInt64( uiMsg, 19, 8);
	m_NOII.iImbalanceDirection  =  m_pCUtil->GetValueChar( uiMsg, 27, 1);
	strcpy(m_NOII.Stock, m_pCUtil->GetValueAlpha( uiMsg, 28, 8));

	m_NOII.dFarPrice =  double (m_pCUtil->GetValueUnsignedLong(uiMsg, 36, 4))/10000;
	
	m_NOII.dNearPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 40, 4))/10000;
	m_NOII.dRefPrice  = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 44, 4))/10000;
	m_NOII.cCrossType = m_pCUtil->GetValueChar(uiMsg, 48, 1);
	m_NOII.cPriceVariation = m_pCUtil->GetValueChar(uiMsg, 49, 1);

************************************************************************************************
	m_pDBLayer->NOII(	m_NOII.cMessageType,
						m_NOII.iLocateCode,
						m_NOII.TrackingNumber,
						m_NOII.iTimeStamp,
						m_NOII.iPairedShares,
						m_NOII.iImbalanceShares,
						m_NOII.iImbalanceDirection,
						m_NOII.Stock,
						m_NOII.dFarPrice,
						m_NOII.dNearPrice,
						m_NOII.dRefPrice,
						m_NOII.cCrossType,
						m_NOII.cPriceVariation
						);
************************************************************************************************

	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::RetailPriceImprovementIndicator(UINT8* uiMsg)
{
	memset(&m_RPPI  , '\0', sizeof(RPII_MESSAGE ));

	m_RPPI.cMessageType			=	'N';
	m_RPPI.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
	m_RPPI.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
	m_RPPI.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
	strcpy(m_RPPI.Stock, m_pCUtil->GetValueAlpha( uiMsg, 11, 8));

	m_RPPI.cInterestFlag  = m_pCUtil->GetValueChar(uiMsg, 19, 1);

************************************************************************************************
	m_pDBLayer->RPII(	m_RPPI.cMessageType,
						m_RPPI.iLocateCode,
						m_RPPI.TrackingNumber,
						m_RPPI.iTimeStamp,
						m_RPPI.Stock,
						m_RPPI.cInterestFlag
						);
************************************************************************************************
	return 0;
}
///////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////
FEED_MESSAGE_STATS  CFillMsgStructs::GetStats()
{
	return theApp.g_Stats;
}
///////////////////////////////////////////////////////////////////////////*/

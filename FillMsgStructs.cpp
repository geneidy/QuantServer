
#include "FillMsgStructs.h"
#include "NQTV.h"
#include "Logger.h"
#include <stdio.h>
#include <time.h>
#include "memory.h"

#include <sys/fcntl.h>
#include <sys/stat.h>

#include "Logger.h"
#include "Includes.h"


char strBuff[39];
//struct _timeb timebuffer;

#define  _SP   0
uint32_t iStopHere = 0;
uint32_t iPassedHere = 0;
////////////////////////////////////////////////////////////////////////////
CFillMsgStructs::CFillMsgStructs(CQuantQueue* pQuantQueue): m_pQuantQueue(pQuantQueue)
{

    time_t ltime = 0;
    struct tm stToday;

    time( &ltime );
    localtime_r( &ltime ,  &stToday);

    memset(&theApp.g_Stats, 0 , sizeof(FEED_MESSAGE_STATS));
    memset(theApp.g_Stats.strStartTime, '\0', sizeof(theApp.g_Stats.strStartTime));
    strftime(theApp.g_Stats.strStartTime, sizeof(theApp.g_Stats.strStartTime), "%Y-%m-%d- %H:%M:%S" , &stToday);
//	strcpy(theApp.g_Stats.strStartTime, m_pCUtil->GetFormatedTime());

    memset(theApp.g_arrTotalMessages, 0, sizeof(theApp.g_arrTotalMessages));
//    memset(theApp.g_arrMessagesPerSec, 0, sizeof(theApp.g_arrMessagesPerSec));
//    memset(theApp.g_arrMaxMessagesPerSec, 0, sizeof(theApp.g_arrMaxMessagesPerSec));
    m_pCUtil	= nullptr;
    m_pCUtil	= new CUtil(theApp.SSettings.szActiveSymbols, theApp.SSettings.arrbActive);  // After refactor

    m_uiRejected = 0;
    m_bConnected = false;

    m_iError = 0;

    i64Counter = 0;

  //    m_pQuantQueue = NULL;
//    m_pQuantQueue = CQuantQueue::Instance();   // Only one instance is allowed of this singelton class

    if (!m_pQuantQueue) {
        Logger::instance().log("Error initializing the Queue", Logger::Error);
        m_iError = 100;  // ::TODO enum the Error
    }
    else {
        Logger::instance().log("Queue initialized in Fill Messages", Logger::Info);
    }
    m_remain.tv_sec = 0;
    m_remain.tv_nsec = 0;

    m_request.tv_sec = 0;
    m_request.tv_nsec = 10000000;   // 1/100 of a second

// Get the file ready to store the stock directory
    struct stat st = {0};

    if (stat("../StockDirectory/", &st) == -1) {
        mkdir("../StockDirectory/", 0700);
    }

    string strTickFile;
    strTickFile.empty();

    strTickFile = "../StockDirectory/";
    strTickFile += m_pCUtil->GetFormatedDate();
    strTickFile += "StockDirectory.qtx";

//    m_fd = open64("./Ticks/QuanticksTickData.Qtx", O_RDWR|O_CREAT, S_IRWXU);
    m_fd = open(strTickFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);

    if (m_fd == -1) {
        Logger::instance().log("Error opening Stock Direcotry File", Logger::Error);
    }

    m_pOrdersMap = nullptr;
    m_pOrdersMap = COrdersMap::instance();
    if (!m_pOrdersMap){
      m_iError = 1000;   // enum later
    }
}
////////////////////////////////////////////////////////////////////////////
int CFillMsgStructs::GetError()
{

    return m_iError;
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

    //	strcpy(theApp.g_Stats.strEndTime, m_pCUtil->GetFormatedTime());

    if (m_pCUtil)
    {
        delete	m_pCUtil;
        m_pCUtil = nullptr;
    }

    if (m_fd) {
        close(m_fd);
    }

    m_bConnected = false;
    m_pOrdersMap->iNInstance--;
}
////////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::DirectToMethod(UINT8* uiMsg)
{

    switch (uiMsg[0])
    {
    case 'S':
        SystemEvent(uiMsg);
        theApp.g_arrTotalMessages[SYSTEM_EVENT ]++;
        break;
    case 'R':
        StockDirectory(uiMsg);
        theApp.g_arrTotalMessages[STOCK_DIRECTORY]++;
        break;
    case 'H':
        StockTradingAction(uiMsg);
        theApp.g_arrTotalMessages[STOCK_TRADING_ACTION]++;
        break;
    case 'Y':
        RegShoRestriction(uiMsg);
        theApp.g_arrTotalMessages[REG_SHO]++;
        break;
    case 'L':
        Market_Participant_Position(uiMsg);
        theApp.g_arrTotalMessages[MARKET_PART_POSITION]++;
        break;
    case 'V':
        MWCBDeclineLevelMessage(uiMsg);
        theApp.g_arrTotalMessages[MWCB_DECLINE_LEVEL]++;
        break;
    case 'W':
        MWCBBreachMessage(uiMsg);
        theApp.g_arrTotalMessages[MWCB_BREACH]++;
        break;
    case 'K':
        IPOQuotingPeriodUpdate(uiMsg);
        theApp.g_arrTotalMessages[IPO_QUOTING_PERIOD_UPDATE]++;
        break;
    case 'A':
        AddOrderNoMPIDMessage(uiMsg);
        theApp.g_arrTotalMessages[ADD_ORDER_NO_MPID]++;
        break;
    case 'F':
        AddOrderWithMPID(uiMsg);
        theApp.g_arrTotalMessages[ADD_ORDER_WTIH_MPID]++;
        break;
    case 'E':
        OrderExecutionMessage(uiMsg);
        theApp.g_arrTotalMessages[ORDER_EXECUTION]++;
        break;
    case 'c':
        OrderExecutionWithPriceMessage(uiMsg);
        theApp.g_arrTotalMessages[ORDER_EXECUTION_WITH_PRICE]++;
        break;
    case 'X':
        OrderCancelMessage(uiMsg);
        theApp.g_arrTotalMessages[ORDER_CANCEL]++;
        break;
    case 'D':
        OrderDelete(uiMsg);
        theApp.g_arrTotalMessages[ORDER_DELETE]++;
        break;
    case 'U':
        OrderReplace(uiMsg);
        theApp.g_arrTotalMessages[ORDER_REPLACE]++;
        break;
    case 'P':
      TradeMessageNonCross(uiMsg);
      //From documentation
      //Since Trade Messages do NOT affect the book, however, they may be ignored by firms just looking to build and track the NASDAQ execution system display.
        theApp.g_arrTotalMessages[TRADE_MESSAGE_NON_CROSS]++;
        break;
    case 'I':
        NOII(uiMsg);
        theApp.g_arrTotalMessages[NETWORK_ORDER_INBALANCE]++;
        break;
    case 'N':
        RetailPriceImprovementIndicator(uiMsg);
        theApp.g_arrTotalMessages[RETAIL_PRICE_IMPROVEMENT]++;
        break;
    default:  // we do not process this type of messages
        break;
    };
    theApp.g_arrTotalMessages[TOTAL_MESSAGES]++;

    /*    // :: TODO Throw away code
        if (i64Counter++ > 217000) { //  Addd messages start at 217090
    	nanosleep(&m_request, NULL);  // sleep a 1/10 of a second
        }
    */
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

    theApp.SSettings.iSystemEventCode = m_IMUSys.SystemEvent.cEventCode; // Available System Wide

    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'S');

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
    m_IMUSys.StockDirectory.eFSI			= m_pCUtil->GetValueChar(uiMsg, 20,1);
    m_IMUSys.StockDirectory.iRoundLotSize		= m_pCUtil->GetValueUnsignedLong( uiMsg, 21, 4);
    m_IMUSys.StockDirectory.cRoundLotsOnly         	=  m_pCUtil->GetValueChar(  uiMsg, 25, 1);
    m_IMUSys.StockDirectory.cIssueClassification	= m_pCUtil->GetValueChar(uiMsg, 26, 1);
    strcpy(m_IMUSys.StockDirectory.strIssueSubType, m_pCUtil->GetValueAlpha(uiMsg, 27, 2));
    m_IMUSys.StockDirectory.cAuthenticity		=  m_pCUtil->GetValueChar(uiMsg, 29, 1);
    m_IMUSys.StockDirectory.cShortSaleThresholdInd = m_pCUtil->GetValueChar(uiMsg, 30, 1);

    m_IMUSys.StockDirectory.cIPOFlag			= m_pCUtil->GetValueChar(uiMsg, 31, 1);
    m_IMUSys.StockDirectory.cLULDRefPrice		= m_pCUtil->GetValueChar(uiMsg, 32, 1);
    m_IMUSys.StockDirectory.cETPFlag			= m_pCUtil->GetValueChar(uiMsg, 33, 1);
    m_IMUSys.StockDirectory.iETPLeverageFactor		= m_pCUtil->GetValueChar(uiMsg, 34, 4);
    m_IMUSys.StockDirectory.cInverseFactor		= m_pCUtil->GetValueChar(uiMsg, 38, 1);

    /*    if (m_pQuantQueue)
            m_pQuantQueue->Enqueue(&m_IMUSys, 'R');
    */
    if (m_fd != -1)
        write(m_fd, &m_IMUSys.StockDirectory , sizeof(STOCK_DIRECTORY_MESSAGE ));

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
    strcpy(m_IMUSys.StockTradingAction.szReason , m_pCUtil->GetValueAlpha(uiMsg, 21, 4));

    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'H');

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

    if (!m_pCUtil->IsSymbolIn(m_IMUSys.MpPosition.szStock)) {
        m_uiRejected++;
        return 1;  // log later
    }


    m_IMUSys.MpPosition.cPrimaryMM			= m_pCUtil->GetValueChar(uiMsg, 23, 1);
    m_IMUSys.MpPosition.cMMMode			= m_pCUtil->GetValueChar(uiMsg, 24, 1);
    m_IMUSys.MpPosition.cMarketParticipantState = m_pCUtil->GetValueChar(uiMsg, 25, 1);

    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'L');

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
    m_IMUSys.MWCBDLM.dLevel3	  =  double (m_pCUtil->GetValueUnsignedInt64(uiMsg, 27, 8))/100000000;


    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'V');


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

    if (!m_pCUtil->IsSymbolIn(m_IMUSys.IPOQutationUpdate.szStock)) {
        m_uiRejected++;
        return 1;  // log later
    }


    m_IMUSys.IPOQutationUpdate.iIPOQuotationReleaseTime = m_pCUtil->GetValueUnsignedLong( uiMsg, 19, 4);
    m_IMUSys.IPOQutationUpdate.cIPOQuotationReleaseQualifier = m_pCUtil->GetValueChar( uiMsg, 23, 1);
    m_IMUSys.IPOQutationUpdate.dIPOPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 24, 4))/10000;

    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'K');


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

    if (!m_pCUtil->IsSymbolIn(m_IMUSys.AddOrderNoMPID.szStock)) {
        m_uiRejected++;
        return 1;  // log later
    }

    m_IMUSys.AddOrderNoMPID.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;

/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'A');
*/
     if (m_pOrdersMap) {
	m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'A');
     }
     
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

    if (!m_pCUtil->IsSymbolIn(m_IMUSys.AddOrderMPID.szStock)) {
        m_uiRejected++;
        return 1;  // log later
    }

    m_IMUSys.AddOrderMPID.dPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;
    strcpy(m_IMUSys.AddOrderMPID.szMPID,  m_pCUtil->GetValueAlpha(uiMsg, 36, 4));
/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'F');*/

     if (m_pOrdersMap) {
	m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'F');
     }



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
/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'E');*/

    if (m_pOrdersMap) {
	m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'E');
    }

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

    m_IMUSys.OrderExecutedWithPrice.dExecutionPrice = double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;
    m_IMUSys.OrderExecutedWithPrice.cPrintable = m_pCUtil->GetValueChar(uiMsg, 31, 1);
/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'c');

 */

  if (m_pOrdersMap) {
    m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'c');
  }

  return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderCancelMessage(UINT8* uiMsg)
{
    memset(&m_IMUSys.OrderCancel, '\0', sizeof(ORDER_CANCEL_MESSAGE ));

    m_IMUSys.OrderCancel.cMessageType		=	'X';
    m_IMUSys.OrderCancel.iLocateCode		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
    m_IMUSys.OrderCancel.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
    m_IMUSys.OrderCancel.iTimeStamp		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
    m_IMUSys.OrderCancel.iOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8);
    m_IMUSys.OrderCancel.iShares		=	m_pCUtil->GetValueUnsignedLong(uiMsg, 19, 4);
/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'X');
*/
     if (m_pOrdersMap) {
	m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'X');
     }
    return 0;
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::OrderDelete(UINT8* uiMsg)
{
    memset(&m_IMUSys.OrderDelete  , '\0', sizeof(ORDER_DELETE_MESSAGE ));

    m_IMUSys.OrderDelete.cMessageType		=	'D';
    m_IMUSys.OrderDelete.iLocateCode		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
    m_IMUSys.OrderDelete.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
    m_IMUSys.OrderDelete.iTimeStamp		=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
    m_IMUSys.OrderDelete.iOrderRefNumber	=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8);

/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'D');
*/
     if (m_pOrdersMap) {
	m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'D');
     }
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
    m_IMUSys.OrderReplace.iOldOrderRefNumber		=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8);
    m_IMUSys.OrderReplace.iNewOrderRefNumber		=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 19, 8);

    m_IMUSys.OrderReplace.iShares			=   m_pCUtil->GetValueUnsignedLong(uiMsg, 27, 4);
    m_IMUSys.OrderReplace.dPrice			=   double (m_pCUtil->GetValueUnsignedLong(uiMsg, 31, 4))/10000;
/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'U');
*/
     if (m_pOrdersMap) {
	m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'U');
     }
    return 0; // :: TODO
}
///////////////////////////////////////////////////////////////////////////
int  CFillMsgStructs::TradeMessageNonCross(UINT8* uiMsg)
{
  /*
  From documentation:
  Since Trade Messages do NOT affect the book, however, they may be ignored by firms just looking to build and track the NASDAQ execution system display.
  */
  
    memset(&m_IMUSys.TradeNonCross  , '\0', sizeof(TRADE_NON_CROSS_MESSAGE ));

    m_IMUSys.TradeNonCross.cMessageType		= 'P';
    m_IMUSys.TradeNonCross.iLocateCode			=	m_pCUtil->GetValueUnsignedLong( uiMsg, 1, 2);
    m_IMUSys.TradeNonCross.TrackingNumber		=	m_pCUtil->GetValueUnsignedLong( uiMsg, 3, 2);
    m_IMUSys.TradeNonCross.iTimeStamp			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 5, 6);
    m_IMUSys.TradeNonCross.iOrderRefNumber		=	m_pCUtil->GetValueUnsignedInt64(uiMsg, 11, 8);

    m_IMUSys.TradeNonCross.cBuySell 			= m_pCUtil->GetValueChar(uiMsg, 19, 1);
    m_IMUSys.TradeNonCross.iShares 			= m_pCUtil->GetValueUnsignedLong(uiMsg, 20, 4);
    strcpy(m_IMUSys.TradeNonCross.szStock,  m_pCUtil->GetValueAlpha(uiMsg, 24, 8));

    if (!m_pCUtil->IsSymbolIn(m_IMUSys.TradeNonCross.szStock)) {
        m_uiRejected++;
        return 1;  // log later
    }


    m_IMUSys.TradeNonCross.dPrice 			= double (m_pCUtil->GetValueUnsignedLong(uiMsg, 32, 4))/10000;
    m_IMUSys.TradeNonCross.iMatchNumber  		= m_pCUtil->GetValueUnsignedInt64(uiMsg, 36, 8);

/*
    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'P');*/

     if (m_pOrdersMap) {
	m_pOrdersMap->FillMemoryMap(&m_IMUSys, 'P');
     }
    
    
    
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
    m_IMUSys.NOII.iPairedShares			=	m_pCUtil->GetValueUnsignedInt64( uiMsg, 11, 8);
    m_IMUSys.NOII.iImbalanceShares     		=  m_pCUtil->GetValueUnsignedInt64( uiMsg, 19, 8);
    m_IMUSys.NOII.iImbalanceDirection  		=  m_pCUtil->GetValueChar( uiMsg, 27, 1);
    strcpy(m_IMUSys.NOII.szStock, m_pCUtil->GetValueAlpha( uiMsg, 28, 8));

    if (!m_pCUtil->IsSymbolIn(m_IMUSys.NOII.szStock)) {
        m_uiRejected++;
        return 1;  // log later
    }


    m_IMUSys.NOII.dFarPrice =  double (m_pCUtil->GetValueUnsignedLong(uiMsg, 36, 4))/10000;

    m_IMUSys.NOII.dNearPrice 		= double (m_pCUtil->GetValueUnsignedLong(uiMsg, 40, 4))/10000;
    m_IMUSys.NOII.dRefPrice  		= double (m_pCUtil->GetValueUnsignedLong(uiMsg, 44, 4))/10000;
    m_IMUSys.NOII.cCrossType 		= m_pCUtil->GetValueChar(uiMsg, 48, 1);
    m_IMUSys.NOII.cPriceVariation 	= m_pCUtil->GetValueChar(uiMsg, 49, 1);

    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'I');

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

    if (!m_pCUtil->IsSymbolIn(m_IMUSys.RPPI.szStock)) {
        m_uiRejected++;
        return 1;  // log later
    }


    m_IMUSys.RPPI.cInterestFlag  = m_pCUtil->GetValueChar(uiMsg, 19, 1);

    if (m_pQuantQueue)
        m_pQuantQueue->Enqueue(&m_IMUSys, 'N');

    return 0;
}
///////////////////////////////////////////////////////////////////////////
FEED_MESSAGE_STATS  CFillMsgStructs::GetStats()
{
    return theApp.g_Stats;
}
/////////////////////////////////////////////////////////////////////////////


#include "SaveToDB.h"
#include "NQTV.h"



CSaveToDB::CSaveToDB()
{
  
}
int CSaveToDB::GetError()
{
   
    return ( m_iError);
 }
///////////////////////////////////////////////////////////
CSaveToDB::~CSaveToDB()
{
   
  
}
/*
  if (iHandle)
  {
    iNumberOfBytes = write( iHandle, &theApp.SSettings, sizeof(SETTINGS));
    close(iHandle);
  }
*/
///////////////////////////////////////////////////////////
/*
typedef struct QT_ITCHMessage_Format{
  uint_least64_t uiMessageSequence;
  
  char  cMessageType;   // same the one we get from NASDAQ
  
  union {
   SYSTEM_EVENT_MESSAGE  		SystemEvent;
   STOCK_DIRECTORY_MESSAGE  		StockDirectory;
   STOCK_TRADING_ACTION_MESSAGE 	StockTradingAction;
   REG_SHO_RESTRICTION_MESSAGE 		RegSho;
   MP_POSITION_MESSAGE			MpPosition;
   MWCBDLM_MESSAGE			MWCBDLM;
   MWCBDBM_MESSAGE			MWCBDBM;
   IPO_QUOTATION_PERIOD_UPDATE_MESSAGE	IPOQutationUpdate;
   ADD_ORDER_NO_MPID_MESSAGE		AddOrderNoMPID;
   ADD_ORDER_MPID_MESSAGE		AddOrderMPID;
   ORDER_EXECUTED_MESSAGE		OrderExecuted;
   ORDER_EXECUTED_WITH_PRICE_MESSAGE	OrderExecutedWithPrice;
   ORDER_CANCEL_MESSAGE			OrderCancel;
   ORDER_DELETE_MESSAGE			OrderDelete;
   ORDER_REPLACE_MESSAGE		OrderReplace;
   TRADE_NON_CROSS_MESSAGE		TradeNonCross;	
   NOII_MESSAGE				NOII;
   RPII_MESSAGE				RPPI;
  };
}QT_ITCH_MESSAGE;  // The format to save in a mempory mapped file 
*/
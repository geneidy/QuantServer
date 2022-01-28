/* 
 * This file is part of the QuantServer (https://github.com/geneidy/QuantServer).
 * Copyright (c) 2017 geneidy.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _DBLAYER_H_
#define _DBLAYER_H_

#include <mysql/mysql.h>

#include "ITCHMessages.h"

#define ADD_TO_TABLE		10
#define UPDATE_TO_TABLE		20
#define DELETE_FROM_TABLE	30

typedef struct
{

	int    SqlCode;
    char   strMessage[200];
}SRETVALUE;

class CDBLayer
{
	int    m_SqlCode;
    char   m_strMessage[200];
	
public:
	MYSQL_STMT*  pSqlStmt;
	
	

public:
	CDBLayer();
	~CDBLayer();
//	SRETVALUE Init();   // initialize the engine
//	SRETVALUE ConnectSharedMemory( char   *ConnectionName, char   *UserID,  char   *Password);
// 	SRETVALUE DisconnectSharedMemory();
// 	SRETVALUE ConnectTCP(char*  strServerName, char*  strDBName, char*  strConnectionName, char*  strUserID, char*  strPassword);
// 	SRETVALUE DisConnectTCP(char*  strServerName, char*  strDBName, char*  strConnectionName, char*  strUserID, char*  strPassword);
// 	SRETVALUE TruncateAllTables(int iCode);
// 	SRETVALUE Backup();
 	SRETVALUE	StockDirectoryUpdate(int iCode);
 	
 	SRETVALUE	StockDirectoryUpdate(int Opcode, char cMessageType, unsigned int iSockLocale, unsigned int iTracking_Number, uint64_t iTimeStamp, char *strStock, unsigned int     eMarketCategory, unsigned int     eFSI, unsigned int		iRoundLotSize, char	cRoundLotsOnly, char	cIssueClassification, char	*strIssueSubType, char	cAuthenticity, char    cShortSaleThresholdInd, char	cIPOFlag, char    cLULDRefPrice, char	cETPFlag, unsigned int		iETPLeverageFactor, char    cInverseFactor);
 	SRETVALUE    StockTradingAction	(int OpCode, char cMessageType, unsigned int iStockLocate, unsigned int iTrackingNumber,uint64_t iTimeStamp, char*	Symbol,	char cTradingState,	char cReserved,	char*   strReason);
 	SRETVALUE	MarketParticipantPosition(char cMessageType, unsigned int iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp,  char* strMPID, char* Stock,	char cPrimaryMM, char cMMMode, char cMarketParticipantState);
 
 	SRETVALUE	RPII(char cMessageType, unsigned int iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, char* Stock, char cInterestFlag);
 	SRETVALUE	SystemEvent( char cMessageType, unsigned int iStockLocale,  unsigned int iTrackingNumber, uint64_t iTimeStamp, char cEventCode);
 
 	SRETVALUE	RegShoRestriction( char cMessageType, unsigned int iLocateCode, uint64_t iTimeStamp, char* Symbol, char RegSHOAction);
 
 	SRETVALUE	MWCBDeclineLevelMessage(char cMessageType,	unsigned int iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, double dLevel1, double dLevel2, double dLevel3);
 	SRETVALUE	MWCBBreachMessage(char cMessageType, 	unsigned int iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, char cBreachLevel);
 
 	SRETVALUE	NOII(char cMessageType, unsigned int iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, uint64_t iPairedShares, uint64_t iImbalanceShares, unsigned int iImbalanceDirection, char *Stock,	double dFarPrice, double dNearPrice, double dRefPrice, char cCrossType,	char cPriceVariation);
 
 	SRETVALUE	IPOQuotingPeriodUpdate(char cMessageType, unsigned int iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp,	char* Stock, unsigned int iIPOQuotationReleaseTime,	char cIPOQuotationReleaseQualifier,	double dIPOPrice);
 
 	SRETVALUE	AddOrderNoMPID(char	MessageType, unsigned int LocateCode, unsigned int TrackingNumber, uint64_t  iTimeStamp, uint64_t iOrderRefNumber, char	cBuySell, unsigned int iShares, char* Symbol, double dPrice);
	SRETVALUE	AddOrderMPID(char	MessageType, unsigned int LocateCode, unsigned int TrackingNumber,uint64_t   iTimeStamp, uint64_t iOrderRefNumber, char	cBuySell, unsigned int iShares, char* Symbol, double dPrice, char* MPID);
 	SRETVALUE	OrderCancel(char cMessageType, unsigned int	iLocateCode, unsigned int iTrackingNumber,uint64_t  iTimeStamp, uint64_t iOrderRefNumber, unsigned int	iShares	);
 	SRETVALUE    OrderDelete(char cMessageType, unsigned int	iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, uint64_t	iOrderRefNumber);
 	SRETVALUE    OrderReplace(char cMessageType, unsigned int	iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, uint64_t	iOrderRefNumber, uint64_t	iNewOrderRefNumber, unsigned int iShares, double	dPrice);
 
 	SRETVALUE    OrderExecuted(char cMessageType, unsigned int	iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, uint64_t	iOrderRefNumber, unsigned int iShares, uint64_t  iOrderMatchNumber);
 	SRETVALUE    OrderExecutedWithPrice(char cMessageType, unsigned int	iLocateCode, unsigned int TrackingNumber, uint64_t iTimeStamp, uint64_t	iOrderRefNumber, unsigned int iShares, uint64_t  iOrderMatchNumber, char	cPrintable, double dExecutionPrice);
// 
// 
	SRETVALUE Commit();


	int GetErrorCode();   // returns m_SqlCode
	char* GetErrorDesc();  // returns m_strMessage

};
#endif

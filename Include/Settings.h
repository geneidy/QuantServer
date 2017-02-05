#pragma once


#include "Includes.h"

// The UI will have controls to fill the struct below.....in addition to the following controls:
// Start   
// Stop
// Pause
// Resume


typedef struct 
{
 // Server properties ... This server 

  char		szServerName[SIZE_OF_FILENAME];

  bool		bMemberOfFarm;	// Y/N
  char		szFarmName[SIZE_OF_FILENAME];
  uint		uiFarmPort;  // for incoming commands in case Member of bMemberOfFarm = Y  (range  5000...65000)
  
  int start_stop_pause;
  int 		iarrRole[NUMBER_OF_ROLES];  // Get it from enum   0= Receive Feed  1= Parse  2= Build Book  3= Save to DB    4= Play Back  5= Distributor

  uint 		uiDistListenOnPort;  // Case Y above....listen on which Port? (range  5000...65000)

// Partition to process

  bool  	bPartitionActive;  // Y/N to  process....can keep the partition info but in an inactive state

  char 		cBeginRange;  // e.g 'A'  or 'G'
  char 		cEndRange;	
  char  	szInclude[9];  // include from another range that was excluded from another partition
  char  	szExclude[9]; 	// Exclude to be included in another partition...eg AAPL
  
  
// Feed connection parameters case option 0
  char		szUserName[SIZE_OF_NAME];
  char		szPassword[SIZE_OF_PASSWORD];
  
  unsigned long	ulIPAddress;
  uint 		uiPort;

  unsigned long	ulIPAddress1;
  uint		uiPort1;

  unsigned long	dwBufferSize;
  
// ODBC connection parameters in case option 3 
  char  	szConnName[SIZE_OF_FILENAME];  // from ODBC
  char		szDBUserName[SIZE_OF_NAME];
  char		szDBPassword[SIZE_OF_PASSWORD];
 
// For Test Feed or Replay

  char		szTestFileName[SIZE_OF_FILENAME];  // Test file name ...pick from UI dialog  
  char		szPlayBackFileName[SIZE_OF_FILENAME];  // Test file name ...pick from UI dialog  
  uint	  	uiDelay;   // range...percent = 0..99       --  0 = full speed  50 = 1/2 speed  75 = 1/4 speed    // or suggest
  
// Logging 
  bool 		bLog;  // Y/N
  ushort 	usLoggingLevel;  // 0: Info   1: Errors  2: Warnings  3:All
  char  	szLogFileName[SIZE_OF_FILENAME];  // Entry field  
  
// System capacity
  uint		uiNumberOfIssues; // Max number of issues approx...9000 = default. Will reserve an entry for each issue in a hash table.
  uint64_t 	ui64SizeOfOrdersMappedFile; // Will set Default later...
  uint64_t 	ui64SizeOfTickDataMappedFile; // Will set Default later...
  
  uint64_t	uiQueueSize;
  
  bool		g_bConnected;
  bool		g_bReceiving;
  int		iStatus;
  int 		iSystemEventCode;  /*/ Get it from System Event Message from the parser....Make it global to know when the system
		Started or closed to set the Open and Close Price.....
	{"O", "Start of Messages. Outside of time stamp messages, the start of day message is the first message sent in any trading day"},
	{"S",  "Start of System hours. This message indicates that NASDAQ is open and ready to start accepting orders"},
	{"Q",  "Start of Market hours. This message is intended to indicate that Market Hours orders are available for execution"},
	{"M",  "End of Market hours. This message is intended to indicate that Market Hours orders are no longer available for execution"},
	{"E",   "End of System hours. It indicates that NASDAQ is now closed and will not accept any new orders today. It is still possible to receive Broken Trade messages and Order Delete messages after the End of Day"},
	{"C",	"End of Messages. This is always the last message sent in any trading day"}
		
*/
}SETTINGS;


/**************

	char const * szMessages [MAX_MESSAGE_TYPES] = {  // CARVED IN STONE....DO NOT CHANGE ORDER...WILL AFFECT the FillMsgStructs and the UI
	"SystemEvent",					//Index 0
	"Stock Directory",				//Index 1
	"Stock Trading Action",			//Index 2
	"Reg Sho Restriction",			//Index 3
	"Add Order",					//Index 4
	"Add Order With MPID",			//Index 5
	"Modify Order",					//Index 6
	"Order Executed",				//Index 7
	"Order Executed With Price",	//Index 8
	"Order Cancel",					//Index 9
	"Order Delete",					//Index 10
	"Order Replace",				//Index 11
	"Trade Non Cross",				//Index 12
	"Trade Cross",					//Index 13
	"Market Maker Position",		//Index 14
	"MWCB Decline Level",			//Index 15	//MarketWideCircuitBreaker"
	"MWCB Breach",					//Index 16
	"IPO Quoting Period",			//Index 17
	"Broken Trade",					//Index 18
	"NOI",							//Index 19
	"RPII",							//Index	20	// Retail price improvement indicator
	"Control",						//Index 21
	"Total Messages"				//Index 22
	};
*********************/


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
#pragma once
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <map>
#include <atomic>
#include <pthread.h>
#include <mutex>

#include "time.h"
#include "unordered_map"

#include "ITCHMessages.h"
#include "OrdersMap.h"
#include "QuantQueue.h"

typedef struct
{
    int iAskLevels;
    int iBidLevels;
} NLEVELS;

typedef struct
{
    uint32_t uiAttribAdd;
    uint32_t uiNonAttribAdd;
    uint32_t uiCancelled;
    uint32_t uiReplaced;
    uint32_t uiDeleted;
    uint32_t uiExecuted;
    uint32_t uiLevelDeleted;
} SLEVELSTAT; // consider adding per Second stats in the future

typedef struct // Moving averages
{
    double dAvg3;  // three day moving average
    double dAvg5;  // five day moving average
    double dAvg9;  // nine day moving average
    double dAvg11; // eleven day moving average
    double dAvg13; // thirteen day moving average
} SMOVING_AVERAGE;

typedef struct SBidAsk
{
    char szMPID[5];
    double dPrice;
    int uiQty;
    int uiNumOfOrders;
    //SLEVELSTAT  	SLevelStat;   // Stats per Level
    SBidAsk *pNextBidAsk; // for the linked list
} SBID_ASK;

typedef struct _OHLC
{
    double dLast;
    double dOpen;
    double dClose;
    double dHigh;
    double dLow;
    uint32_t uiVolume;
    int cTick; // '+'  '-'  '='

    double dVWAP;
    uint64_t uiNumOfTradesWithPrice;
    uint64_t uiNumOfTradesNoPrice;
    uint64_t uiTotalVolume;
    uint64_t uiTotalNumOfTrades;

    struct timeval tOpen;
    struct timeval tLastUpdate;
} OHLC;
typedef struct SBidAskValues // To copy SBID_ASK W/O the pointers....was causing a problem in delete
{
    char szMPID[5];
    double dPrice;
    int uiQty;
    int uiNumOfOrders;
    //SLEVELSTAT  	SLevelStat;   // Stats per Level
} SBID_ASK_VALUES;

typedef struct _SDisplayBook
{

    OHLC TopOfBook;
    SBID_ASK_VALUES *pSBid; // Number of levels apply here
    SBID_ASK_VALUES *pSAsk; // Number of levels apply here

} DISPLAYBOOK;

typedef struct _BookLevels // Per Symbol
{
    SBID_ASK *pTopBid;
    SBID_ASK *pTopAsk;

    uint16_t m_iBidLevels;
    uint16_t m_iAskLevels;
    bool bUpdated;
    OHLC m_OHLC;
    int iIndex; // index to keep track of all Mappings "m_addr" and others
    //    pthread_mutex_t      mtxBidAsk;
    //    pthread_mutexattr_t mtxAttr;

} SBOOK_LEVELS;

typedef unordered_map<string, SBOOK_LEVELS> BookMap; // <Stock Symbol  Book Levels>

class CBuildBook //: public COrdersMap
{
private: // by default
         //    void* 	m_addr;
    int m_fd;
    int m_iNextIndex;
    //    struct stat64 m_sb;

    uint64_t m_ui64NumRequest;

    CQuantQueue *m_pQuantQueue;
    ITCH_MESSAGES_UNION *m_pItchMessageUnion;
    COrdersMap *m_pCOrdersMap;

    pair<BookMap::iterator, bool> m_RetPairBookMap;

    double m_dPrice;
    unsigned int m_uiQty;
    char m_szMPID[5];
    string m_strMsg;

    uint32_t m_iSizeOfBook;
    string m_strPriceMM;

    COMMON_ORDER_MESSAGE *m_pCommonOrder;
    uint64_t m_uiNextOrder;

    ADD_ORDER_NO_MPID_MESSAGE m_Add_Order_No_Mpid;
    ADD_ORDER_MPID_MESSAGE m_Add_Order_Mpid;
    ORDER_REPLACE_MESSAGE m_OrderReplace;

    ORDER_EXECUTED_MESSAGE m_OrderExecuted;                     // 'E'
    ORDER_EXECUTED_WITH_PRICE_MESSAGE m_OrderExecutedWithPrice; // 'c'
    ORDER_CANCEL_MESSAGE m_OrderCancel;                         // 'X'
    char m_iMessage;
    //    timespec 	m_request;
    timespec m_remain;

    SBID_ASK *lpInsert;
    SBID_ASK *lpCurrent;
    SBID_ASK *lpPrevious;

    SBID_ASK *lpMM;
    SBID_ASK *lpPrevMM;

    SBID_ASK m_SBidAsk;

    SLEVELSTAT m_Stats;

    int InitMemoryMappedFile();
    int ProcessAdd(int iMessage);
    int ProcessReplace(int iMessage);
    int ProcessDelete(int iMessage);

    bool AddPriceLevel(int);
    int InitLevelStats();

    void UpdateBook();

    void *ListBookToMMapFile(SBOOK_LEVELS);
    int CreatLOBFileMapping(int iDx);
    int InitMemoryMappedFile(int iDx);

    inline string MakeKey();

    void ListBookStats();

    pair<BookMap::iterator, bool> m_RetPair;

    void CloseBook();

public:
    int m_iError;
    int BuildBookFromOrdersMap();
    CBuildBook();
    ~CBuildBook();

    /*static*/ BookMap m_BookMap;
    /*static*/ BookMap::iterator m_itBookMap;
    //    int 		ListBook(const char *szSymbol, uint32_t uiMaxLevels);

    SBID_ASK *AllocateNode(double fPrice, unsigned int uiQty);
    void InitOHLC();
    SBOOK_LEVELS m_pBook;

    SBID_ASK *m_pTopBid;
    SBID_ASK *m_pTopAsk;

    uint16_t m_iBidLevels;
    uint16_t m_iAskLevels;

    NLEVELS ListBook(char *szSymbol, int);
    NLEVELS FlushBook(char *szSymbol);
    NLEVELS FlushAllBooks();

private:
    /*************************  For Displaying Book    ***********************************/
    DISPLAYBOOK *m_SDisplayBook[NUMBER_OF_BOOKS_TO_DISPALY];
    int m_iFD[NUMBER_OF_BOOKS_TO_DISPALY];
    //int  CreatLOBFileMapping(int );
    //int  InitMemoryMappedFile(int );

    unsigned m_uiSizeOfLob[NUMBER_OF_BOOKS_TO_DISPALY];
    struct stat m_sb[NUMBER_OF_BOOKS_TO_DISPALY];
    struct stat m_st[NUMBER_OF_BOOKS_TO_DISPALY];
    void *m_addr[NUMBER_OF_BOOKS_TO_DISPALY];

    struct timespec m_request;
    /*************************  For Displaying Book    ***********************************/
};

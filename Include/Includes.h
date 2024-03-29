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


#define SIZE_OF_CLT_NAME	30+1
#define SIZE_OF_CLT_PASSWORD   30+1

#define SIZE_OF_NAME    30+1
#define SIZE_OF_PASSWORD 30+1
#define SIZE_OF_FILENAME   255

#define SIZE_OF_SYMBOL   8+1
#define NUMBER_OF_SYMBOLS   5

#define  UINT8  	unsigned char
#define  UINT		unsigned int
#define  ULONG		unsigned long

#define MAX_MESSAGE_TYPES		23

#define			NUMBER_OF_ROLES		14
#define			NUMBER_OF_BOOKS_TO_DISPALY  5

typedef unsigned long   DWORD;

enum FEED_STATE {
    FEED_01_STARTING = 1,
    FEED_02_STARTING,
    FEED_03_STARTING,

    FEED_01_RUNNING,
    FEED_02_RUNNING,
    FEED_03_RUNNING,

    FEED_01_STOPPING,
    FEED_02_STOPPING,
    FEED_03_STOPPING,

    FEED_01_STOPPED,
    FEED_02_STOPPED,
    FEED_03_STOPPED,

    FEED_RECORDING,
    FEED_PLAYBACK
};


enum FeedStatIndex {
    SYSTEM_EVENT = 0,
    STOCK_DIRECTORY,
    STOCK_TRADING_ACTION,
    REG_SHO,
    MARKET_PART_POSITION,
    MWCB_DECLINE_LEVEL,
    MWCB_BREACH,
    IPO_QUOTING_PERIOD_UPDATE,
    ADD_ORDER_NO_MPID,
    ADD_ORDER_WTIH_MPID,
    ORDER_EXECUTION, // 10
    ORDER_EXECUTION_WITH_PRICE,
    ORDER_CANCEL,
    ORDER_DELETE,
    ORDER_REPLACE,
    TRADE_MESSAGE_NON_CROSS,
    NETWORK_ORDER_INBALANCE,
    RETAIL_PRICE_IMPROVEMENT,
    TOTAL_MESSAGES // 18
};



#pragma once


#define SIZE_OF_CLT_NAME	31
#define SIZE_OF_CLT_PASSWORD   31

#define  UINT8  	unsigned char
#define  UINT		unsigned int
#define  ULONG		unsigned long

#define			NUMBER_OF_ROLES		9+1

typedef unsigned long   DWORD;

enum FEED_STATE{
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



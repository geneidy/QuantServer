#pragma once

#define	ARRAY_AM_SIZE	20   // DOMESTIC SIZE 
#define	ARRAY_FRN_SIZE	51	// FOREIGN SIZE
#define	ARRAY_OBJECT_SIZE	51	// FOREIGN SIZE

#define SIZE_OF_CLT_NAME	31
#define SIZE_OF_CLT_PASSWORD   31

#define  UINT8  	unsigned char
#define  UINT		unsigned int
#define  ULONG		unsigned long

typedef unsigned long        DWORD;

/*
typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed _uint64_t      uint64_t, *Puint64_t;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;

*/

typedef struct 
{
  
	char	szUserName[SIZE_OF_CLT_NAME];
	char	szPassword[SIZE_OF_CLT_PASSWORD];
	DWORD	dwIPAddress;
	UINT	uiPort;

	DWORD	dwIPAddress1;
	UINT	uiPort1;

}PIC_CLT_OPTIONS;



enum FEED_STATE{
	FEED_01_STARTING,
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



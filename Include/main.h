#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "string.h"
//#include "Logger.h"
#include <vector>
#include "Settings.h"

#include "SaveToDisk.h"
#include "SaveToDB.h"
#include "BuildBook.h"
#include "OrdersMap.h"
#include "TickDataMap.h"
#include  "ReceiveITCH.h"
#include  "QuantQueue.h"

int SaveSettings();
int LoadSettings();
// int GUI();

enum tstate {
  TS_INACTIVE,
  TS_STARTING,
  TS_STARTED,
  TS_ALIVE,
  TS_TERMINATED,
  TS_JOINED
};

typedef struct ThreadData {
  int idx;
  void* pVoid;
  int  iTotalThreads;
  CQuantQueue *g_pCQuantQueue;
}THREAD_DATA;

THREAD_DATA g_SThreadData;

typedef struct thread_info
{    /* Used as argument to thread_start() */
  pthread_t thread_id;        /* ID returned by pthread_create() */
  enum tstate eState;
  int       iThread_num;       /* Application-defined thread # */
  char     *thread_message;      /* Saying Hello */
}THREAD_INFO;

THREAD_INFO arrThreadInfo[NUMBER_OF_ROLES];
//////////////////////////////////////
void *MainQueue(void* );
//////////////////////////////////////

void *ReceiveFeed(void*);
void *ParseFeed(void*);
void *BuildBook(void*);
void *SaveToDB(void*);
void *PlayBack(void*);

void *NasdTestFile(void*);
void *Distributor(void* pArg);
void *SaveToDisk(void*);
void *OrdersMap(void* pArg);
void *TickDataMap(void*);

void InitThreadLog(int);
void TermThreadLog(int);


void* (*func_ptr[NUMBER_OF_ROLES])(void*) = \
{MainQueue, ReceiveFeed, ParseFeed, OrdersMap, BuildBook, TickDataMap, SaveToDB, PlayBack, NasdTestFile, Distributor, SaveToDisk};  // All Roles for the server functions are here

const char *ThreadMessage[NUMBER_OF_ROLES ] = \
 {"Main Queue", "Receive Feed Thread", "Parse Thread", "Orders Map Thread", "Build Book Thread", "Tick Data Threa",\
  "Save To DB Thread", "Play Back Thread", "NasdTestFile Thread", "Distributor Thread", "SaveToDisk Thread"};
  

#ifdef __cplusplus
extern "C"
{
#endif
  CSaveToDisk* 	pCSaveToDisk;
  CSaveToDB* 	pCSaveToDB;  
  CBuildBook* 	pCBuildBook;
  COrdersMap* 	pCOrdersMap;
  CTickDataMap* pCTickDataMap;
  CReceiveITCH* pCReceiveITCH;
  CQuantQueue * pCQuantQueue;
  
#ifdef __cplusplus
} // extern "C"
#endif 
 

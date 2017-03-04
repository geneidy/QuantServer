#pragma once
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include "BuildBook.h"
#include "Util.h"

enum _tstate {
    _TS_INACTIVE,
    _TS_STARTING,
    _TS_STARTED,
    _TS_ALIVE,
    _TS_TERMINATED,
    _TS_JOINED
};

typedef struct _thread_info
{
    pthread_t 	thread_id;        /* ID returned by pthread_create() */
    int       	iThread_num;       /* Application-defined thread # */
    enum 	_tstate eState;
} BOOK_THREAD_INFO;


typedef struct _ThreadData {
    int 	idx;
    char	szSymbol[SIZE_OF_SYMBOL];
    int		nLevels; //  could use a default here
} BOOK_THREAD_DATA;


typedef struct _SDisplayBook {

    OHLC 	TopOfBook;
    SBID_ASK*	pSBid;  // Number of levels apply here
    SBID_ASK*	pSAsk; 	// Number of levels apply here

} DISPLAYBOOK;
 

class CDisplayBook {

static    int 		m_iError;
    CUtil*  	m_pcUtil;

static    CBuildBook* 		m_pcBuildBook[NUMBER_OF_BOOKS_TO_DISPALY];
static    DISPLAYBOOK* 		m_SDisplayBook[NUMBER_OF_BOOKS_TO_DISPALY];
static    int			m_iFD[NUMBER_OF_BOOKS_TO_DISPALY];
static    int  CreatLOBFileMapping(int );
static    int  InitMemoryMappedFile(int );

static 	  unsigned m_uiSizeOfLob[NUMBER_OF_BOOKS_TO_DISPALY ] ;    
static struct stat  m_sb[NUMBER_OF_BOOKS_TO_DISPALY ] ;    
static struct stat  m_st[NUMBER_OF_BOOKS_TO_DISPALY ] ;    
static void*  m_addr[NUMBER_OF_BOOKS_TO_DISPALY ] ;    

struct timespec    m_request;

public:
    CDisplayBook(CBuildBook*  pCBuildBook);
    ~CDisplayBook();
    
    static BOOK_THREAD_INFO 	m_arrThreadInfo[NUMBER_OF_BOOKS_TO_DISPALY];
    static BOOK_THREAD_DATA 	m_arrBookThreadData[NUMBER_OF_BOOKS_TO_DISPALY];

    
    void DisplaySelected(void);
    static  void* DisplaySingleBook(void* pArg);
    
    static  void  StopDisplaySelected(char*);
    static  void  StopDisplayAllBooks();
    
    
    int GetError();
    char* GetErrorDescription();

};



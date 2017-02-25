#pragma once

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

    OHLC 		TopOfBook;
    SBID_ASK*	pSBid;
    SBID_ASK*	pSAsk;

} DISPLAYBOOK;


class CDisplayBook {

    int 		m_iError;
    CUtil*  	m_pcUtil;

static    CBuildBook* 		m_pcBuildBook;
    DISPLAYBOOK 		m_SDisplayBook;
    

public:
    CDisplayBook(CBuildBook*  pCBuildBook);
    ~CDisplayBook();
    
    static BOOK_THREAD_INFO 	m_arrThreadInfo[NUMBER_OF_BOOKS_TO_DISPALY];
    static BOOK_THREAD_DATA 	m_arrBookThreadData[NUMBER_OF_BOOKS_TO_DISPALY];

    
    static  void DisplaySelected(void);
    static  void* DisplaySingleBook(void* pArg);
    
    static  void  StopDisplaySelected(char*);
    static  void  StopDisplayAllBooks();
    
    
    int GetError();
    char* GetErrorDescription();

};



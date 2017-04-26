#pragma once
#include  <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <map>
#include <atomic>
#include <pthread.h>
#include <mutex>

#include "Util.h"

#include "time.h"

#include "ITCHMessages.h"
#include "OrdersMap.h"


class CSaveOrdersToDisc //: public COrdersMap
{
private:   // by default


    int 	m_ifd;

    uint64_t m_ui64NumRequest;
    
    ITCH_MESSAGES_UNION* 	m_pItchMessageUnion;
    COrdersMap*		m_pCOrdersMap;

    COMMON_ORDER_MESSAGE*  	m_pCommonOrder;
    uint64_t			m_uiNextOrder;

    timespec 	m_request;
    timespec	m_remain;
    CUtil*  m_pcUtil;
    



public:
    int 	m_iError;
    int  	ReadFromOrdersMap();
    CSaveOrdersToDisc();
    ~CSaveOrdersToDisc();

    int GetError();

};

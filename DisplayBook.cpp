
#include "DisplayBook.h"

BOOK_THREAD_INFO CDisplayBook::m_arrThreadInfo[NUMBER_OF_BOOKS_TO_DISPALY];
BOOK_THREAD_DATA CDisplayBook::m_arrBookThreadData[NUMBER_OF_BOOKS_TO_DISPALY];
DISPLAYBOOK* 	 CDisplayBook::m_SDisplayBook [NUMBER_OF_BOOKS_TO_DISPALY];
int		 CDisplayBook::m_iFD[NUMBER_OF_BOOKS_TO_DISPALY];

unsigned  	 CDisplayBook::m_uiSizeOfLob [NUMBER_OF_BOOKS_TO_DISPALY];
struct stat 	 CDisplayBook::m_sb[NUMBER_OF_BOOKS_TO_DISPALY ] ;
struct stat 	 CDisplayBook::m_st[NUMBER_OF_BOOKS_TO_DISPALY ] ;
int 		 CDisplayBook::m_iError = 0;
CBuildBook* 	 CDisplayBook::m_pcBuildBook[NUMBER_OF_BOOKS_TO_DISPALY];
void*  		 CDisplayBook::m_addr[NUMBER_OF_BOOKS_TO_DISPALY ];

///////////////////////////////////////////////////////////////////
CDisplayBook::CDisplayBook(CBuildBook*  pCBuildBook)
{
    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {
        m_pcBuildBook[ii]  = pCBuildBook;
        m_arrThreadInfo[ii].eState = _TS_INACTIVE;

        m_addr[ii] = NULL ;
        m_SDisplayBook[ii] = NULL;
        m_sb[ii] = {0} ;
        m_st[ii] = {0};
        m_uiSizeOfLob[ii] = sizeof(OHLC) + (sizeof(SBID_ASK_VALUES[theApp.SSettings.iBookLevels]) *2 );  // should be the same for all array elements
    }
    m_request.tv_sec = 0;
    m_request.tv_nsec = 100000000;   // 1/10 of a second
}
///////////////////////////////////////////////////////////////////
CDisplayBook::~CDisplayBook()
{
    StopDisplayAllBooks();

    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {
        if (!theApp.SSettings.arrbActive[ii]) { // Non active
            continue;
        }
        msync(m_addr[ii], m_sb[ii].st_size, MS_ASYNC);
        munmap(m_addr[ii], m_sb[ii].st_size);
        close(m_iFD[ii]);
    }
    Logger::instance().log("LOB: Destructed", Logger::Debug);
}
///////////////////////////////////////////////////////////////////
void CDisplayBook::DisplaySelected()
{
    int iTotalThreads = 0;

    string strMessage;

    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {        // spin a thread for each Stock
        if (!theApp.SSettings.arrbActive[ii]) { // Non active
            m_arrThreadInfo[ii].eState = _TS_INACTIVE;
            continue;
        }
        m_arrThreadInfo[ii].iThread_num = ii ;
        m_arrBookThreadData[ii].idx = ii;			// App assigned number
        m_arrBookThreadData[ii].nLevels = theApp.SSettings.iBookLevels; // copy number of levels
        strcpy(m_arrBookThreadData[ii].szSymbol, theApp.SSettings.szActiveSymbols[ii]);  // copy the symbol

        strMessage.clear();
        strMessage = "LOB: Creating Display Book Thread for Symbol: ";
        strMessage += m_arrBookThreadData[ii].szSymbol;
        Logger::instance().log(strMessage, Logger::Debug);

        int iRet = pthread_create(&m_arrThreadInfo[ii].thread_id, NULL, &DisplaySingleBook, &m_arrBookThreadData[ii]);
        nanosleep (&m_request, NULL);  // sleep a 1/10 of a second
        iTotalThreads++;
    }

    int iJoined = 0;

    strMessage.clear();
    strMessage = "LOB: Waiting for Threads to complete";
    Logger::instance().log(strMessage, Logger::Debug);
    
    m_bAllDone = false;
    
    while (iJoined < iTotalThreads) {
        // keep on checking for all terminated threads every three seconds
        for (uint ii = 0;  ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++ ) {
            if ((m_arrThreadInfo[ii].eState == _TS_INACTIVE)|| (m_arrThreadInfo[ii].eState == _TS_JOINED)|| (m_arrThreadInfo[ii].eState == _TS_ALIVE)|| (m_arrThreadInfo[ii].eState == _TS_STARTED))
                continue;
            if (m_arrThreadInfo[ii].eState == _TS_TERMINATED) {
                pthread_join(m_arrThreadInfo[ii].thread_id, NULL);
                m_arrThreadInfo[ii].eState = _TS_JOINED;

                strMessage.clear();
                strMessage = "LOB: Display Book Thread for Symbol: ";
                strMessage += m_arrBookThreadData[ii].szSymbol;
                strMessage += "  Joined";
                Logger::instance().log(strMessage, Logger::Debug);
                iJoined++;
            }
        } // for loop
        sleep(1);
    } // while loop
    Logger::instance().log("LOB: All Display Book threads joined", Logger::Debug);
    
    m_bAllDone = true;
}
///////////////////////////////////////////////////////////////////
void CDisplayBook::StopDisplaySelected(char* szSymbol)
{

}
///////////////////////////////////////////////////////////////////
void CDisplayBook::StopDisplayAllBooks()
{

    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {
        m_arrThreadInfo[ii].eState = _TS_TERMINATED;
    }
}
///////////////////////////////////////////////////////////////////
void* CDisplayBook::DisplaySingleBook(void* pArg)
{

    BOOK_THREAD_DATA SThreadData;

    SThreadData =  *((BOOK_THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    int nLevels = SThreadData.nLevels;

    string strToFind(SThreadData.szSymbol);

    struct timespec    request;

    request.tv_sec = 0;
    request.tv_nsec = 100000000;   // 1/10 of a second


    SBOOK_LEVELS SBookLevels;
    SBID_ASK* pTemp;

    if (CreatLOBFileMapping(idx) == -1) {  // consider moving this block higher
        m_iError = 120; // enum later
        return NULL;
    }

    m_arrThreadInfo[idx].eState = _TS_ALIVE;

//     m_pcBuildBook[idx]->m_itBookMap = m_pcBuildBook[idx]->m_BookMap.find(strToFind);
//     if (m_pcBuildBook[idx]->m_itBookMap == m_pcBuildBook[idx]->m_BookMap.end())
//         return  NULL;

    int nDisplayedLevels = 0;

    while (theApp.SSettings.iStatus != STOPPED) {
        if (m_arrThreadInfo[idx].eState == _TS_TERMINATED) {
            break;
        }

        m_pcBuildBook[idx]->m_itBookMap = m_pcBuildBook[idx]->m_BookMap.find(strToFind);
        if (m_pcBuildBook[idx]->m_itBookMap == m_pcBuildBook[idx]->m_BookMap.end()) {
            sleep(1); // Book Not constructed yet
            continue;
        }

        SBookLevels = m_pcBuildBook[idx]->m_itBookMap->second;
        if (!SBookLevels.bUpdated) {
            nanosleep (&request, NULL);  // sleep a 1/10 of a second
            continue;
        }
        m_pcBuildBook[idx]->m_itBookMap->second.bUpdated = false;

        m_SDisplayBook[idx]->TopOfBook.dOpen 	=   SBookLevels.m_OHLC.dOpen;
        m_SDisplayBook[idx]->TopOfBook.dClose 	=   SBookLevels.m_OHLC.dClose;
        m_SDisplayBook[idx]->TopOfBook.dHigh 	=   SBookLevels.m_OHLC.dHigh;
        m_SDisplayBook[idx]->TopOfBook.dLow 	=   SBookLevels.m_OHLC.dLow;
        m_SDisplayBook[idx]->TopOfBook.dLast 	=   SBookLevels.m_OHLC.dLast;
        m_SDisplayBook[idx]->TopOfBook.uiVolume = SBookLevels.m_OHLC.uiVolume;

        m_SDisplayBook[idx]->TopOfBook.dVWAP 		= SBookLevels.m_OHLC.dVWAP;
        m_SDisplayBook[idx]->TopOfBook.uiTotalVolume 	= SBookLevels.m_OHLC.uiTotalVolume;
        m_SDisplayBook[idx]->TopOfBook.uiTotalNumOfTrades = SBookLevels.m_OHLC.uiTotalNumOfTrades;
        m_SDisplayBook[idx]->TopOfBook.cTick = SBookLevels.m_OHLC.cTick;

//     Bid Levels
        nDisplayedLevels = 0;
        SBookLevels = m_pcBuildBook[idx]->m_itBookMap->second;

	//Clear all levels 
//	memset(m_SDisplayBook[idx]->pSBid, 0, (sizeof(SBID_ASK_VALUES))*nLevels);
	
        while (SBookLevels.pTopBid != NULL) {  // Print the Bid Levels
// 	  if (SBookLevels.bUpdating)
// 	      continue;
//
            if (nDisplayedLevels++ >= nLevels){
//	      memset(m_SDisplayBook[idx]->pSBid, 0, (sizeof(SBID_ASK_VALUES))*nLevels);           
	      break;
	    }

//             cout << SBookLevels.pTopBid->dPrice << " " << SBookLevels.pTopBid->szMPID << " " << SBookLevels.pTopBid->uiQty << " "<< SBookLevels.pTopBid->uiNumOfOrders << endl;
            m_SDisplayBook[idx]->pSBid[nDisplayedLevels].dPrice 	= SBookLevels.pTopBid->dPrice;
            strcpy(m_SDisplayBook[idx]->pSBid[nDisplayedLevels].szMPID, SBookLevels.pTopBid->szMPID);
            m_SDisplayBook[idx]->pSBid[nDisplayedLevels].uiQty 		= SBookLevels.pTopBid->uiQty;
            m_SDisplayBook[idx]->pSBid[nDisplayedLevels].uiNumOfOrders 	= SBookLevels.pTopBid->uiNumOfOrders;

            pTemp = SBookLevels.pTopBid;
            SBookLevels.pTopBid = pTemp->pNextBidAsk;  // could have done:  SBookLevels.pTopBid = SBookLevels.pTopBid->pNextBidAsk
        }// while (SBookLevels.pTopBid != NULL) {

//      Ask Levels
        nDisplayedLevels = 0;
        SBookLevels = m_pcBuildBook[idx]->m_itBookMap->second;

	//Clear all levels 
	memset(m_SDisplayBook[idx]->pSAsk, 0, (sizeof(SBID_ASK_VALUES))*nLevels);

        while (SBookLevels.pTopAsk != NULL) {  // Print the Ask Levels
//             cout<< SBookLevels.pTopAsk->dPrice << " "  << SBookLevels.pTopAsk->szMPID << " " << SBookLevels.pTopAsk->uiQty << " "<< SBookLevels.pTopAsk->uiNumOfOrders << endl;
            if (nDisplayedLevels++ >= nLevels)
                break;

//             cout << SBookLevels.pTopBid->dPrice << " " << SBookLevels.pTopBid->szMPID << " " << SBookLevels.pTopBid->uiQty << " "<< SBookLevels.pTopBid->uiNumOfOrders << endl;
            m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].dPrice 	= SBookLevels.pTopAsk->dPrice;
            strcpy(m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].szMPID, SBookLevels.pTopAsk->szMPID);
            m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].uiQty 		= SBookLevels.pTopAsk->uiQty;
            m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].uiNumOfOrders 	= SBookLevels.pTopAsk->uiNumOfOrders;

            pTemp = SBookLevels.pTopAsk;
            SBookLevels.pTopAsk = pTemp->pNextBidAsk;
        }// while (SBookLevels.pTopAsk != NULL) 
    }
/*
    if  (m_SDisplayBook[idx]->pSBid != NULL) {
        delete[] m_SDisplayBook[idx]->pSBid;
        m_SDisplayBook[idx]->pSBid = NULL;
    }

    if  (m_SDisplayBook[idx]->pSAsk != NULL) {
        delete[] m_SDisplayBook[idx]->pSAsk;
        m_SDisplayBook[idx]->pSAsk = NULL;
    }
    if  (m_SDisplayBook[idx] != NULL) {
        delete m_SDisplayBook[idx];
        m_SDisplayBook[idx] = NULL;
    }
*/
    m_arrThreadInfo[idx].eState = _TS_TERMINATED;
    return NULL;  // log later
}
///////////////////////////////////////////////////////////////////
int CDisplayBook::CreatLOBFileMapping(int iDx)
{

    if (stat("../LOB/", &m_st[iDx]) == -1) {
        mkdir("../LOB/", 0700);
    }

    string strLOBFile;
    strLOBFile.empty();

    strLOBFile = "../LOB/";
    strLOBFile += theApp.SSettings.szActiveSymbols[iDx];

    m_iFD[iDx] =  open(strLOBFile.c_str(), O_RDWR|O_CREAT, S_IRWXU);

    if (m_iFD[iDx] == -1) {
        Logger::instance().log("LOB: open File Mapping Error", Logger::Error);
        m_iError = 100;
        // Set error code and exit
    }

    if (!m_iError) {
        if (fstat(m_iFD[iDx], &m_sb[iDx]) == -1) {
            Logger::instance().log("LOB: Error fstat", Logger::Error);
            m_iError = 110;
            // Set error code and exit
        }
        else {
            if (!InitMemoryMappedFile(iDx)) {
                Logger::instance().log("LOB: Error Initializing File Mapping", Logger::Error);
                close(m_iFD[iDx]);
                m_iError = 120;
                // Set error code and exit
            }
            else {
                m_addr[iDx] = mmap(NULL, m_sb[iDx].st_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_iFD[iDx], 0);

                if (m_addr[iDx] == MAP_FAILED) {
                    Logger::instance().log("LOB: Error  Mapping Failed", Logger::Error);
                    m_iError = 130;
                    // Set error code and exit
                }
                m_SDisplayBook[iDx] = (DISPLAYBOOK*) m_addr[iDx];  //  cast in DISPLAYBOOK...now you have an array in memory of DISPLAYBOOK
            }
        }
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////////
int CDisplayBook::InitMemoryMappedFile(int iDx)
{
    m_SDisplayBook[iDx] = new (DISPLAYBOOK);

    m_SDisplayBook[iDx]->TopOfBook.dHigh 	= 0;
    m_SDisplayBook[iDx]->TopOfBook.dOpen 	= 0;
    m_SDisplayBook[iDx]->TopOfBook.dClose 	= 0;
    m_SDisplayBook[iDx]->TopOfBook.dLast 	= 0;
    m_SDisplayBook[iDx]->TopOfBook.dLow 	= 0;
    m_SDisplayBook[iDx]->TopOfBook.dVWAP 	= 0;
    m_SDisplayBook[iDx]->TopOfBook.tLastUpdate 	= {0};
    m_SDisplayBook[iDx]->TopOfBook.tOpen       	= {0};


    m_SDisplayBook[iDx]->TopOfBook.uiTotalNumOfTrades = 0;
    m_SDisplayBook[iDx]->TopOfBook.uiTotalVolume = 0;
    m_SDisplayBook[iDx]->TopOfBook.uiVolume = 0;
    m_SDisplayBook[iDx]->TopOfBook.uiNumOfTradesWithPrice = 0;
    m_SDisplayBook[iDx]->TopOfBook.uiNumOfTradesNoPrice = 0;

    m_SDisplayBook[iDx]->pSBid = new SBID_ASK_VALUES[m_arrBookThreadData[iDx].nLevels];
    m_SDisplayBook[iDx]->pSAsk = new SBID_ASK_VALUES[m_arrBookThreadData[iDx].nLevels];


    // Throw away code begin
    int iSizeOfBidAsk = sizeof(*m_SDisplayBook[iDx]->pSAsk);
    int iSizeOfTopOfBook = sizeof(m_SDisplayBook[iDx]->TopOfBook);
    //Throw away code end


    if (m_sb[iDx].st_size < m_uiSizeOfLob[iDx]) { // Fresh file
        Logger::instance().log("LOB: Initializing Mapped File ", Logger::Debug);

        write(m_iFD[iDx], &m_SDisplayBook[iDx]->TopOfBook, sizeof(OHLC));  // init with NULL

        for (int ii=0; ii < m_arrBookThreadData[iDx].nLevels; ii++) {  // could have conbined the next to lines...i.e. *2 
            write(m_iFD[iDx], &m_SDisplayBook[iDx]->pSBid, sizeof(SBID_ASK_VALUES));  // init with NULL
            write(m_iFD[iDx], &m_SDisplayBook[iDx]->pSAsk, sizeof(SBID_ASK_VALUES));  // init with NULL
        }
    }

    Logger::instance().log("LOB: Finished Initializing  Mapped File", Logger::Debug);

    fstat(m_iFD[iDx], &m_sb[iDx]);
    if (m_sb[iDx].st_size < m_uiSizeOfLob[iDx]) {
        Logger::instance().log("LOB: Error Initializing Mapped File", Logger::Debug);
        m_iError = 200; // enum later
        return false;
    }
    m_iError = 0; // enum later
    return true;
}
///////////////////////////////////////////////////////////////////
int CDisplayBook::GetError()
{
    return m_iError;
}
///////////////////////////////////////////////////////////////////
char* CDisplayBook::GetErrorDescription()
{
    return NULL;  //see the Logger message for now
}
///////////////////////////////////////////////////////////////////

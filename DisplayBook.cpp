
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
	m_uiSizeOfLob[ii] = 0 ;    
    }
    m_pcUtil = NULL;
//    m_pcUtil = new CUtil(theApp.SSettings.szActiveSymbols);
  
}
///////////////////////////////////////////////////////////////////
CDisplayBook::~CDisplayBook()
{
    if (m_pcUtil) {
        delete m_pcUtil;
        m_pcUtil = NULL;
    }
    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {
	msync(m_addr[ii], m_sb[ii].st_size, MS_ASYNC);
	munmap(m_addr[ii], m_sb[ii].st_size);
    }
 
}
///////////////////////////////////////////////////////////////////
void CDisplayBook::DisplaySelected()
{

    for (int ii = 0; ii < NUMBER_OF_BOOKS_TO_DISPALY; ii++) {        // spin a thread for each Stock
        if (!theApp.SSettings.arrbActive[ii]) { // Non active
            m_arrThreadInfo[ii].eState = _TS_INACTIVE;
            continue;
        }
        m_arrThreadInfo[ii].iThread_num = ii ;
        m_arrBookThreadData[ii].idx = ii;			// App assigned number
        m_arrBookThreadData[ii].nLevels = theApp.SSettings.iBookLevels; // copy number of levels
        strcpy(m_arrBookThreadData[ii].szSymbol, theApp.SSettings.szActiveSymbols[ii]);  // copy the symbol

        int iRet = pthread_create(&m_arrThreadInfo[ii].thread_id, NULL, &DisplaySingleBook, &m_arrBookThreadData[ii]);
    }
}
///////////////////////////////////////////////////////////////////
void CDisplayBook::StopDisplaySelected(char* szSymbol)
{

}
///////////////////////////////////////////////////////////////////
void CDisplayBook::StopDisplayAllBooks()
{


}
///////////////////////////////////////////////////////////////////
void* CDisplayBook::DisplaySingleBook(void* pArg)
{

    BOOK_THREAD_DATA SThreadData;

    SThreadData =  *((BOOK_THREAD_DATA*)pArg) ;
    int idx = SThreadData.idx;
    int nLevels = SThreadData.nLevels;

    string strToFind(SThreadData.szSymbol);

    SBOOK_LEVELS SBookLevels;
    SBID_ASK* pTemp;

    if (CreatLOBFileMapping(idx) == -1) {
        m_iError = 120; // enum later
        return NULL;
    }

    m_SDisplayBook[idx]->pSBid = new SBID_ASK[nLevels];
    m_SDisplayBook[idx]->pSAsk = new SBID_ASK[nLevels];

    m_uiSizeOfLob[idx] = sizeof(m_SDisplayBook);

    m_arrThreadInfo[idx].eState == _TS_ALIVE;

    m_pcBuildBook[idx]->m_itBookMap = m_pcBuildBook[idx]->m_BookMap.find(strToFind);
    if (m_pcBuildBook[idx]->m_itBookMap == m_pcBuildBook[idx]->m_BookMap.end())
        return  NULL;

    SBookLevels = m_pcBuildBook[idx]->m_itBookMap->second;

    int nDisplayedLevels = 0;

    while (theApp.SSettings.iStatus != STOPPED) {

//         SBookLevels = m_pcBuildBook->m_itBookMap->second;

        m_SDisplayBook[idx]->TopOfBook.dOpen 	=   SBookLevels.m_OHLC.dOpen;
        m_SDisplayBook[idx]->TopOfBook.dClose =   SBookLevels.m_OHLC.dClose;
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

        while (SBookLevels.pTopBid != NULL) {  // Print the Bid Levels

            if (nDisplayedLevels++ >= nLevels)
                break;
//             cout << SBookLevels.pTopBid->dPrice << " " << SBookLevels.pTopBid->szMPID << " " << SBookLevels.pTopBid->uiQty << " "<< SBookLevels.pTopBid->uiNumOfOrders << endl;
            m_SDisplayBook[idx]->pSBid[nDisplayedLevels].dPrice = SBookLevels.pTopBid->dPrice;
            strcpy(m_SDisplayBook[idx]->pSBid[nDisplayedLevels].szMPID, SBookLevels.pTopBid->szMPID);
            m_SDisplayBook[idx]->pSBid[nDisplayedLevels].uiQty = SBookLevels.pTopBid->uiQty;
            m_SDisplayBook[idx]->pSBid[nDisplayedLevels].uiNumOfOrders = SBookLevels.pTopBid->uiNumOfOrders;

            pTemp = SBookLevels.pTopBid;
            SBookLevels.pTopBid = pTemp->pNextBidAsk;
        }// while (SBookLevels.pTopBid != NULL) {

//      Ask Levels
        nDisplayedLevels = 0;

        while (SBookLevels.pTopAsk != NULL) {  // Print the Ask Levels
//             cout<< SBookLevels.pTopAsk->dPrice << " "  << SBookLevels.pTopAsk->szMPID << " " << SBookLevels.pTopAsk->uiQty << " "<< SBookLevels.pTopAsk->uiNumOfOrders << endl;
            if (nDisplayedLevels++ >= nLevels)
                break;
//             cout << SBookLevels.pTopBid->dPrice << " " << SBookLevels.pTopBid->szMPID << " " << SBookLevels.pTopBid->uiQty << " "<< SBookLevels.pTopBid->uiNumOfOrders << endl;
            m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].dPrice = SBookLevels.pTopAsk->dPrice;
            strcpy(m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].szMPID, SBookLevels.pTopAsk->szMPID);
            m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].uiQty = SBookLevels.pTopAsk->uiQty;
            m_SDisplayBook[idx]->pSAsk[nDisplayedLevels].uiNumOfOrders = SBookLevels.pTopAsk->uiNumOfOrders;

            pTemp = SBookLevels.pTopAsk;
            SBookLevels.pTopAsk = pTemp->pNextBidAsk;
        }// while (SBookLevels.pTopAsk != NULL) {
    }

    if  (m_SDisplayBook[idx]->pSBid) {
        delete[] m_SDisplayBook[idx]->pSBid;
        m_SDisplayBook[idx]->pSBid = NULL;
    }

    if  (m_SDisplayBook[idx]->pSAsk) {
        delete[] m_SDisplayBook[idx]->pSAsk;
        m_SDisplayBook[idx]->pSAsk = NULL;
    }

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
        Logger::instance().log("LOB open File Mapping Error", Logger::Error);
        m_iError = 100;
        // Set error code and exit
    }

    if (!m_iError) {
        if (fstat(m_iFD[iDx], &m_st[iDx]) == -1) {
            Logger::instance().log("LOB Error fstat", Logger::Error);
            m_iError = 110;
            // Set error code and exit
        }
        else {
            if (!InitMemoryMappedFile(iDx)) {
                Logger::instance().log("Error Initializing LOB File Mapping", Logger::Error);
                close(m_iFD[iDx]);
                m_iError = 120;
                // Set error code and exit
            }
            else {
                m_addr[iDx] = mmap(NULL, m_sb[iDx].st_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_iFD[iDx], 0);

                if (m_addr[iDx] == MAP_FAILED) {
                    Logger::instance().log("Error Mapping Failed", Logger::Error);
                    m_iError = 130;
                    // Set error code and exit
                }
                m_SDisplayBook[iDx] = (DISPLAYBOOK*) m_addr[iDx];  //  cast in DISPLAYBOOK...now you have an array in memory of DISPLAYBOOK
            }
            m_uiSizeOfLob[iDx] = sizeof(DISPLAYBOOK); // Avoid computing the size million times every second
        }
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////////
int CDisplayBook::InitMemoryMappedFile(int iDx)
{
    if (m_sb[iDx].st_size < m_uiSizeOfLob[iDx]) { // Fresh file
        Logger::instance().log("Initializing Mapped File...This will take few minutes", Logger::Debug);
        write(m_iFD[iDx], &m_SDisplayBook, m_uiSizeOfLob[iDx]);  // init with NULL
    }

    Logger::instance().log("Finished Initializing LOB Mapped File", Logger::Debug);

    fstat(m_iFD[iDx], &m_sb[iDx]);
    if (m_sb[iDx].st_size < m_uiSizeOfLob[iDx]) { // Fresh file
        Logger::instance().log("Error Initializing LOB Mapped File", Logger::Debug);
        m_iError = 200; // enum later
        return false;
    }
    m_iError = 0; // enum later
    return true;
}
///////////////////////////////////////////////////////////////////
int CDisplayBook::GetError()
{
    return 1;
}
///////////////////////////////////////////////////////////////////
char* CDisplayBook::GetErrorDescription()
{
    return NULL;
}
///////////////////////////////////////////////////////////////////

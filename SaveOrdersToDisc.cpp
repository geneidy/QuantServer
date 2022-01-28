#include <iostream>
#include <sstream>
#include <iomanip>

#include "SaveOrdersToDisc.h"
#include "NQTV.h"
#include "Logger.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
CSaveOrdersToDisc::CSaveOrdersToDisc()
{
    m_iError = 0;
    m_pCOrdersMap = nullptr;

    m_pCOrdersMap = COrdersMap::instance();

    if (!m_pCOrdersMap)
    {
        m_iError = 100;
        Logger::instance().log("Build Book... Obtaining File Mapping Error", Logger::Error);
    }

    m_uiNextOrder = 0;
    m_request.tv_nsec = 100000000; // 1/10 of a micro second

    m_ui64NumRequest = 0;

    struct stat64 st = {0};

    if (stat64("../Orders/", &st) == -1)
    {
        mkdir("../Orders/", 0700);
    }

    m_pcUtil = new CUtil();

    string strOrdersFile;
    strOrdersFile.empty();

    strOrdersFile = "../Orders/";
    strOrdersFile += m_pcUtil->GetFormatedDate();
    strOrdersFile += "QuanticksOrders.qtx";

    m_ifd = open64(strOrdersFile.c_str(), O_RDWR | O_CREAT, S_IRWXU);

    if (m_ifd == -1)
    {
        Logger::instance().log("Save Orders To Disc...Error Opening File: ", Logger::Error);
        Logger::instance().log(strOrdersFile, Logger::Error);
        m_iError = 100;
        // Set error code and exit
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CSaveOrdersToDisc::~CSaveOrdersToDisc()
{
    string strMsg;
    strMsg.clear();

    strMsg = "Save Orders To Disc...Total Orders Written: ";
    strMsg += to_string(m_ui64NumRequest);

    Logger::instance().log("Save Orders To Disc... Destructing...Please Wait", Logger::Info);
    Logger::instance().log(strMsg, Logger::Info);

    m_pCOrdersMap->iNInstance--;
}
////////////////////////////////////////////////////
int CSaveOrdersToDisc::ReadFromOrdersMap() // Entry point for processing...Called from a while loop in Main.cpp
{

    int iMessage;
    int iSizeOfCommonOrder = sizeof(COMMON_ORDER_MESSAGE);

    while (theApp.SSettings.iStatus != STOPPED)
    {
        if (!m_pCOrdersMap)
            break;

        if (m_pCommonOrder == nullptr)
        {
            m_ui64NumRequest--;
            nanosleep(&m_request, &m_remain); // sleep a 1/10 of a second
            continue;
        }

        m_pCommonOrder = m_pCOrdersMap->GetMemoryMappedOrder(m_ui64NumRequest++);
        iMessage = m_pCommonOrder->cMessageType;

        switch (iMessage)
        {
        case 'A': // Add orders No MPID
        case 'F': // Add orders
                  //      case 'E':  	// Order executed
                  //      case 'c':  	// Order executed  with price
        case 'X': // Order Cancel
        case 'D': // Order deleted
        case 'U': // Order Replace
            break;
        default:
            break;
            //return 0;
        }
        write(m_ifd, m_pCommonOrder, iSizeOfCommonOrder);
    } //     while (theApp.SSettings.iStatus != STOPPED) {
    return 0;
}
////////////////////////////////////////////////////
int CSaveOrdersToDisc::GetError()
{
    return m_iError;
}
//////////////////////////////////////////////////////////////////////////////////

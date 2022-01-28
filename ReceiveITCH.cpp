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
#include "memory.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <time.h>
#include "ReceiveITCH.h"
#include "NQTV.h"
#include "DBLayer.h"

CReceiveITCH::CReceiveITCH(CQuantQueue *pQueue) //(socket *Sock): m_Socket(*Sock)
{

    bool bInitError = false;
    m_iError = FEED_PLAYBACK;
    m_iLen = 0;

    m_pCUtil = NULL;
    m_pCUtil = new CUtil();

    //m_iOutputBuffLen	= (theApp.m_SOptions.dwBufferSize) * COMP_FACTOR;		// Compression factor for inflating the receive buffer
    m_iTVBufLen = theApp.SSettings.dwBufferSize; // in case we go overboard by stopping at the edge of a new message;
    m_pszRecvBufLen = theApp.SSettings.dwBufferSize;

    m_pszTVBuf = NULL;
    m_pszRecvBuf = NULL;
    m_pszOutputBuf = NULL;

    m_pszRecvBuf = new UINT8[m_pszRecvBufLen + 1]; // Data received from Socket
    //m_pszOutputBuf		= new   UINT8 [ m_iOutputBuffLen + 1];
    m_pszTVBuf = new UINT8[m_iTVBufLen + 1000]; // guard for extra...

    if ((m_pszTVBuf == NULL) || (m_pszRecvBuf == NULL)) //|| (m_pszOutputBuf == NULL))
    {
        m_iError = 100; // enum all the errors > 100   GTE 100 means do not continue
        Logger::instance().log("Error Allocating Buffers in Constructor", Logger::Debug);
    }

    m_pFillStructs = NULL;
    m_pFillStructs = new CFillMsgStructs(pQueue);
    if (!m_pFillStructs)
    {
        m_iError = 100; // enum all the errors > 100   GTE 100 means do not continue
        Logger::instance().log("Error Initializing m_pFillStructs in Constructor", Logger::Debug);
    }

    m_pLast = NULL;
    m_pBegin = NULL;
}
#define BUFFER_EXTRA 300
/////////////////////////////////////////////////////////////////////////////////////////
int CReceiveITCH::ProcessFeed()
{
    time_t lReceiveTime = 0;

    time(&lReceiveTime);

    m_pBegin = &m_pszTVBuf[0];
    m_pLast = m_pBegin;

    int iTerationNum = 0;
    int iPrevMsgLen = 0;

    uint64_t ulTotalBytesToBeProcessed = 0;
    uint64_t ulBytesLeft = BUFFER_EXTRA;

    while (m_pBegin)
    {
        iTerationNum++;
        memset(m_strPacketLength, '\0', 3);
        memmove(m_strPacketLength, m_pBegin, 2);
        m_iPacketLength = m_pCUtil->GetValueShort(m_strPacketLength, 0, 2);

        ulTotalBytesToBeProcessed += (iPrevMsgLen + 2);   //  total to "BE" processed
        ulBytesLeft = m_iLen - ulTotalBytesToBeProcessed; // bytes left AFTER processing this one

        if (ulBytesLeft < BUFFER_EXTRA) // buffer for extra large message
        {
            ulBytesLeft += 2;
            break;
        }

        memset(m_strPacketType, '\0', 2);
        memmove(m_strPacketType, m_pBegin + 2, 1);

        m_iPacketType = m_pCUtil->GetValueChar(m_strPacketType, 0, 1);

        if (m_iPacketLength) // :: TODO guard for extensive length
        {
            memset(m_szMessage, '\0', sizeof(m_szMessage)); // reset with every EOM
            memmove(m_szMessage, m_pBegin + 2, m_iPacketLength);

            //switch (m_iPacketType)
            //{
            //case 'S': //SequencedMsg();
            m_pFillStructs->DirectToMethod(m_szMessage);
            //			break;
            //	case 'H': //ServerHB
            //			break;
            //	case 'Z': //End of Session
            //			theApp.g_bReceiving = false;
            //				break;
            //	default:
            //		break;   // a message that we don't process
            //	};
        }
        else
        {
            break;
        }
        iPrevMsgLen = m_iPacketLength;
        m_pLast = m_pBegin;
        m_pBegin += (m_iPacketLength + 2);

        if (theApp.SSettings.iStatus == STOPPED)
        {
            Logger::instance().log("Nasd Test File: Process Feed Returning Before Completion", Logger::Info);
            m_pszRecvBuf = &m_pszRecvBuf[0];
            break;
        }

        //        if (!theApp.g_bReceiving)
        //            break;
    }; //

    m_iLen = ulBytesLeft;
    // Adjust
    if (m_iLen)
    {
        memmove(&m_pszTVBuf[0], m_pBegin, m_iLen); //Put the remaining "Partial message" in temp storage before clearing the buffer
    }
    m_pBegin = NULL;

    return 1; // for now until we document a return value
}
/////////////////////////////////////////////////////////////////////////////////////////////
int CReceiveITCH::ReadFromTestFile(const char *strFileName) ///  ignore the formal paramter for now
{

    int iHandle = open(strFileName, O_RDWR);

    if (iHandle < 0)
    {
        string strLogMessage;
        strLogMessage = "Opening file: ";
        strLogMessage += strFileName;
        strLogMessage += " Failed";

        // redundant...just for debug purposes
        switch (iHandle)
        {
        case EACCES:
            break;
        case EEXIST:
            break;
        case EINVAL:
            break;
        case EMFILE:
            break;
        case ENOENT:
            break;
        }
        //      strLogMessage += iHandle;
        Logger::instance().log(strLogMessage, Logger::Debug);
        return false;
    }

    int64_t i64NumberOfBytes = 0;
    uint64_t ulTotalBytes = 0;

    m_iIteration = 0;

    if (iHandle)
    {
        m_iLen = 0;
        do
        {
            memset(m_pszRecvBuf, '\0', theApp.SSettings.dwBufferSize);

            i64NumberOfBytes = read(iHandle, m_pszRecvBuf, theApp.SSettings.dwBufferSize);
            memmove(&(m_pszTVBuf[m_iLen]), m_pszRecvBuf, i64NumberOfBytes); // Accumulate in m_szTVBuf
            if (theApp.SSettings.iStatus == STOPPED)
            {
                Logger::instance().log("Nasd Test File: Test File Returning Before Completion", Logger::Info);
                m_pszRecvBuf = &m_pszRecvBuf[0];
                break;
            }

            m_iLen += i64NumberOfBytes;

            m_iIteration++;
            ulTotalBytes += i64NumberOfBytes;
            ProcessFeed();

        } while (i64NumberOfBytes); //while ((i64NumberOfBytes != -1) && (i64NumberOfBytes != 0) && (theApp.g_bReceiving));
    }
    close(iHandle);
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
CReceiveITCH::~CReceiveITCH(void)
{
    //m_Socket = INVALID_socket;
    //if (!bInflateEnd)
    //{
    //	(void)inflateEnd(&m_zstStream);
    //	bInflateEnd = true;
    //}
    /*
    if (m_pszTVBuf != NULL)
    {
        delete[] m_pszTVBuf;
        m_pszTVBuf = NULL;
    }

       if (m_pszRecvBuf != NULL)
     {
        delete[] m_pszRecvBuf;
         m_pszRecvBuf = NULL;
     }

    if (m_pszOutputBuf != NULL)
    {
        delete[] m_pszOutputBuf;
        m_pszOutputBuf = NULL;
    }
*/
    if (m_pFillStructs)
    {
        delete m_pFillStructs;
        m_pFillStructs = NULL;
    }
    if (m_pCUtil)
    {
        delete m_pCUtil;
        m_pCUtil = NULL;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
int CReceiveITCH::GetError(void)
{
    return m_iError;
}
/////////////////////////////////////////////////////////////////////////////////////////////

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
#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "QuantQueue.h"
#include "Util.h"

#include "Includes.h"
#include "FillMsgStructs.h"

#define MSG_LENGTH 255

#define COMP_FACTOR 10 // compression factor

class CReceiveITCH
{
    int m_Socket;

public:
    //	CReceiveITCH(socket* );
    CReceiveITCH(CQuantQueue *pQueue);
    virtual ~CReceiveITCH(void);

    int ReadFromTestFile(const char *strFileName);
    int ProcessFeed();

    int GetError();

    int m_iTVBufLen;
    int m_iOutputBuffLen;
    int m_pszRecvBufLen;

    char szMsg[MSG_LENGTH];

    UINT8 *m_pLast;
    UINT8 *m_pBegin;
    uint64_t m_iLen;

    UINT8 *m_pszTVBuf;
    UINT8 *m_pszRecvBuf;
    UINT8 *m_pszOutputBuf;

    UINT8 m_szMessage[SOUP_MESSAGE_SIZE]; // each individual message

private:
    int m_iPacketLength;
    int m_iPacketType;

    UINT8 m_strPacketLength[3];
    UINT8 m_strPacketType[2];

    CFillMsgStructs *m_pFillStructs;
    CUtil *m_pCUtil;

    int m_iIteration;
    int m_iError;
};

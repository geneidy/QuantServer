#include "memory.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <time.h>
#include "ReceiveITCH.h"
#include "NQTV.h"


CReceiveITCH::CReceiveITCH () //(socket *Sock): m_Socket(*Sock)
{
//	int		iRet = 0;
	bool 	bInitError = false;
	m_iLen = 0;

	m_pCUtil = NULL;
	m_pCUtil = new CUtil();


	//m_iOutputBuffLen	= (theApp.m_SOptions.dwBufferSize) * COMP_FACTOR;		// Compression factor for inflating the receive buffer
	m_iTVBufLen		=   theApp.m_SOptions.dwBufferSize;   // in case we go overboard by stopping at the edge of a new message;
	m_pszRecvBufLen =   theApp.m_SOptions.dwBufferSize;


	m_pszTVBuf		= NULL;
	m_pszRecvBuf	= NULL;
	m_pszOutputBuf	= NULL;
 
	m_pszRecvBuf		= new   UINT8 [ m_pszRecvBufLen +1 ]; // Data received from Socket
	//m_pszOutputBuf		= new   UINT8 [ m_iOutputBuffLen + 1];
	m_pszTVBuf			= new   UINT8 [ m_iTVBufLen + 1000];        // guard for extra...

	if ((m_pszTVBuf == NULL) || (m_pszRecvBuf == NULL)) //|| (m_pszOutputBuf == NULL))
	{
		bInitError = true;
		// log error 
		return;
	}

	m_pFillStructs = NULL;
	m_pFillStructs = new CFillMsgStructs();
	if (!m_pFillStructs)
	{
		// log error 
		return;
	}

	m_pLast = NULL;
	m_pBegin = NULL;
}
#define BUFFER_EXTRA    300
/////////////////////////////////////////////////////////////////////////////////////////
int  CReceiveITCH::ProcessFeed()
{
//	int	i			= 0;
	time_t	lReceiveTime	= 0;
//	int		iMessageLength	= 0;

	time(&lReceiveTime);
//	int	ii = 0;

	m_pBegin = &m_pszTVBuf[0];
	m_pLast = m_pBegin;

	int iTerationNum = 0;
	int iPrevMsgLen = 0;

	uint64_t  ulTotalBytesToBeProcessed = 0;
	uint64_t  ulBytesLeft = BUFFER_EXTRA;
 
 	while (m_pBegin )
	{
		iTerationNum++;
		memset(m_strPacketLength, '\0', 3);
 		memmove(m_strPacketLength, m_pBegin, 2);
		m_iPacketLength =  m_pCUtil->GetValueShort(m_strPacketLength, 0, 2); 

		ulTotalBytesToBeProcessed += (iPrevMsgLen + 2);  //  total to "BE" processed
		ulBytesLeft = m_iLen - ulTotalBytesToBeProcessed ;   // bytes left AFTER processing this one

		if (ulBytesLeft < BUFFER_EXTRA)   // buffer for extra large message
		{
			ulBytesLeft += 2;
 			break; 
		}
		
		memset(m_strPacketType, '\0', 2);
		memmove(m_strPacketType, m_pBegin + 2, 1);

		m_iPacketType = m_pCUtil->GetValueChar(m_strPacketType, 0, 1);

		if (m_iPacketLength)  // :: TODO guard for extensive length 
		{
			memset(m_szMessage, '\0', sizeof(m_szMessage));  // reset with every EOM
			memmove (m_szMessage, m_pBegin + 2, m_iPacketLength);

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

		if (!theApp.g_bReceiving)
			break;
	}; //  

	m_iLen = ulBytesLeft ;
	 // Adjust
	if (m_iLen)
	{
		memmove(&m_pszTVBuf[0], m_pBegin, m_iLen);			//Put the remaining "Partial message" in temp storage before clearing the buffer
 	}
	m_pBegin = NULL;

	return 1;   // for now until we document a return value
}
/////////////////////////////////////////////////////////////////////////////////////////////
int CReceiveITCH::ReadFromTestFile(char* strFileName)  ///  ignore the formal paramter for now
{
	 
	int iHandle = open(theApp.strFeedFileName.c_str(), O_RDWR );  
	
	if ( iHandle < 0 ) {
	  printf("Opening file : Failed\n");
	  printf ("Error no is : %d\n", errno);
	  printf("Error description is : %s\n",strerror(errno));
	  return iHandle;
	}

// redundant...just for debug purposes
	switch (iHandle)
	{
		case EACCES:
			return false;
		case EEXIST:
			return false;
		case EINVAL:
			return false;
		case EMFILE:
			return false;
		case ENOENT:
			return false;
	}
 	int64_t		i64NumberOfBytes = 0;
	uint64_t	ulTotalBytes = 0; 

	m_iIteration = 0;
 
 	if (iHandle)
	{
		m_iLen = 0;
 		do
		{
			memset(m_pszRecvBuf, '\0', theApp.dwBufferSize);

			i64NumberOfBytes = read(iHandle, m_pszRecvBuf, theApp.dwBufferSize);
			memmove(&(m_pszTVBuf[m_iLen]), m_pszRecvBuf, i64NumberOfBytes);		// Accumulate in m_szTVBuf

			m_iLen += i64NumberOfBytes;

			m_iIteration++;
			ulTotalBytes+= i64NumberOfBytes;

			ProcessFeed();

			if (!theApp.g_bReceiving)
				break;

		} while ((i64NumberOfBytes != -1) && (i64NumberOfBytes != 0) && (theApp.g_bReceiving));
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
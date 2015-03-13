#include "memory.h"

#include "/usr/include/sys/socket.h"
//#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "Util.h"
//#include "Winsock2.h"

#define SIZEOFBUF 27

int iValue = 0;
unsigned long ulValue = 0;

uint64_t i64Value = 0;
char strBuffer[SIZEOFBUF];
char strValue[256];  // sorry for the hard coded value
char cValue;

CUtil::CUtil(void)
{
	//m_strValue.Empty();
}
//////////////////////////////////////////////////////////////////////////////
CUtil::~CUtil(void)
{
	 
}
/////////////////////////////////////////////////////////////////////////////
char* CUtil::FloatToCharP(double fIn)
{
/*	memset(m_pChar, '\0', SIZE_OF_RET_BUFF);
	int	err = 0;

	err = _gcvt_s(m_pChar, SIZE_OF_RET_BUFF, fIn, 15);
*/
	return m_pChar;
}
/////////////////////////////////////////////////////////////////////////////
char* CUtil::IntToCharP(int iIn)
{
/*	memset(m_pChar, '\0', SIZE_OF_RET_BUFF);
	int	err = 0;

	err = _itoa_s(iIn, m_pChar, 10);
*/
	return m_pChar;
}
/////////////////////////////////////////////////////////////////////////////
double  CUtil::GetValueDouble(UINT8 *uiMsg, int iOffset, int iLength)
{
	double dValue = 0;
	/*
	memset(strBuffer, '\0', SIZEOFBUF);
	if (iLength >= SIZEOFBUF)
		iLength = SIZEOFBUF -1;
	memmove(strBuffer,  uiMsg +iOffset,  iLength);  
	dValue = atof(strBuffer);
	*/
	return dValue;
} 
////////////////////////////////////////////////////////////////////////////
short	CUtil::GetValueShort(UINT8 *uiMsg, int iOffset, int iLength)
{
	unsigned short uiBuff = 0;

 	memmove(&uiBuff,  uiMsg +iOffset,  iLength);   
	iValue = ntohs(uiBuff);

	return iValue;
}
////////////////////////////////////////////////////////////////////////////
int	CUtil::GetValueInt(UINT8 *uiMsg, int iOffset, int iLength)
{
	int uiBuff = 0;

	memmove(&uiBuff,  uiMsg +iOffset,  iLength);   
	iValue = ntohl(uiBuff);

	return iValue;
}
////////////////////////////////////////////////////////////////////////////
unsigned long	CUtil::GetValueUnsignedLong(UINT8 *uiMsg, int iOffset, int iLength)
{
	unsigned long ulBuff = 0;

 	memmove(&ulBuff,  uiMsg +iOffset,  iLength);   
	ulValue = ntohl(ulBuff);

	return ulValue;
}
/*///////////////////////////////////////////////////////////////////////////
#define _WS2_32_WINSOCK_SWAP_LONGLONG(l)            \
            ( ( ((l) >> 56) & 0x00000000000000FFLL ) |       \
              ( ((l) >> 40) & 0x000000000000FF00LL ) |       \
              ( ((l) >> 24) & 0x0000000000FF0000LL ) |       \
              ( ((l) >>  8) & 0x00000000FF000000LL ) |       \
              ( ((l) <<  8) & 0x000000FF00000000LL ) |       \
              ( ((l) << 24) & 0x0000FF0000000000LL ) |       \
              ( ((l) << 40) & 0x00FF000000000000LL ) |       \
              ( ((l) << 56) & 0xFF00000000000000LL ) )

*///////////////////////////////////////////////////////////////////////////
uint64_t   CUtil::GetValueUnsignedInt64(UINT8 *uiMsg, int iOffset, int iLength)
{
	uint64_t i64Buff = 0;

	memset(&i64Buff, '\0', 8);
	memmove(&i64Buff,  uiMsg +iOffset,  iLength);   

 	i64Value =   ntohl(i64Buff);  // NEED an i64 functions.....should be ntohll !!!   //_WS2_32_WINSOCK_SWAP_LONGLONG(i64Buff);

//	GetTimeFromNano(i64Value);

	return i64Value;
}
////////////////////////////////////////////////////////////////////////////
char*   CUtil::GetValueAlpha(UINT8 *uiMsg, int iOffset, int iLength)
{
//	char strValue[256];  // sorry for the hard coded value
 	memset(strValue, '\0', 256);
	if (iLength > 255)
		iLength = 255;

	memmove(strValue,  uiMsg +iOffset,  iLength);  

	return strValue;
}
////////////////////////////////////////////////////////////////////////////
char    CUtil::GetValueChar(UINT8 *uiMsg, int iOffset, int iLength)
{
	cValue = '\0';
	cValue = uiMsg[iOffset];

	return cValue;
}
////////////////////////////////////////////////////////////////////////////

 
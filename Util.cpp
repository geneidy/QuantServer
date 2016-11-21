#include "memory.h"
// #include <boost/chrono.hpp>

//#include <sys/types.h>
#include <stdlib.h>>
#include <arpa/inet.h>
#include "Util.h"

#include <stdio.h>
#include <chrono>
#include <ctime>

#define SIZEOFBUF 27

int iValue = 0;
unsigned long ulValue = 0;

uint64_t i64Value = 0;
char strBuffer[SIZEOFBUF];
char strValue[256];  // sorry for the hard coded value
char cValue;

CUtil::CUtil(void)
{
  m_i64Nanos 	= 1E9;
  m_iHours 	= m_i64Nanos *  60 * 60; 
  m_iMinutes 	= m_i64Nanos * 60; 
  m_iSeconds 	= 1 * 1E9 ;
  m_iMilliSeconds = 1 * 1E3;
  m_iMicroSeconds = 1 * 1E6;
  m_iNanoSeconds  = 0;
}

//////////////////////////////////////////////////////////////////////////////
char* CUtil::GetTimeFromNano(uint64_t ui64NanoTime)
{
  lldiv_t Div;
  int iHours = 0;
  int iMinutes = 0;
  int iSeconds = 0;
  int iMilliSeconds = 0;
  int iMicroSeconds = 0;
  uint64_t ui64Nanos = 0;
  int i = 0; 
  
  uint64_t iRem = 0;
  // Put in a while loop
  Div= lldiv(ui64NanoTime,  1000); 
  m_iNanoSeconds = Div.rem;
  
  Div = lldiv(ui64NanoTime - m_iNanoSeconds, 1000);
  m_iMicroSeconds = Div.rem;
  
  Div = lldiv(ui64NanoTime - m_iMicroSeconds, 1000);
  m_iMilliSeconds = Div.rem;
  
  Div = lldiv(ui64NanoTime - m_iMilliSeconds, 1000);
  m_iSeconds = Div.rem;
  
  
  
  
  
  
  
    
  Div= lldiv(ui64NanoTime,  m_iHours); 
  iHours = Div.quot;
  iRem   = Div.rem;
  
  Div = lldiv(iRem, m_iMinutes);
  iMinutes = Div.quot;
  iRem = Div.rem;
  
  
  
}

//////////////////////////////////////////////////////////////////////////////
CUtil::~CUtil(void)
{
	 
}

/////////////////////////////////////////////////////////////////////////////
using namespace std::chrono;

void CUtil::GetTimeWithSeconds(void)
{
  high_resolution_clock::time_point p = high_resolution_clock::now();

  milliseconds ms = duration_cast<milliseconds>(p.time_since_epoch());

  seconds s = duration_cast<seconds>(ms);
  std::time_t t = s.count();
  std::size_t fractional_seconds = ms.count() % 1000;

//  std::cout << std::ctime(&t) << std::endl;
//  std::cout << fractional_seconds << std::endl;
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

// 	i64Value =   ntohl(i64Buff);  // NEED an i64 functions.....should be ntohll !!!   //_WS2_32_WINSOCK_SWAP_LONGLONG(i64Buff);
 	i64Value =   be64toh(i64Buff);  // Found it

	GetTimeFromNano(i64Value);

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
void CUtil::print_error(MYSQL* conn, char* message) 
{
  /*fprintf(stderr, "%s\n", message);
  if (conn != NULL){
    fprintf(stderr, "Error %u (%s)\n",
    mysql_errno(conn), mysql_error(conn));
  }
  */
}
///////////////////////////////////////////////////////////////////////////////////
void CUtil::print_stmt_error (MYSQL_STMT* stmt, char* message) 
{
/*	fprintf (stderr, "%s\n", message);
	if (stmt != NULL)
	{
		fprintf(stderr, "Error %u (%s): %s\n",
				mysql_stmt_errno(stmt),
				mysql_stmt_sqlstate(stmt),
				mysql_stmt_error(stmt));
	}
	*/
}
#include "time.h"
///////////////////////////////////////////////////////////////////////////////////
char* CUtil::GetFormatedDate()
{
	struct tm stToday;
     	time_t ltime = 0;
	
	time( &ltime );
        localtime_r( &ltime ,  &stToday);

	memset(m_szLogDate, 0 , SIZE_OF_FORMATED_DATE);
	strftime(m_szLogDate, SIZE_OF_FORMATED_DATE, "%Y-%m-%d-" , &stToday);
	return m_szLogDate;
}
///////////////////////////////////////////////////////////////////////////////////
char* CUtil::GetFormatedTime()
{
	struct tm stToday;
     	time_t ltime = 0;
	
	time( &ltime );
        localtime_r( &ltime ,  &stToday);

	
	memset(m_szLogTime, 0 , SIZE_OF_FORMATED_TIME);
	strftime(m_szLogTime, SIZE_OF_FORMATED_TIME, "%H-%M-%S" , &stToday);
	return m_szLogTime;
}
///////////////////////////////////////////////////////////////////////////////////

/*
#include <iostream>
#include <chrono>
#include <ctime>

using namespace std::chrono;

int main()
{
  high_resolution_clock::time_point p = high_resolution_clock::now();

  milliseconds ms = duration_cast<milliseconds>(p.time_since_epoch());

  seconds s = duration_cast<seconds>(ms);
  std::time_t t = s.count();
  std::size_t fractional_seconds = ms.count() % 1000;

  std::cout << std::ctime(&t) << std::endl;
  std::cout << fractional_seconds << std::endl;
}
example result:

Thu Oct 11 19:10:24 2012

925
  
 */



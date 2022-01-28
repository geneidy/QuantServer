#pragma once

#define INVALID_INPUT -1
#define SIZE_OF_TEMP_BUFF 128
#define SIZE_OF_RET_BUFF 25

#include <unordered_set>
#include <string>
#include "Includes.h"
#include <inttypes.h>
#include <mysql/mysql.h>

using namespace std;

class CUtil
{
private:
	char m_pChar[SIZE_OF_RET_BUFF];
	uint64_t m_i64Nanos;
	uint64_t m_iHours;
	uint64_t m_iMinutes;
	uint64_t m_iSeconds;
	uint64_t m_iMilliSeconds;
	uint64_t m_iMicroSeconds;
	uint64_t m_iNanoSeconds;

	unordered_set<string> m_SymbolSet;
	unordered_set<string>::iterator m_it;
	pair<unordered_set<string>::iterator, bool> m_Ret;

public:
	CUtil(void);
	CUtil(char szActiveSymbols[NUMBER_OF_SYMBOLS][SIZE_OF_SYMBOL], bool *bActive);
	~CUtil(void);

	//	CString		FloatToCString(double fIn);
	//	CString		IntToCString(int iIn);
	//	CString		UnsingedToCString(unsigned uIn);

	char *FloatToCharP(double fIn);
	char *IntToCharP(int iIn);

	//	CString		PadCString(CString srtIn);

	//	double		CStringToFloat(CString strIn);
	//	unsigned	int CStringToUI(CString strIn);

	//	CString		m_strValue;

	// Helper function for parsing the ITCH Messages
	double GetValueDouble(UINT8 *uiMsg, int iOffset, int iLength);
	short GetValueShort(UINT8 *uiMsg, int iOffset, int iLength);
	int GetValueInt(UINT8 *uiMsg, int iOffset, int iLength);
	uint64_t GetValueUnsignedInt64(UINT8 *uiMsg, int iOffset, int iLength);
	unsigned long GetValueUnsignedLong(UINT8 *uiMsg, int iOffset, int iLength);
	char *GetValueAlpha(UINT8 *uiMsg, int iOffset, int iLength);
	char GetValueChar(UINT8 *uiMsg, int iOffset, int iLength);

	//	uint64_t GetTimeFromNano(uint64_t);
	char *GetFormatedDate();
	char *GetFormatedTime();

	// Helper function for MySQL
	void print_error(MYSQL *conn, char *message);
	void print_stmt_error(MYSQL_STMT *stmt, char *message);
#define SIZE_OF_FORMATED_DATE 12 + 1
#define SIZE_OF_FORMATED_TIME 19 + 1

	char m_szLogDate[SIZE_OF_FORMATED_DATE];
	char m_szLogTime[SIZE_OF_FORMATED_TIME];

	char *GetTimeFromNano(uint64_t);
	void GetTimeWithSeconds();
	bool CheckInclude(char *szStock);
	bool IsSymbolIn(char *szSymbolIn);
};

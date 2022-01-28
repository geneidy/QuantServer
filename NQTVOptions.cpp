// QTNQTVOptions.cpp : implementation file
//

#include "stdafx.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <share.h>
#include <errno.h>

#include "NQTV.h"
#include "NQTVOptions.h"
#include "afxdialogex.h"

// CNQTVOptions dialog

IMPLEMENT_DYNAMIC(CNQTVOptions, CDialogEx)

CNQTVOptions::CNQTVOptions(CWnd *pParent /*=NULL*/)
	: CDialogEx(CNQTVOptions::IDD, pParent), m_dwSystemEvent(0), m_dwSystemDirectory(0), m_dwStockTradingAction(0), m_dwSHORestriction(0), m_dwMarketMakerPosition(0), m_dwMWCBDLM(0), m_dwIPOQuotation(0), m_dwNOII(0), m_dwRPPI(0), m_dwMWCBDBM(0), m_dwTradeNonCross(0), m_dwAll(0), m_dwAddOrderNoMPID(0), m_dwAddOrderWithMPID(0), m_dwOrderExecuted(0), m_OrderExecWithPrice(0), m_dwOrderCancel(0), m_dwOrderReplace(0), m_dwOrderDelete(0)
{
	m_bInit = false; // initiated externally without the dialog
}
///////////////////////////////////////////////////////////////////////
CNQTVOptions::CNQTVOptions(bool bInit) : m_bInit(m_bInit)
{
	LoadOptions();
}
///////////////////////////////////////////////////////////////////////
CNQTVOptions::~CNQTVOptions()
{
}
///////////////////////////////////////////////////////////////////////
void CNQTVOptions::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_dwSystemEvent);
	DDX_Text(pDX, IDC_EDIT2, m_dwSystemDirectory);
	DDX_Text(pDX, IDC_EDIT3, m_dwStockTradingAction);
	DDX_Text(pDX, IDC_EDIT4, m_dwSHORestriction);
	DDX_Text(pDX, IDC_EDIT5, m_dwMarketMakerPosition);
	DDX_Text(pDX, IDC_EDIT6, m_dwMWCBDLM);
	DDX_Text(pDX, IDC_EDIT8, m_dwIPOQuotation);
	DDX_Text(pDX, IDC_EDIT10, m_dwNOII);
	DDX_Text(pDX, IDC_EDIT11, m_dwRPPI);
	DDX_Text(pDX, IDC_EDIT7, m_dwMWCBDBM);
	DDX_Text(pDX, IDC_EDIT19, m_dwTradeNonCross);
	DDX_Text(pDX, IDC_EDIT20, m_dwAll);
	DDX_Text(pDX, IDC_EDIT13, m_dwAddOrderNoMPID);
	DDX_Text(pDX, IDC_EDIT14, m_dwAddOrderWithMPID);
	DDX_Text(pDX, IDC_EDIT12, m_dwOrderExecuted);
	DDX_Text(pDX, IDC_EDIT15, m_OrderExecWithPrice);
	DDX_Text(pDX, IDC_EDIT16, m_dwOrderCancel);
	DDX_Text(pDX, IDC_EDIT17, m_dwOrderReplace);
	DDX_Text(pDX, IDC_EDIT18, m_dwOrderDelete);
}
///////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CNQTVOptions, CDialogEx)
END_MESSAGE_MAP()
///////////////////////////////////////////////////////////////////////

// CNQTVOptions message handlers
/////////////////////////////////////////////////////////////////////////////////////
BOOL CNQTVOptions::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LoadOptions();

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////
BOOL CNQTVOptions::LoadOptions()
{
	LOCALFILEDATA SOptions;
	memset(&SOptions, '\0', sizeof(LOCALFILEDATA));

	memset(&theApp.SMemoryMappedFileSSize, '\0', sizeof(LOCALFILEDATA));

	int ihDatFile = -1;

	_sopen_s(&ihDatFile, "QtNQTVItch.Ini", _O_CREAT | _O_RDWR | _O_BINARY | _O_SEQUENTIAL, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	int iNumberOfBytes = 0;

	if (ihDatFile)
	{
		iNumberOfBytes = _read(ihDatFile, &SOptions, sizeof(LOCALFILEDATA));
		_close(ihDatFile);
	}

	if ((iNumberOfBytes == -1))
		int xxx = 0; // ::TODO log error

	if (ihDatFile)
	{
		m_dwSystemEvent = SOptions.dwSYSTEM_EVENT_MESSAGE;
		m_dwSystemDirectory = SOptions.dwSTOCK_DIRECTORY_MESSAGE;
		m_dwStockTradingAction = SOptions.dwSTOCK_TRADING_ACTION_MESSAGE;
		m_dwSHORestriction = SOptions.dwREG_SHO_RESTRICTION_MESSAGE;
		m_dwMarketMakerPosition = SOptions.dwMP_POSITION_MESSAGE;
		m_dwMWCBDLM = SOptions.dwMWCBDLM_MESSAGE;
		m_dwIPOQuotation = SOptions.dwIPO_QUOTATION_PERIOD_UPDATE_MESSAGE;
		m_dwNOII = SOptions.dwNOII_MESSAGE;
		m_dwRPPI = SOptions.dwRPPI_MESSAGE;
		m_dwMWCBDBM = SOptions.dwMWCBDBM_MESSAGE;
		m_dwTradeNonCross = SOptions.dwTRADE_NON_CROSS_MESSAGE;
		m_dwAll = SOptions.dwAllFeed;
		m_dwAddOrderNoMPID = SOptions.dwADD_ORDER_NO_MPID_MESSAGE;
		m_dwAddOrderWithMPID = SOptions.dwADD_ORDER_MPID_MESSAGE;
		m_dwOrderExecuted = SOptions.dwORDER_EXECUTED_MESSAGE;
		m_OrderExecWithPrice = SOptions.dwORDER_EXECUTED_WITH_PRICE_MESSAGE;
		m_dwOrderCancel = SOptions.dwORDER_CANCEL_MESSAGE;
		m_dwOrderReplace = SOptions.dwORDER_REPLACE_MESSAGE;
		m_dwOrderDelete = SOptions.dwORDER_DELETE_MESSAGE;
		if (!m_bInit)
			UpdateData(false);

		theApp.SMemoryMappedFileSSize = SOptions;
	}

	// TODO:  Add extra initialization here

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}
///////////////////////////////////////////////////////////////////////
void CNQTVOptions::OnOK()
{

	LOCALFILEDATA SOptions;
	memset(&SOptions, '\0', sizeof(LOCALFILEDATA));

	UpdateData(true);

	SOptions.dwSYSTEM_EVENT_MESSAGE = m_dwSystemEvent;
	SOptions.dwSTOCK_DIRECTORY_MESSAGE = m_dwSystemDirectory;
	SOptions.dwSTOCK_TRADING_ACTION_MESSAGE = m_dwStockTradingAction;
	SOptions.dwREG_SHO_RESTRICTION_MESSAGE = m_dwSHORestriction;
	SOptions.dwMP_POSITION_MESSAGE = m_dwMarketMakerPosition;
	SOptions.dwMWCBDLM_MESSAGE = m_dwMWCBDLM;
	SOptions.dwIPO_QUOTATION_PERIOD_UPDATE_MESSAGE = m_dwIPOQuotation;
	SOptions.dwNOII_MESSAGE = m_dwNOII;
	SOptions.dwRPPI_MESSAGE = m_dwRPPI;
	SOptions.dwMWCBDBM_MESSAGE = m_dwMWCBDBM;
	SOptions.dwTRADE_NON_CROSS_MESSAGE = m_dwTradeNonCross;
	SOptions.dwAllFeed = m_dwAll;
	SOptions.dwADD_ORDER_NO_MPID_MESSAGE = m_dwAddOrderNoMPID;
	SOptions.dwADD_ORDER_MPID_MESSAGE = m_dwAddOrderWithMPID;
	SOptions.dwORDER_EXECUTED_MESSAGE = m_dwOrderExecuted;
	SOptions.dwORDER_EXECUTED_WITH_PRICE_MESSAGE = m_OrderExecWithPrice;
	SOptions.dwORDER_CANCEL_MESSAGE = m_dwOrderCancel;
	SOptions.dwORDER_REPLACE_MESSAGE = m_dwOrderReplace;
	SOptions.dwORDER_DELETE_MESSAGE = m_dwOrderDelete;

	theApp.SMemoryMappedFileSSize = SOptions; // copy to server options

	int ihDatFile = -1;

	_sopen_s(&ihDatFile, "QtNQTVItch.Ini", _O_CREAT | _O_TRUNC | _O_RDWR | _O_BINARY | _O_SEQUENTIAL, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	int iNumberOfBytes = 0;

	if (ihDatFile)
	{
		iNumberOfBytes = _write(ihDatFile, &SOptions, sizeof(LOCALFILEDATA));
		_close(ihDatFile);
	}

	if ((iNumberOfBytes == -1))
		int xxx = 0; // ::TODO log error

	CDialogEx::OnOK();
}
/////////////////////////////////////////////////////////////////////////////////////
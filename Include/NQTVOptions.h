#pragma once

// CNQTVOptions dialog

class CNQTVOptions : public CDialogEx
{
	DECLARE_DYNAMIC(CNQTVOptions)

public:
	CNQTVOptions(CWnd *pParent = NULL); // standard constructor
	CNQTVOptions(bool bInit);
	virtual ~CNQTVOptions();

	BOOL LoadOptions();
	// Dialog Data
	enum
	{
		IDD = IDD_DIALOG_OPTIONS
	};

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV support
	bool m_bInit;

	DECLARE_MESSAGE_MAP()
public:
	DWORD m_dwSystemEvent;
	DWORD m_dwSystemDirectory;
	DWORD m_dwStockTradingAction;
	DWORD m_dwSHORestriction;
	DWORD m_dwMarketMakerPosition;
	DWORD m_dwMWCBDLM;
	DWORD m_dwIPOQuotation;
	DWORD m_dwNOII;
	DWORD m_dwRPPI;
	DWORD m_dwMWCBDBM;
	DWORD m_dwTradeNonCross;
	DWORD m_dwAll;
	DWORD m_dwAddOrderNoMPID;
	DWORD m_dwAddOrderWithMPID;
	DWORD m_dwOrderExecuted;
	DWORD m_OrderExecWithPrice;
	DWORD m_dwOrderCancel;
	DWORD m_dwOrderReplace;
	DWORD m_dwOrderDelete;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};

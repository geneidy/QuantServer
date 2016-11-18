#pragma once

// CConProperties dialog

class CConProperties : public CDialog
{
	DECLARE_DYNAMIC(CConProperties)

public:
	CConProperties(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConProperties();

// Dialog Data
	enum { IDD = IDD_DIALOG_CON_PROPERTIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strPassword;
	CString m_strUserName;
	DWORD m_dwIPAddress;
	DWORD m_dwIPAddress1;
	UINT m_uiPort1;
	UINT m_uiPort;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	int LoadOptions();
	DWORD m_dwBufferSize;
	CComboBox m_ctrlCBIPs;
	CString m_strIPs;
	int FillCBInterfaceList();

	afx_msg void OnCbnSelchangeComboIp();
	afx_msg void OnCbnSelendokComboIp();
	afx_msg void OnCbnEditchangeComboIp();
};

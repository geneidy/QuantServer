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

// CConProperties dialog

class CConProperties : public CDialog
{
	DECLARE_DYNAMIC(CConProperties)

public:
	CConProperties(CWnd *pParent = NULL); // standard constructor
	virtual ~CConProperties();

	// Dialog Data
	enum
	{
		IDD = IDD_DIALOG_CON_PROPERTIES
	};

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV support

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

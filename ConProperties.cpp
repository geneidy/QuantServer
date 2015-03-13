// ConProperties.cpp : implementation file
//

#include "stdafx.h"
#include "NQTV.h"
#include "ConProperties.h"
#include "afxdialogex.h"
#include "ITCHIncludes.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h> 
#include <share.h>
#include <errno.h>


// CConProperties dialog

IMPLEMENT_DYNAMIC(CConProperties, CDialog)

CConProperties::CConProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CConProperties::IDD, pParent)
	, m_strPassword(_T(""))
	, m_strUserName(_T(""))
	, m_dwIPAddress(0)
	, m_dwIPAddress1(0)
	, m_uiPort1(0)
	, m_uiPort(0)
	, m_dwBufferSize(0)
	, m_strIPs(_T(""))
{

}
/////////////////////////////////////////////////////////////////////////////////////
CConProperties::~CConProperties()
{

}
/////////////////////////////////////////////////////////////////////////////////////
void CConProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_strPassword);
	DDV_MaxChars(pDX, m_strPassword, 15);
	DDX_Text(pDX, IDC_EDIT1, m_strUserName);
	DDV_MaxChars(pDX, m_strUserName, 15);
	DDX_IPAddress(pDX, IDC_IPADDRESS2, m_dwIPAddress);
	DDX_IPAddress(pDX, IDC_IPADDRESS1, m_dwIPAddress1);
	DDX_Text(pDX, IDC_EDIT3, m_uiPort1);
	DDX_Text(pDX, IDC_EDIT4, m_uiPort);
	DDX_Text(pDX, IDC_EDIT5, m_dwBufferSize);
	DDV_MinMaxUInt(pDX, m_dwBufferSize, 1000, 1000000000);
	DDX_Control(pDX, IDC_COMBO_IP, m_ctrlCBIPs);
	DDX_CBString(pDX, IDC_COMBO_IP, m_strIPs);
}
/////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CConProperties, CDialog)
	ON_BN_CLICKED(IDOK, &CConProperties::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_IP, &CConProperties::OnCbnSelchangeComboIp)
	ON_CBN_SELENDOK(IDC_COMBO_IP, &CConProperties::OnCbnSelendokComboIp)
	ON_CBN_EDITCHANGE(IDC_COMBO_IP, &CConProperties::OnCbnEditchangeComboIp)
END_MESSAGE_MAP()
// CConProperties message handlers
/////////////////////////////////////////////////////////////////////////////////////
void CConProperties::OnBnClickedOk()
{
	LOB_VIEW_CLT_OPTIONS SOptions;
	UpdateData(true);

	strcpy_s(SOptions.szUserName, SIZE_OF_CLT_NAME, (const char*) m_strUserName.GetString());
	strcpy_s(SOptions.szPassword, SIZE_OF_CLT_PASSWORD, (const char*)m_strPassword.GetString());

	SOptions.dwIPAddress	=	m_dwIPAddress;
	SOptions.uiPort			=	m_uiPort;

	SOptions.dwIPAddress1	=	m_dwIPAddress1;
	SOptions.uiPort1		=	m_uiPort1;
	SOptions.dwBufferSize   =   m_dwBufferSize;

	int	ihDatFile = -1;

	_sopen_s(&ihDatFile, "QtCltOptions.Ini", _O_CREAT|_O_TRUNC|_O_RDWR |_O_BINARY |_O_SEQUENTIAL, _SH_DENYNO, _S_IREAD | _S_IWRITE );  

	int	iNumberOfBytes = 0;
	
	if (ihDatFile)
	{
		iNumberOfBytes = _write( ihDatFile, &SOptions, sizeof(LOB_VIEW_CLT_OPTIONS));
		_close(ihDatFile);
	}
	theApp.m_SOptions = SOptions;

	OnOK();
	CDialog::OnOK();
}
/////////////////////////////////////////////////////////////////////////////////////
BOOL CConProperties::OnInitDialog()
{
	CDialog::OnInitDialog();

	LoadOptions();
	FillCBInterfaceList();
	UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
/////////////////////////////////////////////////////////////////////////////////////
int CConProperties::LoadOptions()
{
	int	ihDatFile = -1;

	errno_t err = _sopen_s(&ihDatFile, "QtCltOptions.Ini", _O_RDWR | _O_BINARY |_O_SEQUENTIAL, _SH_DENYNO, _S_IREAD | _S_IWRITE );  

	if	( err != 0 )
	{
		return TRUE; 
	}
// redundant...just for debug purposes
	switch (err)
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

	int	iNumberOfBytes = 0;

	LOB_VIEW_CLT_OPTIONS	SOptions;	
	memset(&SOptions, '\0', sizeof(SOptions));

	if (ihDatFile)
	{
		iNumberOfBytes = _read(ihDatFile, &SOptions, sizeof(LOB_VIEW_CLT_OPTIONS));
		_close(ihDatFile);
	}

	if (iNumberOfBytes == -1)
		return false;  // log error

	if (ihDatFile)
	{
		m_strUserName	=	SOptions.szUserName;
		m_strPassword	=	SOptions.szPassword;
		m_dwIPAddress	=	SOptions.dwIPAddress;
		m_uiPort		=	SOptions.uiPort;
		m_dwIPAddress1	=	SOptions.dwIPAddress1;
		m_uiPort1		=	SOptions.uiPort1;
		m_dwBufferSize  =	SOptions.dwBufferSize;

		theApp.m_SOptions = SOptions;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
void CConProperties::OnCbnSelendokComboIp()
{

	int ii = m_ctrlCBIPs.GetCurSel();
	UpdateData(true);
	m_ctrlCBIPs.GetLBText(ii, m_strIPs);

}
//////////////////////////////////////////////////////////////////////////////////////
void CConProperties::OnCbnEditchangeComboIp()
{
	int ii = m_ctrlCBIPs.GetCurSel();
	UpdateData(true);
	m_ctrlCBIPs.GetLBText(ii, m_strIPs);
}
//////////////////////////////////////////////////////////////////////////////////////
void CConProperties::OnCbnSelchangeComboIp()
{
	int ii = m_ctrlCBIPs.GetCurSel();
	UpdateData(true);
	m_ctrlCBIPs.GetLBText(ii, m_strIPs);
}
/////////////////////////////////////////////////////////////////////////////////////
int CConProperties::FillCBInterfaceList()
{
	WORD versionRequested = 0;
	WSADATA wsaData;
	socket sSocket;
	DWORD dReturned = 0;
	INTERFACE_INFO localAddr[10];

	int iNumInterfaces = 0;
 	int iCounter;

	sSocket = INVALID_socket;

	versionRequested = MAKEWORD(2, 0);
	if (WSAStartup(versionRequested, &wsaData ) == 0)
    {
	   if ((sSocket = socket(AF_INET, SOCK_DGRAM, 0)) != INVALID_socket)
         {
	      if (socket_ERROR != WSAIoctl(sSocket, SIO_GET_INTERFACE_LIST, NULL, 0, &localAddr, sizeof(localAddr), &dReturned, NULL, NULL))
          {
		   	iCounter = 0;
			iNumInterfaces = dReturned/sizeof(INTERFACE_INFO);
	         for(int ii = 0; ii < iNumInterfaces; ii++)
   	         {
		         if((localAddr[ii].iiFlags & IFF_UP) && (localAddr[ii].iiFlags & IFF_MULTICAST) && !(localAddr[ii].iiFlags & IFF_LOOPBACK) )
                  {
					  m_ctrlCBIPs.AddString( inet_ntoa(((SOCKADDR_IN *)&localAddr[ii].iiAddress)->sin_addr));
					  iCounter++;
                  }
	            }
          }
        closesocket(sSocket);
        }
		WSACleanup();
    }
	return(iCounter);
}
///////////////////////////////////////////////////////////////////////////

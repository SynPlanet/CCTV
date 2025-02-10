///////////////////////////////////////////////////////////////////////////////////
// Copyright(c) 2011 by Samsung Techwin, Inc.
//
// This software is copyrighted by, and is the sole property of Samsung Techwin.
// All rigths, title, ownership, or other interests in the software remain the
// property of Samsung Techwin. This software may only be used in accordance with
// the corresponding license agreement. Any unauthorized use, duplication,
// transmission, distribution, or disclosure of this software is expressly
// forbidden.
//
// This Copyright notice may not be removed or modified without prior written
// consent of Samsung Techwin.
//
// Samsung Techwin reserves the right to modify this software without notice.
//
// Samsung Techwin, Inc.
// http://developer.samsungtechwin.com/
///////////////////////////////////////////////////////////////////////////////////
// PtzDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Ptz.h"
#include "PtzDlg.h"

// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// This files are installed in {$SDK path}\sample_code\include 
// You should include this files
// -----------------------------------------------------------------------
#include "XnsCommon.h"
#include "XnsDeviceInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Macro for OutputDebugString()
#define DBG_LOG(...) do{\
	CString strMessage = _T("");\
	strMessage.AppendFormat(_T("(%S:%d)"), __FUNCTION__, __LINE__);	\
	strMessage.AppendFormat(__VA_ARGS__);\
	OutputDebugString(strMessage);\
}while(0);

// Macro for AfxMessageBox()
#define MESSAGE_BOX(...) do{\
	CString strMessage = _T("");\
	strMessage.Format(__VA_ARGS__);\
	;\
}while(0);
//AfxMessageBox(strMessage);
// Macro for EditText control (Device control)
#define WLOGD(...) do{\
	CString strMessage = _T("");\
	strMessage.AppendFormat(_T("[XnsSdkDevice] "));\
	strMessage.AppendFormat(__VA_ARGS__);\
	long nLen = m_ctrlEditLog.GetWindowTextLength(); \
	m_ctrlEditLog.SetFocus(); \
	m_ctrlEditLog.SetSel(nLen, nLen); \
	m_ctrlEditLog.ReplaceSel(strMessage); \
}while(0);

// Macro for EditText control (Window control)
#define WLOGW(...) do{\
	CString strMessage = _T("");\
	strMessage.AppendFormat(_T("[XnsSdkWindow] "));\
	strMessage.AppendFormat(__VA_ARGS__);\
	long nLen = m_ctrlEditLog.GetWindowTextLength(); \
	m_ctrlEditLog.SetFocus(); \
	m_ctrlEditLog.SetSel(nLen, nLen); \
	m_ctrlEditLog.ReplaceSel(strMessage); \
}while(0);

// CAboutDlg dialog used for App About

// class CAboutDlg : public CDialog
// {
// public:
// 	CAboutDlg();
// 
// // Dialog Data
// 	enum { IDD = IDD_ABOUTBOX };
// 
// 	protected:
// 	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
// 
// // Implementation
// protected:
// 	DECLARE_MESSAGE_MAP()
// };

// CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
// {
// }

// void CAboutDlg::DoDataExchange(CDataExchange* pDX)
// {
// 	CDialog::DoDataExchange(pDX);
// }
// 
// BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
// END_MESSAGE_MAP()


// CPtzDlg dialog


PTZ_GetValue	pCB_GetPtzValues = NULL;


CPtzDlg::CPtzDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPtzDlg::IDD, pParent)
	, m_strModelName(_T(""))
	, m_strAnyCmrModelName(_T(""))
	, m_nPort(4520)
	, m_strId(_T("admin"))
	, m_strPasswd(_T("4321"))
	, m_hDevice(0)
	, m_hMediaSource(0)
	, m_nPtzSpeed(0)
	, m_bAreaZoom(0)
	, m_nStartX(0)
	, m_nStartY(0)
	, m_nEndX(0)
	, m_nEndY(0)
	, m_nControlId(0)
	, m_nPan(0)
	, m_nTilt(0)
	, m_nZoom(0)
	, m_bMenuControl(0)
	, m_bIsMouseDown(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	CString strMessage = _T("");
}

/*
//Destroying the modeless dialog
void CPtzDlg::PostNcDestroy(void)
{	
	CDialog::PostNcDestroy();
	CWnd* a = GetParent();
	PostMessage(WM_CLOSE,0,0);
}
*/

void CPtzDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_XNSSDKDEVICECTRL, m_ctrlXnsDevice);
	DDX_Control(pDX, IDC_COMBO_MODEL_LIST, m_ctrlModelList);
	DDX_Control(pDX, IDC_IPADDRESS, m_ctrlIpAddress);
	DDX_Text(pDX, IDC_COMBO_MODEL_LIST, m_strModelName);
	DDX_Text(pDX, IDC_EDIT_ID, m_strId);
	DDX_Text(pDX, IDC_EDIT_PW, m_strPasswd);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_EDIT_LOG_WIN, m_ctrlEditLog);
	DDX_Slider(pDX, IDC_SLIDER_PTZ_SPEED, m_nPtzSpeed);
	DDV_MinMaxInt(pDX, m_nPtzSpeed, 1, 100);
	DDX_Text(pDX, IDC_EDIT_PAN, m_nPan);
	DDX_Text(pDX, IDC_EDIT_TILT, m_nTilt);
	DDX_Text(pDX, IDC_EDIT_ZOOM, m_nZoom);
	DDX_Control(pDX, IDC_COMBO_AUTOSCAN, m_ctrlAutoScan);
	DDX_Control(pDX, IDC_COMBO_AUTOPAN, m_ctrlAutoPan);
}

BEGIN_MESSAGE_MAP(CPtzDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CPtzDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CPtzDlg::OnBnClickedButtonDisconnect)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_AREAZOOM, &CPtzDlg::OnBnClickedCheckAreazoom)
	ON_BN_CLICKED(IDC_BUTTON_ZOOM_1X, &CPtzDlg::OnBnClickedButtonZoom1x)
	ON_BN_CLICKED(IDC_BUTTON_GET_PTZ, &CPtzDlg::OnBnClickedButtonGetPtz)
	ON_BN_CLICKED(IDC_BUTTON_SET_PTZ, &CPtzDlg::OnBnClickedButtonSetPtz)
	ON_BN_CLICKED(IDC_CHECK_MENU_ON, &CPtzDlg::OnBnClickedCheckMenuOn)
	ON_BN_CLICKED(IDC_BUTTON_MENU_UP, &CPtzDlg::OnBnClickedButtonMenuUp)
	ON_BN_CLICKED(IDC_BUTTON_MENU_DOWN, &CPtzDlg::OnBnClickedButtonMenuDown)
	ON_BN_CLICKED(IDC_BUTTON_MENU_LEFT, &CPtzDlg::OnBnClickedButtonMenuLeft)
	ON_BN_CLICKED(IDC_BUTTON_MENU_RIGHT, &CPtzDlg::OnBnClickedButtonMenuRight)
	ON_BN_CLICKED(IDC_BUTTON_MENU_ENTER, &CPtzDlg::OnBnClickedButtonMenuEnter)
	ON_BN_CLICKED(IDC_BUTTON_MENU_CANCEL, &CPtzDlg::OnBnClickedButtonMenuCancel)
	ON_BN_CLICKED(IDC_BUTTON_PRESET, &CPtzDlg::OnBnClickedButtonPreset)
	ON_CBN_SELCHANGE(IDC_COMBO_AUTOPAN, &CPtzDlg::OnCbnSelchangeComboAutopan)
	ON_CBN_SELCHANGE(IDC_COMBO_AUTOSCAN, &CPtzDlg::OnCbnSelchangeComboAutoscan)
//	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CPtzDlg message handlers

BOOL CPtzDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_ctrlIpAddress.SetAddress(192,168,0,149);

	GetDlgItem(IDC_BUTTON_UP)->SetWindowTextW(_T("¡è"));
	GetDlgItem(IDC_BUTTON_UPRIGHT)->SetWindowTextW(_T("¢Ö"));
	GetDlgItem(IDC_BUTTON_UPLEFT)->SetWindowTextW(_T("¢Ø"));
	GetDlgItem(IDC_BUTTON_DOWN)->SetWindowTextW(_T("¡é"));
	GetDlgItem(IDC_BUTTON_DOWNRIGHT)->SetWindowTextW(_T("¢Ù"));
	GetDlgItem(IDC_BUTTON_DOWNLEFT)->SetWindowTextW(_T("¢×"));
	GetDlgItem(IDC_BUTTON_LEFT)->SetWindowTextW(_T("¡ç"));
	GetDlgItem(IDC_BUTTON_RIGHT)->SetWindowTextW(_T("¡æ"));
	m_nPtzSpeed = 50;
	
 	ShowWindow(SW_HIDE);

// 	HWND hwnd_win = ::GetForegroundWindow();
// 	::ShowWindow(hwnd_win, SW_HIDE);

	Init_DVC();

	UpdateData(FALSE);
 //	UpdateWindow();

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Initializes the DLL files. 
	// For this, XnsActiveX library requires config.xml, device.xml, 
	// and xns.xml files and the DLL file list should be mentioned 
	// in Xns.xml file. The path of the DLL file can not exceed 512 bytes
	// in length. The XnsActiveX library searches for xns.xml using 
	// XnsSDKDevice.ocx installed in "{$SDK path}\Config" folder.
	// -----------------------------------------------------------------------
// 		CDXnsSdkDevice		*p_ctrlXnsDevice = NULL;	// XnsDevice control
// 		p_ctrlXnsDevice = new CDXnsSdkDevice;
// 
// 		p_ctrlXnsDevice->Create(_T("STATIC"), _T("Hi"), WS_CHILD | WS_VISIBLE,	CRect(0, 0, 20, 20), NULL, 1234);
// 		long nRet_ = p_ctrlXnsDevice->Initialize();
//	AFX_MANAGE_STATE(AfxGetAppModuleState( ));


// 	long nRet = m_ctrlXnsDevice.Initialize();
// 	if (nRet != ERR_SUCCESS)
// 	{
// 		WLOGD(_T("Initialize() fail: errno=[%d]\n"), nRet);
// 	}
// 	DBG_LOG(_T("Start\n"));
// 
// 	// [ XNS ACTIVEX HELP ]
// 	// -----------------------------------------------------------------------
// 	// Initializes the XnsSdkWindow control. 
// 	// Namely, this will specify the window handle in order to display 
// 	// images on the screen. 
// 	// -----------------------------------------------------------------------
// 
// 	// [ XNS ACTIVEX HELP ]
// 	// -----------------------------------------------------------------------
// 	// Assigns memory space for saving device information. 
// 	// This function will return the device handle, which the application 
// 	// can use to control the device.
// 	// [in] Device ID The value should be a greater integer than 0.
// 	//      Minimum value: 1 , Maximum value: 3000
// 	// -----------------------------------------------------------------------
// 	m_hDevice = m_ctrlXnsDevice.CreateDevice(1);
// 	if (m_hDevice == 0)
// 	{
// 		WLOGD(_T("CreateDevice() fail\n"));
// 	}
// 	
// 
// 	// [ XNS ACTIVEX HELP ]
// 	// -----------------------------------------------------------------------
// 	// Reads device list from {$SDK path}\Bin\Config\xns.xml. 
// 	// Finds information mentioned in the configuration file before returning. 
// 	// To enable this function, follow the steps below:
// 	// < Procedure >
// 	// Step 1. 	Use FindVendor(), FindModel(), FindDevice() to start searching.
// 	// Step 2. 	Use GetFindSize to get the array size.
// 	// Step 3. 	Use GetFindString() to obtain the vendor name and model name.
// 	// Step 4. 	Use GetFindLong() and GetFindDouble0 to obtain the device ID. 
// 	//			(Repeat this process if necessary)
// 	// Step 5. 	Call CloseFind0 to end the search.
// 	// -----------------------------------------------------------------------
// 	// Returns the handle of the Find array. 
// 	// Returns NULL(0) if no model list is found.
// 	long nFind = m_ctrlXnsDevice.FindModel(_T("samsung"));
// 
// 	// Returns the length of the array that is created using FindVendor(), 
// 	// FindModel(), and FindDevice().
// 	long nCount = m_ctrlXnsDevice.GetFindSize(nFind);
// 
// 	CString strModel;
// 	for (int i = 0 ; i < nCount ; i++)
// 	{
// 		// Reurns data of nIndex from the array created using FindVendor() 
// 		// and FindModel(). If nIndex is larger than the actual array, 
// 		// "" will be returned.
// 		strModel = m_ctrlXnsDevice.GetFindString(nFind, i);
// 		m_ctrlModelList.AddString(strModel);
// 	}
// 	// Closes the Find command. Releases the memory space assinged by FindXXX().
// 	m_ctrlXnsDevice.CloseFind(nFind);
// 

	m_ctrlModelList.SetCurSel(1);
// 	int idx = m_ctrlModelList.FindStringExact(0, _T("SNP-3301"));
// 	if(idx != CB_ERR) {
// 		m_ctrlModelList.SetCurSel(idx);
// 		m_ctrlModelList.GetLBText(m_ctrlModelList.GetCurSel(), m_strModelName);
// 		UpdateData(FALSE);
// 	}

	// Scan combo list
	m_ctrlAutoScan.AddString(_T("None"));
	for(int i=0;i<6;i++)
	{
		CString strIndex;
		strIndex.Format(_T("Scan [%d]"), i+1);
		m_ctrlAutoScan.AddString(strIndex);
	}
	m_ctrlAutoScan.SetCurSel(0);

	// auto pan combo list
	m_ctrlAutoPan.AddString(_T("None"));
	for(int i=0;i<4;i++)
	{
		CString strIndex;
		strIndex.Format(_T("Auto pan[%d]"), i+1);
		m_ctrlAutoPan.AddString(strIndex);
	}
	m_ctrlAutoPan.SetCurSel(0);

	setBtnStatus(SL_STATUS_DISCONNECTED);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPtzDlg::Init_DVC(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	long nRet = m_ctrlXnsDevice.Initialize();

	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("Initialize() fail: errno=[%d]\n"), nRet);
	}
	DBG_LOG(_T("Start\n"));

//	ShowWindow(SW_HIDE);

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Initializes the XnsSdkWindow control. 
	// Namely, this will specify the window handle in order to display 
	// images on the screen. 
	// -----------------------------------------------------------------------

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Assigns memory space for saving device information. 
	// This function will return the device handle, which the application 
	// can use to control the device.
	// [in] Device ID The value should be a greater integer than 0.
	//      Minimum value: 1 , Maximum value: 3000
	// -----------------------------------------------------------------------
	m_hDevice = m_ctrlXnsDevice.CreateDevice(1);
	if (m_hDevice == 0)
	{
		WLOGD(_T("CreateDevice() fail\n"));
	}


	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Reads device list from {$SDK path}\Bin\Config\xns.xml. 
	// Finds information mentioned in the configuration file before returning. 
	// To enable this function, follow the steps below:
	// < Procedure >
	// Step 1. 	Use FindVendor(), FindModel(), FindDevice() to start searching.
	// Step 2. 	Use GetFindSize to get the array size.
	// Step 3. 	Use GetFindString() to obtain the vendor name and model name.
	// Step 4. 	Use GetFindLong() and GetFindDouble0 to obtain the device ID. 
	//			(Repeat this process if necessary)
	// Step 5. 	Call CloseFind0 to end the search.
	// -----------------------------------------------------------------------
	// Returns the handle of the Find array. 
	// Returns NULL(0) if no model list is found.
	long nFind = m_ctrlXnsDevice.FindModel(_T("samsung"));

	// Returns the length of the array that is created using FindVendor(), 
	// FindModel(), and FindDevice().
	long nCount = m_ctrlXnsDevice.GetFindSize(nFind);

	CString strModel;
	for (int i = 0 ; i < nCount ; i++)
	{
		// Reurns data of nIndex from the array created using FindVendor() 
		// and FindModel(). If nIndex is larger than the actual array, 
		// "" will be returned.
		strModel = m_ctrlXnsDevice.GetFindString(nFind, i);
		m_ctrlModelList.AddString(strModel);

		if (i == 1){
			m_strAnyCmrModelName = strModel;
		}
	}
	// Closes the Find command. Releases the memory space assinged by FindXXX().
	m_ctrlXnsDevice.CloseFind(nFind);


}

void CPtzDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
// 		CAboutDlg dlgAbout;
// 		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPtzDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPtzDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPtzDlg::OnBnClickedButtonConnect()
{

	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if( !m_hDevice ){
		return;
	}

	CString strIpAddress;
	BYTE a1, a2, a3, a4;
	m_ctrlIpAddress.GetAddress(a1, a2, a3, a4);
	strIpAddress.Format(_T("%d.%d.%d.%d"), a1, a2, a3, a4);

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Sets the device informaton so that the application can connect 
	// to the device.
	// -----------------------------------------------------------------------
	m_ctrlXnsDevice.SetConnectionInfo(	m_hDevice,					// [in] Device handle
																			_T("Samsung"),				// [in] Fixed as 'Samsung'
																			m_strModelName,				// [in] Name of model to connect to. The maximum length allowed is 126-byte.
																			XADDRESS_IP, strIpAddress,	// [in] Address type, actual address according to addresstype.
																			m_nPort, 0,					// [in] Port number, port number for web access	
																			m_strId, m_strPasswd		// [in] Login ID, password
																		);

	//WLOGD(_T("Connect. ip=%s, port=%d, id=%s, password=%s\n"), strIpAddress, m_nPort, m_strId, m_strPasswd);

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Enables the application to connect to the device using the connection 
	// information. You can use SetConnectionInfo() to configure the 
	// connection settings. This function performs as non-blocking function, 
	// and will be returned immediately even if the connection is not 
	// completed. The connection result will be transferred through the event. 
	// When connection is made successfully, the OnDeviceStatusChanged() event
	// will occur. When failed, the OnConnectFailed event occurs.
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.ConnectNonBlock(	m_hDevice,		// [in] Device handle
																								TRUE,			// [in] Flag to decide where to forcibly log in or not.
																								TRUE			// [in] If this value is 1, try to connect again.
																							);
	if(nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ConnectNonBlock() fail: errno=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
		setBtnStatus(SL_STATUS_DISCONNECTED);
	}
}

// see function "OnBnClickedButtonConnect"
int CPtzDlg::ConnectToDevice( T_NetAddress*	p_net_addr_in )
{
		// TODO: Add your control notification handler code here
//	UpdateData(TRUE);
	USES_CONVERSION;

	if( !m_hDevice ){
		return 1;
	}

	CString strIpAddress;

	strIpAddress.Format(_T("%d.%d.%d.%d"), p_net_addr_in->IP[0], p_net_addr_in->IP[1], p_net_addr_in->IP[2], p_net_addr_in->IP[3]);
	m_nPort = 4520;

	WCHAR *bstr_login = A2W(p_net_addr_in->Login);
	WCHAR *bstr_Password = A2W(p_net_addr_in->Password);

	m_strId.Format(_T("%s"), bstr_login);
	m_strPasswd.Format(_T("%s"), bstr_Password);

	//m_strId.Format(_T("admin"));
	//m_strPasswd.Format(_T("4321"));

	///// test
	char mes[_MAX_PATH];
	sprintf_s(mes, _MAX_PATH, "Login:%s; Psw:%s; Port: %d", p_net_addr_in->Login, p_net_addr_in->Password, p_net_addr_in->Port);
	//MessageBoxA(0, mes, "Info", 0);
	/////
	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Sets the device informaiton so that the application can connect 
	// to the device.
	// -----------------------------------------------------------------------
	m_ctrlXnsDevice.SetConnectionInfo(	m_hDevice,					// [in] Device handle
																			_T("Samsung"),				// [in] Fixed as 'Samsung'
																			m_strAnyCmrModelName,				// [in] Name of model to connect to. The maximum length allowed is 126-byte.
																			XADDRESS_IP, strIpAddress,	// [in] Address type, actual address according to addresstype.
																			m_nPort, 0,					// [in] Port number, port number for web access	
																			m_strId, m_strPasswd		// [in] Login ID, password
																		);

	//WLOGD(_T("Connect. ip=%s, port=%d, id=%s, password=%s\n"), strIpAddress, m_nPort, m_strId, m_strPasswd);

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Enables the application to connect to the device using the connection 
	// information. You can use SetConnectionInfo() to configure the 
	// connection settings. This function performs as non-blocking function, 
	// and will be returned immediately even if the connection is not 
	// completed. The connection result will be transferred through the event. 
	// When connection is made successfully, the OnDeviceStatusChanged() event
	// will occur. When failed, the OnConnectFailed event occurs.
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.ConnectNonBlock(	m_hDevice,		// [in] Device handle
																								TRUE,			// [in] Flag to decide where to forcibly log in or not.
																								TRUE			// [in] If this value is 1, try to connect again.
																							);
	if(nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ConnectNonBlock() fail: errno=[%d](%s)\n"), nRet, m_ctrlXnsDevice.GetErrorString(nRet));
		setBtnStatus(SL_STATUS_DISCONNECTED);
		return 1;
	}else{
		/////
		//MessageBoxA(0, "Connect GOOD", "Info", 0);
		/////
	}
	return 0;
	
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPtzDlg::OnBnClickedButtonDisconnect()
{
	// TODO: Add your control notification handler code here
	if( !m_hMediaSource ) 
	{
		WLOGD(_T("May be device is not connected\n"));
		setBtnStatus(SL_STATUS_DISCONNECTED);
		return;
	}
	else
	{
		//WLOGW(_T("Stop receiving media stream data\n"));

		// Terminates transferring media stream data from the device. The media 
		// stream data will be separated by hMediaSource 
		// (i.e., phMediaSource of OpenMedia()).

		if( !m_hDevice ){
			return;
		}

		m_ctrlXnsDevice.CloseMedia(m_hDevice, m_hMediaSource);
	}

	// Disconnects from the device.
	m_ctrlXnsDevice.Disconnect(m_hDevice);
	//WLOGD(_T("Disconnect\n"));

	m_hMediaSource = 0;
	setBtnStatus(SL_STATUS_DISCONNECTED);
}

// see function "OnBnClickedButtonDisconnect"
int CPtzDlg::DisconnectDevice( T_NetAddress*	p_net_addr_in )
{
	// TODO: Add your control notification handler code here
	if( !m_hMediaSource ) 
	{
		WLOGD(_T("May be device is not connected\n"));
		setBtnStatus(SL_STATUS_DISCONNECTED);
		return 1;
	}
	else
	{
		//WLOGW(_T("Stop receiving media stream data\n"));

		// Terminates transferring media stream data from the device. The media 
		// stream data will be separated by hMediaSource 
		// (i.e., phMediaSource of OpenMedia()).

		if( !m_hDevice ){
			return 1;
		}

		m_ctrlXnsDevice.CloseMedia(m_hDevice, m_hMediaSource);
	}

	// Disconnects from the device.
	m_ctrlXnsDevice.Disconnect(m_hDevice);
	//WLOGD(_T("Disconnect\n"));

	m_hMediaSource = 0;
	setBtnStatus(SL_STATUS_DISCONNECTED);
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


BEGIN_EVENTSINK_MAP(CPtzDlg, CDialog)
	ON_EVENT(CPtzDlg, IDC_XNSSDKDEVICECTRL, 10, CPtzDlg::OnDeviceStatusChangedXnssdkdevicectrl, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPtzDlg, IDC_XNSSDKDEVICECTRL, 3, CPtzDlg::OnConnectFailedXnssdkdevicectrl, VTS_I4 VTS_I4)
	ON_EVENT(CPtzDlg, IDC_XNSSDKWINDOWCTRL, 5, CPtzDlg::OnLButtonDownXnssdkwindowctrl, VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPtzDlg, IDC_XNSSDKWINDOWCTRL, 6, CPtzDlg::OnLButtonUpXnssdkwindowctrl, VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPtzDlg, IDC_XNSSDKWINDOWCTRL, 11, CPtzDlg::OnMouseMoveXnssdkwindowctrl, VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPtzDlg, IDC_XNSSDKDEVICECTRL, 31, CPtzDlg::OnGetPtzPosXnssdkdevicectrl, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPtzDlg, IDC_XNSSDKDEVICECTRL, 6, CPtzDlg::OnGetPresetListXnssdkdevicectrl, VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPtzDlg, IDC_XNSSDKDEVICECTRL, 38, CPtzDlg::OnGetHPtzListXnssdkdevicectrl, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// This event occurs if the device status has changed. It occurs if the 
// application uses Connect() to connect or reconnect to the device after 
// disconnected. When reconnecting, the third argument nDeviceStatus is 
// true, all media must be reopened using ReopenAllStream().
// -----------------------------------------------------------------------
void CPtzDlg::OnDeviceStatusChangedXnssdkdevicectrl(long nDeviceID, long nErrorCode, long nDeviceStatus, long nHddCondition)
{
	// TODO: Add your message handler code her
	if (nErrorCode != ERR_SUCCESS || nDeviceStatus != 1)
	{
		return;
	}

	if( !m_hDevice ){
		return;
	}

	//WLOGD(_T("Connected: deviceID=%d, errcode=%d, deviceStatus=%d\n"), nDeviceID, nErrorCode, nDeviceStatus);
	setBtnStatus(SL_STATUS_CONNECTED);
	
	if(m_hMediaSource) 
	{
		WLOGD(_T("Media stream was already opend\n"));
		return;
	}

	int xret = m_ctrlXnsDevice.GetControlType(m_hDevice, 1);

	//WLOGD(_T("Open media stream:%d\n"), xret);
	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Returns the type of control module corresponding to the control ID.
	//
	// < DVR control module structure >
	// +------------------------------------------------------------
	// Control Module(Device, 1) ----------------- Control ID : 1
	// |---- Control Module(Camera, 1) ----------- Control ID : 2
	// |---- Control Module(Camera, 2) ----------- Control ID : 3
	// |---- Control Module(Camera, 3) ----------- Control ID : 4
	// |---- ....
	// |---- Control Module(Camera,16) ----------- Control ID : 17
	// |---- Control Module(Sensor-in, 1) -------- Control ID : 18
	// |---- ...
	// |---- Control Module(Sensor-in, 16) ------- Control ID : 34
	// |---- Control Module(Digital-out, 1) ------ Control ID : 35
	// |---- ...
	// |---- Control Module(Digital-out, 4) ------ Control ID : 38
	// +------------------------------------------------------------
	//
	// < Network Camera control module structure >
	// +------------------------------------------------------------
	// Control Module(Device, 1) ----------------- Control ID : 1
	// |---- Control Module(Camera, 1) ----------- Control ID : 2
	//       |--- Control Module(Video, 1) ------- Control ID : 3
	//       |--- Control Module(Video, 2) ------- Control ID : 4
	//       |--- ...
	//       |--- Control Module(Video, 10) ------ Control ID : 12
	//       |--- Control Module(Sensor-in, 1) --- Control ID : 13
	//       |--- Control Module(Digital-out, 1) - Control ID : 14
	// +------------------------------------------------------------
	// -----------------------------------------------------------------------
	if (m_ctrlXnsDevice.GetControlType(m_hDevice, 1) & XCTL_DVR)
	{
		// Returns the number of control modules. The application can get 
		// the number of a specific type of control modules, and can get 
		// also the whole number of video recorders or cameras.
		// DVR control id is start with 2
		int nCount = m_ctrlXnsDevice.GetControlCount(m_hDevice, XCTL_CAMERA);
		for(int i=0; i<nCount ; i++)
		{
			// Returns the capabilities of the control module.
			if (m_ctrlXnsDevice.GetControlCapability(m_hDevice, i+2, XCTL_CAP_LIVE) &&
				// Gets the status of the control module.
				// DVR control id is start with 2
				m_ctrlXnsDevice.GetControlStatus(m_hDevice, i+2, 1))
			{
				// When called, it will start getting media streams from the 
				// device. The receiving media streams will, then, be 
				// forwarded to the XnsSdkWindow component that will play 
				// the streams after decoding. phMediaSource is needed to 
				// link the stream data with XnsSdkWindow. 
				// The value can be obtained from a parameter (out-parameter)
				// of OpenMedia(). When XnsSdkWindow receives this value, 
				// it can get stream data from the device. 
				// phMediaSource is also used for controlling playback of 
				// multimedia files. As a result, the application should keep this value at all times.
				m_ctrlXnsDevice.OpenMedia(
																	m_hDevice,			// [in] Device handle. This value is returned from CreateDevice().
																	i+2,					// [in] Control ID of video control module. 
																	MEDIA_TYPE_LIVE,	// [in] Media type.
																	0,					// [in] Play start time. Format: 4-byte time_t.
																	0,					// [in] Play end time. Format: 4-byte time_t.
																	&m_hMediaSource		// [out] Media stream ID. This is needed for controlling 
																						//       the media stream and also used as a parameter 
																						//       of XnsSdkWindow::Start().
					);

				m_nControlId = i+2;
				//WLOGD(_T("DVR TYPE,  control id = %d\n"), m_nControlId);
				return;
			}
		}
	}
	else if (m_ctrlXnsDevice.GetControlType(m_hDevice, 1) & (XCTL_NETCAM |XCTL_ENCODER))
	{	
		int nCount = m_ctrlXnsDevice.GetControlCount(m_hDevice, XCTL_VIDEO);
		if (nCount==0)
			nCount = m_ctrlXnsDevice.GetControlCount(m_hDevice, XCTL_CAMERA);
		for(int i=0; i<nCount ; i++)
		{
			// Network camera and encoder control id is start with 3
			if (m_ctrlXnsDevice.GetControlCapability(m_hDevice, i+3, XCTL_CAP_LIVE) &&
				m_ctrlXnsDevice.GetControlStatus(m_hDevice, i+3, 1))
			{
				m_ctrlXnsDevice.OpenMedia(m_hDevice, i+3, MEDIA_TYPE_LIVE, 0, 0, &m_hMediaSource);

				m_nControlId = i+3;
				//WLOGD(_T("NETCAM or ENCODER TYPE,  control id = %d\n"), m_nControlId);
				return;
			}
		}
	}
	//WLOGD(_T("No valid device.."));
	
}

// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// If the application has failed in non-blocking connection using 
// ConnectNonBlock(), the OnConnectFailed event occurs. As Connect() 
// returns an immediate error message if failed, it does not trigger 
// this event. 
// -----------------------------------------------------------------------
void CPtzDlg::OnConnectFailedXnssdkdevicectrl(long nDeviceID, long nErrorCode)
{
	// TODO: Add your message handler code here
	//WLOGD(_T("Disconnected: deviceID=%d, errcode=[%d](%s)\n"), nDeviceID, nErrorCode, m_ctrlXnsDevice.GetErrorString(nErrorCode));

	MESSAGE_BOX(_T("Connection fail.."));
	setBtnStatus(SL_STATUS_DISCONNECTED);
}



void CPtzDlg::setBtnStatus(SLIVE_BUTTON_STATUS nStatus)
{
	bool conn = false, disconn = false, play = false, stop = false;
	switch(nStatus)
	{
	case SL_STATUS_CONNECTED:
		conn = false;
		disconn = true;
		play = true;
		stop = false;
		break;
	case SL_STATUS_DISCONNECTED:
		conn = true;
		disconn = false;
		play = false;
		stop = false;
		break;
	}

	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(conn);
	GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(disconn);
}

//////////////////////////////////////////////////////////////////////////
void CPtzDlg::ReleaseDevices_Manually(void)
{
	if( m_hDevice )
	{
		// [ XNS ACTIVEX HELP ]
		// -----------------------------------------------------------------------
		// Stops receiving the stream data from the media source handle.  
		// -----------------------------------------------------------------------
		if( m_hMediaSource )
		{

			// Terminates transferring media stream data from the device. 
			// The media stream data will be separated by hMediaSource 
			// (i.e., phMediaSource of OpenMedia()).
			m_ctrlXnsDevice.CloseMedia( m_hDevice, m_hMediaSource );
		}

		// [ XNS ACTIVEX HELP ]
		// -----------------------------------------------------------------------
		// Returns the connection status of the device.
		// -----------------------------------------------------------------------
		if( m_ctrlXnsDevice.GetDeviceStatus(m_hDevice) == XDEVICE_STATUS_CONNECTED )
		{
			m_ctrlXnsDevice.Disconnect(m_hDevice);
		}

		// Cancels the memory space assigned for the device information by 
		// CreateDevice(). The device should be disconnected from the application
		// before this function is called.
		m_ctrlXnsDevice.ReleaseDevice(m_hDevice);

		m_hDevice = 0;
	}

	PostMessage(WM_CLOSE,0,0);
	PostQuitMessage(0);
}

//////////////////////////////////////////////////////////////////////////
void CPtzDlg::OnDestroy()
{
	if( m_hDevice )
	{
		// [ XNS ACTIVEX HELP ]
		// -----------------------------------------------------------------------
		// Stops receiving the stream data from the media source handle.  
		// -----------------------------------------------------------------------
		if( m_hMediaSource )
		{

			// Terminates transferring media stream data from the device. 
			// The media stream data will be separated by hMediaSource 
			// (i.e., phMediaSource of OpenMedia()).
			m_ctrlXnsDevice.CloseMedia( m_hDevice, m_hMediaSource );
		}

		// [ XNS ACTIVEX HELP ]
		// -----------------------------------------------------------------------
		// Returns the connection status of the device.
		// -----------------------------------------------------------------------
		if( m_ctrlXnsDevice.GetDeviceStatus(m_hDevice) == XDEVICE_STATUS_CONNECTED )
		{
			m_ctrlXnsDevice.Disconnect(m_hDevice);
		}

		// Cancels the memory space assigned for the device information by 
		// CreateDevice(). The device should be disconnected from the application
		// before this function is called.
		m_ctrlXnsDevice.ReleaseDevice(m_hDevice);
	}

	CDialog::OnDestroy();

}

int CPtzDlg::moveCamera(const int nCommand)
{
	if( !m_hDevice || !m_nControlId ){
		return 1;
	}

	long ret;
	//UpdateData(FALSE);

	///WLOGD(_T("MoveCamera, command=[%d], speed=%d\n"), nCommand, m_nPtzSpeed);

// 	if( nCommand != XPTZ_STOP)
// 		m_ctrlXnsDevice.ControlPtz(m_hDevice, m_nControlId, XPTZ_STOP, 30);

	/////
	//MessageBoxA(0, "!!!moveCamera!!!", "Info", 0);
	/////

	// Returns the capabilities of the control module.
	long bIsCap = m_ctrlXnsDevice.GetControlCapability(m_hDevice, m_nControlId, XCTL_CAP_PTZ_SPEED);

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Moves the camera to the predefined PTZ position. This function is 
	// valid as long as the application is receiving media stream from the 
	// camera. 
	// -----------------------------------------------------------------------
	//	Command			Value	Description
	// -----------------------------------------------------------------------
	//	XPTZ_UP			1		Tilt up
	//	XPTZ_DOWN		2		Tilt down
	//	XPTZ_LEFT		3		Pan left
	//	XPTZ_RIGHT		4		Pan right
	//	XPTZ_UPLEFT		5		Tilt up and pan left
	//	XPTZ_UPRIGHT	6		Tilt up and pan right
	//	XPTZ_DOWNLEFT	7		Tilt down and pan left
	//	XPTZ_DOWNRIGHT	8		Tilt down and pan right
	//	XPTZ_ZOOMIN		9		Zoom in
	//	XPTZ_ZOOMOUT	10		Zoom out
	//	XPTZ_STOP		11		Stop the PTZ moving
	//	XPTZ_FOCUS_NEAR	12		Focus near
	//	XPTZ_FOCUS_FAR	13		Focus far
	//	XPTZ_FOCUS_STOP	14		Stop focus moving
	//	XPTZ_IRIS_OPEN	15		Open iris
	//	XPTZ_IRIS_CLOSE	16		Close iris
	// -----------------------------------------------------------------------
	if (bIsCap)
	{
		ret = m_ctrlXnsDevice.ControlPtz(
			m_hDevice,		// [in] Device handle. This value is returned from CreateDevice().
			m_nControlId,	// [in] Control ID.
			nCommand,		// [in] PTZ command 
			m_nPtzSpeed		// [in] If nPtzCommand is a speed-related command, 
							// this value indicates the PTZ operation speed. 
							// (1~100) This value is valid only if the camera 
							// supports the XCTL_CAP_PTZ_SPEED capability.
			);
	}else{
		ret = ERR_UNKNOWN;
	}
//		ret = m_ctrlXnsDevice.ControlPtz(m_hDevice, m_nControlId, nCommand, 30);

	if(ret != ERR_SUCCESS)
	{
		WLOGD(_T("ControlPtz() fail:: error=[%d](%s)\n"), ret, m_ctrlXnsDevice.GetErrorString(ret));
		return 1;
	}
	
	return 0;
}


// Read mouse event for PTZ functions
BOOL CPtzDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	CRect rect;
	if(pMsg->message == WM_LBUTTONDOWN) 
	{
		CButton *pButton = (CButton *)GetDlgItem(IDC_BUTTON_ZOOM_IN);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_ZOOMIN);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_ZOOM_OUT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_ZOOMOUT);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_UP);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_UP);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_UPLEFT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_UPLEFT);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_UPRIGHT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_UPRIGHT);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_DOWN);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_DOWN);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_DOWNLEFT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_DOWNLEFT);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_DOWNRIGHT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_DOWNRIGHT);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_LEFT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_LEFT);
		}

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_RIGHT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
		{
			moveCamera(XPTZ_RIGHT);
		}
	}
	else if(pMsg->message == WM_LBUTTONUP) 
	{
		bool is_stop = false;

		CButton *pButton = (CButton *)GetDlgItem(IDC_BUTTON_ZOOM_IN);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_ZOOM_OUT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_UP);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_UPLEFT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_UPRIGHT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_DOWN);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_DOWNLEFT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_DOWNRIGHT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_LEFT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		pButton = (CButton *)GetDlgItem(IDC_BUTTON_RIGHT);
		pButton->GetWindowRect(rect); 
		if(rect.PtInRect(pMsg->pt))
			is_stop = true;

		if(is_stop)
		{
			moveCamera(XPTZ_STOP);
		}
	}


	return CDialog::PreTranslateMessage(pMsg);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int CPtzDlg::Move_Left(void)
{
	return moveCamera(XPTZ_LEFT);
}
int CPtzDlg::Move_Right(void)
{
	return moveCamera(XPTZ_RIGHT);
}
int CPtzDlg::Move_Up(void)
{
	return moveCamera(XPTZ_UP);
}
int CPtzDlg::Move_Down(void)
{
	return moveCamera(XPTZ_DOWN);
}
int CPtzDlg::Move_LeftUp(void)
{
	return moveCamera(XPTZ_UPLEFT);
}
int CPtzDlg::Move_LeftDown(void)
{
	return moveCamera(XPTZ_DOWNLEFT);
}
int CPtzDlg::Move_RightUp(void)
{
	return moveCamera(XPTZ_UPRIGHT);
}
int CPtzDlg::Move_RightDown(void)
{
	return moveCamera(XPTZ_DOWNRIGHT);
}

int CPtzDlg::Stop_Rotate(void)
{
	return moveCamera(XPTZ_STOP);
}

void CPtzDlg::Move_Speed(unsigned int spd_in) // [0...100]
{
	m_nPtzSpeed = spd_in;
}

int CPtzDlg::ZoomIn(void)
{
	return moveCamera(XPTZ_ZOOMIN);
}

int CPtzDlg::ZoomOut(void)
{
	return moveCamera(XPTZ_ZOOMOUT);
}

int CPtzDlg::Move_FocusNear(void)
{
	return moveCamera(XPTZ_FOCUS_NEAR);
}

int CPtzDlg::Move_FocusFar(void)
{
	return moveCamera(XPTZ_FOCUS_FAR);
}

int CPtzDlg::Stop_Focus_Moving(void)
{
	return moveCamera(XPTZ_FOCUS_STOP);
}

/////////////
void CPtzDlg::CallBack_GetPtzValues(void* ptr_GetPtzValues)
{
		pCB_GetPtzValues = static_cast<PTZ_GetValue>(ptr_GetPtzValues);
}

int CPtzDlg::GetPtzValues(void)
{
	if( !m_hDevice || !m_nControlId ){
		return 1;
	}

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Returns an absolute coordinate of the camera, is available when 
	// application receives media stream from the camera. 
	// The value themselves could be read by using OnGetPtzPos event handler. 
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.GetPtzPos(m_hDevice, m_nControlId);
	if(nRet != ERR_SUCCESS) 
	{
		WLOGD(_T("GetPtzPos() fail:: nRet = [%d](%s)\n"), nRet, m_ctrlXnsDevice.GetErrorString(nRet));
		return 1;
	}
	return 0;
}

int CPtzDlg::SetPtzValues(int pan_vl, int tilt_vl, int zoom_vl)
{
	if( !m_hDevice || !m_nControlId ){
		return 1;
	}

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Change the absolute coordinate of the camera, is available when application receives media stream from the camera.
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.SetPtzPos(	m_hDevice, 
																					m_nControlId, 
																					pan_vl, 
																					tilt_vl, 
																					zoom_vl
																				);
	if(nRet != ERR_SUCCESS) 
	{
		WLOGD(_T("SetPtzPos() fail:: nRet = [%d](%s)\n"), nRet, m_ctrlXnsDevice.GetErrorString(nRet));
		return 1;
	}
	return 0;
}
/////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPtzDlg::OnBnClickedCheckAreazoom()
{
	// TODO: Add your control notification handler code here
	m_bAreaZoom = !m_bAreaZoom;
	//WLOGD(_T("Area Zoom setting=%d\n"), m_bAreaZoom);
}

// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// This event occurs if a left button on the mouse is pressed in 
// XnSdkWindow. This event is valid only if both XnsSdkWindow 
// handle and parent window handle are reset to Null.  
//
// Parameters
// - nFlags
// [in] Same as the flag used in window message by WM_MOUSEMOVE
// * MK_CONTROL: [Ctrl] key is pressed.
// * MK_MBUTTON: Middle button on the mouse is pressed.
// * MK_RBUTTON: Right button on the mouse is pressed.
// - nX
// [in] X coordinate of the mouse pointer.
// - nY
// [in] Y coordinate of the mouse pointer.
// -----------------------------------------------------------------------
void CPtzDlg::OnLButtonDownXnssdkwindowctrl(long nFlags, long nX, long nY)
{
	m_bIsMouseDown = true;
	// TODO: Add your message handler code here
	if (!m_bAreaZoom) 
		return;

	m_nStartX = nX;
	m_nStartY = nY;

}

// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// This event occurs if the left mouse button is pressed and released 
// in XnSdkWindow. This event is valid only if both XnsSdkWindow 
// handle and parent window handle are reset to Null.
// -----------------------------------------------------------------------
void CPtzDlg::OnLButtonUpXnssdkwindowctrl(long nFlags, long nX, long nY)
{
	if( !m_hDevice || !m_nControlId ){
		return;
	}

	m_bIsMouseDown = false;
	// TODO: Add your message handler code here
	if (!m_bAreaZoom) 
		return;

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Returns the width of the source image. 
	// Returns the height of the source image.
	// -----------------------------------------------------------------------
	long nImageWidth =  600;
	long nImageHeight = 600;

	CRect rt; 
	long nWindowWidth = rt.Width();
	long nWindowHeight = rt.Height();

	m_nEndX = nX;
	m_nEndY = nY;

	WLOGW(_T("On LButtonUp XNSWindow flag=%d, x=%d, y=%d, imagew=%d, imgaeh=%d, windoww=%d, windowh=%d\n"), 
		nFlags, nX, nY, nImageWidth, nImageHeight, nWindowWidth, nWindowHeight);

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Set up AreaZoom feature, which is available when application 
	// receives media stream from the camera. 
	// Moves the camera to the Area zoom position. 
	// This function is valid as long as the application is receiving 
	// media stream from the camera. 
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.AreaZoom(
		m_hDevice,			// Device handle. This value is returned from CreateDevice().
		m_nControlId,		// Control ID.
		m_nStartX,			// X-axis start position
		m_nStartY,			// Y-axis start position
		m_nEndX,			// X-axis end position
		m_nEndY,			// Y-axis end position
		nWindowWidth,		// Window width
		nWindowHeight,		// Window height
		nImageWidth,		// Image width
		nImageHeight		// Image height
		);

	if(nRet != ERR_SUCCESS) 
	{
		WLOGD(_T("AreaZoom() fail:: nRet = [%d](%s)\n"), nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}

}

// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// This event occurs if the mouse pointer is moving. This event is valid 
// only if both XnsSdkWindow handle and parent window handle are 
// reset to Null.
// -----------------------------------------------------------------------
void CPtzDlg::OnMouseMoveXnssdkwindowctrl(long nFlags, long nX, long nY)
{
	// TODO: Add your message handler code here
	if (!m_bAreaZoom) 
		return;

	if (!m_bIsMouseDown)
		return;
}

void CPtzDlg::OnBnClickedButtonZoom1x()
{
	// TODO: Add your control notification handler code here

	if( !m_hDevice || !m_nControlId ){
		return;
	}

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Performs the zoom-1x. This function is valid as long as the application 
	// is receiving media stream from the camera.
	// Parameters
	// - hDevice
	//   [in] Device handle. This value is returned from CreateDevice().
	// - nControlID
	//	 [in] Control ID.
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.Zoom1X(m_hDevice, m_nControlId);
	if(nRet != ERR_SUCCESS) 
	{
		WLOGD(_T("AreaZoom() fail:: nRet = [%d](%s)\n"), nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonGetPtz()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Returns an absolute coordinate of the camera, is available when 
	// application receives media stream from the camera. 
	// The value themselves could be read by using OnGetPtzPos event handler. 
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.GetPtzPos(m_hDevice, m_nControlId);
	if(nRet != ERR_SUCCESS) 
	{
		WLOGD(_T("GetPtzPos() fail:: nRet = [%d](%s)\n"), nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonSetPtz()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if( !m_hDevice || !m_nControlId ){
		return;
	}
	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Change the absolute coordinate of the camera, 
	// is available when application receives media stream from the camera.
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.SetPtzPos(	m_hDevice, 
																					m_nControlId, 
																					m_nPan, 
																					m_nTilt, 
																					m_nZoom
																				);
	if(nRet != ERR_SUCCESS) 
	{
		WLOGD(_T("SetPtzPos() fail:: nRet = [%d](%s)\n"), nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}


// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// This event occurs when GetPtzPos() is called. One of the parameter 
// of event handler is the handle of preset list, which used to get the 
// absolute coordinate of camera.
//
// Parameters
// - nDeviceID
//   [in] Device ID.
// - nControlID
//   [in] Control ID.
// - nErrorCode
//   [in] return value. 
// - nPan
//   [in] Absolute value of Pan. 
// - nTilt
//   [in] Absolute value of Tilt. 
// - nZoom
//   [in] Absolute value of Zoom. 
// -----------------------------------------------------------------------
void CPtzDlg::OnGetPtzPosXnssdkdevicectrl(	long nDeviceID, 
																						long nControlID, 
																						long nErrorCode, 
																						long nPan, 
																						long nTilt, 
																						long nZoom
																					)
{
	// TODO: Add your message handler code here
	//WLOGD(_T("Get Ptz Position device_id=%d, pan=%d, tilt=%d, zoom=%d, ret=%d\n"), nDeviceID, nPan, nTilt, nZoom, nErrorCode);

	m_nPan = nPan;
	m_nTilt = nTilt; 
	m_nZoom = nZoom;

	//////////////////////////////////////////////////////////////////////////
	// send PTZ values using callback function
	if (pCB_GetPtzValues){
		pCB_GetPtzValues(nPan, nTilt, nZoom);
	}
	//////////////////////////////////////////////////////////////////////////
	/*
	CString strTmp;
	strTmp.Format(_T("%d"), nPan);
	GetDlgItem(IDC_EDIT_PAN)->SetWindowText(strTmp);
	strTmp.Format(_T("%d"), nTilt);
	GetDlgItem(IDC_EDIT_TILT)->SetWindowText(strTmp);
	strTmp.Format(_T("%d"), nZoom);
	GetDlgItem(IDC_EDIT_ZOOM)->SetWindowText(strTmp);
	*/
}

void CPtzDlg::OnBnClickedCheckMenuOn()
{
	// TODO: Add your control notification handler code here
	m_bMenuControl = !m_bMenuControl;

	if( !m_hDevice || !m_nControlId ){
		return;
	}

	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Returns the capabilities of the control module.
	// -----------------------------------------------------------------------
	long bCap = m_ctrlXnsDevice.GetControlCapability(	m_hDevice,				// Device handle
																										m_nControlId,			// Control ID
																										XCTL_CAP_CAM_MENU		// Capability ID
																									);
	if (!bCap)
	{
		WLOGD(_T("No capability for the menu control\n"));
		return;
	}


	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Controls the OSD menus of the camera. This function is valid as long 
	// as the application is receiving media stream from the camera.(ex.SND-5080)
	// -----------------------------------------------------------------------
	//  Command			Value	Description
	// -----------------------------------------------------------------------
	//	XMENU_UP		1		Menu up.
	//	XMENU_DOWN		2		Menu down.
	//	XMENU_LEFT		3		Menu left or prev menu.
	//	XMENU_RIGHT		4		Menu right or next menu.
	//	XMENU_ENTER		5		Menu enter, select.
	//	XMENU_ON		6		Menu on.
	//	XMENU_OFF		7		Menu off.
	//	XMENU_CANCEL	8		Cancel (Changes are not saved).
	// -----------------------------------------------------------------------
	int nRet = m_ctrlXnsDevice.ControlMenu(	m_hDevice,
																					m_nControlId,
																					(m_bMenuControl?XMENU_ON:XMENU_OFF)
																				);
	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ControlMenu() fail: ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonMenuUp()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}

	int nRet = m_ctrlXnsDevice.ControlMenu(	m_hDevice,
																					m_nControlId,
																					XMENU_UP
																				);
	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ControlMenu() fail: ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonMenuDown()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}
	int nRet = m_ctrlXnsDevice.ControlMenu(	m_hDevice,
																					m_nControlId,
																					XMENU_DOWN
																				);
	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ControlMenu() fail: ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonMenuLeft()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}
	int nRet = m_ctrlXnsDevice.ControlMenu(	m_hDevice,
																					m_nControlId,
																					XMENU_LEFT
																				);
	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ControlMenu() fail: ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonMenuRight()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}
	int nRet = m_ctrlXnsDevice.ControlMenu(	m_hDevice,
																					m_nControlId,
																					XMENU_RIGHT
																				);
	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ControlMenu() fail: ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonMenuEnter()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}
	int nRet = m_ctrlXnsDevice.ControlMenu(	m_hDevice,
																					m_nControlId,
																					XMENU_ENTER
																				);
	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ControlMenu() fail: ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}

void CPtzDlg::OnBnClickedButtonMenuCancel()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}

	int nRet = m_ctrlXnsDevice.ControlMenu(	m_hDevice,
																					m_nControlId,
																					XMENU_CANCEL
																				);
	if (nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ControlMenu() fail: ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
	}
}


// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// Open a dialog for preset functions
// -----------------------------------------------------------------------
void CPtzDlg::OnBnClickedButtonPreset()
{
	if( !m_hDevice || !m_nControlId ){
		return;
	}
	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Gets the preset list. A preset is a user-defined position of the camera. 
	// The number of presets available varies depending on the model. 
	// However, the XNS ActiveX library can support up to 20 presets. 
	// The result of this function will be forwarded by the OnGetPresetList 
	// event. This function is valid as long as the application is receiving 
	// media stream from the camera.
	// -----------------------------------------------------------------------
	int nRet = m_ctrlXnsDevice.GetPresetList(m_hDevice, m_nControlId);
	//int nRet = m_ctrlXnsDevice.GetHPtzList(m_hDevice, m_nControlId, XHPTZ_SCAN);
	if(nRet != ERR_SUCCESS)
	{
		WLOGD(_T("GetPresetList() fail. ret=[%d](%s)\n"), 
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
		return;
	}

}

// [ XNS ACTIVEX HELP ]
// -----------------------------------------------------------------------
// This event occurs if GetPresetList() is called, the handle of the 
// preset list will be forwarded to the parameter of event handler. 
// This handle is input as a parameter of GetPresetCount(), and is used
// to get information about the preset.
// 
// Parameters
// - nDeviceID
//   [in] Device ID.
// - nControlID
//   [in] Control ID.
// - hPresetListList
//   [in] Handle to indicate the preset list.
// -----------------------------------------------------------------------
void CPtzDlg::OnGetPresetListXnssdkdevicectrl(long nDeviceID, long nControlID, long hPresetListList)
{
	// TODO: Add your message handler code here
	long nNumber;
	long nPresetCount;
	CString strPresetName;
	CMap< int, int, CString, CString > cmPresetList;

	// Gets the number of presets defined. 
	// This value is used to get the preset name and information. 
	// This function is valid as long as the application is receiving 
	// media stream from the camera.
	nPresetCount = m_ctrlXnsDevice.GetPresetCount(hPresetListList);
	//WLOGD(_T("Event OnGetPresetList.. nDevice[%d], nControlId[%d], PresetCount[%d]\n"),	nDeviceID, nControlID, nPresetCount);

	for( int i=0 ; i<nPresetCount ; i++ )
	{
		// Returns the preset number and name corresponding to the 
		// given index in the preset list of XnsSdkDevice. 
		strPresetName = m_ctrlXnsDevice.GetPreset(
			hPresetListList,	// [in] Handle of the preset list. 
			i,					// [in] Preset index to get the preset information (start with 1).
			&nNumber			// [out] Preset index specified in XnsSdkDevice.
			);
		//WLOGD(_T("PresetList[%02d] name=[%s]\n"), nNumber, strPresetName);
		cmPresetList.SetAt(nNumber, strPresetName);
	}

}

void CPtzDlg::OnCbnSelchangeComboAutoscan()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}

	int nIndex = m_ctrlAutoScan.GetCurSel();
	if (nIndex == 0)
	{
		// stop action
		moveCamera(XPTZ_UP);
		return;
	}


	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// Performs operations of Preset, Autopan, Scan and Pattern. 
	// This function is valid as long as the application is receiving 
	// media stream from the camera.
	// 
	// Parameters
	// - hDevice
	//	 [in] Device handle. This value is returned from CreateDevice().
	//	nControlID
	//	 [in] Control ID.
	// - nType
	//	 [in] Camera operations.
	//	 * XHPTZ_PRESET(1): Preset
	//	 * XHPTZ_AUTOPAN(2): Autopan(Swing)
	//	 * XHPTZ_SCAN(3): Scan
	//	 * XHPTZ_PATTERN(4): Pattern
	// - nNumber
	//	 [in] Index of camera's list of features (Preset, AutoPan, Scan, Pattern). 
	//        (start with 1) For instance, if multiple presets are defined, 
	//        a list of presets will be created and the user should specify 
	//        the index of the list.
	// -----------------------------------------------------------------------
	long nRet = m_ctrlXnsDevice.ExecuteHPtz(	m_hDevice,
																						m_nControlId,
																						XHPTZ_TOUR,
																						nIndex
																					);
	if(nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ExecuteHPtz() failed: ret=[%d](%s)\n"),
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
		return;
	}
	//WLOGD(_T("Scanning index=[%d]\n"), nIndex);
}


void CPtzDlg::OnCbnSelchangeComboAutopan()
{
	// TODO: Add your control notification handler code here
	if( !m_hDevice || !m_nControlId ){
		return;
	}

	int nIndex = m_ctrlAutoPan.GetCurSel();
	if (nIndex == 0)
	{
		// stop action
		moveCamera(XPTZ_UP);
		return;
	}

	long nRet = m_ctrlXnsDevice.ExecuteHPtz(	m_hDevice,
																						m_nControlId,
																						XHPTZ_AUTOPAN,
																						nIndex
																					);
	if(nRet != ERR_SUCCESS)
	{
		WLOGD(_T("ExecuteHPtz() failed: ret=[%d](%s)\n"),
			nRet, m_ctrlXnsDevice.GetErrorString(nRet));
		return;
	}
	//WLOGD(_T("Auto panning index=[%d]\n"), nIndex);
}


void CPtzDlg::OnGetHPtzListXnssdkdevicectrl(long nDeviceID, long nControlID, long nType, long hHptzList)
{
	// TODO: Add your message handler code here
	long nNumber;
	long nPresetCount;
	CString strHptzName;
	CMap< int, int, CString, CString > cmPresetList;

	// Gets the number of presets defined. 
	// This value is used to get the preset name and information. 
	// This function is valid as long as the application is receiving 
	// media stream from the camera.
	nPresetCount = m_ctrlXnsDevice.GetPresetCount(hHptzList);
	//WLOGD(_T("Event OnGetHPtzList.. nDevice[%d], nControlId[%d], nType[%d], PresetCount[%d]\n"), nDeviceID, nControlID, nType, nPresetCount);

	for( int i=0 ; i<nPresetCount ; i++ )
	{
		// Returns the preset number and name corresponding to the 
		// given index in the preset list of XnsSdkDevice. 
		strHptzName = m_ctrlXnsDevice.GetPreset(
			hHptzList,	// [in] Handle of the preset list. 
			i,					// [in] Preset index to get the preset information (start with 1).
			&nNumber			// [out] Preset index specified in XnsSdkDevice.
			);
		//WLOGD(_T("HPtzList[%02d] name=[%s]\n"), nNumber, strHptzName);
	}
}


//void CPtzDlg::OnNcDestroy()
//{
//	//CDialog::PostNcDestroy();
//
//
//	CDialog::OnNcDestroy();
//
//	// TODO: Add your message handler code here
//}

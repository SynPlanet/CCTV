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
// KOREA
// http://www.samsungtechwin.co.kr
///////////////////////////////////////////////////////////////////////////////////
// PtzDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "CDXnsSdkDevice.h"
#include "resource.h"

#include "../CaptureIP/Misc.h"

typedef enum
{
	SL_STATUS_CONNECTED = 0,
	SL_STATUS_DISCONNECTED,
}SLIVE_BUTTON_STATUS;

// CPtzDlg dialog
class CPtzDlg : public CDialog
{
// Construction
public:
	CPtzDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PTZ_DIALOG };

	int Move_Left(void);
	int Move_Right(void);
	int Move_Up(void);
	int Move_Down(void);

	int Move_LeftUp(void);
	int Move_RightUp(void);
	int Move_RightDown(void);
	int Move_LeftDown(void);
	void Move_Speed(unsigned int spd_in); // [0...100]
	int Stop_Rotate(void);

	int ZoomIn(void);
	int ZoomOut(void);
	int Move_FocusNear(void);
	int Move_FocusFar(void);
	int Stop_Focus_Moving(void);

	// see function "OnBnClickedButtonConnect"
	int ConnectToDevice( T_NetAddress*	p_net_addr_in );
	int DisconnectDevice( T_NetAddress*	p_net_addr_in );

	int GetPtzValues(void);
	int SetPtzValues(int pan_vl, int tilt_vl, int zoom_vl);

	void CallBack_GetPtzValues(void* ptr_GetPtzValues);

	void ReleaseDevices_Manually(void);

	//Destroying the modeless dialog
	//void PostNcDestroy(void);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void Init_DVC(void);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	// [ XNS ACTIVEX HELP ]
	// -----------------------------------------------------------------------
	// XNS Device control and Window control variables
	// -----------------------------------------------------------------------
	CDXnsSdkDevice		m_ctrlXnsDevice;	// XnsDevice control
	long				m_hDevice;			// Device handle
	long				m_hMediaSource;		// Media stream ID
	long				m_nControlId;		// Control ID

	CComboBox			m_ctrlModelList;
	CString				m_strModelName;
	CString				m_strAnyCmrModelName;

	CIPAddressCtrl		m_ctrlIpAddress;
	int					m_nPort;
	CString				m_strId;
	CString				m_strPasswd;
	bool				m_bAreaZoom;
	int					m_nStartX;
	int					m_nStartY;
	int					m_nEndX;
	int					m_nEndY;

	int					m_nPtzSpeed;
	int					m_nPan;
	int					m_nTilt;
	int					m_nZoom;
	bool				m_bMenuControl;
	CComboBox			m_ctrlAutoScan;
	CComboBox			m_ctrlAutoPan;

	bool				m_bIsMouseDown;


	void setBtnStatus(SLIVE_BUTTON_STATUS nStatus);
	int moveCamera(const int nCommand);
public:
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	DECLARE_EVENTSINK_MAP()
	void OnDeviceStatusChangedXnssdkdevicectrl(long nDeviceID, long nErrorCode, long nDeviceStatus, long nHddCondition);
	void OnConnectFailedXnssdkdevicectrl(long nDeviceID, long nErrorCode);
	CEdit m_ctrlEditLog;
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedCheckAreazoom();
	void OnLButtonDownXnssdkwindowctrl(long nFlags, long nX, long nY);
	void OnLButtonUpXnssdkwindowctrl(long nFlags, long nX, long nY);
	void OnMouseMoveXnssdkwindowctrl(long nFlags, long nX, long nY);
	afx_msg void OnBnClickedButtonZoom1x();
	afx_msg void OnBnClickedButtonGetPtz();
	afx_msg void OnBnClickedButtonSetPtz();

	void OnGetPtzPosXnssdkdevicectrl(long nDeviceID, long nControlID, long nErrorCode, long nPan, long nTilt, long nZoom);
	afx_msg void OnBnClickedCheckMenuOn();
	afx_msg void OnBnClickedButtonMenuUp();
	afx_msg void OnBnClickedButtonMenuDown();
	afx_msg void OnBnClickedButtonMenuLeft();
	afx_msg void OnBnClickedButtonMenuRight();
	afx_msg void OnBnClickedButtonMenuEnter();
	afx_msg void OnBnClickedButtonMenuCancel();
	afx_msg void OnBnClickedButtonPreset();
	void OnGetPresetListXnssdkdevicectrl(long nDeviceID, long nControlID, long hPresetListList);

	afx_msg void OnCbnSelchangeComboAutopan();
	afx_msg void OnCbnSelchangeComboAutoscan();
	void OnGetHPtzListXnssdkdevicectrl(long nDeviceID, long nControlID, long nType, long hHptzList);
//	afx_msg void OnNcDestroy();
};

// CDXnsSdkDevice.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDXnsSdkDevice

class CDXnsSdkDevice : public CWnd
{
protected:
	DECLARE_DYNCREATE(CDXnsSdkDevice)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x9BED9251, 0xE8E7, 0x4B67, { 0xB2, 0x81, 0xAD, 0xC0, 0x6B, 0xA7, 0x98, 0x8D } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID, 
						CCreateContext* pContext = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); 
	}

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, 
				UINT nID, CFile* pPersist = NULL, BOOL bStorage = FALSE,
				BSTR bstrLicKey = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); 
	}

// Attributes
public:

// Operations
public:

	long Initialize()
	{
		long result;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	long FindVendor()
	{
		long result;
		InvokeHelper(0x2, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	long FindModel(LPCTSTR szVendorName)
	{
		long result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x3, DISPATCH_METHOD, VT_I4, (void*)&result, parms, szVendorName);
		return result;
	}
	long FindDevice()
	{
		long result;
		InvokeHelper(0x4, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	long GetFindSize(long hFind)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x5, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hFind);
		return result;
	}
	CString GetFindString(long hFind, long nIndex)
	{
		CString result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x6, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hFind, nIndex);
		return result;
	}
	long GetFindLong(long hFind, long nIndex)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x7, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hFind, nIndex);
		return result;
	}
	double GetFindDouble(long hFind, long nIndex)
	{
		double result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x8, DISPATCH_METHOD, VT_R8, (void*)&result, parms, hFind, nIndex);
		return result;
	}
	void CloseFind(long hFind)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x9, DISPATCH_METHOD, VT_EMPTY, NULL, parms, hFind);
	}
	long AutoScan(long nScanDuration)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0xa, DISPATCH_METHOD, VT_I4, (void*)&result, parms, nScanDuration);
		return result;
	}
	long IPInstall(LPCTSTR szVendorName, LPCTSTR szModelName, LPCTSTR szIP, LPCTSTR szSubnetMask, LPCTSTR szGateway, LPCTSTR szMac, long nPort, long nHttpPort, LPCTSTR szPassword)
	{
		long result;
		static BYTE parms[] = VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_BSTR ;
		InvokeHelper(0xb, DISPATCH_METHOD, VT_I4, (void*)&result, parms, szVendorName, szModelName, szIP, szSubnetMask, szGateway, szMac, nPort, nHttpPort, szPassword);
		return result;
	}
	long SetEventDiscardTime(long nEventType, long nDiscardTime)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0xc, DISPATCH_METHOD, VT_I4, (void*)&result, parms, nEventType, nDiscardTime);
		return result;
	}
	long CreateDevice(long nDeviceID)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0xd, DISPATCH_METHOD, VT_I4, (void*)&result, parms, nDeviceID);
		return result;
	}
	void ReleaseDevice(long hDevice)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0xe, DISPATCH_METHOD, VT_EMPTY, NULL, parms, hDevice);
	}
	long GetDeviceID(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0xf, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long GetDeviceStatus(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x10, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetVendorName(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x11, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetModelName(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x12, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetVersion(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x13, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	long GetAddressType(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x14, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetIP(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x15, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetMac(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x16, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetURL(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x17, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	long GetPort(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x18, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long GetHttpPort(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x19, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetUserID(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x1a, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetPassword(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x1b, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	void SetConnectionInfo(long hDevice, LPCTSTR szVendorName, LPCTSTR szModelName, long nAddressType, LPCTSTR szAddress, long nPort, long nHttpPort, LPCTSTR szUserID, LPCTSTR szPassword)
	{
		static BYTE parms[] = VTS_I4 VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR VTS_I4 VTS_I4 VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x1c, DISPATCH_METHOD, VT_EMPTY, NULL, parms, hDevice, szVendorName, szModelName, nAddressType, szAddress, nPort, nHttpPort, szUserID, szPassword);
	}
	long GetLatestUpdateTime(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x1d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long GetControlCount(long hDevice, long nType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x1e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nType);
		return result;
	}
	long GetControlType(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x1f, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long GetControlNumber(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x20, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long GetControlDepth(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x21, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	CString GetControlLocalName(long hDevice, long nControlID)
	{
		CString result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x22, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long GetControlStatus(long hDevice, long nControlID, long nStatusID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x23, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nStatusID);
		return result;
	}
	long GetControlCapability(long hDevice, long nControlID, long nCapabilityID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x24, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nCapabilityID);
		return result;
	}
	void ChangeControlLocalName(long hDevice, long nControlID, LPCTSTR szName)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_BSTR ;
		InvokeHelper(0x25, DISPATCH_METHOD, VT_EMPTY, NULL, parms, hDevice, nControlID, szName);
	}
	long LoadDeviceProfile(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x26, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long SaveDeviceProfile(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x27, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long DeleteDeviceProfile(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x28, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	CString ReadConfigValue(LPCTSTR szKey)
	{
		CString result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x29, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, szKey);
		return result;
	}
	long WriteConfigValue(LPCTSTR szKey, LPCTSTR szValue)
	{
		long result;
		static BYTE parms[] = VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x2a, DISPATCH_METHOD, VT_I4, (void*)&result, parms, szKey, szValue);
		return result;
	}
	long DateToTimet(long nYear, long nMonth, long nDay, long nHour, long nMinute, long nSecond)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x2b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, nYear, nMonth, nDay, nHour, nMinute, nSecond);
		return result;
	}
	void TimetToDate(long tTime, long * pnYear, long * pnMonth, long * pnDay, long * pnHour, long * pnMinute, long * pnSecond)
	{
		static BYTE parms[] = VTS_I4 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4 ;
		InvokeHelper(0x2c, DISPATCH_METHOD, VT_EMPTY, NULL, parms, tTime, pnYear, pnMonth, pnDay, pnHour, pnMinute, pnSecond);
	}
	void DeleteConfigValue(LPCTSTR szKey)
	{
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x2d, DISPATCH_METHOD, VT_EMPTY, NULL, parms, szKey);
	}
	long FindConfigChildeKeyList(LPCTSTR szKey)
	{
		long result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x2e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, szKey);
		return result;
	}
	long Connect(long hDevice, long bForced, long bReconnect)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x2f, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, bForced, bReconnect);
		return result;
	}
	long Disconnect(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x30, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long AcquireMediaAuthority(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x31, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long ReleaseMediaAuthority(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x32, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long OpenMedia(long hDevice, long nControlID, long nMediaType, long tStart, long tEnd, long * phMediaSource)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PI4 ;
		InvokeHelper(0x33, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType, tStart, tEnd, phMediaSource);
		return result;
	}
	long CloseMedia(long hDevice, long hMediaSource)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x34, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, hMediaSource);
		return result;
	}
	long Play(long hDevice, long nControlID, double dSpeed)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_R8 ;
		InvokeHelper(0x35, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, dSpeed);
		return result;
	}
	long Pause(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x36, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long Seek(long hDevice, long nControlID, long tPos)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x37, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, tPos);
		return result;
	}
	long FrameAdvance(long hDevice, long nControlID, long nDelta)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x38, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nDelta);
		return result;
	}
	long SearchCalendar(long hDevice, long nControlID, long tStart)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x39, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, tStart);
		return result;
	}
	long SearchDay(long hDevice, long nControlID, long tStart, long tEnd, long nRecType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x3a, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, tStart, tEnd, nRecType);
		return result;
	}
	long SetTalk(long hDevice, long nControlID, long bState)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x3b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, bState);
		return result;
	}
	long SetListen(long hDevice, long nControlID, long nMediaType, long bState)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x3c, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType, bState);
		return result;
	}
	long SendAudioData(long hDevice, long hData, long nDataSize, long nTimestamp)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x3d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, hData, nDataSize, nTimestamp);
		return result;
	}
	long SetAlarm(long hDevice, long nControlID, long bState)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x3e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, bState);
		return result;
	}
	long ControlPtz(long hDevice, long nControlID, long nPtzCommand, long nSpeed)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x3f, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nPtzCommand, nSpeed);
		return result;
	}
	long ControlPtzFreeMove(long hDevice, long nControlID, long nX, long nY)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x40, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nX, nY);
		return result;
	}
	long ControlPtzSpeedZoomIn(long hDevice, long nControlID, long nSpeed)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x41, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nSpeed);
		return result;
	}
	long ControlPtzSpeedZoomOut(long hDevice, long nControlID, long nSpeed)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x42, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nSpeed);
		return result;
	}
	long ControlPPtz(long hDevice, long nControlID, long nX, long nY, long nZ)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x43, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nX, nY, nZ);
		return result;
	}
	long ControlMenu(long hDevice, long nControlID, long nMenuCommand)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x44, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMenuCommand);
		return result;
	}
	long ExecuteHPtz(long hDevice, long nControlID, long nType, long nNumber)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x45, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nType, nNumber);
		return result;
	}
	long GetPresetList(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x46, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long SetPreset(long hDevice, long nControlID, long nNumber, LPCTSTR szName)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_BSTR ;
		InvokeHelper(0x47, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nNumber, szName);
		return result;
	}
	long DeletePreset(long hDevice, long nControlID, long nNumber)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x48, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nNumber);
		return result;
	}
	long DeletePresetAll(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x49, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long UploadSoftware(long hDevice, LPCTSTR szFile)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_BSTR ;
		InvokeHelper(0x4a, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, szFile);
		return result;
	}
	long ImportConfig(long hDevice, LPCTSTR szFile, long bWithNetSetting)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_BSTR VTS_I4 ;
		InvokeHelper(0x4b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, szFile, bWithNetSetting);
		return result;
	}
	long ExportConfig(long hDevice, LPCTSTR szFile)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_BSTR ;
		InvokeHelper(0x4c, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, szFile);
		return result;
	}
	long ConnectNonBlock(long hDevice, long bForced, long bReconnect)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x4d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, bForced, bReconnect);
		return result;
	}
	long GetCameraCount(long hTimeline)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x4e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hTimeline);
		return result;
	}
	long GetTimelineCount(long hTimeline, long nCameraIndex)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x4f, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hTimeline, nCameraIndex);
		return result;
	}
	long GetPresetCount(long hPresetList)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x50, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hPresetList);
		return result;
	}
	CString GetPreset(long hPresetList, long nPresetIndex, long * pnNumber)
	{
		CString result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_PI4 ;
		InvokeHelper(0x51, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hPresetList, nPresetIndex, pnNumber);
		return result;
	}
	long GetParentControlID(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x52, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long CopyXml(LPCTSTR szFile1, LPCTSTR szKey1, LPCTSTR szFile2, LPCTSTR szKey2)
	{
		long result;
		static BYTE parms[] = VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x53, DISPATCH_METHOD, VT_I4, (void*)&result, parms, szFile1, szKey1, szFile2, szKey2);
		return result;
	}
	long StartLocalRecording(long hDevice, long nControlID, LPCTSTR szFile, long nFileType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_BSTR VTS_I4 ;
		InvokeHelper(0x54, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, szFile, nFileType);
		return result;
	}
	long StopLocalRecording(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x55, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long StartBackup(long hDevice, long nControlID, LPCTSTR szFile, long nFileType, long tStart, long tEnd)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_BSTR VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x56, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, szFile, nFileType, tStart, tEnd);
		return result;
	}
	long StopBackup(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x57, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long CreateFileReader(LPCTSTR szFile)
	{
		long result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x58, DISPATCH_METHOD, VT_I4, (void*)&result, parms, szFile);
		return result;
	}
	void ReleaseFileReader(long hReader)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x59, DISPATCH_METHOD, VT_EMPTY, NULL, parms, hReader);
	}
	CString GetFileName(long hReader)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x5a, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hReader);
		return result;
	}
	long GetMediaSource(long hReader)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x5b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hReader);
		return result;
	}
	long GetTimelineHandle(long hReader)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x5c, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hReader);
		return result;
	}
	long PlayReader(long hReader, double dSpeed)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_R8 ;
		InvokeHelper(0x5d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hReader, dSpeed);
		return result;
	}
	long PauseReader(long hReader)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x5e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hReader);
		return result;
	}
	long OpenStream(long hDevice, long nControlID, long nType, long tStart, long tEnd)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x5f, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nType, tStart, tEnd);
		return result;
	}
	long CloseStream(long hDevice, long nControlID, long nType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x60, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nType);
		return result;
	}
	long SuspendAllStream(long hDevice, long nMediaType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x61, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nMediaType);
		return result;
	}
	long ResumeAllStream(long hDevice, long nMediaType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x62, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nMediaType);
		return result;
	}
	CString GetConfigPath()
	{
		CString result;
		InvokeHelper(0x63, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	long ReopenAllStream(long hDevice, long nMediaType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x64, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nMediaType);
		return result;
	}
	void GetTimeline(long hTimeline, long nCameraIndex, long nTimelineIndex, long nType, long * ptStart, long * ptEnd, long * pnRecType, long * pnRecID)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4 ;
		InvokeHelper(0x65, DISPATCH_METHOD, VT_EMPTY, NULL, parms, hTimeline, nCameraIndex, nTimelineIndex, nType, ptStart, ptEnd, pnRecType, pnRecID);
	}
	long IsPlaying(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x66, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long HasAuthority(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x67, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long ToggleDvrRecording(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x68, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long ResetDvrEvent(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x69, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetDDNSID(long hDevice)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x6a, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice);
		return result;
	}
	long GetSetupPageStyle(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x6b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long SetEventFilter(long nEventType, long bFiltering)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x6c, DISPATCH_METHOD, VT_I4, (void*)&result, parms, nEventType, bFiltering);
		return result;
	}
	long SuspendStream(long hDevice, long nControlID, long nMediaType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x6d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType);
		return result;
	}
	long ResumeStream(long hDevice, long nControlID, long nMediaType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x6e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType);
		return result;
	}
	long Reconnect(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x6f, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	CString GetControlRemoteName(long hDevice, long nControlID)
	{
		CString result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x70, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long GetDeviceLocalTime(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x71, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long LoadDeviceConfig()
	{
		long result;
		InvokeHelper(0x72, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	void SaveDeviceConfig()
	{
		InvokeHelper(0x73, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void SaveConfig()
	{
		InvokeHelper(0x74, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void LoadConfig()
	{
		InvokeHelper(0x75, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	long GetDeviceUTC(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x76, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long LocalTimeToDeviceUTC(long hDevice, long tLocal)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x77, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, tLocal);
		return result;
	}
	long UTCToDeviceLocalTime(long hDevice, long tUTC)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x78, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, tUTC);
		return result;
	}
	long IsDuplicatedTimeInDST(long hDevice, long tLocal)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x79, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, tLocal);
		return result;
	}
	long IsEmptyTimeInDST(long hDevice, long tLocal)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x7a, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, tLocal);
		return result;
	}
	long GetMotionGrid(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x7b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long MotionSearch(long hDevice, long nControlID, long tStart, long tEnd, long nGridWidth, long nGridHeight, long pGridArray)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x7c, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, tStart, tEnd, nGridWidth, nGridHeight, pGridArray);
		return result;
	}
	long GetDeviceDSTStartUTC(long hDevice, long lYear)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x7d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, lYear);
		return result;
	}
	long GetDeviceDSTEndUTC(long hDevice, long lYear)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x7e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, lYear);
		return result;
	}
	CString GetErrorString(long nErrorCode)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x7f, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, nErrorCode);
		return result;
	}
	long GetDeviceHandle(long nDeviceID)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x80, DISPATCH_METHOD, VT_I4, (void*)&result, parms, nDeviceID);
		return result;
	}
	long FrameAdvanceReader(long hReader)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x81, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hReader);
		return result;
	}
	long ActivateEventCallback(long bActivate, long nShrPort, long nDvrPort)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x82, DISPATCH_METHOD, VT_I4, (void*)&result, parms, bActivate, nShrPort, nDvrPort);
		return result;
	}
	long SendSerialData(long hDevice, long nSendDataLength, long nRecvDataLength, long pSerialData, long pCmdID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x83, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nSendDataLength, nRecvDataLength, pSerialData, pCmdID);
		return result;
	}
	long OpenMediaEx(long hDevice, long nControlID, long nMediaType, long tStart, long tEnd)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x84, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType, tStart, tEnd);
		return result;
	}
	long AreaZoom(long hDevice, long nControlID, long nStartX, long nStartY, long nEndX, long nEndY, long nTileWidth, long nTileHeight, long nFrameWidth, long nFrameHeight)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x85, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nStartX, nStartY, nEndX, nEndY, nTileWidth, nTileHeight, nFrameWidth, nFrameHeight);
		return result;
	}
	long Zoom1X(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x86, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long GetPtzPos(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x87, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long SetPtzPos(long hDevice, long nControlID, long nPan, long nTilt, long nZoom)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x88, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nPan, nTilt, nZoom);
		return result;
	}
	long CreateDeviceEx()
	{
		long result;
		InvokeHelper(0x89, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	long GetTimelineStartTime(long hTimeline, long nCameraIndex, long nTimelineIndex, long nType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x8a, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hTimeline, nCameraIndex, nTimelineIndex, nType);
		return result;
	}
	long GetTimelineEndTime(long hTimeline, long nCameraIndex, long nTimelineIndex, long nType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x8b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hTimeline, nCameraIndex, nTimelineIndex, nType);
		return result;
	}
	long GetTimelineRecType(long hTimeline, long nCameraIndex, long nTimelineIndex, long nType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x8c, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hTimeline, nCameraIndex, nTimelineIndex, nType);
		return result;
	}
	long GetTimelineRecId(long hTimeline, long nCameraIndex, long nTimelineIndex, long nType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x8d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hTimeline, nCameraIndex, nTimelineIndex, nType);
		return result;
	}
	long SearchOverLap(long hDevice, long tStart, long tEnd)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x8e, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, tStart, tEnd);
		return result;
	}
	long OpenMediaOverLap(long hDevice, long nControlID, long nMediaType, long tStart, long tEnd, long nTrackID, long * phMediaSource)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PI4 ;
		InvokeHelper(0x8f, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType, tStart, tEnd, nTrackID, phMediaSource);
		return result;
	}
	long OpenMediaExOverLap(long hDevice, long nControlID, long nMediaType, long tStart, long tEnd, long nTrackID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x90, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType, tStart, tEnd, nTrackID);
		return result;
	}
	long OpenStreamOverLap(long hDevice, long nControlID, long nMediaType, long tStart, long tEnd, long OverLap)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x91, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nMediaType, tStart, tEnd, OverLap);
		return result;
	}
	long SearchDayOverLap(long hDevice, long nControlID, long tStart, long tEnd, long nRecType, long nTrackID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x92, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, tStart, tEnd, nRecType, nTrackID);
		return result;
	}
	long StartBackupOverLap(long hDevice, long nControlID, LPCTSTR szFile, long nFileType, long tStart, long tEnd, long nTrackID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_BSTR VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x93, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, szFile, nFileType, tStart, tEnd, nTrackID);
		return result;
	}
	long PlayOverLap(long hDevice, long nControlID, double dSpeed, long nTrackID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_R8 VTS_I4 ;
		InvokeHelper(0x94, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, dSpeed, nTrackID);
		return result;
	}
	long SeekOverLap(long hDevice, long nControlID, long tPos, long nTrackID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x95, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, tPos, nTrackID);
		return result;
	}
	long FrameAdvanceOverLap(long hDevice, long nControlID, long nDelta, long nTrackID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x96, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nDelta, nTrackID);
		return result;
	}
	long SearchPos(long hDevice, long tStart, long tEnd, long nTrackID, long bIsAllMatch, long bIsCaseSensitive, LPCTSTR szText)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR ;
		InvokeHelper(0x97, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, tStart, tEnd, nTrackID, bIsAllMatch, bIsCaseSensitive, szText);
		return result;
	}
	long CancelPos(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x98, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}
	long GetPtzPosSpeed(long hDevice, long nControlID)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x99, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID);
		return result;
	}
	long SetPtzPosSpeed(long hDevice, long nControlID, long nPan, long nTilt, long nZoom, long nSpeed)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x9a, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nPan, nTilt, nZoom, nSpeed);
		return result;
	}
	long PowerRestart(long hDevice)
	{
		long result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x9b, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice);
		return result;
	}

	long GetHPtzList(long hDevice, long nControlID, long nType)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4;
		InvokeHelper(156, DISPATCH_METHOD, VT_I4, (void*)&result, parms, hDevice, nControlID, nType);
		return result;
	}
};

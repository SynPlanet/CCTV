#include "StdAfx.h"

#include "JoyControl.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////
extern int GetCmrIDfromJoy( UINT nmb_btn_joy	// device number (was set by pushing joystick button)
													);
//////////////////////////////////////////////////////////////////////////

struct XINPUT_DEVICE_NODE
{
	DWORD dwVidPid;
	XINPUT_DEVICE_NODE* pNext;
};

struct DI_ENUM_CONTEXT
{
	DIJOYCONFIG* pPreferredJoyCfg;
	bool bPreferredJoyCfgValid;
	CJoyControl* pJoyControl;
};

bool                    g_bFilterOutXinputDevices = false;
XINPUT_DEVICE_NODE*     g_pXInputDeviceList = NULL;

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
BOOL CALLBACK		EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL CALLBACK		EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );

//-----------------------------------------------------------------------------
// Returns true if the DirectInput device is also an XInput device.
// Call SetupForIsXInputDevice() before, and CleanupForIsXInputDevice() after
//-----------------------------------------------------------------------------
bool IsXInputDevice( const GUID* pGuidProductFromDirectInput )
{
	// Check each xinput device to see if this device's vid/pid matches
	XINPUT_DEVICE_NODE* pNode = g_pXInputDeviceList;
	while( pNode )
	{
		if( pNode->dwVidPid == pGuidProductFromDirectInput->Data1 )
			return true;
		pNode = pNode->pNext;
	}

	return false;
}
//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,	VOID* p_Val_loc )
{
	IDirectInputDevice2 *p_JoyDevice = ( IDirectInputDevice2 *)p_Val_loc;
	if (!p_JoyDevice){
		return DIENUM_STOP;
	}

	static int nSliderCount = 0;  // Number of returned slider controls
	static int nPOVCount = 0;     // Number of returned POV controls

	// For axes that are returned, set the DIPROP_RANGE property for the
	// enumerated axis in order to scale min/max values.
	if( pdidoi->dwType & DIDFT_AXIS )
	{
		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof( DIPROPRANGE );
		diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
		diprg.diph.dwHow = DIPH_BYID;
		diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
		diprg.lMin = -MAX_DEVIATION_AXIS;
		diprg.lMax = +MAX_DEVIATION_AXIS;

		// Set the range for the axis
		if( FAILED( p_JoyDevice->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
			return DIENUM_STOP;
	}
	// Set the UI to reflect what objects the joystick supports
	if( pdidoi->guidType == GUID_XAxis ){
		;
	}
	if( pdidoi->guidType == GUID_YAxis ){
		;
	}
	if( pdidoi->guidType == GUID_ZAxis ){
		;
	}
	if( pdidoi->guidType == GUID_RxAxis ){
		;
	}
	if( pdidoi->guidType == GUID_RyAxis ){
		;
	}
	if( pdidoi->guidType == GUID_RzAxis ){
		;
	}
	if( pdidoi->guidType == GUID_Slider ){
		nSliderCount++;
	}
	if( pdidoi->guidType == GUID_POV ){
		nPOVCount++;
	}

	return DIENUM_CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated joystick. If we find one, create a device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
    DI_ENUM_CONTEXT* pEnumContext = ( DI_ENUM_CONTEXT* )pContext;
    HRESULT hr;

    if( g_bFilterOutXinputDevices && IsXInputDevice( &pdidInstance->guidProduct ) )
        return DIENUM_CONTINUE;

    // Skip anything other than the perferred joystick device as defined by the control panel.  
    // Instead you could store all the enumerated joysticks and let the user pick.
    if( pEnumContext->bPreferredJoyCfgValid && !IsEqualGUID( pdidInstance->guidInstance, pEnumContext->pPreferredJoyCfg->guidInstance ) )
        return DIENUM_CONTINUE;

    // Obtain an interface to the enumerated joystick.
    hr = pEnumContext->pJoyControl->m_pDI->CreateDevice( pdidInstance->guidInstance, (LPDIRECTINPUTDEVICE8 *)&pEnumContext->pJoyControl->m_pJoyDevice, NULL );

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( FAILED( hr ) )
        return DIENUM_CONTINUE;

    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}

//////////////////////////////////////////////////////////////////////////
CJoyControl::CJoyControl(void)
{
	m_pDI = NULL;
	m_pJoyDevice = NULL;

	m_flag_Enable = false;

	ZeroMemory((void*)&js_old, sizeof(js_old));

	m_JoysCount = 0;

	ZeroMemory((void*)&m_JoyData, sizeof(m_JoyData));

//	Init_Joystick();
}

CJoyControl::~CJoyControl(void)
{
	ReleaseJoystick();
}

//////////////////////////////////////////////////////////////////////////
// initialization joystick
void	CJoyControl::Init_Joystick(void)
{
	HINSTANCE hInst =	GetModuleHandle( NULL );// OR === AfxGetInstanceHandle();
	HRESULT hres;

	if(FAILED(hres = DirectInput8Create(	hInst,
																				DIRECTINPUT_VERSION, (REFIID)IID_IDirectInput8,
																				(VOID **)&m_pDI,
																				NULL
																			)
						)
		){
		Check_result(hres);
		return;
	}

	if (!m_pDI){
		return;
	}

	DIJOYCONFIG PreferredJoyCfg = {0};
	DI_ENUM_CONTEXT enumContext;
	enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
	enumContext.bPreferredJoyCfgValid = false;
	enumContext.pJoyControl = this;

	IDirectInputJoyConfig8* pJoyConfig = NULL;
	if( FAILED( hres = m_pDI->QueryInterface( IID_IDirectInputJoyConfig8, ( void** )&pJoyConfig ) )
		){
			Check_result(hres);
			return;
	}
	// Look for a simple joystick we can use for this sample program.
	if( FAILED(hres = m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, &enumContext, DIEDFL_ATTACHEDONLY ) )
		){
			Check_result(hres);
			return;
	}

// It doesnot work
// 	if(FAILED(hres = m_pDI->CreateDevice(GUID_Joystick, (LPDIRECTINPUTDEVICE8 *)&m_pJoyDevice, NULL) )
// 		){
// 			Check_result(hres);
// 			return;
// 	}

	if (!m_pJoyDevice){
		return;
	}
	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
	if(FAILED(hres =  m_pJoyDevice->SetDataFormat(&c_dfDIJoystick2) )
		){
			Check_result(hres);
			return;
	}

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
// 	if(FAILED(hres =  m_pJoyDevice->SetCooperativeLevel(hInst, DISCL_EXCLUSIVE|DISCL_BACKGROUND) )
// 		){
// 			Check_result(hres);
// 			return;
// 	}

	// Enumerate the joystick objects. The callback function enabled user
	// interface elements for objects that are found, and sets the min/max
	// values property for discovered axes.
  if( FAILED(hres = m_pJoyDevice->EnumObjects( EnumObjectsCallback, ( VOID* )m_pJoyDevice, DIDFT_ALL ) ) 
		){
			Check_result(hres);
			return;
	}
	m_flag_Enable = true;

	printf("Joystick enable: TRUE!\n");

	m_JoysCount = 1;
}

//////////////////////////////////////////////////////////////////////////
void	CJoyControl::Check_result(HRESULT Res)
{
	/**/
	if( (Res == DI_OK) || (Res == S_OK) ){
		printf("OK\n");
	}else{
		if(Res == S_FALSE){	
			//printf("False\n");
		}
		if(Res == DI_PROPNOEFFECT){	
			//printf("The change in device properties had no effect. This value is equal to the S_FALSE standard COM return value. \n");
		}
		if(Res == DIERR_INVALIDPARAM){
			printf("An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value. \n");
		}
		if(Res == DIERR_NOTINITIALIZED){
			printf("This object has not been initialized\n");
		}
		if(Res == DIERR_OBJECTNOTFOUND){
			printf("he requested object does not exist\n");
		}
		if(Res == DIERR_UNSUPPORTED){
			printf("The function called is not supported at this time. This value is equal to the E_NOTIMPL standard COM return value. \n");
		}
		if(Res == DIERR_OTHERAPPHASPRIO){
			printf("Another application has a higher priority level, preventing this call from succeeding. This value is equal to the E_ACCESSDENIED standard COM return value. This error can be returned when an application has only foreground access to a device but is attempting to acquire the device while in the background. \n");
		}
		if(Res == DIERR_INPUTLOST){	
			printf("Access to the input device has been lost. It must be reacquired\n");
		}
		if(Res == DIERR_NOTACQUIRED){	
			printf("The operation cannot be performed unless the device is acquired.\n");
		}
		if(Res == E_PENDING){	
			printf("Data is not yet available\n");
		}
		if(Res == DIERR_INCOMPLETEEFFECT){	
			printf("The effect could not be downloaded because essential information is missing. For example, no axes have been associated with the effect, or no type-specific information has been supplied\n");
		}
		if(Res == DIERR_NOTEXCLUSIVEACQUIRED){	
			printf("The operation cannot be performed unless the device is acquired in DISCL_EXCLUSIVE mode\n");
		}
		if(Res == DIERR_UNSUPPORTED){	
			printf("The function called is not supported at this time. This value is equal to the E_NOTIMPL standard COM return value.\n");
		}
		if(Res == DI_EFFECTRESTARTED){	
			printf(" The effect was stopped, the parameters were updated, and the effect was restarted.\n");
		}
		if(Res == DI_DOWNLOADSKIPPED){	
			printf("The parameters of the effect were successfully updated, but the effect could not be downloaded because the associated device was not acquired in exclusive mode.\n");
		}
		if(Res == DI_TRUNCATED){	
			printf("The parameters of the effect were successfully updated, but some of them were beyond the capabilities of the device and were truncated to the nearest supported value.\n");
		}
		if(Res == DI_TRUNCATEDANDRESTARTED){	
			printf("Equal to DI_EFFECTRESTARTED | DI_TRUNCATED.\n");
		}
		if(Res == DIERR_EFFECTPLAYING){	
			printf("The parameters were updated in memory but were not downloaded to the device because the device does not support updating an effect while it is still playing.\n");
		}
	}//else
}

//////////////////////////////////////////////////////////////////////////
// Release any DirectInput objects
void CJoyControl::ReleaseJoystick(void)
{
	// Unacquire the device one last time just in case 
	// the app tried to exit while the device is still acquired.
	if( m_pJoyDevice ){
		m_pJoyDevice->Unacquire();
		m_pJoyDevice->Release();
		m_pJoyDevice = NULL;
	}

	if( m_pDI ){
		m_pDI->Release();
		m_pDI = NULL;
	}

	ZeroMemory((void*)&m_JoyData, sizeof(m_JoyData));
}

//////////////////////////////////////////////////////////////////////////
int CJoyControl::Retrieve_Joy_Data(void)
{
	HRESULT hres;

  DIJOYSTATE2 js;           // DInput joystick state 
	ZeroMemory(&js, sizeof(DIJOYSTATE2));

  if( !m_pJoyDevice )
      return -1;
/**/

  // Poll the device to read the current state
  hres = m_pJoyDevice->Poll();
  if( FAILED( hres ) )
  {
      // DInput is telling us that the input stream has been
      // interrupted. We aren't tracking any state between polls, so
      // we don't have any special reset that needs to be done. We
      // just re-acquire and try again.
      hres = m_pJoyDevice->Acquire();
      while( hres == DIERR_INPUTLOST )
          hres = m_pJoyDevice->Acquire();

      // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
      // may occur when the app is minimized or in the process of 
      // switching, so just try again later 
      return -1;
  }

	Check_result(hres);

  // Get the input's device state
	if( FAILED( hres = m_pJoyDevice->GetDeviceState( sizeof( DIJOYSTATE2 ), &js ) ) ){

		Check_result(hres);
		return -1; // The device should have been acquired during the Poll()
	}

  // Display joystick state to dialog

	//define data for output screen
	m_JoyData.state_controle = 0;	// set default values
	m_JoyData.speed_moving = 0;

	if (js.lY > DEAD_ZONE_DEVIATION){
		m_JoyData.state_controle |= T_ControlPTZ::DOWN;
	}else{
		if (js.lY < -DEAD_ZONE_DEVIATION){
			m_JoyData.state_controle |= T_ControlPTZ::UP;
		}
	}
	if( js.lX > DEAD_ZONE_DEVIATION ) {
		m_JoyData.state_controle |= T_ControlPTZ::RIGHT;
	}else{
		if (js.lX < -DEAD_ZONE_DEVIATION){
			m_JoyData.state_controle |= T_ControlPTZ::LEFT;
		}
	}

	if( (js.lZ > DEAD_ZONE_DEVIATION) || (js.lRz > DEAD_ZONE_DEVIATION) ){
		m_JoyData.state_controle |= T_ControlPTZ::ZOOM_IN;
	}else{
		if( (js.lZ < -DEAD_ZONE_DEVIATION) || (js.lRz < -DEAD_ZONE_DEVIATION) ){
			m_JoyData.state_controle |= T_ControlPTZ::ZOOM_OUT;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	float speed_normalized = 0;
	// get speed
	unsigned int joy_deviation_loc = (unsigned int)sqrt(float(js.lX * js.lX + js.lY * js.lY + js.lZ * js.lZ));

	// there was changed [26.01.15]
	if( joy_deviation_loc > MAX_DEVIATION_AXIS - 100){
		m_JoyData.state_controle |= T_ControlPTZ::SPEED_HIGH;
	}else{
		if( joy_deviation_loc > DEAD_ZONE_DEVIATION){
			m_JoyData.state_controle |= T_ControlPTZ::SPEED_LOW;

			// calculation speed
				speed_normalized = (float)(joy_deviation_loc - DEAD_ZONE_DEVIATION) / (float)(MAX_DEVIATION_AXIS - DEAD_ZONE_DEVIATION);
				m_JoyData.speed_moving = 30;
		}else{
				m_JoyData.speed_moving = 0;
		}
	}

	// old
// 	if( joy_deviation_loc > FORSAGE_DEVIATION){
// 		m_JoyData.state_controle |= T_ControlPTZ::FORSAGE;
// 	}else{
// 		if( joy_deviation_loc > HIGH_SPEED_DEVIATION){
// 			m_JoyData.state_controle |= T_ControlPTZ::SPEED_HIGH;
// 		}else{
// 			if( joy_deviation_loc > DEAD_ZONE_DEVIATION){
// 				m_JoyData.state_controle |= T_ControlPTZ::SPEED_LOW;
// 
// 				// calculation speed
// 				speed_normalized = (float)(joy_deviation_loc - DEAD_ZONE_DEVIATION) / (float)(HIGH_SPEED_DEVIATION - DEAD_ZONE_DEVIATION);
// 				m_JoyData.speed_moving = speed_normalized * 60.0f;	// 60% whole speed
// 			}else{
// 				m_JoyData.speed_moving = 0;
// 			}
// 		}

	if ((m_JoyData.state_controle == T_ControlPTZ::ZOOM_IN) || (m_JoyData.state_controle == T_ControlPTZ::ZOOM_OUT)){
		m_JoyData.state_controle |= T_ControlPTZ::SPEED_LOW;
	}

	//////////////////////////////////////////////////////////////////////////
	//m_JoyData.speed_LeftRight *= 100.0f;

  // Axes
  js.lX;    js.lY;    js.lZ;
  js.lRx;   js.lRy;   js.lRz;

  // Slider controls
  js.rglSlider[0];    js.rglSlider[1];

  // Points of view
  js.rgdwPOV[0];    js.rgdwPOV[1];    js.rgdwPOV[2];    js.rgdwPOV[3];

  // Fill up text with which buttons are pressed
  for( int i = 0; i < 128; i++ ){
		// button are pressed
		if( js.rgbButtons[i] & 0x80 ){
			// define buttons for "Network Controller SAMSUNG Model: SPC-2000"
			switch(i)
			{
				case 9: m_JoyData.state_controle |= T_ControlPTZ::BASE_POS;	break;
				case 10:
					{
						m_JoyData.state_controle |= T_ControlPTZ::ZOOM_OUT;		 //m_JoyData.state_controle |= T_ControlPTZ::FOCUS_NEAR;	break;
						m_JoyData.state_controle |= T_ControlPTZ::SPEED_HIGH;
						break;
					}
					
				case 11:
					{
						m_JoyData.state_controle |= T_ControlPTZ::ZOOM_IN;
						m_JoyData.state_controle |= T_ControlPTZ::SPEED_HIGH;
						break;
					}
					
				// get number current camera
				default: m_JoyData.cur_cam = GetCmrIDfromJoy(i); 	break;	
			}
    }
	}

	return 0;
}
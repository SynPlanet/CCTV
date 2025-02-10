#pragma once

#include <dinput.h>             // DirectInput
#include <dinputd.h>

#include "CommonIP.h"

using namespace JoyDevice;
using namespace IP_Camera;

class CJoyControl
{
public:
	CJoyControl(void);
	~CJoyControl(void);

	int		Retrieve_Joy_Data(void);

	const bool IsJoyEnable(void){return m_flag_Enable;};

	LPDIRECTINPUT8/*IDirectInput*/					m_pDI;
	LPDIRECTINPUTDEVICE8/*IDirectInputDevice2*/   m_pJoyDevice;

	T_DataPTZ	m_JoyData;

protected:
	// initialization joystick
	void	Init_Joystick(void);

	// Release variables
	void ReleaseJoystick(void);

private:
	UINT 	nIDEvent_time_my;
	UINT 	time_out;

	UINT m_JoysCount;	// whole number joysticks was connect (!!! FOR NEXT UPDATE !!!)

	DIJOYSTATE2	js_old;//структура предыдущего состояния

	bool Device_Active;

	void	Check_result(HRESULT Res);//процедура проверки результата выполнения функции

	bool m_flag_Enable;	// the flag define joystick was connected
};


#include "StdAfx.h"
#include "PTZ_Device.h"
#include "PtzDlg.h"
#include "Ptz.h"

CPtzDlg *g_pPtzDlg = NULL;

T_NetAddress	g_NetAddr_Dev;

CPTZ_Device::CPTZ_Device(void)
{
	g_pPtzDlg = NULL;

	ZeroMemory((void*)&g_NetAddr_Dev, sizeof(g_NetAddr_Dev) );
}

CPTZ_Device::~CPTZ_Device(void)
{
	if (g_pPtzDlg){
		Release_SDK_Samsung();
	}

	Disconnect_Dev(NULL);
}
//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::Init_SDK_Samsung( void )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	if (!g_pPtzDlg){

		g_pPtzDlg = new CPtzDlg();
		//  
		g_pPtzDlg->Create(IDD_PTZ_DIALOG, NULL);
		//g_pPtzDlg->ShowWindow(SW_HIDE);
		g_pPtzDlg->UpdateWindow();

		return 0;
	}
	return 1;
}
int  CPTZ_Device::Release_SDK_Samsung( void )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	if (g_pPtzDlg){
		g_pPtzDlg->Stop_Rotate();
		g_pPtzDlg->Stop_Focus_Moving();

		HMODULE hModule = ::GetModuleHandle(NULL);
		AfxTermLocalData(hModule, 0);
		//AfxTlsRelease();
		//g_pPtzDlg->EndDialog(1);
	//	g_pPtzDlg->PostNcDestroy();

		g_pPtzDlg->ReleaseDevices_Manually();

		g_pPtzDlg->DestroyWindow();

		delete g_pPtzDlg;
		g_pPtzDlg = NULL;

		return 0;
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::Connect_Dev( T_NetAddress*	p_net_addr_in )
{
	if (g_pPtzDlg){
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

		if( !(			(g_NetAddr_Dev.IP[0] == p_net_addr_in->IP[0] )
						&&  (g_NetAddr_Dev.IP[1] == p_net_addr_in->IP[1] )
						&&  (g_NetAddr_Dev.IP[2] == p_net_addr_in->IP[2] )
						&&  (g_NetAddr_Dev.IP[3] == p_net_addr_in->IP[3] )
				)
			){

			// first initialization
			if ( (g_NetAddr_Dev.IP[0] == 0) &&
					 (g_NetAddr_Dev.IP[1] == 0) &&
					 (g_NetAddr_Dev.IP[2] == 0) &&
					 (g_NetAddr_Dev.IP[3] == 0)
					){
						;
			}else{
				g_pPtzDlg->DisconnectDevice(p_net_addr_in);
			}
			g_NetAddr_Dev = *p_net_addr_in;
			
			return g_pPtzDlg->ConnectToDevice(p_net_addr_in);
		}
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::Disconnect_Dev( T_NetAddress*	p_net_addr_in )
{
	if (g_pPtzDlg){
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

		return g_pPtzDlg->DisconnectDevice(p_net_addr_in);
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::Right( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_Right();
	}
	return 1;
}
int  CPTZ_Device::Left( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_Left();
	}
	return 1;
}

int  CPTZ_Device::Up( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_Up();
	}
	return 1;
}

int  CPTZ_Device::Down( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_Down();
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::RightUp( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_RightUp();
	}
	return 1;
}

int  CPTZ_Device::RightDown( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_RightDown();
	}
	return 1;
}

int  CPTZ_Device::LeftUp( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_LeftUp();
	}
	return 1;
}

int  CPTZ_Device::LeftDown( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_LeftDown();
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::Speed( unsigned int speed_in ) // [0...100]
{
	if (g_pPtzDlg){
		g_pPtzDlg->Move_Speed(speed_in);
		return 0;
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::Stop_Rotate( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Stop_Rotate();
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::ZoomIn( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->ZoomIn();
	}
	return 1;
}

int  CPTZ_Device::ZoomOut( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->ZoomOut();
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////
int  CPTZ_Device::Focus_Near( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_FocusNear();
	}
	return 1;
}

int  CPTZ_Device::Focus_Far( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Move_FocusFar();
	}
	return 1;
}

int  CPTZ_Device::Stop_Focus_Moving( void )
{
	if (g_pPtzDlg){
		return g_pPtzDlg->Stop_Focus_Moving();
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////
// set request for callback return values
int  CPTZ_Device::GetPtzStatic( void )
{
	if (g_pPtzDlg){
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

		return g_pPtzDlg->GetPtzValues();
	}
	return 1;
}

int  CPTZ_Device::SetPtzStatic( int pan_in, int tilt_in, int zoom_in )
{
	if (g_pPtzDlg){
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

		return g_pPtzDlg->SetPtzValues(pan_in, tilt_in, zoom_in);
	}
	return 1;
}

void CPTZ_Device::CallBackFunc_GetPtzValues(void* ptr_GetPtzValues)
{
	if (g_pPtzDlg){
		g_pPtzDlg->CallBack_GetPtzValues(ptr_GetPtzValues);
	}
	return;
}
//////////////////////////////////////////////////////////////////////////
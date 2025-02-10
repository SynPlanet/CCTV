// CaptureIP.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
//#include "CaptureIP.h"
//#include "CameraIP.h"
#include "Extern.h"
#include "Proccesor.h"
#include "NetPTZControl.h"
#include "NetControlReceive.h"
#include "CommonTimerThread.h"
#include "ControlHDD.h"

#include "MetaMoviesHDD.h"

#include "Synh_Objs.h"

static ListCamerasIP	gList_Cameras_IP;

BITMAP	g_base_ImageBmp;	// base image for drawing 
HBITMAP g_hBaseImageBMP = NULL;
BOOL g_FlagInit_BMP = FALSE;

HBITMAP Load_Bmp(char *path_Bmp);

static CProccesor	g_ProccesorClass;
static CNetPTZControl g_NetPTZControl;
static CNetControlReceive	g_NetControlReceive;
static CCommonTimerThread		g_CmnTimerGrabThreads;	// common timer for grabber threads

static CMetaMoviesHDD		g_MoviesHDD;	// class define list meta data movies for playing

static unsigned int		g_MaxTimeRecMinutes;	// maximum time recording movie

static CSynh_Barrier g_Synh_Barrier_StateCam_1;
static CSynh_Barrier g_Synh_Barrier_RenameFiles_1;

static CSynh_Barrier g_Synh_Barrier_StateCam_2;
static CSynh_Barrier g_Synh_Barrier_RenameFiles_2;

static CControlHDD	g_ControlHDD;

// critical section GetBMP
static CSynh_CritSect		m_SynhCS_GetBMP;

///// CProxyVLC	g_ProxyVLC;

Logger::CLog g_Logger(NULL);

char g_PathBase_VideoRec[_MAX_PATH] = "";
char g_PathBase_VideoPlay[_MAX_PATH] = "";

 char g_PathBase_AudioRec[_MAX_PATH] = "";
 char g_PathBase_AudioPlay[_MAX_PATH] = "";

 char g_PathFinal_VideoRec[_MAX_PATH] = "";
 char g_PathFinal_VideoPlay[_MAX_PATH] = "";

 char g_PathFinal_AudioRec[_MAX_PATH] = "";
 char g_PathFinal_AudioPlay[_MAX_PATH] = "";


 char g_Path_MountFolder[MAX_SIGN_NAME_FOLDER] = "";

//////////////////////////////////////////////////////////////////////////
#include "../AudioStreamer/AudioStreamerData.h"
using namespace AudioDevice;


/*static*/ AudioDevice::AudioData g_AudioData;	//by using keyword "static" - the instance could not be request from another CPP files!!!

// initialization extern Audio module
int Init_AudioImport(void* pProxyVLC);

// release extern Audio module
int Release_AudioImport(void);
//////////////////////////////////////////////////////////////////////////

//UINT g_StateMovie = T_DeviceState::STOP;
//////////////////////////////////////////////////////////////////////////
// define ID camera by using the joystick number
// "-1" - camera was not found
int GetCmrIDfromJoy( UINT nmb_btn_joy	// device number (was set by pushing joystick button)
										);

// release all IP cameras
void ReleaseAllCamerasIP( void );

void RenameFilesinFolder(UINT flag_wrt);
//////////////////////////////////////////////////////////////////////////

Set_PlayerState p_Set_PlayerState = NULL;
Joy_IsConnect p_Joy_IsConnect = NULL;
Joy_Control	p_Joy_Control = NULL;
GetTimeMovie	p_GetTimeMovie = NULL;
SetMovieFolder	p_SetMovieFolder = NULL;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CallBackFunc_PlayerState(void* ptr_CBFunc_RecState)
{
	p_Set_PlayerState = static_cast<Set_PlayerState>(ptr_CBFunc_RecState);
}

//////////////////////////////////////////////////////////////////////////
// bind callback function for sending out information joystick connection and control ones
void CallBackFunc_JoyControl(void* ptr_CBFunc_JoyIsConnect, void* ptr_CBFunc_JoyControl)
{
	p_Joy_IsConnect = static_cast<Joy_IsConnect>(ptr_CBFunc_JoyIsConnect);
	p_Joy_Control = static_cast<Joy_Control>(ptr_CBFunc_JoyControl);
}

//////////////////////////////////////////////////////////////////////////
// bind callback function for getting out count time from player
void CallBackFunc_GetTimePlayer(void* ptr_CBFunc_GetTimePlayer)
{
	p_GetTimeMovie = static_cast<GetTimeMovie>(ptr_CBFunc_GetTimePlayer);

	/////
	//sound control
//	if (g_AudioData.bEnable){
//		((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetFuncGetTimeMovie(p_GetTimeMovie);
//	}
	/////
}

//////////////////////////////////////////////////////////////////////////
// bind callback function for getting out folder for the movie playing
void CallBackFunc_SetMovieFolder(void* ptr_CBFunc_SetMovieFolder)
{
	p_SetMovieFolder = static_cast<SetMovieFolder>(ptr_CBFunc_SetMovieFolder);

	//////////////////////////////////////////////////////////////////////////
	// TEST

// 	if (p_SetMovieFolder){
// 		CHAR folder_name[_MAX_PATH] = {"HELLO WORLD!!!"};
// 		p_SetMovieFolder(folder_name, strlen(folder_name));
// 	}

	//////////////////////////////////////////////////////////////////////////
}
//////////////////////////////////////////////////////////////////////////
// initialization general modules
int Initialize_ALL(const UINT PTZ_enable)
{
	// set Logs streaming
	g_Logger.SetDirectionLog(0);	// '0' - create log File
	//	ShowWindow( GetConsoleWindow(), SW_SHOWMINIMIZED );

	//!!! set initialization before Audio module !!!
	if (PTZ_enable){
		g_NetPTZControl.Init();
	}else{
		printf("PTZ_enable: FALSE!\n");
	}

	// initialization SDK VLC
	//Init_VLC_SDK();
/////	bool flag_enable = g_ProxyVLC.IsEnableVLC();

	///
	//set Audio module
 	if (!g_AudioData.bEnable){	// the first initialization
 		Init_AudioImport(NULL/*&g_ProxyVLC*/);	// initialization extern Audio module

		// if there are Audio module connection -> pass synchronize objects into Audio module controlling classes
		if (g_AudioData.bEnable){
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetBarrierCtrlCam(&g_Synh_Barrier_StateCam_1, &g_Synh_Barrier_StateCam_2);
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetBarrierCtrlCam(&g_Synh_Barrier_StateCam_1, &g_Synh_Barrier_StateCam_2);
		}
 	}
	///

	g_MaxTimeRecMinutes = SAVE_VIDEO_MINUTES;

	g_CmnTimerGrabThreads.p_gList_Cameras_IP = &gList_Cameras_IP;

	// set pointer to the global barrier of state mode
	g_CmnTimerGrabThreads.m_pSynh_Bar_StCam_1 =	&g_Synh_Barrier_StateCam_1;
	g_CmnTimerGrabThreads.m_pSynh_Bar_StCam_2 =	&g_Synh_Barrier_StateCam_2;

	g_CmnTimerGrabThreads.Init_TimerThread();

	g_ControlHDD.Init_TimerThread();

	//function requests a minimum resolution for periodic timers.
	timeBeginPeriod(1);

	// the single initialization bitmap
	if (!g_FlagInit_BMP){
		Load_Bmp(IMAGE_DEFAULT);

		g_FlagInit_BMP = TRUE;
	}

	//g_Logger.PrintMessage("Initialize_ALL()-> OK");

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// release modules was initialized
int Release_ALL(void)
{
	printf("CaptureIP.dll Release->Begin...!\n");
	printf(">>\n");
	//////////////////////////////////
	g_ControlHDD.Release();

	//////////////////////////////////
 	g_CmnTimerGrabThreads.Release();

	//////////////////////////////////
	// delete all classes IP cameras
	ReleaseAllCamerasIP();
	//////////////////////////////////
	// release bitmap
	if (g_hBaseImageBMP){
		DeleteObject(g_hBaseImageBMP);

		g_hBaseImageBMP = NULL;
		g_FlagInit_BMP = FALSE;
	}
	//////////////////////////////////

	// Release SDK VLC
	//Release_VLC_SDK();
/////	bool flag_enable = g_ProxyVLC.IsEnableVLC();

	//////////////////////////////////
	// release output control video writing
	if (g_NetControlReceive.IsEnable()){
		g_NetControlReceive.Release();
	}

	printf("NetPTZControl->Release...!\n");
	printf(">>>\n");
	// release PTZ control (thread)
		g_NetPTZControl.Release();
	printf("<<<\n");

	printf("AudioModule->Release...!\n");
	printf(">>>\n");
	// release extern Audio module
		Release_AudioImport();
	printf("<<<\n");

	printf("<<\n");
	printf("CaptureIP.dll Release...Finish!\n");

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// create IP stream
int CreateStreamIP(	UINT					cmr_ID_in,				// camera number
										T_NetAddress*	net_addr_in,			// net address
										char*					path_stream_out		// video stream path (was opened)
									)
{
	printf("CreateStreamIP[%d]:Begin... \n", cmr_ID_in);
	sprintf(path_stream_out, "");

	if(/* (path_Stream == NULL) || */(cmr_ID_in < 0) ){
		printf("CreateStreamIP[%d]-> return: %d (Wrong stream's number!)\n", cmr_ID_in, -2);
		return -2;
	}

	// set default values
	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

	CCameraIP	*pCameraIP_loc = NULL;
	CCameraIP	*cameraIP_new;

	// find the same streaming camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
		it_CameraIP = gList_Cameras_IP.begin();

		std::advance(it_CameraIP, nmb_cmr_loc);
		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		if (	(pCameraIP_loc->GetNmbCameraIP() == cmr_ID_in)
// 						||
// 					(!strcmp(pCameraIP_loc->GetStreamPath(), path_Stream))
			){
				if (!pCameraIP_loc->Create_StreamIP(net_addr_in)){

					printf("CreateStreamIP[%d]-> return: %d (Stream was not opened!)\n", cmr_ID_in, -1);
					return -1;	// streaming path is disable
				}
				//copy video path upstairs
				sprintf(path_stream_out, pCameraIP_loc->GetStreamPath());

				//printf("Video stream path(IP[%d]): %s\n", cmr_ID_in, path_stream_out);
				printf("CreateStreamIP[%d]: %s (Stream was initialized)\n", cmr_ID_in, path_stream_out);
			//this one camera was already initialized
			return 1;
		}
	}

	BOOL enable_thread = TRUE;

	// create new class of IP camera and set parameters for this one
	cameraIP_new = new CCameraIP(	Type_SDK_API::OPEN_CV,				// SDK for reading stream
																Type_SDK_API::OPEN_CV,		// SDK for write stream
																Type_Stream::IP_STREAM		// type stream
															);

	// set number processor for the future thread
	cameraIP_new->SetNmbProcessor(g_ProccesorClass.GetNmb_Processor4Thread());
	//	cameraIP_new->SetEnableThread_Bmp(enable_thread);
	cameraIP_new->SetBaseBMP(g_hBaseImageBMP);

	cameraIP_new->SetMaxTimeRecMinutes(g_MaxTimeRecMinutes);
	cameraIP_new->SetNmbCameraIP(cmr_ID_in);

	cameraIP_new->SetPathRecording(g_PathFinal_VideoRec);

	cameraIP_new->m_pSynh_Bar_StCam_1 =	&g_Synh_Barrier_StateCam_1;
	cameraIP_new->m_pSynh_Bar_RnmFile_1 =	&g_Synh_Barrier_RenameFiles_1;

	cameraIP_new->m_pSynh_Bar_StCam_2 =	&g_Synh_Barrier_StateCam_2;
	cameraIP_new->m_pSynh_Bar_RnmFile_2 =	&g_Synh_Barrier_RenameFiles_2;

	// try to open the stream (!!! GRABBER THREAD !!!)
	int flag_open_stream = cameraIP_new->Create_StreamIP(net_addr_in);

	if (flag_open_stream){
		//////////////////////////////////////////////////////////////////////////
		g_CmnTimerGrabThreads.m_SynhCS_Timer.Set_Critical_Section();
		{
			m_SynhCS_GetBMP.Set_Critical_Section();
			{
				//append new camera IP to cameras list 
				gList_Cameras_IP.push_back(cameraIP_new);

			}
			m_SynhCS_GetBMP.Leave_Critical_Section();
		}
		g_CmnTimerGrabThreads.m_SynhCS_Timer.Leave_Critical_Section();
		//////////////////////////////////////////////////////////////////////////

		//copy video path upstairs
		sprintf(path_stream_out, cameraIP_new->GetStreamPath());

		printf("CreateStreamIP[%d]: Video stream was open: %s -> OK!!!\n", cmr_ID_in, path_stream_out);
	}else{
		delete cameraIP_new;

		//printf("Video stream path(IP[%d]): %s\n", cmr_ID_in, path_stream_out);
		printf("CreateStreamIP[%d]: %s (Stream was not opened!)\n", cmr_ID_in, path_stream_out);
		return -1;	// streaming path is disable
	}

	return 0; // the video stream is opened
}

//////////////////////////////////////////////////////////////////////////
// create file stream(for play video)
int CreateStreamFile(	UINT					cmr_ID_in,				// camera number
											UINT					nmb_file_in,			// number file for the playing(number '0' is the first one)
											char*					path_stream_out		// video stream path (was opened)
										)
{
	printf("CreateStreamFile[%d]:Begin... \n", cmr_ID_in);

	int flag_res = 0;

	if(cmr_ID_in < 0){
		printf("CreateStreamFile[%d]-> return: %d (Wrong stream's number!)\n", cmr_ID_in, -2);
		return -2;
	}

	// set default
	sprintf(path_stream_out, "");

	//////////////////////////////////////////////////////////////////////////
	// find camera ID by the same ID was get
	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

	CCameraIP	*pCameraIP_loc = NULL;
	CCameraIP	*cameraIP_new = NULL;

	// find the same streaming camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
		it_CameraIP = gList_Cameras_IP.begin();

		std::advance(it_CameraIP, nmb_cmr_loc);
		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		// the same camera ID was found
		if (pCameraIP_loc->GetNmbCameraIP() == cmr_ID_in) {

			printf("CreateStreamFile[%d]-> return: %d (Stream was initialized)\n", cmr_ID_in, 1);
			//this one camera was already initialized
			return 1;
		}
	}
	///////////////////////////////////////
	// get parameters for forming filename output

	// find the same movie by name
	char *nm_loc = (char*)g_MoviesHDD.GetNameMovie(nmb_file_in);
	sprintf(path_stream_out, nm_loc);	//copy video path upstairs
	printf("Video stream path(FILE[%d]): %s\n", cmr_ID_in, path_stream_out);
	///////////////////////////////////////

	// create new class of object camera and set parameters for this one
	cameraIP_new = new CCameraIP(	Type_SDK_API::OPEN_CV,				// SDK for reading stream
																Type_SDK_API::OPEN_CV,		// SDK for write stream
																Type_Stream::FILE_STREAM		// type stream
															);

	// set number processor for the future thread
	cameraIP_new->SetNmbProcessor(g_ProccesorClass.GetNmb_Processor4Thread());
	cameraIP_new->SetBaseBMP(g_hBaseImageBMP);
	
	//unsigned int lng_total_recs = g_MoviesHDD.GetLengthMovie(nmb_file_in);

	cameraIP_new->SetNmbCameraIP(cmr_ID_in);

	// try to open the stream (!!! GRABBER THREAD !!!)
	int flag_open_stream = cameraIP_new->Create_StreamFile_SDK(&g_MoviesHDD, nmb_file_in);

	// Set state of flag for movie play
	//cameraIP_new->SetControlFlag(g_StateMovie);
	//cameraIP_new->SetPathRecording(g_PathFinal_VideoRec);

	// set pointer to the global barrier of state mode
	cameraIP_new->m_pSynh_Bar_StCam_1 =	&g_Synh_Barrier_StateCam_1;
	cameraIP_new->m_pSynh_Bar_StCam_2 =	&g_Synh_Barrier_StateCam_2;

	if (flag_open_stream){
		//////////////////////////////////////////////////////////////////////////
		g_CmnTimerGrabThreads.m_SynhCS_Timer.Set_Critical_Section();
		{
			if (!gList_Cameras_IP.size()) {	// for function control: "p_GetTimeMovie"
				cameraIP_new->Patch_FirstListUnit();
			}

			m_SynhCS_GetBMP.Set_Critical_Section();
			{
				//append new camera IP to cameras list 
				gList_Cameras_IP.push_back(cameraIP_new);

			}
			m_SynhCS_GetBMP.Leave_Critical_Section();
		}
		g_CmnTimerGrabThreads.m_SynhCS_Timer.Leave_Critical_Section();
		//////////////////////////////////////////////////////////////////////////

	}else{
		delete cameraIP_new;

		printf("CreateStreamFile[%d]-> return(del): %d (Stream was not opened!)\n", cmr_ID_in, -1);
		return -1;	// streaming path is disable
	}
	///////////////////////////////////////
	printf("CreateStreamFile[%d]-> return: %d (Stream was open) -> OK!!!\n", cmr_ID_in, 0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// getting screenshot of current camera( using BMP format )
const int GetBMP_NmbCamera(	const UINT nmb_Cmr,				// number of current camera
														UINT &size_Bmp_W,			// bitmap width
														UINT &size_Bmp_H,			// bitmap height
														UINT &bit_pxl_Bmp,		// bitmap per pixel
														VOID **buf_Bmp				// bitmap buffer
													)
{
	CCameraIP	*pCameraIP_loc = NULL;

	m_SynhCS_GetBMP.Set_Critical_Section();
	{
		int cnt_cmr_IP = gList_Cameras_IP.size();
		IterCameraIP it_CameraIP = gList_Cameras_IP.begin();
	
		// find need streaming camera
		for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc) {

			pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

			if ((pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr)/* && (nmb_cmr_loc != 3) */) { //need camera IP was found

				pCameraIP_loc->Set_CS_TimerGrabber();	//!!! Sync.grab thread(CameraIP)  [12.02.15]

				// get single image from camera IP
				if ((*buf_Bmp = (VOID *)pCameraIP_loc->GetImageBuf_Output()) == NULL) {
					pCameraIP_loc->GetDimImage_Output(size_Bmp_W, size_Bmp_H);
					bit_pxl_Bmp = pCameraIP_loc->GetBitsPerPxl_Img();

					pCameraIP_loc->Leave_CS_TimerGrabber();	//!!! Sync.grab thread(CameraIP)  [12.02.15]

					m_SynhCS_GetBMP.Leave_Critical_Section();
					return -2;
				}
				pCameraIP_loc->GetDimImage_Output(size_Bmp_W, size_Bmp_H);
				bit_pxl_Bmp = 24;//pCameraIP_loc->GetBitsPerPxl_Img();

				if ((size_Bmp_W > 0) && (size_Bmp_H > 0) && (bit_pxl_Bmp > 0) && (buf_Bmp)) {
					pCameraIP_loc->Leave_CS_TimerGrabber();	//!!! Sync.grab thread(CameraIP)  [12.02.15]

					m_SynhCS_GetBMP.Leave_Critical_Section();
					return 0;
				}
				else {
					pCameraIP_loc->Leave_CS_TimerGrabber();	//!!! Sync.grab thread(CameraIP)  [12.02.15]

					m_SynhCS_GetBMP.Leave_Critical_Section();
					return -1;
				}
			}
			std::advance(it_CameraIP, 1);
		}
	}
	m_SynhCS_GetBMP.Leave_Critical_Section();

	return -1;
}

//////////////////////////////////////////////////////////////////////////
//! get parameters from camera
const int GetParamsCamera(	const UINT nmb_Cmr,		// number of current camera
														UINT &fps_dev,				// the number frames per second getting from device(CameraIP)
														UINT &img_W_dev,	// width image from device(CameraIP)
														UINT &img_H_dev		// height image from device(CameraIP)
													) 
{
	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

	CCameraIP	*pCameraIP_loc = NULL;

	float fps_dev_loc = 0;			// the number frames per second getting from device(CameraIP)
	UINT img_W_dev_loc = 0;			// width image from device(CameraIP)
	UINT img_H_dev_loc = 0;			// height image from device(CameraIP)

	// find need streaming camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){

		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		if (pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr){ //need camera IP was found
			pCameraIP_loc->Get3ParamCmr(fps_dev_loc, img_W_dev, img_H_dev);

			fps_dev = (UINT)fps_dev_loc;

			return 0;
		}
		std::advance(it_CameraIP, 1);
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
//set parameters for streaming camera
int SetParamsCamera(	const UINT nmb_Cmr,		// number of current camera
											const UINT dim_img_W,		// width image CameraIP(if exist)
											const UINT dim_img_H		// height image CameraIP(if exist)
										)
{
	return -2;
}

//////////////////////////////////////////////////////////////////////////
//Set Flag Camera IP support rotation (360)
BOOL SetCamera_360(	const UINT nmb_Cmr,				// number of current camera
										const UCHAR flag_360_in		// definition flag of control using CameraIP 360
									)
{
	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.end();

	CCameraIP	*pCameraIP_loc = NULL;

	// find need streaming camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
		it_CameraIP = gList_Cameras_IP.begin();

		std::advance(it_CameraIP, nmb_cmr_loc);
		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		if (	pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr){ //need camera IP was found

			pCameraIP_loc->SetFlag360((BOOL)flag_360_in);

			if (flag_360_in){
				g_NetPTZControl.SetCameraPTZ_Control(pCameraIP_loc);
			}

			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//Get Flag Camera IP support rotation (360)
BOOL GetCamera_360(	const UINT nmb_Cmr				// number of current camera
									)
{

	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.end();

	CCameraIP	*pCameraIP_loc = NULL;

	// find need streaming camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
		it_CameraIP = gList_Cameras_IP.begin();

		std::advance(it_CameraIP, nmb_cmr_loc);
		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		if (	pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr){ //need camera IP was found

			pCameraIP_loc->GetFlag360();

			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//! update state video stream for each device
const int UpdataStateStream(	const UINT nmb_Cmr,		// number of current camera[0 ... N]
															const UINT new_state	// new state for further control audio-video streaming(see T_DeviceState)
														)
{
	printf("UpdataStateStream(); nmb_Cmr = %d\n", nmb_Cmr);

	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

	if (!cnt_cmr_IP){	// defence from block thread(using CSharp wrapping method)

		printf("UpdataStateStream()->Finish(-2) \n");
		return -2;
	}

	CCameraIP	*pCameraIP_loc = NULL;

	/////
	// control Audio
	unsigned int cnt_Audio_obj = 0;

	if (g_AudioData.bEnable){
		if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
			cnt_Audio_obj = ((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->GetCountStreams();
		}else{
		//if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
			cnt_Audio_obj = ((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->GetCountStreams();
		}
	}
	//
	unsigned int cnt_obj_bar = cnt_cmr_IP + 1 + 1 + cnt_Audio_obj;	// "+1" - it is my thread(stream "CaptureIP.cpp") + 1 (thread: CCommonTimerThread)

	// !!! need set numbers threads "+1" it is my thread "+1" (thread: CCommonTimerThread) !!!
	// set numbers thread for barrier using
	g_Synh_Barrier_StateCam_1.SetNmbBarrier(cnt_obj_bar);
	g_Synh_Barrier_StateCam_2.SetNmbBarrier(cnt_obj_bar);

	//////////////////////////////////////////////////////////////////////////
	// command for all cameras
	if (nmb_Cmr == -1){
		//g_StateMovie = flag_wrt;
		//////////////////////////////////////////////////////////////////////////
		// set synchronize camera using
		InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 1);
		{
			/////////////////////////////////////
			//sound control
// 			if (g_AudioData.bEnable){
// 				if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
// 					;//((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetStateDevice(new_state);
// 				}else{
// 					//if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
// 					((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetStateDevice(new_state);
// 				}
// 			}
			/////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			// block the first ones(align all threads to "the first point")
			g_Synh_Barrier_StateCam_1.Block();
			{
				/////
				// transfer state in to CommonTimer for next control 
				g_CmnTimerGrabThreads.SetStateDevice(new_state);
				/////

				it_CameraIP = gList_Cameras_IP.begin();
				// reseach all streaming camera
				for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){

					pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

					unsigned int state_rec_loc = pCameraIP_loc->GetStateRec_SDK();
					bool flag_start_thread = pCameraIP_loc->GetEnableThread();

					// Set state of flag for control video
					pCameraIP_loc->SetControlFlag(new_state);

					// create file before set states for Cameras
					if(		(new_state == T_DeviceState::WRITE)						// new state recording
						&&	(state_rec_loc == T_DeviceState::STOP)				// old state recording
						&&	flag_start_thread														// grabbing thread was opened
						){
							// set path finally recording
							//SetPathRecording(m_short_path_rec);

							pCameraIP_loc->WriteFrameMPEG_SDK();
							//g_Logger.PrintMessage("Create File");
					}
					std::advance(it_CameraIP, 1);
				}

				/////////////////////////////////////
				//sound control
				if (g_AudioData.bEnable){
					if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
						((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetStateDevice(new_state);
					}else{
						//if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
						((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetStateDevice(new_state);
					}
				}
				/////////////////////////////////////
			}
			InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 0);

			// !!!unblock all stopped threads!!! ("second point" )
			g_Synh_Barrier_StateCam_2.Block();
		}
		//////////////////////////////////////////////////////////////////////////
		//for Sergey
		//RenameFilesinFolder(flag_wrt);

		printf("UpdataStateStream()->Finish(0) \n");
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// !!! need set numbers threads "+1" it is my thread "+1" (thread: CCommonTimerThread) !!!
	// set synchronize camera using
	InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 1);
	{
		//////////////////////////////////////////////////////////////////////////
		//sound control
// 		if (g_AudioData.bEnable){
// 			if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
// 				;//((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetStateDevice(flag_wrt);
// 			}else{
// 				//if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
// 				((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetStateDevice(new_state);
// 			}
// 		}
		//////////////////////////////////////////////////////////////////////////
		g_Synh_Barrier_StateCam_1.Block();
		{
			/////
			// transfer state in to CommonTimer for next control 
			g_CmnTimerGrabThreads.SetStateDevice(new_state);
			/////

			//////////////////////////////////////////////////////////////////////////
			//sound control
			if (g_AudioData.bEnable){
				if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
					((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetStateDevice(new_state);
				}else{
					//if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
					;((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetStateDevice(new_state);
				}
			}
			//////////////////////////////////////////////////////////////////////////

			it_CameraIP = gList_Cameras_IP.begin();

			// find need streaming camera
			for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
				pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

				if (	pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr){ //need camera IP was found

					// Set state of flag for control video
					pCameraIP_loc->SetControlFlag(new_state);

					InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 0);
					g_Synh_Barrier_StateCam_2.Block();

					printf("UpdataStateStream()->Finish(0) \n");

					return 0;
				}
				std::advance(it_CameraIP, 1);
			}


		}
		InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 0);
		g_Synh_Barrier_StateCam_2.Block();
	}
	//////////////////////////////////////////////////////////////////////////

	printf("UpdataStateStream()->Finish(-1) \n");

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// set new parameter bit per pixel for getting image
int SetBitImgCamera(	const UINT nmb_Cmr,			// number of current camera
											const UINT bit_pxl_img	// bit per pixel for getting image
										)
{
	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.end();

	CCameraIP	*pCameraIP_loc = NULL;

	// find need streaming camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
		it_CameraIP = gList_Cameras_IP.begin();

		std::advance(it_CameraIP, nmb_cmr_loc);
		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		if (	pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr){ //need camera IP was found

			if (!pCameraIP_loc->SetBitsPerPxl_Img(bit_pxl_img)){
				return 0;
			}else{
				return -2;
			}
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
// release stream IP camera(include allocated memory) by number
int ReleaseCameraIP_Nmb(const UINT nmb_Cmr_del	// number camera IP for delete
												)
{
	printf("Start Release device by number: %d\n", nmb_Cmr_del);

	g_CmnTimerGrabThreads.m_SynhCS_Timer.Set_Critical_Section();
	{
		// delete all Cameras IP
		int cnt_cmr_IP = gList_Cameras_IP.size();
		IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

		printf("Release CameraIP:%d\n", nmb_Cmr_del);

		CCameraIP	*pCameraIP_loc = NULL;

		// find need streaming camera
		for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc) {

			pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

			if (pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr_del) { //need camera IP was found
				printf("CameraIP was found:%d\n", nmb_Cmr_del);

				pCameraIP_loc->ReleaseGrabThread();

				delete pCameraIP_loc;

				printf("Delete from list device: %d\n", nmb_Cmr_del);

				gList_Cameras_IP.remove(*it_CameraIP);

				// it was last element
				if (cnt_cmr_IP == 1) {
					printf("Delete last device:\n");

					// just in case
					gList_Cameras_IP.clear();
					//////////////////////////////////
				}
				g_CmnTimerGrabThreads.m_SynhCS_Timer.Leave_Critical_Section();

				printf("Finish Release device by number: %d\n", nmb_Cmr_del);
				return 0;
			}

			std::advance(it_CameraIP, 1);
		}
	}
	g_CmnTimerGrabThreads.m_SynhCS_Timer.Leave_Critical_Section();
	printf("Error: Finish Release device by number: %d. Device was not found!\n", nmb_Cmr_del);

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// 
HBITMAP Load_Bmp(char *path_Bmp)
{
	//BITMAPFILEHEADER m_pBuffer;
	//BITMAPINFO m_Header;

	//FILE *file_bmp;

	/* Open for read (will fail if file "data" does not exist) */
// 	if( (file_bmp  = fopen( path_Bmp, "rb" )) == NULL ){
// 		printf( "The file was not opened\n" );
// 
// 		return -1;
// 	}

//	fread(&m_pBuffer, sizeof(BITMAPFILEHEADER), 1, file_bmp);
//	fread(&m_Header, sizeof(BITMAPINFO), 1, file_bmp);

	HINSTANCE hinst = NULL;
	UINT uType = IMAGE_BITMAP;
	int cxDesired	= 0;
	int cyDesired = 0;

	UINT fuLoad = LR_LOADFROMFILE | LR_CREATEDIBSECTION;		// LR_CREATEDIBSECTION -> create right BIT value (24/32)
	//UINT sz_base_ImageBmp = MAX_VIDEO_WIDTH * MAX_VIDEO_HEIGHT * MAX_VIDEO_BYTE_PXL;


	//загружаем BITMAP по заданному пути системы
	g_hBaseImageBMP = (HBITMAP)LoadImageA(	hinst,														// handle of the instance containing the image
																					path_Bmp,												// name or identifier of image
																					uType,														// type of image
																					cxDesired,		  // desired width
																					cyDesired,     // desired height
																					fuLoad														// load flags
 																				);
	////////////
	// получаем всю информацию о BMP
	GetObject( g_hBaseImageBMP, sizeof( g_base_ImageBmp ), &g_base_ImageBmp );

// 	// calculate BMP length
// 	int sz_lbmp_loading = m_ImageBmp_base.bmWidth * m_ImageBmp_base.bmHeight * m_ImageBmp_base.bmBitsPixel / 8;
// 	
// 	// memory which was allocated for output image has low size
// 	if (sz_lbmp_loading > m_sz_Bmp_out){
// 		m_sz_Bmp_out = sz_lbmp_loading;
// 
// 		m_Dimension_Bmp_out.cx = m_ImageBmp_base.bmWidth;
// 		m_Dimension_Bmp_out.cy = m_ImageBmp_base.bmHeight;
// 
// 		// reallocated buffer image
// 		if (m_pBuf_Bmp){
// 			free( m_pBuf_Bmp );
// 		}
// 		// create clearing buffer
// 		m_pBuf_Bmp = (char *)malloc(m_sz_Bmp_out);
// 	}

	// get body from BMP
// 	LONG buf_sz_bmp = 0;
// 	buf_sz_bmp = GetBitmapBits(	g_hBaseImageBMP,
// 															sz_base_ImageBmp,
// 															m_pBuf_Bmp
// 														);	//Copies the bit pattern of the BMP into the buffer

	//fread(m_pBuf_Bmp, sizeof(unsigned char), m_sz_Bmp_out, file_bmp); // read the rest of the data at once
//	DeleteObject(g_hBaseImageBMP);

	/* Close stream */
// 	if( fclose( file_bmp ) ){
// 		printf( "The file 'data' was not closed\n" );
// 	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
BOOL Joystick_Control( UINT nmb_Cmr, UINT state_in )
{
	if(g_NetPTZControl.IsJoyEnable()){
		return FALSE;
	}

	//printf("Joystick_Control:state_controle:%d", state_in);

	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

	CCameraIP	*pCameraIP_loc = NULL;

	// find need streaming camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
		
		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		if (	pCameraIP_loc->GetNmbCameraIP() == nmb_Cmr){ //need camera IP was found

			// SEND command to CameraIP 360 from virtual joystick
			if (pCameraIP_loc->GetFlag360()){
				UINT state_new = state_in;

				if (state_in > 0){
					// set maximum speed
					state_new |= T_ControlPTZ::SPEED_HIGH;//FORSAGE
				}

				//!!! set new PTZ data( sent packet ) !!!
				if (g_NetPTZControl.IsEnable()){	// is PTZ Control enable
					g_NetPTZControl.SetJoyControl( nmb_Cmr, state_new);
				}
			}

			return TRUE;
		}
		std::advance(it_CameraIP, 1);
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//! set paths for Video playing and recording (except folder(get from INET) and except filename)
INT SetPathsVideo(	const CHAR *path_record_in,	// video recording path		[_MAX_PATH]
										const CHAR *path_play_in		// playing path for video files	[_MAX_PATH]
									)
{
	// join paths path to save for using by creating camera
	sprintf(g_PathBase_VideoRec, path_record_in);
	sprintf(g_PathFinal_VideoRec, path_record_in);
	//sprintf(g_PathFinal_VideoRec, "%s\\%s", g_PathBase_VideoRec, g_Path_MountFolder);

	sprintf(g_PathBase_VideoPlay, path_play_in);
	sprintf(g_PathFinal_VideoPlay, path_play_in);
	//sprintf(g_PathFinal_VideoPlay, "%s\\%s", g_PathBase_VideoPlay, g_Path_MountFolder);
	//

	// TEST
	printf("SetPathsVideo()-> Begin..\n");
		printf("path_record: %s\n", path_record_in);
		printf("path_play: %s\n", path_play_in);
	printf("SetPathsVideo()-> ...End\n");
	//
	////////////////////////
	g_CmnTimerGrabThreads.m_SynhCS_Timer.Set_Critical_Section();
	{
		// set video recording path
		int cnt_cmr_IP = gList_Cameras_IP.size();
		IterCameraIP it_CameraIP = gList_Cameras_IP.end();
		CCameraIP	*pCameraIP_loc = NULL;

		// find need streaming camera
		for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
			it_CameraIP = gList_Cameras_IP.begin();

			std::advance(it_CameraIP, nmb_cmr_loc);
			pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

			pCameraIP_loc->SetPathRecording((const char*)g_PathFinal_VideoRec);
		}
	}
 	g_CmnTimerGrabThreads.m_SynhCS_Timer.Leave_Critical_Section();
	////////////////////////

	// set playing path for video files
	unsigned int cam_files = 0;

	cam_files = g_MoviesHDD.ParseFolder(path_play_in);

	return cam_files;
}

//////////////////////////////////////////////////////////////////////////
// set paths for Audio playing and recording (except folder(get from INET) and except filename)
BOOL SetPathsAudio(	const CHAR *path_record_in,	// audio recording path		[_MAX_PATH]
										const CHAR *path_play_in		// playing path for audio files	[_MAX_PATH]
									)
{
	// join paths path to save for using by creating camera
	sprintf(g_PathBase_AudioRec, path_record_in);
	sprintf(g_PathFinal_AudioRec, path_record_in);
	//sprintf(g_PathFinal_AudioRec, "%s\\%s", g_PathBase_AudioRec, g_Path_MountFolder);

	sprintf(g_PathBase_AudioPlay, path_play_in);
	sprintf(g_PathFinal_AudioPlay, path_play_in);
	//sprintf(g_PathFinal_AudioPlay, "%s\\%s", g_PathBase_AudioPlay, g_Path_MountFolder);
	//

	// TEST
	printf("SetPathsAudio()-> Begin..\n");
		printf("path_record: %s\n", path_record_in);
		printf("path_play: %s\n", path_play_in);
	printf("SetPathsAudio()-> ...End\n");
	//

	if (g_AudioData.bEnable){
		if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetPath(g_PathFinal_AudioPlay);
		}else{
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetPath(g_PathFinal_AudioRec);
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//! set the listening port (TCP protocol) for the control writing IP video
// set in application just one
BOOL SetPort_VideoControl(	const UINT port_in		//  port for the listening
													)
{
	g_NetControlReceive.InitGetCommandNet(port_in);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//! set current camera(for right control (joystick/mouse))
BOOL SetCurrentCamera(	const UINT nmb_cur_Cmr_in		// the current number of camera [0...N]
											)
{
	g_NetPTZControl.SetNmbCam(nmb_cur_Cmr_in);

	//joining PTZ camera with control
	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.end();

	CCameraIP	*pCameraIP_loc = NULL;

	// find only PTZ camera
	for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
		it_CameraIP = gList_Cameras_IP.begin();

		std::advance(it_CameraIP, nmb_cmr_loc);
		pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

		if (	pCameraIP_loc->GetNmbCameraIP() == nmb_cur_Cmr_in){ //need camera IP was found

			if (pCameraIP_loc->GetFlag360()){
				g_NetPTZControl.SetCameraPTZ_Control(pCameraIP_loc);
				break;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// define ID camera by using the joystick number
// "-1" - camera was not found
int GetCmrIDfromJoy( UINT nmb_btn_joy	// device number (was set by pushing joystick button)
										)
{
	int cnt_cmr_IP = gList_Cameras_IP.size();

	if (cnt_cmr_IP < nmb_btn_joy +1){
		return 0;
	}

	IterCameraIP it_CameraIP_loc = gList_Cameras_IP.begin();
	IterCameraIP it_CameraIP_end = gList_Cameras_IP.end();

	CCameraIP	*pCameraIP_loc = NULL;

	if (cnt_cmr_IP){

		std::advance(it_CameraIP_loc, nmb_btn_joy);

		if (it_CameraIP_end != it_CameraIP_loc){
			pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP_loc);

			if (pCameraIP_loc){
				return pCameraIP_loc->GetNmbCameraIP();
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// release all IP cameras
void ReleaseAllCamerasIP( void )
{
	int nmb_del_device = 0;
	g_CmnTimerGrabThreads.m_SynhCS_Timer.Set_Critical_Section();
	{
		// delete all Cameras IP
		int cnt_cmr_IP = gList_Cameras_IP.size();
		IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

		CCameraIP	*pCameraIP_loc = NULL;

		// find need streaming camera
		for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc) {

			pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

			nmb_del_device = pCameraIP_loc->GetNmbCameraIP();

			printf("Start Release device by number: %d\n", nmb_del_device);
			{
				pCameraIP_loc->ReleaseGrabThread();
				delete pCameraIP_loc;
			}
			printf("Finish Release device by number: %d\n", nmb_del_device);

			gList_Cameras_IP.remove(*it_CameraIP);
			nmb_cmr_loc = -1;
			cnt_cmr_IP = gList_Cameras_IP.size();
			////////////////
			// it was last element
			if (cnt_cmr_IP == 0) {
				break;
			}
			////////////////
			std::advance(it_CameraIP, 1);
		}
		gList_Cameras_IP.clear();
	}
	g_CmnTimerGrabThreads.m_SynhCS_Timer.Leave_Critical_Section();
}
//////////////////////////////////////////////////////////////////////////
// set maximum time for recording all movies
BOOL SetTimeRecMinutes(	const UINT mimutes_in				// time [min] for recording single movie
											)
{
	g_MaxTimeRecMinutes = mimutes_in;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//! set beginning time to play for file movie (for all objects camera)
BOOL SetTimePlay(	const UINT time_play_msec	// set time for playing video	[msec]
								)
{
		printf("SetTimePlay()\n");

	// delete all Cameras IP
	int cnt_cmr_IP = gList_Cameras_IP.size();

	if (!cnt_cmr_IP){	// defence from block thread(using CSharp wrapping method)
		return -1;
	}
	UINT time_play_msec_mod = time_play_msec;
	/*if(time_play_msec_mod <= 1000){
		time_play_msec_mod = 1200;
	}
*/
	IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

	CCameraIP	*pCameraIP_loc = NULL;

	/// control Audio
	unsigned int cnt_Audio_obj = 0;
	if (g_AudioData.bEnable){
		if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
			cnt_Audio_obj = ((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->GetCountStreams();
		}else{
		//just in case
		//if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
			cnt_Audio_obj = ((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->GetCountStreams();
		}
	}
	///

	unsigned int cnt_obj_bar = cnt_cmr_IP + 1 + 1 + cnt_Audio_obj;	// "+1" - it is my thread + 1 (thread: CCommonTimerThread)

	//////////////////////////////////////////////////////////////////////////
	// command for all cameras
	//g_StateMovie = T_DeviceState::PLAY;

	// set numbers thread for barrier using
	g_Synh_Barrier_StateCam_1.SetNmbBarrier(cnt_obj_bar);
	g_Synh_Barrier_StateCam_2.SetNmbBarrier(cnt_obj_bar);

	//////////////////////////////////////////////////////////////////////////
	// !!! need set numbers threads + 1 before barrier using !!!

	// set synchronize camera using
	InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 1);
	{
		// block the first ones(align all threads to "the first point")
		g_Synh_Barrier_StateCam_1.Block();
		{

			// find need streaming camera
			for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
				pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

				// Set state of flag for movie play
				//pCameraIP_loc->SetControlFlag(g_StateMovie);

				pCameraIP_loc->Set_TimeStartMovie(time_play_msec_mod);
				std::advance(it_CameraIP, 1);
			}

			//////////////////////////////////////////////////////////////////////////
			//sound control
			if (g_AudioData.bEnable){
				if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
					((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->Set_TimeStartTrack(time_play_msec_mod);
				}
			}
			//////////////////////////////////////////////////////////////////////////
		}
		InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 0);

		// !!!unblock all stopped threads!!! ("second point" )
		g_Synh_Barrier_StateCam_2.Block();
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////
// It is not using
BOOL GetTotalTimePlay(	UINT &total_time_out 	// total time video files for playing	[msec]
											)
{
	total_time_out = (UINT)g_MoviesHDD.GetLengthMovie(0);

	if (total_time_out != -1){
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// for Sergey
void RenameFilesinFolder(UINT flag_wrt)
{
	static UINT old_state = 0;

	HANDLE FindHandle;
	WIN32_FIND_DATAA FindData;
	char	path_old[_MAX_PATH];

	int nmb_file = 1;
	char new_nm_file[_MAX_PATH];
	//char folder_file_loc[_MAX_PATH];

	if( ( (old_state == T_DeviceState::WRITE) && (flag_wrt == T_DeviceState::STOP) )
				||
			( (old_state == T_DeviceState::PAUSE) && (flag_wrt == T_DeviceState::STOP) )
		){
			//////////////////////////////////////////////////////////////////////////
			Sleep(250);
			// get total file folder in it
// 			sprintf(folder_file_loc, "%s\\*.avi", g_Path_VideoRec);
// 
// 			FindHandle = FindFirstFileA(folder_file_loc, &FindData);
// 			nmb_file = 0;
// 			//searching files in current folder
// 			do{
//  				if (strstr(FindData.cFileName, ".avi")){
// 					nmb_file++;
// 				}
// 			} while(FindNextFileA( FindHandle, &FindData ));
			//////////////////////////////////////////////////////////////////////////
			int cnt_cmr_IP = gList_Cameras_IP.size();

			unsigned int cnt_obj_bar = cnt_cmr_IP + 1;	// "+1" - it is my thread

			g_Synh_Barrier_RenameFiles_1.SetNmbBarrier(cnt_obj_bar);
			g_Synh_Barrier_RenameFiles_2.SetNmbBarrier(cnt_obj_bar);

			// set synchronize camera using
			InterlockedExchange(&g_Synh_Barrier_RenameFiles_1.m_InterLock_flag, 1);
			{
				g_Synh_Barrier_RenameFiles_1.Block();
				{
					GetCurrentDirectoryA(_MAX_PATH, path_old);

					SetCurrentDirectoryA(g_PathFinal_VideoRec);	/* _getcwd(path_reg, LNG_MSG); */

					FindHandle = FindFirstFileA(NAME_VIDEO_REC_FIND, &FindData);
					nmb_file = 1;
					//searching files in current folder
					do{
						if (strstr(FindData.cFileName, NAME_VIDEO_REC_TAIL)){
							itoa(nmb_file, new_nm_file, 10);

							int len = strlen(new_nm_file);
							sprintf(new_nm_file + len, NAME_VIDEO_REC_TAIL);

							rename(FindData.cFileName, new_nm_file);

							nmb_file++;
						}

					} while(FindNextFileA( FindHandle, &FindData ));

					//close the specified search handle
					FindClose(FindHandle);

					SetCurrentDirectoryA(path_old);
				}
			}
			InterlockedExchange(&g_Synh_Barrier_RenameFiles_1.m_InterLock_flag, 0);
			g_Synh_Barrier_RenameFiles_2.Block();
	}
	old_state = flag_wrt;
}

//////////////////////////////////////////////////////////////////////////
 //! set manual control for player (use Slider or TCP client)
 const INT SetPlayerManualControl(	const BOOL flag	// state flag
																)
 {
	 return 0;
 }

//////////////////////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////
//! get state video stream for each device
const LONG GetStateCamera(	const UINT nmb_Cmr		// number of current camera[0 ... N]
												)
{
	printf("GetStateCamera\n");

	int cnt_cmr_IP = gList_Cameras_IP.size();
	IterCameraIP it_CameraIP = gList_Cameras_IP.begin();

	if (!cnt_cmr_IP){	// defence from block thread(using CSharp wrapping method)
		return -1;
	}

	CCameraIP	*pCameraIP_loc = NULL;
	/////	control Audio
	unsigned int cnt_Audio_obj = 0;

	if (g_AudioData.bEnable){
		if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
			cnt_Audio_obj = ((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->GetCountStreams();
		}else{
		//if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
			cnt_Audio_obj = ((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->GetCountStreams();
		}
	}
	/////
	unsigned int cnt_obj_bar = cnt_cmr_IP + 1 + 1 + cnt_Audio_obj;	// "+1" - it is my thread + 1 (thread: CCommonTimerThread)

	// !!! need set numbers threads "+1" it is my thread "+1" (thread: CCommonTimerThread) !!!
	// set numbers thread for barrier using
	g_Synh_Barrier_StateCam_1.SetNmbBarrier(cnt_obj_bar);
	g_Synh_Barrier_StateCam_2.SetNmbBarrier(cnt_obj_bar);

	//////////////////////////////////////////////////////////////////////////
	unsigned long state_rec_loc = -1;

	//////////////////////////////////////////////////////////////////////////
	// set synchronize camera using
	InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 1);
	{
		// block the first ones(align all threads to "the first point")
		g_Synh_Barrier_StateCam_1.Block();
		{
			it_CameraIP = gList_Cameras_IP.begin();
			// reseach all streaming camera
			for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){

				pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

				if( (nmb_Cmr == nmb_cmr_loc)			// camera was found
						||
						(nmb_Cmr == -1)								// command for the first camera
					){
					state_rec_loc = pCameraIP_loc->GetStateRec_SDK();
					break;
				}

				std::advance(it_CameraIP, 1);
			}//for (int nmb_cmr_loc = 0;)
		}
		InterlockedExchange(&g_Synh_Barrier_StateCam_1.m_InterLock_flag, 0);

		// !!!unblock all stopped threads!!! ("second point" )
		g_Synh_Barrier_StateCam_2.Block();
	}
	return state_rec_loc;
	//////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////
// set Audio device using(for the playing and recording)
INT SetAudioDevice(	UINT	device_nmb_in,	// device number [0-7]
										BOOL	flag_enable_in	// flag enabling device	(TRUE / FALSE)
									)
{
	if (g_AudioData.bEnable){

		if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetAudioDevice(device_nmb_in, flag_enable_in);
		}else{
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetAudioDevice(device_nmb_in, flag_enable_in);
		}
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
// set type interface (playing or recording)
INT SetTypeInterface(	UINT	type_intarface	// type interface (T_DeviceState::PLAY === 1) || (T_DeviceState::WRITE === 3)
										)
{
	// management of AUDIO
	if (type_intarface == T_DeviceState::PLAY){

		if (g_AudioData.bEnable){
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->Disconnect();
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->Connect();
		}
		printf("SetTypeInterface()-> T_DeviceState::PLAY\n");

		InterlockedExchange(&g_AudioData.lTypeFunc, (LONG)T_DeviceState::PLAY);
	}else{

		if (g_AudioData.bEnable){
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->Disconnect();
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->Connect();
		}

		printf("SetTypeInterface()-> T_DeviceState::WRITE\n");

		// set value by atomic operation
		InterlockedExchange(&g_AudioData.lTypeFunc, (LONG)T_DeviceState::WRITE);
	}

	return type_intarface;
}
//////////////////////////////////////////////////////////////////////////
// set folder name for Audio and Video recording (get from INET)
BOOL SetFolderNet(	const CHAR *name_fld_in	// name folder [32]
									)
{
	// save name folder for the mount
	sprintf(g_Path_MountFolder, name_fld_in);

	// join paths path to save for using by creating camera
	sprintf(g_PathFinal_AudioRec, "%s\\%s", g_PathBase_AudioRec, g_Path_MountFolder);
	sprintf(g_PathFinal_AudioPlay, "%s\\%s", g_PathBase_AudioPlay, g_Path_MountFolder);

	// join paths path to save for using by creating camera
	sprintf(g_PathFinal_VideoRec, "%s\\%s", g_PathBase_VideoRec, g_Path_MountFolder);
	sprintf(g_PathFinal_VideoPlay, "%s\\%s", g_PathBase_VideoPlay, g_Path_MountFolder);
	//

	// TEST
	printf("SetFolderNet()-> Begin..\n");
		printf("MOUNT folder(TCP): %s\n", g_Path_MountFolder);
		printf("PathFinal_AudioRec: %s\n", g_PathFinal_AudioRec);
		printf("PathFinal_AudioPlay: %s\n", g_PathFinal_AudioPlay);

		printf("PathFinal_VideoRec: %s\n", g_PathFinal_VideoRec);
		printf("PathFinal_VideoPlay: %s\n", g_PathFinal_VideoPlay);
	printf("SetFolderNet()-> ...End\n");
	//
	////////////////////////
	// FOR AUDIO
	//////////////////////////////////////////////////////////////////////////
	if (g_AudioData.bEnable){
		if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->SetPath(g_PathFinal_AudioPlay);
		}else{
			((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->SetPath(g_PathFinal_AudioRec);
		}
	}

	// there was opened ARCHIVE 
	if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
		// translate Control for an out of interface using
		if (p_SetMovieFolder){
			CHAR folder_name[_MAX_PATH];
			sprintf_s(folder_name, _MAX_PATH, /*g_PathFinal_VideoPlay*/name_fld_in);
			p_SetMovieFolder(folder_name, strlen(folder_name));

			//return TRUE;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// FOR VIDEO
	////////////////////////
	g_CmnTimerGrabThreads.m_SynhCS_Timer.Set_Critical_Section();
	{
		// set video recording path
		int cnt_cmr_IP = gList_Cameras_IP.size();
		IterCameraIP it_CameraIP = gList_Cameras_IP.end();
		CCameraIP	*pCameraIP_loc = NULL;

		// find need streaming camera
		for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){
			it_CameraIP = gList_Cameras_IP.begin();

			std::advance(it_CameraIP, nmb_cmr_loc);
			pCameraIP_loc = static_cast<CCameraIP*>(*it_CameraIP);

			pCameraIP_loc->SetPathRecording((const char*)g_PathFinal_VideoRec);
		}
	}
	g_CmnTimerGrabThreads.m_SynhCS_Timer.Leave_Critical_Section();

	return TRUE;
}
#include "StdAfx.h"
#include "CameraIP.h"

#include "TestTime.h"

#include "MetaMoviesHDD.h"

#include <Winsock2.h>
#include <direct.h>




#include "../AudioStreamer/AudioStreamerData.h"
extern AudioDevice::AudioData g_AudioData;

CTestTime	g_TimeStep;

CLic	g_Licence;

// player OCV
//http://www.codeproject.com/Articles/339206/An-Introduction-to-OpenCV-Displaying-and-Manipulat
//http://stackoverflow.com/questions/15332446/playing-a-video-in-opencv


//https://forum.videolan.org/viewtopic.php?f=32&t=87031&p=401480&hilit=OpenCV#p401480

//events
//http://stackoverflow.com/questions/26459580/stream-camera-into-memory-using-libvlc-and-display-a-frame
extern Set_PlayerState p_Set_PlayerState;

//////////////////////////////////////////////////////////////////////////
DWORD /*WINAPI */CCameraIP::Func_Grab_Thread(LPVOID lpParam)
{

	CCameraIP *p_cmr_IP = (CCameraIP *)lpParam;

	// just in case
	if (!p_cmr_IP){
		return 1;
	}else{
		if( !p_cmr_IP->m_pSynh_Bar_StCam_1 /*|| !p_cmr_IP->m_pSynh_Bar_RnmFile_1*/
				||
				!p_cmr_IP->m_pSynh_Bar_StCam_2 /*|| !p_cmr_IP->m_pSynh_Bar_RnmFile_2*/
			){
			return 1;
		}
	}

	InterlockedExchange(&p_cmr_IP->m_FlagGrabbing_Thread, 1);

	DWORD waitResult = WAIT_OBJECT_0;

	float current_FPS = 0;
	unsigned int state_rec_loc = 0;
	ULONG	time_save_video_sec = 0; // 

	const void* pBuf_grab = NULL;
	bool res_params = true;

	LONGLONG cnt_frames_wrt = 0;			// frames was written
	LONGLONG cnt_frames_start = 0;	// initial number frames writting before

	DWORD dwWaitResult = 0;

//	if (p_cmr_IP->IsOpenedStreamSDK()){

	float fps_loc = 0;

	// calculate video frame for finish writing
	time_save_video_sec = p_cmr_IP->m_total_frames_save_video;

	LONG cnt_atom = 0;
	bool flag_change_atom = false;

	LONGLONG	cnt_real_frames = 0;
	LONGLONG frames_lost = 0;
	LONGLONG frm_wr = 0;

	bool flag_cnt_frames_save = false;	

	while (InterlockedCompareExchange(&p_cmr_IP->m_FlagGrabbing_Thread, 1,1)){

		if (InterlockedCompareExchange(&p_cmr_IP->m_pSynh_Bar_StCam_1->m_InterLock_flag, 1,1)){
			printf("Block begin(Cam:%d)...\n", p_cmr_IP->m_nmb_Cmr);
			// stop thread to align "the first point"
			p_cmr_IP->m_pSynh_Bar_StCam_1->Block();

			;// something happen to another thread

			// stop thread to align "second point"   (!!!unblock another threads!!!)
			p_cmr_IP->m_pSynh_Bar_StCam_2->Block();
			printf("...Block end(Cam:%d)\n", p_cmr_IP->m_nmb_Cmr);
		}

		try
		{
			//set synh. flag -> control the recording state(STOP/PAUSE/WRITE)
			p_cmr_IP->Set_CS_StateRec();
			{
				if (InterlockedCompareExchange(&p_cmr_IP->m_flag_grabber, 1,1)){
					try
					{

						p_cmr_IP->Set_CS_TimerGrabber();		//!!! Sync.grab thread(CameraIP)  [12.02.15]
						{
							// turning sleeping for the grabber
							res_params = p_cmr_IP->Get3ParamCmr(current_FPS, (unsigned int&)(p_cmr_IP->m_Dimension_Img_out.cx), (unsigned int&)p_cmr_IP->m_Dimension_Img_out.cy);
				
							pBuf_grab = p_cmr_IP->Grab_Step_SDK();

							// just in case
				
							if (p_cmr_IP->IsEnable_SDK()){
								p_cmr_IP->m_pBuf_Img_Out = (void*)pBuf_grab;
							}
						}

						p_cmr_IP->Leave_CS_TimerGrabber();		//!!! Sync.grab thread(CameraIP)  [12.02.15]

						// just in case
						if (!( pBuf_grab && res_params && p_cmr_IP->m_Dimension_Img_out.cx && p_cmr_IP->m_Dimension_Img_out.cy) ){
							// leave synh. flag for writing state
							p_cmr_IP->Leave_CS_StateRec();
							Sleep(1);

							continue;
						}
					}
					catch (char *ex_str)
					{
						printf("p_cmr_IP->Set_CS_StateRec(): %s\n", ex_str);

						p_cmr_IP->Leave_CS_TimerGrabber();		//!!! Sync.grab thread(CameraIP)  [12.02.15]
						p_cmr_IP->Leave_CS_StateRec();
					}

					//current_FPS = (int)p_cmr_IP->m_OCV_capture.get(CV_CAP_PROP_FPS);	// Get Frame rate for each frame

					if (p_cmr_IP->IsEnable_SDK()){
						p_cmr_IP->m_pBuf_Img_Out = (void*)pBuf_grab;
					}
		
					///////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////
					if (p_cmr_IP->m_Type_Stream == Type_Stream::IP_STREAM){
						//Type_Stream::IP_STREAM
						switch (p_cmr_IP->m_WriteState)
						{
								// Write to HDD
							case T_DeviceState::WRITE:
								{
									//////////////////////////////////////////////////////////////////////////
									if (!flag_cnt_frames_save){
										flag_cnt_frames_save = true;
									}

									p_cmr_IP->m_SynhCS_CntFrames.Set_Critical_Section();
									{
										// set initial number frames for writing 
 										if (!cnt_frames_start){
 											cnt_frames_start = p_cmr_IP->m_lNmbFrame4Wrt_CmnTm;

											printf("p_cmr_IP->m_lNmbFrame4Wrt_CmnTm: %llu \n", p_cmr_IP->m_lNmbFrame4Wrt_CmnTm);
 										}
								
										frames_lost = labs( (p_cmr_IP->m_lNmbFrame4Wrt_CmnTm - cnt_frames_start) - cnt_frames_wrt);
									}
									p_cmr_IP->m_SynhCS_CntFrames.Leave_Critical_Section();
									//////////////////////////////////////////////////////////////////////////

									// save current frame and other if it was lost
									for (frm_wr = 0; frm_wr < frames_lost; ++frm_wr){

										// the writing streaming frame
										if (p_cmr_IP->WriteFrameMPEG_SDK()){

											++p_cmr_IP->m_cnt_frames_save;

											// control AUDIO recording
											if( InterlockedCompareExchange(&p_cmr_IP->m_flag_AudioControl, 1,1) ){
												p_cmr_IP->AudioControl(p_cmr_IP->m_cnt_frames_save);
											}

											//test 
											//printf("Cmr:%d: %u\n", p_cmr_IP->GetInfoAddress()->IP[3], p_cmr_IP->m_cnt_frames_save);
											//

											// save count frames was written		// lose frame was counted
											++cnt_frames_wrt;

											///// examine the state of recording
											state_rec_loc = p_cmr_IP->GetStateRec_SDK();	// realtime state recording

											if ( (state_rec_loc != p_cmr_IP->m_WriteState_old)
														&&
														!frm_wr	//	just only the first lost frame
													){
												if (p_Set_PlayerState){

													// run callback function up to *.EXE
													p_Set_PlayerState(p_cmr_IP->m_nmb_Cmr, state_rec_loc);

													// save new state recording
													p_cmr_IP->m_WriteState = state_rec_loc;
												}

												p_cmr_IP->m_WriteState_old = p_cmr_IP->m_WriteState;
											}

											//////////
											// find the time limit for save video
											--time_save_video_sec;

											if (time_save_video_sec <= 0 ){

												// finish streaming was time is out
												p_cmr_IP->FinishWritingMPEG_SDK();

												// increase version number recording
												InterlockedIncrement(&p_cmr_IP->nmb_ver_rec);			//	p_cmr_IP->nmb_ver_rec ++;

												// set a new recording name
												p_cmr_IP->ChangeNameRecording();

												//ones more launch writing stream
												p_cmr_IP->WriteFrameMPEG_SDK();
												time_save_video_sec = p_cmr_IP->m_total_frames_save_video;
											}
										}
									}
								}
								break;

							case T_DeviceState::STOP:
								{
									//////////////////////////////////////////////////////////////////////////
									if (flag_cnt_frames_save){

										printf("Total_Frames_Video[Cmr:%d]: %u (Sec: %f)\n", p_cmr_IP->GetInfoAddress()->IP[3], p_cmr_IP->m_cnt_frames_save, p_cmr_IP->m_cnt_frames_save/25.0);
										p_cmr_IP->m_cnt_frames_save = 0;

										flag_cnt_frames_save = false;
									}

									//////////////////////////////////////////////////////////////////////////
									// the finish streaming frame
									p_cmr_IP->FinishWritingMPEG_SDK();

									///// examine the state of recording
									state_rec_loc = p_cmr_IP->GetStateRec_SDK();

									if (state_rec_loc != p_cmr_IP->m_WriteState_old){

										if (p_Set_PlayerState){

											// run callback function up to *.EXE
											p_Set_PlayerState(p_cmr_IP->m_nmb_Cmr, state_rec_loc);

											// save new state recording
											p_cmr_IP->m_WriteState = state_rec_loc;
										}
										// increase version number recording
										InterlockedIncrement(&p_cmr_IP->nmb_ver_rec);			//	p_cmr_IP->nmb_ver_rec ++

										// set a new recording name
										p_cmr_IP->ChangeNameRecording();

										p_cmr_IP->m_WriteState_old = p_cmr_IP->m_WriteState;
									}
									time_save_video_sec = p_cmr_IP->m_total_frames_save_video;
									/////

									//////////////////////////////////////////////////////////////////////////
									// !!!garbage!!!
									if (InterlockedCompareExchange(&p_cmr_IP->m_pSynh_Bar_RnmFile_1->m_InterLock_flag, 1,1)){
										// stop thread to align "the first point"
										p_cmr_IP->m_pSynh_Bar_RnmFile_1->Block();

										;// something happen to another thread

										// stop thread to align "second point"   (!!!unblock another threads!!!)
										p_cmr_IP->m_pSynh_Bar_RnmFile_2->Block();
									}
									//////////////////////////////////////////////////////////////////////////
									cnt_frames_wrt = 0;
									cnt_frames_start = 0;
								}
								break;

							case T_DeviceState::PAUSE:
								{
									cnt_frames_wrt = 0;
									cnt_frames_start = 0;
								}
								break;

							case T_DeviceState::PLAY:
								{
									int r = 8;
								}
								break;

							default: break;
						}
					}

					///////////////////////////////////////////////////////////////
					// set value by atomic operation
					InterlockedExchange(&p_cmr_IP->m_flag_grabber, 0);
				}else{
					Sleep(1);
				}
			}
			// leave synh. flag for writing state
			p_cmr_IP->Leave_CS_StateRec();
		}
		catch (char *ex_str)
		{

			printf("p_cmr_IP->Set_CS_StateRec(): %s\n", ex_str);
			// 
			// leave synh. flag for writing state
			p_cmr_IP->Leave_CS_StateRec();
		}

	}//while(p_cmr_IP->m_FlagGrabbing_Thread)
	InterlockedExchange(&p_cmr_IP->m_FlagGrabbing_Thread, 0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
CCameraIP::CCameraIP(	long type_read_module_in,
											long type_wr_module_in,
											long type_stream_in
										):
			m_Type_Read_Module( type_read_module_in),
			m_Type_Write_Module( type_wr_module_in ),
			m_Type_Stream( type_stream_in )
{
// 	m_Type_Read_Module = Type_SDK_API::VLC;
// 	m_Type_Write_Module = Type_SDK_API::OPEN_CV;
// 	m_Type_Stream = Type_Stream::FILE_STREAM;

	m_nmb_Cmr = -1;
	m_pBuf_Bmp = NULL;
	m_pBuf_Img_Out = NULL;
	m_sz_Img_out = 0;
	m_MAX_Size_Buf = 0;

	m_nmb_Processor = -1;

	m_BitPerPxl_Img = 24;

	m_hGrabThread = NULL;
	m_dwGrabThreadId = -1;

	m_hImageBMP_base = NULL;

	m_WriteState = T_DeviceState::STOP;
	m_WriteState_old = T_DeviceState::STOP;

	sprintf(m_filename_rec, "");
	sprintf(m_folder_rec, "");
	sprintf(m_short_path_rec, "");
	sprintf(m_fullname_rec, "");

	sprintf(path_stream, "111");

	m_Flag_360 = FALSE;

	InterlockedExchange(&nmb_ver_rec, 0);

	InterlockedExchange(&m_FlagGrabbing_Thread, 0);

	m_SynhCS_CntFrames.Set_Critical_Section();
	{
		InterlockedExchange64(&m_lCntFrame_CmnTimer, 0);
		InterlockedExchange64(&m_lNmbFrame4Wrt_CmnTm, 0);
		InterlockedExchange(&m_flag_AudioControl, 0);

		m_dFreqTicks_CmnTimer = 0.0;

		m_dLastTick_CmnTimer = 0.0;

		InterlockedExchange(&m_flag_grabber, 0);
	}
	m_SynhCS_CntFrames.Leave_Critical_Section();

	m_pSynh_Bar_StCam_1 = NULL;
	m_pSynh_Bar_RnmFile_1 = NULL;

	m_pSynh_Bar_StCam_2 = NULL;
	m_pSynh_Bar_RnmFile_2 = NULL;

	m_time_save_video = /*Minute*/(SAVE_VIDEO_MINUTES * 60) + /*Second*/(0) ;

	m_total_frames_save_video = m_time_save_video * 40;	// 25 Hz	// base calculate video frames for finish writing


	m_cnt_frames_save = 0;


	ZeroMemory((void*) &m_VideoParams, sizeof(m_VideoParams));
	ZeroMemory((void*) &m_VideoParams_last, sizeof(m_VideoParams_last));
	
	ZeroMemory((void*) &m_Dimension_Img_out, sizeof(m_Dimension_Img_out));
	ZeroMemory((void*) &m_MAX_Dimension_Buf, sizeof(m_MAX_Dimension_Buf));
	ZeroMemory((void*) &m_NetAddress, sizeof(m_NetAddress));
	ZeroMemory((void*) &m_hCrSctn_TimerGrab, sizeof(m_hCrSctn_TimerGrab));
//	ZeroMemory((void*) &m_hCrSctn_StateRec, sizeof(m_hCrSctn_StateRec));
}

CCameraIP::~CCameraIP(void)
{
/////	m_VLC.Release();

	//if (m_Type_Stream == Type_Stream::IP_STREAM ){
		m_OpenCV.Release();
	//}else{
		m_OpenCV_Player.Release();
//	}

	if (m_pBuf_Bmp){
		free( m_pBuf_Bmp );
	}

	m_pBuf_Img_Out = NULL;

	ReleaseGrabThread();
}

//////////////////////////////////////////////////////////////////////////
// reallocate output BMP buffer
int CCameraIP::CreateBufferImage(	const unsigned int dim_X,			// width image
																	const unsigned int dim_Y,			// height image
																	const unsigned int byte_pxl		// byte per pixel for image
																)
{
	// save buffer(output) dimensions
	if( (dim_X < MAX_VIDEO_WIDTH)){
		m_MAX_Dimension_Buf.cx = MAX_VIDEO_WIDTH;
	}else{
		m_MAX_Dimension_Buf.cx = dim_X;
	}
	if( (dim_Y < MAX_VIDEO_HEIGHT)){
		m_MAX_Dimension_Buf.cy = MAX_VIDEO_HEIGHT;
	}else{
		m_MAX_Dimension_Buf.cy = dim_Y;
	}

	// save values bit per pixels
	m_BitPerPxl_Img = byte_pxl*8;

	// calculate maximum BMP length for output
	m_MAX_Size_Buf = m_MAX_Dimension_Buf.cx * m_MAX_Dimension_Buf.cy * byte_pxl;

	// reallocated buffer image
	if (m_pBuf_Bmp){
		free( m_pBuf_Bmp );
		m_pBuf_Bmp = NULL;
	}

	// allocate memory for BMP buffer
	m_pBuf_Bmp = (char *)malloc(m_MAX_Size_Buf);
	///

	// update buffers for inherited modules
/////	m_VLC.Release();
/////	m_VLC.Init(m_MAX_Dimension_Buf.cx, m_MAX_Dimension_Buf.cy, MAX_VIDEO_4_BYTE_PXL);

	if (m_Type_Stream == Type_Stream::IP_STREAM ){
		m_OpenCV.Release();
		m_OpenCV.Init(m_MAX_Dimension_Buf.cx, m_MAX_Dimension_Buf.cy, MAX_VIDEO_3_BYTE_PXL);
	}else{
		m_OpenCV_Player.Release();
		m_OpenCV_Player.Init(m_MAX_Dimension_Buf.cx, m_MAX_Dimension_Buf.cy, MAX_VIDEO_3_BYTE_PXL);
	}

	// save values for output
	m_Dimension_Img_out = m_MAX_Dimension_Buf;
	m_sz_Img_out = m_MAX_Size_Buf;
	m_BitPerPxl_Img = 24;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set number camera in system
void	CCameraIP::SetNmbCameraIP(unsigned int nmb_cmr_in)
{
// 	if (m_nmb_Cmr != -1){
// 		return;
// 	}

	m_nmb_Cmr = nmb_cmr_in;

// 	if (m_Type_Stream == Type_Stream::IP_STREAM ){
// 		// create finishing full path recording
//  		CreatePathRecording();
// 
//  		// set name for recording class
//  		SetNameVideoRec_SDK(m_fullname_rec);
// 	}
}

//////////////////////////////////////////////////////////////////////////
// create stream IP using path stream of the camera
//bool CCameraIP::Create_StreamIP(char *path_strm_in)
bool CCameraIP::Create_StreamIP(T_NetAddress	*net_addr)
{

	if (!net_addr){
		return false;
	}
	char path_stream_loc[_MAX_PATH] = "";

	int nmb_sign_str = 0;
	//add protocol

	if (strlen(net_addr->Header)){
		nmb_sign_str = sprintf(path_stream_loc, "%s", net_addr->Header);
	}

	//add login and password
	if (strlen(net_addr->Login) || strlen(net_addr->Password)){
		nmb_sign_str += sprintf(path_stream_loc + nmb_sign_str, "%s:%s@", net_addr->Login, net_addr->Password);
	}

	//add login and password
	nmb_sign_str += sprintf(path_stream_loc + nmb_sign_str, "%d.%d.%d.%d", net_addr->IP[0], net_addr->IP[1], net_addr->IP[2], net_addr->IP[3]);

	//add port
	if (!net_addr->Port){
		nmb_sign_str += sprintf(path_stream_loc + nmb_sign_str, ":80");
	}else{
		nmb_sign_str += sprintf(path_stream_loc + nmb_sign_str, ":%d", net_addr->Port);
	}
	// add tail
	if (strlen(net_addr->Tail)){
		nmb_sign_str += sprintf(path_stream_loc + nmb_sign_str, "%s", net_addr->Tail);
	}
	if (!strcmp(path_stream, path_stream_loc)){	// paths are identical
		return IsOpenedStreamSDK();
	}

	//////////////////////////////////////////////////////////////////////////

// 	if (m_nmb_Cmr == 0){
// 		//m_OCV_capture.open("rtsp://admin:1234@192.168.0.99:554/live/h264");		//H264
// 
// 		m_OCV_capture.open("Video_OLS_2.avi");
// 	}else{
// 		//m_OCV_capture.open("rtsp://192.168.0.98:554/live/main");							//H264
// 
// 		m_OCV_capture.open("1.avi");
//	rtsp://admin:1234@192.168.0.149/profile2/media.smp
// 	}
	// for test http://user:pass@cam_address:8081/cgi/mjpg/mjpg.cgi?.mjpg

	try
	{
		/////
		/**/
		//add login and password
		char ip_str[_MAX_PATH] = "";
		char port_str[_MAX_PATH] = "";

		sprintf(ip_str, "%d.%d.%d.%d", net_addr->IP[0], net_addr->IP[1], net_addr->IP[2], net_addr->IP[3]);

		if (!net_addr->Port) {
			sprintf(port_str, "80");
		} else {
			sprintf(port_str, "%d", net_addr->Port);
		}
		
		// include check for connection by ONVIF protocol (OpenCV - has during time by cameraIP connection ~30 second )
		m_ConnectOnvif.Init(ip_str, port_str, net_addr->Tail);

		m_ConnectOnvif.Terminate();

		bool res_ONVIF_connect = (bool)m_ConnectOnvif.GetFlagConnect();

		if (!res_ONVIF_connect) {
			return false;
		}
		
		/////

		if (Create_StreamIP_SDK(path_stream_loc, m_nmb_Cmr)){

			CopyMemory((void*)&m_NetAddress, (void*)net_addr, sizeof(m_NetAddress));
				
			sprintf_s(path_stream, _MAX_PATH, path_stream_loc);

			//////////////////////////////////////////////////////////////////////////
			// get needed parameters for timer grabber
			float fps_loc = 0;
			unsigned int w_loc = 0; 
			unsigned int h_loc = 0;
			Get3ParamCmr(fps_loc, w_loc, h_loc);
			if (fps_loc == 0.0f){
				fps_loc = 0.01;
			}

			// critical section for count frames handle
			m_SynhCS_CntFrames.Set_Critical_Section();
			{
				m_dFreqTicks_CmnTimer = (1000.0 / fps_loc);// * 10000.0/*li_StartingTime_loc.QuadPart*/;
				InterlockedExchange64(&m_lCntFrame_CmnTimer, 0);
				InterlockedExchange64(&m_lNmbFrame4Wrt_CmnTm, 0);
				InterlockedExchange(&m_flag_AudioControl, 0);

				m_dLastTick_CmnTimer = 0.0;
			}
			m_SynhCS_CntFrames.Leave_Critical_Section();
			//////////////////////////////////////////////////////////////////////////
			// correctly calculate video frame for finish writing
			m_total_frames_save_video = m_time_save_video *  fps_loc;

			LaunchThreadGrabber();
		}
	}

	catch(cv::Exception& e ){
		const char* err_msg = e.what();
		std::cout << "exception caught: " << err_msg << std::endl;
	}

	return IsOpenedStreamSDK();
}

//////////////////////////////////////////////////////////////////////////
// release thread grabbing
void CCameraIP::ReleaseGrabThread(void)
{
	if (m_hGrabThread){

		printf("CameraIP[%d]->ReleaseGrabThread...\n", m_nmb_Cmr);

		InterlockedExchange(&m_flag_grabber, 0);
		InterlockedExchange(&m_FlagGrabbing_Thread, 0);

		// take time for thread to resume work
		WaitForSingleObject(m_hGrabThread, INFINITE);

		//TerminateThread(m_hGrabThread, NULL/*STILL_ACTIVE*/);

		CloseHandle(m_hGrabThread);

		m_hGrabThread = NULL;
	}

	printf("...CameraIP[%d]->ReleaseGrabThread\n", m_nmb_Cmr);

	// delete sync.object
	
	DeleteCriticalSection(&m_hCrSctn_TimerGrab);
	ZeroMemory((void*) &m_hCrSctn_TimerGrab, sizeof(m_hCrSctn_TimerGrab));
/*
	DeleteCriticalSection(&m_hCrSctn_StateRec);	
	ZeroMemory((void*) &m_hCrSctn_StateRec, sizeof(m_hCrSctn_StateRec));
	*/
}

//////////////////////////////////////////////////////////////////////////
// Get single image(screenshot from camera IP) buffer output
const void* CCameraIP::GetImageBuf_Output(void)
{
	if(!InterlockedCompareExchange(&m_FlagGrabbing_Thread, 0, 0) 
			||
		(m_pBuf_Img_Out == NULL)
		){	// ===> if (!m_FlagGrabbing_Thread ){	// get BMP buffer except new thread 

		// drawing base BMP
		m_pBuf_Img_Out = DrawBaseBMP();

		// return values from base image
		m_BitPerPxl_Img = m_ImageBmp_base.bmBitsPixel;
		m_Dimension_Img_out.cx = m_ImageBmp_base.bmWidth;
		m_Dimension_Img_out.cy = m_ImageBmp_base.bmHeight;

	}else{	// if (!m_FlagGrabbing_Thread)
		// set size 24 bit per pixel as getting image video IP is 3 byte
		m_BitPerPxl_Img = 24;
	}

	// get BMP buffer from
	return (void*)m_pBuf_Img_Out;
}

//////////////////////////////////////////////////////////////////////////
// save of the handle base BMP(output)
void CCameraIP::SetBaseBMP(HBITMAP h_bmp_in)
{
	m_hImageBMP_base = h_bmp_in;

		// get the size of base BMP
	if (m_hImageBMP_base){

		// get full information from base BMP
		GetObject( m_hImageBMP_base, sizeof( m_ImageBmp_base ), &m_ImageBmp_base );

		// allocate max memory for launching any grabbers
		CreateBufferImage(m_ImageBmp_base.bmWidth, m_ImageBmp_base.bmHeight, MAX_VIDEO_3_BYTE_PXL);

	}else{
		// allocate max memory for launching any grabbers
		CreateBufferImage(MAX_VIDEO_WIDTH, MAX_VIDEO_HEIGHT, MAX_VIDEO_3_BYTE_PXL);
	}

	//get buffer from base BMP
//	DrawBaseBMP();
}

//////////////////////////////////////////////////////////////////////////
//get buffer from base BMP
void* CCameraIP::DrawBaseBMP(void)
{
	LONG buf_sz_bmp = 0;

	// get base BMP
	if (m_hImageBMP_base && m_pBuf_Bmp){

		// get full information from base BMP
		GetObject( m_hImageBMP_base, sizeof( m_ImageBmp_base ), &m_ImageBmp_base );

		if( (m_ImageBmp_base.bmWidth <= m_MAX_Dimension_Buf.cx)	&&
				(m_ImageBmp_base.bmHeight <= m_MAX_Dimension_Buf.cy) &&
				(m_ImageBmp_base.bmBitsPixel <= m_BitPerPxl_Img)
			){
			buf_sz_bmp = GetBitmapBits(	m_hImageBMP_base,
																	m_sz_Img_out,
																	m_pBuf_Bmp
																);	//Copies the bit pattern of the BMP into the buffer
		}

		////////////

	}else{
		// set black color into Image
		ZeroMemory((void*) m_pBuf_Bmp, m_sz_Img_out);
	}

	return (void *)m_pBuf_Bmp;
}

//////////////////////////////////////////////////////////////////////////
// launch thread for grabber
bool CCameraIP::LaunchThreadGrabber( void )
{
	// if opening stream -> create thread for image grabbing 
	if (	!IsOpenedStreamSDK()
					||
				!InterlockedCompareExchange(&m_FlagGrabbing_Thread, 0, 0) 	// ===> "!m_FlagGrabbing_Thread
		){

		//release old memory 
		if (m_hGrabThread){
			ReleaseGrabThread();
		}

		// create watching event for change state of recoding(STOP/PAUSE/WRITE)
// 		m_hSemaphore_Thread = CreateSemaphore(	NULL,           // default security attributes
// 																						1,  // available count (when a thread enters, it decreases)
// 																						1,  // maximum count the semaphore object
// 																						NULL						// unnamed semaphore
// 																					);          
		
		// create critical section object for sync. grabber from CommonTimer
		InitializeCriticalSection( &m_hCrSctn_TimerGrab );
/*
		// create critical section (sync.object) for change state of recoding(STOP/PAUSE/WRITE)
		InitializeCriticalSection( &m_hCrSctn_StateRec );
		*/
		m_hGrabThread = CreateThread(	NULL,
																	0,
																	(LPTHREAD_START_ROUTINE)Func_Grab_Thread,
																	(void*)this,
																	0,
																	&m_dwGrabThreadId
																);
		if (m_hGrabThread){
			// set accuracy processor for the created thread
			DWORD err_ = SetThreadIdealProcessor(m_hGrabThread, m_nmb_Processor);

			bool err = SetThreadPriority(m_hGrabThread, THREAD_PRIORITY_ABOVE_NORMAL/*THREAD_PRIORITY_NORMAL*//*THREAD_PRIORITY_HIGHEST*/);


			//////////////////////////////////////////////////////////////////////////
			// add 01.02.2015
			if (m_Type_Stream == Type_Stream::IP_STREAM ){
				// change full filename recording
				ChangeNameRecording();
				//////////////////////////////////////////////////////////////////////////
			}
			//Sleep(500);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Get IP Camera parameters
int CCameraIP::GetCameraParams(	T_VideoParams &video_params_out		// video parameters
															)
{
/*	if (m_Type_Read_Module == Type_SDK_API::VLC){

		if (m_VLC.IsOpenedStream() && m_VLC.IsEnable()){
			// Get IP Camera parameters(using VLC API)
			m_VLC.GetCameraParams(m_VideoParams);
			
			if( (m_VideoParams.FPS != video_params_out.FPS)
					||
					(m_VideoParams.width_frame != video_params_out.width_frame)
					||
					(m_VideoParams.height_frame != video_params_out.height_frame)
				){

					CopyMemory((void *)(&video_params_out), (void*)&m_VideoParams, sizeof(T_VideoParams));
					return 1;	// there were the changes to IP Cameras
			}

			CopyMemory((void *)(&video_params_out), (void*)&m_VideoParams, sizeof(T_VideoParams));
			return 0;	// there were not the changes to IP Cameras
		}
	}else{
	*/
		if (m_Type_Stream == Type_Stream::IP_STREAM ){
			//Type_SDK_API::OPEN_CV
			if (m_OpenCV.IsOpenedStream() && m_OpenCV.IsEnable()){

				// Get Full Information about stream( see: "opencv2refman.pdf")
				m_OpenCV.GetCameraParams(m_VideoParams);

				if( (m_VideoParams.FPS != video_params_out.FPS)
						||
						(m_VideoParams.width_frame != video_params_out.width_frame)
						||
						(m_VideoParams.height_frame != video_params_out.height_frame)
					){

						CopyMemory((void *)(&video_params_out), (void*)&m_VideoParams, sizeof(T_VideoParams));
						return 1;	// there were the changes to IP Cameras
				}

				CopyMemory((void *)(&video_params_out), (void*)&m_VideoParams, sizeof(T_VideoParams));
				return 0;	// there were not the changes to IP Cameras
			}
		}else{
			;
		}
	/*}*/

	return -1;	// the stream is not opened
}

//////////////////////////////////////////////////////////////////////////
// Get the main IP Camera parameters
bool CCameraIP::Get3ParamCmr(	float &fps_dev,		// the number frames per second getting from device(CameraIP)
															UINT &img_W_dev,	// width image from device(CameraIP)
															UINT &img_H_dev		// height image from device(CameraIP)
														)
{
/*	if (m_Type_Read_Module == Type_SDK_API::VLC){

		if (m_VLC.IsOpenedStream() && m_VLC.IsEnable()){

			if (!m_VLC.Get_3Params(fps_dev, img_W_dev, img_H_dev)){
				// save values
				m_VideoParams.FPS = fps_dev;							// === m_VideoParams_last.FPS
				m_VideoParams.width_frame = img_W_dev;		// === m_VideoParams_last.width_frame
				m_VideoParams.height_frame = img_H_dev;		// === m_VideoParams_last.height_frame
				return true;
			}else{
				return false;
			}
		}
	}else{
	*/
		if (m_Type_Stream == Type_Stream::IP_STREAM ){

			//Type_SDK_API::OPEN_CV
			if (m_OpenCV.IsOpenedStream() && m_OpenCV.IsEnable()){

				// Get Information about stream( see: "opencv2refman.pdf")
				if (!m_OpenCV.Get_3Params(fps_dev, img_W_dev, img_H_dev)){
					// save values
					m_VideoParams.FPS = fps_dev;							// === m_VideoParams_last.FPS
					m_VideoParams.width_frame = img_W_dev;		// === m_VideoParams_last.width_frame
					m_VideoParams.height_frame = img_H_dev;		// === m_VideoParams_last.height_frame
					return true;
				}else{
					return false;
				}
			}
		}else{
			//Type_Stream::FILE_STREAM;

			if (m_OpenCV_Player.IsOpenedStream() && m_OpenCV_Player.IsEnable()){

				// Get Information about stream( see: "opencv2refman.pdf")
				if (!m_OpenCV_Player.Get_3Params(fps_dev, img_W_dev, img_H_dev)){
					// save values
					m_VideoParams.FPS = fps_dev;							// === m_VideoParams_last.FPS
					m_VideoParams.width_frame = img_W_dev;		// === m_VideoParams_last.width_frame
					m_VideoParams.height_frame = img_H_dev;		// === m_VideoParams_last.height_frame
					return true;
				}else{
					return false;
				}
			}

			fps_dev = 0;
			img_W_dev = 0;
			img_H_dev = 0;
		}
	/*}*/

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Set the bit per pixel for getting image
int CCameraIP::SetBitsPerPxl_Img(const	unsigned int bit_pxl_in		// bit per pixel for getting image
																)
{
	if( (bit_pxl_in == 24) || (bit_pxl_in == 32) ){
		m_BitPerPxl_Img = bit_pxl_in;

		return 0;
	}else{
		m_BitPerPxl_Img = 24;
	}
	return -1;	// unsupported type
}

//////////////////////////////////////////////////////////////////////////
// Set state of flag for control video
int CCameraIP::SetControlFlag(const	unsigned int control_state_in		// flag of the control (see.CommonData::T_DeviceState)
															)
{
	if ( (control_state_in < T_DeviceState::STOP) || (control_state_in > T_DeviceState::SEEK)){
		return -1;		// error
	}

	// set synh. flag for writing state
	Set_CS_StateRec();
	{
		// check file exist => create file the one
// 		if (Path_File_Exists(m_fullname_rec)){
// 			m_WriteState = T_DeviceState::STOP;
// 		}else{
// 			m_WriteState = control_state_in;
// 		}
		m_WriteState = control_state_in;

		if (m_Type_Stream == Type_Stream::FILE_STREAM){
			ControlMovie_SDK(m_WriteState);
		}
	}
	// leave synh. flag for writing state
	Leave_CS_StateRec();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// check opened of reading video stream
const bool CCameraIP::IsOpenedStreamSDK(void) const
{
	/*
	// reading stream was opened by VLC SDK
	if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.IsOpenedStream();
	}else{
	*/
		if (m_Type_Stream == Type_Stream::IP_STREAM ){
			// reading stream was opened by OpenCV SDK
			return m_OpenCV.IsOpenedStream();
		}else{
			return m_OpenCV_Player.IsOpenedStream();
		}
	/*}*/

	return false;
}

//////////////////////////////////////////////////////////////////////////
// release stream SDK
const void CCameraIP::ReleaseSDK(void)
{
	/*
	// reading stream was opened by VLC SDK
	if (m_Type_Read_Module == Type_SDK_API::VLC){
		m_VLC.Release();
	}else{
	*/
		if (m_Type_Stream == Type_Stream::IP_STREAM ){
			// releasing stream was opened by OpenCV SDK
			m_OpenCV.Release();
		}else{
			// releasing stream was opened by OpenCV SDK
			m_OpenCV_Player.Release();
		}
	/*}*/

	return;
}

//////////////////////////////////////////////////////////////////////////
// create stream IP using path stream of the camera( by using SDK )
const bool CCameraIP::Create_StreamIP_SDK(const char *path_stream_in, int nmb_Camera_in)
{
	/*
	// reading stream was opened by VLC SDK
	if (m_Type_Read_Module == Type_SDK_API::VLC){

		//set streaming type
		m_VLC.SetStreamingType(m_Type_Stream);

		return m_VLC.Create_StreamIP(path_stream_in);
	}else{
	*/
		// reading stream was opened by OpenCV SDK
		return m_OpenCV.Create_StreamIP(path_stream_in, nmb_Camera_in);
	/*}*/

	return false;
}

//////////////////////////////////////////////////////////////////////////
// create stream static file using path stream( by using SDK )
const bool CCameraIP::Create_StreamFile_SDK(CMetaMoviesHDD*	p_MetaDataMovie_in, unsigned int nmb_movie_in)
{
	try
	{
		/*
		// reading stream was opened by VLC SDK
		if (m_Type_Read_Module == Type_SDK_API::VLC){
		
			//set streaming type
			m_VLC.SetStreamingType(m_Type_Stream);

			m_VLC.Create_StreamFile(0, false);

			return	LaunchThreadGrabber();
		}else{
		*/
			if (m_OpenCV_Player.Create_StreamFile(p_MetaDataMovie_in, nmb_movie_in)){
				//////////////////////////////////////////////////////////////////////////
				// get needed parameters for timer grabber
				float fps_loc = 0;
				unsigned int w_loc = 0; 
				unsigned int h_loc = 0;
				Get3ParamCmr(fps_loc, w_loc, h_loc);
				if (fps_loc == 0.0f){
					fps_loc = 0.01;
				}

			// critical section for count frames handle
				m_SynhCS_CntFrames.Set_Critical_Section();
				{
					m_dFreqTicks_CmnTimer = (1000.0 / fps_loc);// * 10000.0/*li_StartingTime_loc.QuadPart*/;
					InterlockedExchange64(&m_lCntFrame_CmnTimer, 0);
					InterlockedExchange64(&m_lNmbFrame4Wrt_CmnTm, 0);
					InterlockedExchange(&m_flag_AudioControl, 0);

					m_dLastTick_CmnTimer = 0.0;
				}
				m_SynhCS_CntFrames.Leave_Critical_Section();
				//////////////////////////////////////////////////////////////////////////
				return	LaunchThreadGrabber();
			}
		/*}*/
	}// try
	catch(cv::Exception& e ){
		const char* err_msg = e.what();
		std::cout << "exception caught: " << err_msg << std::endl;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// the only step grabber( by using SDK )
const void* CCameraIP::Grab_Step_SDK( void )
{
	/*
	// reading stream was opened by VLC SDK
	if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.GrabStep();
	}else{
	*/
		// reading stream was opened by OpenCV SDK
		if (m_Type_Stream == Type_Stream::IP_STREAM ){
			return m_OpenCV.GrabStep();
		}else{
			return m_OpenCV_Player.GrabStep();
		}
	/*}*/

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// is enable modules( by using SDK )
const bool CCameraIP::IsEnable_SDK( void ) const
{
	// reading stream was opened by VLC SDK
/*	if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.IsEnable();
	}else{
*/
		if (m_Type_Stream == Type_Stream::IP_STREAM ){
			return m_OpenCV.IsEnable();
		}else{
			return m_OpenCV_Player.IsEnable();
		}
/*	} */

	return false;
}

//////////////////////////////////////////////////////////////////////////
// set name video file for recording( by using SDK )
void CCameraIP::SetNameVideoRec_SDK()
{
	// reading stream was opened by VLC SDK
/* if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.SetNameVideoRec(m_fullname_rec);
	}else{
	*/
		return m_OpenCV.SetNameVideoRec(m_fullname_rec);
	/*}*/
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// write the single frame ( by using SDK )
int CCameraIP::WriteFrameMPEG_SDK(void)
{	
	// reading stream was opened by VLC SDK
/*	if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.WriteFrameMPEG();
	}else{
*/
		return m_OpenCV.WriteFrameMPEG();
/*	} 

	return 0;*/
}

//////////////////////////////////////////////////////////////////////////
// writing finish video frame ( by using SDK )
int CCameraIP::FinishWritingMPEG_SDK(void)
{
	// reading stream was opened by VLC SDK
/*	if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.FinishWritingMPEG();
	}else{
	*/
		return m_OpenCV.FinishWritingMPEG();
/*	} 

	return 0;*/
}

//////////////////////////////////////////////////////////////////////////
// set states for control movie( by using SDK )
int CCameraIP::ControlMovie_SDK( const unsigned int state_in	//see T_DeviceState
																)
{
	// reading stream was opened by VLC SDK
/*	if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.ControlMovie(state_in, 0);
	}else{
	*/
		return m_OpenCV_Player.ControlMovie(state_in, -1);
/*	} 

	return 0;*/
}

//////////////////////////////////////////////////////////////////////////
// get state recording ( by using SDK )
unsigned int CCameraIP::GetStateRec_SDK(void) const
{
	// reading stream was opened by VLC SDK
/*	if (m_Type_Read_Module == Type_SDK_API::VLC){
		return m_VLC.GetStateRec();
	}else{

*/
		return m_OpenCV.GetStateRec();
/*	} 

	return 0;*/
}

//////////////////////////////////////////////////////////////////////////
// set path recording for the camera(except fullname)
void	CCameraIP::SetPathRecording(const char* path_in)
{
	sprintf(m_short_path_rec, "%s", path_in);

	if( (m_nmb_Cmr != -1)
				&&
			InterlockedCompareExchange(&m_FlagGrabbing_Thread, 1, 1)		 // === "m_FlagGrabbing_Thread"
		){
		///// examine the state of recording
		unsigned int state_rec_loc = GetStateRec_SDK();

		//there is recording now!!!
		if( (state_rec_loc == T_DeviceState::WRITE) || (state_rec_loc == T_DeviceState::PAUSE) ){
			//////////////////////
			//////////////////////
			//START SYNCRONIZATION

			// set synh. flag for writing state
			Set_CS_StateRec();
			{
				m_WriteState = T_DeviceState::STOP;	// the finish streaming frame

				// finish writing
				FinishWritingMPEG_SDK();
				//////////////////////

				// increase version number recording
				InterlockedIncrement(&nmb_ver_rec);			//	nmb_ver_rec ++;

				// change full filename recording
				ChangeNameRecording();

				m_WriteState = T_DeviceState::WRITE;	// the finish streaming frame

				// begin writing stream newly
				WriteFrameMPEG_SDK();
			}
			// leave synh. flag for writing state
			Leave_CS_StateRec();

			//FINISH SYNCRONIZATION
			//////////////////////
			//////////////////////
		}else{
			// change full filename recording
			ChangeNameRecording();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// create finishing full path recording
void CCameraIP::CreatePathRecording( void )
{
	// get folder path and create directory
	if (strlen(m_short_path_rec) ){
		sprintf(m_fullname_rec, "%s\\%s\\", m_short_path_rec, m_folder_rec);
	}else{
		sprintf(m_fullname_rec, ".\\%s\\", m_folder_rec);
	}

	int err = 0;

	// make directory if no exist
	err = mkdir(m_fullname_rec);
	if(!err){
		InterlockedExchange(&nmb_ver_rec, 0);
	}

	sprintf(m_filename_rec, "%s_%d%d%d%d%s%d%s", NAME_VIDEO_REC_HEADER, /*m_nmb_Cmr,*/
																						 m_NetAddress.IP[0], m_NetAddress.IP[1], m_NetAddress.IP[2], m_NetAddress.IP[3],
																						 NAME_VIDEO_REC_SUFFIX,
																						 nmb_ver_rec,
																						 NAME_VIDEO_REC_TAIL
				);

	sprintf(m_fullname_rec + strlen(m_fullname_rec), "%s", m_filename_rec );
}

//////////////////////////////////////////////////////////////////////////
// set maximum time for recording one movie
void CCameraIP::SetMaxTimeRecMinutes(	const UINT mimutes_in				// time [min] for recording single movie
																		)
{
	if (mimutes_in > 0){
		// translate time from minutes to second
		m_time_save_video = mimutes_in * 60 + /*Second*/(0);
	}else{
		;// error
	}
}

//////////////////////////////////////////////////////////////////////////
// set flag 360 degrees using
BOOL CCameraIP::SetFlag360(	const bool flag_in				// the flag define camera tag of 360 degrees 
													)
{

	m_Flag_360 = flag_in;
// 	if (m_Flag_360){
// 
// 		char loc_path[_MAX_PATH];
// 		sprintf_s(loc_path, _MAX_PATH, "192.168.0.149:4520");
// 
// 		// delete net sending for camera
// 		if (m_NetSendCmr360.IsEnable()){
// 			m_NetSendCmr360.Release();
// 		}
// 		// create net sending using thread
// 		m_NetSendCmr360.Init(loc_path);
// 	}else{
// 		// delete net sending
// 		m_NetSendCmr360.Release();
// 	}
 	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// set critical section in working state
void CCameraIP::Set_CS_TimerGrabber(void)
{
//	if (m_FlagGrabbing_Thread){
		EnterCriticalSection( &m_hCrSctn_TimerGrab );
//	}
};

// try to set critical section in working state
const BOOL CCameraIP::TrySet_CS_TimerGrabber(void)
{
//	if (m_FlagGrabbing_Thread){
//		return TryEnterCriticalSection ( &m_hCrSctn_TimerGrab );
//	}else{
		return FALSE;
//	}
};

// set working out critical section 
void CCameraIP::Leave_CS_TimerGrabber(void)
{
//	if (m_FlagGrabbing_Thread){
		LeaveCriticalSection( &m_hCrSctn_TimerGrab );
//	}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// set critical section in working state
void CCameraIP::Set_CS_StateRec(void)
{
//	if (m_FlagGrabbing_Thread){
	//	EnterCriticalSection( &m_hCrSctn_StateRec );
//	}
};

// try to set critical section in working state
const BOOL CCameraIP::TrySet_CS_StateRec(void)
{
//	TryEnterCriticalSection ( &m_hCrSctn_StateRec );

	return FALSE;
};

// set working out critical section 
void CCameraIP::Leave_CS_StateRec(void)
{
//	if (m_FlagGrabbing_Thread){
//		LeaveCriticalSection( &m_hCrSctn_StateRec );
//	}
};

//////////////////////////////////////////////////////////////////////////
// set start time movie
bool CCameraIP::Set_TimeStartMovie(unsigned int tm_start_play_in	// time to play movie [msec]
																	)
{
	bool out;
	// set synh. flag for writing state
	Set_CS_StateRec();
	{
		out = false;

		// reading stream was opened by VLC SDK
/*		if (m_Type_Read_Module == Type_SDK_API::VLC){
			out = m_VLC.ControlMovie(T_DeviceState::PLAY, tm_start_play_in);
		}else{
*/
			out = m_OpenCV_Player.ControlMovie(m_OpenCV_Player.GetStatePlayback(), tm_start_play_in);

// 			if (m_OpenCV_Player.GetStatePlayback() == T_DeviceState::PLAY){
// 				out = m_OpenCV_Player.ControlMovie(T_DeviceState::PLAY, tm_start_play_in);
// 			}else{
// 				out = m_OpenCV_Player.ControlMovie(T_DeviceState::SEEK, tm_start_play_in);
// 			}
/*		} */

	}
	// leave synh. flag for writing state
	Leave_CS_StateRec();

	return out;
}
//////////////////////////////////////////////////////////////////////////
// change full filename recording
void CCameraIP::ChangeNameRecording(void)
{
	// add 01.02.2015
	if (m_Type_Stream == Type_Stream::IP_STREAM ){

		// create finishing full path recording
		CreatePathRecording();

		// set name for recording class
		SetNameVideoRec_SDK();
		//////////////////////////////////////////////////////////////////////////
	}
}
//////////////////////////////////////////////////////////////////////////
// the patch to function control "p_GetTimeMovie" (It's for Player!!! see.CProcPlayerOCV)
void CCameraIP::Patch_FirstListUnit(void)
{
	m_OpenCV_Player.m_bFlag_FirstObj = TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// transfer data in Audio module (count frames for exact timer)
int CCameraIP::AudioControl(const LONG nmb_frame_wr_in) const
{	
	// exam AUDIO if it's exist
	// set synchronization AUDIO by the first device within one's times
	if( /*(nmb_cmr_loc == 0) && */g_AudioData.bEnable){

		if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::WRITE, T_DeviceState::WRITE)){
			// set synchronization with AUDIO module

			// put the copy whole count frames into AUDIO module for synchronization m_dLastTick_CmnTimer
			if (((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->m_pAsioWriter){
				InterlockedExchange64(	&(((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->m_pAsioWriter->m_lCntFrame_CmnTimerCAMs),
																nmb_frame_wr_in
															);
			}
		}
	}

	return 0;
}
/*
//////////////////////////////////////////////////////////////////////////
// Get single image(screenshot from camera IP) buffer output
const void* CCameraIP::GetImageBuf_Output(void)
{
m_BitPerPxl_Img = 24;	
if (!m_FlagGrabbing_Thread ){	// get BMP buffer except new thread
// 		if (m_OpenCV.IsOpenedStream() ){
// 
// 			//	int fps = m_OCV_capture.get(CV_CAP_PROP_FPS);
// 			;
// 			if (m_OpenCV.GrabStep()){
// 				int fps = 0;
// 				int _w_img = 0;
// 				int _h_img = 0;
// 
// 				// get parameters real time
// 				m_OpenCV.Get_3Params(fps, (int&)m_Dimension_Img_out.cx, (int&)m_Dimension_Img_out.cy);
// 
// 				if ( m_sz_Img_out < m_OpenCV.GetBuffer_Size() ){
// 					CreateBufferImage(_w_img, _h_img, 24);
// 				}
// 				if (m_OpenCV.IsEnable() && m_OpenCV.m_pBuf_out){
// 					if (m_BitPerPxl_Img == 24){
// 						CopyMemory(m_pBuf_Bmp, m_OpenCV.m_pBuf_out, m_Dimension_Img_out.cx * m_Dimension_Img_out.cy *3);
// 					}else{
// 
// 						// slowly ( for testing )
// 						Mat fr_locSize(Size( m_Dimension_Img_out.cx, m_Dimension_Img_out.cy),CV_8UC4);
// 
// 						cvtColor(*m_OpenCV.GetReadImg(), fr_locSize, CV_RGB2RGBA);
// 						m_pBuf_Bmp = fr_locSize.data;
// 						CopyMemory(m_pBuf_Bmp, fr_locSize.data, m_Dimension_Img_out.cx * m_Dimension_Img_out.cy *4);
// 					}
// 				}
// 				return (void*)m_pBuf_Bmp;
// 			}
// 		}
// drawing base BMP
//DrawBaseBMP();
}// if (!m_FlagGrabbing_Thread)

// get BMP buffer from
return (void*)m_pBuf_Bmp;
}


//////////////////////////////////////////////////////////////////////////
// translating string IP to struct T_NetAddress
int	CCameraIP::TranslateStr2AddrIP(char *path_strm_in)
{
if (!path_strm_in){
return -1;
}
if (!strlen(path_strm_in)){
return -1;
}
//////////////////////////////////////////////////////////////////////////
char str1_loc[5] = "://";
char str2_loc[5] = "@";
char str3_loc[5] = ":";
char str4_loc[5] = "/";
char *pdest = NULL;
char str_IP[64];
char str_Port[10];
char str_Log[16];
char str_Pas[16];
char str_LogPas[32];
char *pdest_log = NULL;
int  res_str_log;
int  res_str_psw;

int  res_str1;
int  res_str2;
int  res_str3;
int  res_str4;
in_addr var;
struct sockaddr_in var_loc;

pdest = strstr(path_strm_in, str1_loc);	// find protocol

if (pdest){
res_str1 = pdest - path_strm_in + 0 + strlen(str1_loc);

pdest = strstr(path_strm_in + res_str1, str2_loc);
if (pdest){
res_str2 = pdest - path_strm_in + 0 + strlen(str2_loc);

strncpy_s(str_LogPas, res_str2 - res_str1, path_strm_in + res_str1, res_str2 - res_str1 - 1 );
/////

pdest_log = strstr(str_LogPas, str3_loc);	// find login

if (pdest_log){
res_str_log = pdest_log - str_LogPas + 0 + strlen(str3_loc);

strncpy_s(str_Log, res_str_log, str_LogPas, res_str_log - 1 );
sprintf_s(str_Pas, strlen(pdest_log), pdest_log + 1 );

}
/////
pdest = strstr(pdest, str3_loc);	// find IP address
if (pdest){

res_str3 = pdest - path_strm_in + 0 + strlen(str3_loc);
strncpy_s(str_IP, res_str3 - res_str2, path_strm_in + res_str2, res_str3 - res_str2 - 1 );

var_loc.sin_addr.s_addr = inet_addr(str_IP);	// get IP Address

pdest = strstr(pdest, str4_loc);	// find port
if (pdest){
res_str4 = pdest - path_strm_in + 0 + strlen(str4_loc);

strncpy_s(str_Port, res_str4 - res_str3, path_strm_in + res_str3, res_str4 - res_str3 - 1 );

}
}
//var =  inet_addr((const char *)pdest);
}else{
;// "path_stream" has not IP address
}
m_NetAddress.IP[0] = var_loc.sin_addr.s_net;		// network
m_NetAddress.IP[1] = var_loc.sin_addr.s_host;		// host on imp
m_NetAddress.IP[2] = var_loc.sin_addr.s_lh;			// network
m_NetAddress.IP[3] = var_loc.sin_addr.s_impno;	// imp #

sprintf_s(m_NetAddress.Login, _MAX_PATH/4, str_Log);
sprintf_s(m_NetAddress.Password, _MAX_PATH/4, str_Pas);
m_NetAddress.Port = atoi(str_Port);

}else{
;// "path_stream" has not IP address
}
}

// 			//////////////////////////
// 			MessageBoxA(NULL, path_stream_loc, "!!!Connect GOOD!!!", MB_OK);
// 			//////////////////////////

*/
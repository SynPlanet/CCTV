#pragma once


//#include "ProcVLC.h"
#include "ProcOpenCV.h"
#include "NetSendCmr360.h"

#include "MetaMoviesHDD.h"

#include "Synh_Objs.h"

#include "ProcPlayerOCV.h"

#include "./Onvif/ConnectOnvif.h"

using namespace std;

//!!!
//http://stackoverflow.com/questions/9447993/opencv-multithreading-windows-net-delays-several-seconds-from-video-capture

// save video
//http://stackoverflow.com/questions/13323264/opencv-doesnt-save-the-video
//http://stackoverflow.com/questions/12054907/opencv-videowriter-wont-write-anything-although-cvwritetoavi-does/12059944#12059944
//http://docs.opencv.org/doc/tutorials/highgui/video-write/video-write.html

//http://stackoverflow.com/questions/9387392/how-can-i-use-the-opencv-ffmpeg-video-i-o-rather-than-the-directshow-one-in-wind
//

#define MAX_SEMAPHORE 5

//////////////////////////////////////////////////////////////////////////
class CCameraIP
{
public:
	CCameraIP(	long type_read_module_in,
							long type_wr_module_in,
							long type_stream_in
						);
	~CCameraIP(void);

	IP_Camera::T_VideoParams		m_VideoParams_last;

	// Get the main IP Camera parameters
	bool Get3ParamCmr(	float &fps_dev,		// the number frames per second getting from device(CameraIP)
											UINT &img_W_dev,	// width image from device(CameraIP)
											UINT &img_H_dev		// height image from device(CameraIP)
										) ;

	// set number camera in system
	void	SetNmbCameraIP(unsigned int nmb_cmr_in);

	// translating string IP to struct T_NetAddress
//	int	TranslateStr2AddrIP(char *path_strm_in);

	// set path recording for the camera(except fullname)
	void SetPathRecording(const char* path_in);

	// set flag 360 degrees using
	BOOL SetFlag360(	const bool flag_in				// the flag define camera tag of 360 degrees 
									);

	// get flag 360 degrees using
	BOOL GetFlag360( void ) const
	{
		return m_Flag_360;
	};

	// set number processor for the future thread
	void	SetNmbProcessor(unsigned int nmb_processor_in)
	{
		m_nmb_Processor = nmb_processor_in;
	}

	// create stream IP using path stream of the camera
	bool Create_StreamIP(T_NetAddress	*net_addr);

	// set flag define getting bitmap image by launching thread
	void SetEnableThread(bool flag)
	{
		InterlockedExchange(&m_flag_grabber, (LONG)flag);
		InterlockedExchange(&m_FlagGrabbing_Thread, (LONG)flag);
	}

	// set flag define getting bitmap image by launching thread
	bool GetEnableThread(void)
	{
		return (bool)m_FlagGrabbing_Thread;
	}

	//////////////////////////////////////////////////////////////////////////
	// threading function
	static DWORD Func_Grab_Thread(LPVOID lpParam);

	// launch thread for grabber
	bool LaunchThreadGrabber( void );
	//////////////////////////////////////////////////////////////////////////

	// get number camera in system
	unsigned int GetNmbCameraIP(void)
	{
		return m_nmb_Cmr;
	}

	// Get path stream of camera IP
	char* GetStreamPath(void)
	{
		return (char*)path_stream;
	}

	// Get single image(screenshot from camera IP) buffer output
	const void* GetImageBuf_Output(void);

	// Get dimension single image output
	const void GetDimImage_Output(	unsigned int &w_out,
																	unsigned int &h_out
																)
	{
		w_out = m_Dimension_Img_out.cx;
		h_out = m_Dimension_Img_out.cy;
	}

	// Set the bit per pixel for getting image
	int SetBitsPerPxl_Img(const	unsigned int bit_pxl_in		// bit per pixel for getting image
												);

	// Get the bit per pixel for getting image
	const int GetBitsPerPxl_Img( void ){	return m_BitPerPxl_Img;	}

	// Set state of flag for control video
	int SetControlFlag(	const	unsigned int control_state_in		// flag of the control (see.CommonData::T_DeviceState)
										);

	// Set state of flag for control video
	const	unsigned int GetWritingFlag( void ){	 return m_WriteState;	}



	// reallocate output BMP buffer
	int CreateBufferImage(	const unsigned int dim_X,			// width image
													const unsigned int dim_Y,			// height image
													const unsigned int byte_pxl		// byte per pixel for image
												);

	// save of the handle base BMP(output)
	void SetBaseBMP(HBITMAP h_bmp_in);

	//get buffer from base BMP
	void* DrawBaseBMP(void);

	// loading BMP image
	//int Load_Bmp(char *path_Bmp);

	// release thread grabbing
	void ReleaseGrabThread(void);

	// driving camera by joystick(real or virtual(interface))
// 	int SetJoyControlNet(	T_DataPTZ			*p_data_PTZ,
// 												unsigned char type_device							//T_JoyDevice::enum
// 											);

	// get pointer address information
	const T_NetAddress* GetInfoAddress( void ){ return &m_NetAddress; }


	// set critical section in working state
	void Set_CS_TimerGrabber(void);
	// try to set critical section in working state
	const BOOL TrySet_CS_TimerGrabber(void);
	// set working out critical section 
	void Leave_CS_TimerGrabber(void);

	// set critical section in working state
	void Set_CS_StateRec(void);
	// try to set critical section in working state
	const BOOL TrySet_CS_StateRec(void);
	// set working out critical section 
	void Leave_CS_StateRec(void);

	// get class define for movie list (if exist)
	//void SetClsMoviesList(CMetaMoviesHDD *pMoviesHDD_in) { m_pMoviesHDD = pMoviesHDD_in; };

	// set maximum time for recording one movie
	void SetMaxTimeRecMinutes(	const UINT mimutes_in				// time [min] for recording single movie
														);

	// set start time movie
	bool Set_TimeStartMovie(unsigned int tm_start_play_in);

	// write the single frame
	int WriteFrameMPEG_SDK(void);

	// get state recording ( by using SDK )
	unsigned int GetStateRec_SDK(void) const;

	// create stream file movie
	const bool Create_StreamFile_SDK(CMetaMoviesHDD*	p_MetaDataMovie_in, unsigned int nmb_movie_in);

	// the patch to function control "p_GetTimeMovie" (It's for Player!!! see.CProcPlayerOCV)
	void Patch_FirstListUnit(void);

	// get real time was played (global time)
	LONGLONG GetRealTimePlayed(void) const { return m_OpenCV_Player.Get_RT_Played(); }

	LONG GetRealTimeState(void) const { return m_OpenCV_Player.Get_RT_State(); }

	// transfer data in Audio module (count frames for exact timer)
	int AudioControl(const LONG nmb_frame_wr_in) const;
	
public:
	CSynh_Barrier *m_pSynh_Bar_StCam_1;		// sinh. object barrier the state camera
	CSynh_Barrier *m_pSynh_Bar_StCam_2;		// sinh. object barrier the state camera
	CSynh_Barrier *m_pSynh_Bar_RnmFile_1;		// sinh. object barrier for rename file (!!!garbage!!!)
	CSynh_Barrier *m_pSynh_Bar_RnmFile_2;		// sinh. object barrier for rename file (!!!garbage!!!)

protected:
	// Get IP Camera parameters
	int GetCameraParams( T_VideoParams &video_params_out		// video parameters
											);
		char path_stream[_MAX_PATH]; // stream path 

	// create finishing full path recording
	void CreatePathRecording( void );

	// check opened of reading video stream (SDK VLC and OpenCV using)
	const bool IsOpenedStreamSDK(void) const;

	// create stream IP using path stream of the camera( by using SDK )
	const bool Create_StreamIP_SDK(const char *path_stream_in, int nmb_Camera_in);

	// release stream SDK
	const void ReleaseSDK(void);

	// the only step grabber( by using SDK )
	const void* Grab_Step_SDK( void );

	// is enable modules( by using SDK )
	const bool IsEnable_SDK( void ) const;

	// set name video file for recording( by using SDK )
	void SetNameVideoRec_SDK(void);

	// write the single frame
//	int WriteFrameMPEG_SDK(void);

	// writing finish video frame ( by using SDK )
	int FinishWritingMPEG_SDK(void);

	// get state recording ( by using SDK )
//	unsigned int GetStateRec_SDK(void) const;

	// set states for control movie( by using SDK )
	int ControlMovie_SDK( const	unsigned int state_in	//see T_DeviceState
											);

	// change full filename recording
	void ChangeNameRecording(void);

public:
		volatile LONGLONG m_lCntFrame_CmnTimer;
		volatile LONGLONG m_lNmbFrame4Wrt_CmnTm;

		double	m_dFreqTicks_CmnTimer;
		double m_dLastTick_CmnTimer;

		volatile LONG m_flag_grabber;

		volatile LONG m_flag_AudioControl;	// flag Audio control from Common Timer

	  // critical section for count frames handle
		CSynh_CritSect		m_SynhCS_CntFrames;

		CConnectOnvif	m_ConnectOnvif;

private:
	T_NetAddress	m_NetAddress;	// saving data from output interface

	long	m_Type_Read_Module;		// type modules using for reading	(see: Type_SDK_API)
	long	m_Type_Write_Module;	// type modules using for writing	(see: Type_SDK_API)
	long	m_Type_Stream;				//	streaming type (IP or File stream using) (see: Type_Stream)

	unsigned int m_nmb_Cmr;	// number cameras
	//char path_stream[_MAX_PATH]; // stream path 

	unsigned int m_nmb_Processor;	// number Processor

	BITMAP	m_ImageBmp_base;	// base image for drawing 
	HBITMAP m_hImageBMP_base;

	void *m_pBuf_Bmp;					// pointer to buffer BMP (24 bit / pixel)

	unsigned int m_BitPerPxl_Img;	// bit per pixel for getting image

	unsigned int m_sz_Img_out;	// BMP size for final output 
	SIZE m_Dimension_Img_out;		// BMP dimension for final output
	void *m_pBuf_Img_Out;			// pointer to buffer for output (24 bit / pixel)

	unsigned int m_MAX_Size_Buf;			// Maximum size buffer
	SIZE m_MAX_Dimension_Buf;		// Maximum dimension buffer


	CProcOpenCV	m_OpenCV;
	CProcPlayerOCV	m_OpenCV_Player;
/////	CProcVLC		m_VLC;

	// OpenCV
	// !!! FOR WRITE !!!
	UINT m_WriteState;	//needed state (IP_Camera::T_VideoStat)
	UINT m_WriteState_old;	//state from last state (IP_Camera::T_VideoStat)
	//

	//////////////////////////////////////////////////////////////////////////
	char m_filename_rec[_MAX_PATH];		// filename(+ version) include extension
	char m_folder_rec[_MAX_PATH];			// was gotten from INET
	char m_short_path_rec[_MAX_PATH];				// path except folder and further
	char m_fullname_rec[_MAX_PATH];		// path, folder, name(+ version), extension
	volatile LONG nmb_ver_rec;		// version number of the recording
	//////////////////////////////////////////////////////////////////////////

	UINT	m_time_save_video; // maximum time video recording   [sec]
	ULONG m_total_frames_save_video;

	UINT m_cnt_frames_save;	

	// Thread parameters
	HANDLE		m_hGrabThread;
	DWORD			m_dwGrabThreadId;

	volatile LONG m_FlagGrabbing_Thread;

	//HANDLE	m_hSemaphore_Thread;

	IP_Camera::T_VideoParams		m_VideoParams;

	bool m_Flag_360;	// is camera 360

//	CNetSendCmr360	m_NetSendCmr360;

	CRITICAL_SECTION m_hCrSctn_TimerGrab;	// critical section object for sync. grabber from CommonTimer
//	CRITICAL_SECTION m_hCrSctn_StateRec;	// critical section object for sync. states recording
};

//////////////////////////////////////////////////////////////////////////
// old
/*
// Get single image(screenshot from camera IP) buffer output
const void* GetImageBuf_Output(void)
{
if (m_OCV_capture.isOpened()){

//	int fps = m_OCV_capture.get(CV_CAP_PROP_FPS);

if (m_OCV_capture.grab()){
m_OCV_capture.retrieve(m_OCV_frame_RT, 0);

m_Dimension_Img_out.cx = m_OCV_frame_RT.cols;
m_Dimension_Img_out.cy = m_OCV_frame_RT.rows;

if (m_BitPerPxl_Img == 24){
CopyMemory(m_pBuf_Bmp, m_OCV_frame_RT.data, m_Dimension_Img_out.cx * m_Dimension_Img_out.cy *3);
}else{

// slowly ( for testing )
Mat fr_locSize(Size(m_OCV_frame_RT.cols, m_OCV_frame_RT.rows),CV_8UC4);

cvtColor(m_OCV_frame_RT, fr_locSize, CV_RGB2RGBA);

CopyMemory(m_pBuf_Bmp, fr_locSize.data, m_Dimension_Img_out.cx * m_Dimension_Img_out.cy *4);
}
// 				m_OCV_capture >> m_OCV_frame_RT;
// 			//m_OCV_capture.get(CAP_INTELPERC_IMAGE_GENERATOR | cv::CAP_PROP_POS_FRAMES);
// 
// 			if (m_OCV_frame_RT.empty()){
// 
// 				
// 				return NULL;
// 			}else{
// 				m_Dimension_Img_out.cx = m_OCV_frame_RT.cols;
// 				m_Dimension_Img_out.cy = m_OCV_frame_RT.rows;
// 
// 				if (m_BitPerPxl_Img == 24){
// 					CopyMemory(m_pBuf_Bmp, m_OCV_frame_RT.data, m_Dimension_Img_out.cx * m_Dimension_Img_out.cy *3);
// 				}else{
// 					// slowly ( for testing )
// 
// 					Mat fr_locSize(Size(m_OCV_frame_RT.cols, m_OCV_frame_RT.rows),CV_8UC4);
// 
// 					cvtColor(m_OCV_frame_RT, fr_locSize, CV_RGB2RGBA);
// 
// 					CopyMemory(m_pBuf_Bmp, fr_locSize.data, m_Dimension_Img_out.cx * m_Dimension_Img_out.cy *4);
// 				}


//imwrite("123.bmp", fr_locSize);
//////////////////////////////////////////////////////////////////////////
// 				string r;
// 				int  type = fr_locSize.type();
// 				uchar depth = type & CV_MAT_DEPTH_MASK;
// 				uchar chans = 1 + (type >> CV_CN_SHIFT);
// 
// 				switch ( depth ) {
// 					case CV_8U:  r = "8U"; break;
// 					case CV_8S:  r = "8S"; break;
// 					case CV_16U: r = "16U"; break;
// 					case CV_16S: r = "16S"; break;
// 					case CV_32S: r = "32S"; break;
// 					case CV_32F: r = "32F"; break;
// 					case CV_64F: r = "64F"; break;
// 					default:     r = "User"; break;
// 				}
// 
// 				r += "C";
// 				r += (chans+'0');
//////////////////////////////////////////////////////////////////////////

//CV_BGR2HSV //CV_8UC4
//int s =  sizeof(m_OCV_frame_RT.data);
//imshow("video", m_frame_RT);

//return (void*)fr_locSize.data;

//CopyMemory(m_pBuf_Bmp, m_OCV_frame_RT.data, m_Dimension_Img_out.cx * m_Dimension_Img_out.cy *3);
return (void*)m_pBuf_Bmp;
}
}

return (void*)m_pBuf_Bmp;
}
*/
#pragma once

#include "WriterMPEG.h"
#include "CommonIP.h"
//#include "NetControlReceive.h"

#include "Synh_Objs.h"

using namespace IP_Camera;

class CProcOpenCV
{
public:
	CProcOpenCV(void);
	~CProcOpenCV(void);

	// create stream IP using path stream of the camera
	bool Create_StreamIP(const char *path_strm_in, int nmb_Camera_in);

	// the only step grabber
	const void* GrabStep(void);

	// Get IP Camera parameters(using OpenCV API)
	int GetCameraParams(	T_VideoParams &video_params_out		// video parameters
											);

	// Get IP Camera parameters(using OpenCV API)
	int Get_3Params(	float &fps_dev,		// the number frames per second getting from device(CameraIP)
										UINT &img_W_dev,	// width image from device(CameraIP)
										UINT &img_H_dev		// height image from device(CameraIP)
									);

	// set name video file for recording
	void SetNameVideoRec(char* p_name_video);

	// is open video stream
	bool IsOpenedStream(void) const;

//	const Mat*	GetReadImg(void){ if (m_pReadData){ return m_pReadData->image;} return NULL; }

	// write the single frame
	int WriteFrameMPEG(void);

	// writing finish video frame(for correct saving)
	int FinishWritingMPEG(void);

	void *m_pBuf_out;			// pointer for output to buffer BMP (24 bit / pixel)

	// initialization OpenCV using
	int Init(	int width_in,
						int height_in,
						int channel_in
					);

	// Releasing OpenCV ( context, memory, etc )
	void Release(void);

	// get reading Image(buffer) size
	int GetBuffer_Size( void );

/*
	// Set the bit per pixel for getting image
	int SetBitsPerPxl_Img(const	unsigned int bit_pxl_in		// bit per pixel for getting image
												);

	// Get the bit per pixel for getting image
	const int GetBitsPerPxl_Img( void );
*/
	 bool IsEnable(void) const;

	 // get state recording
	 unsigned int GetStateRec(void) const;

	 // threading function for grabbing frame each time 
	 static DWORD Func_Grab_RTSP_Thread(LPVOID lpParam);


public:
	cv::VideoCapture	m_OCV_capture;	// class for picture grabbing

	// critical section for frame RTSP thread
	CSynh_CritSect		m_SynhCS_RTSP_Frame;

	Mat					m_frame_Reading;

	char m_nm_video_stream[_MAX_PATH];

	int m_number_Cmr;

private:
	//Mat m_OCV_frame_RT;	// frame real Time

	// !!! FOR WRITE !!!
	//cv::VideoWriter m_OCV_Writer;		// class for writing video to HDD

	//UINT m_WriteState;	//CommonData::T_DeviceState
	char m_nm_video_rec[_MAX_PATH];

	//
	CWriterMPEG	m_WriterMPEG;

	// pointer for reading data OpenCV using
	GraberData	*m_pReadData;


	int m_ReadingWidth;
	int m_ReadingHeight;
	int m_Bit_Pxls;

	bool m_bFlagEnable;			// flag are used to show enable class

	// Thread parameters
	HANDLE		m_hGrabThread_RTSP;
	DWORD			m_dwGrabThreadId_RTSP;

	volatile LONG m_FlagGrabbing_RTSP;
	volatile LONG m_FlagStreamIsOpen_RTSP;
	volatile LONG m_FlagGetParams_RTSP;

	T_VideoParams m_video_params_RTPS;

	// object for event for send PTZ command 
	HANDLE m_hEvent_InitOCV;
};


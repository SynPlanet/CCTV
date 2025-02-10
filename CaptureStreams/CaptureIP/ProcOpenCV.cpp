#include "StdAfx.h"
#include "ProcOpenCV.h"


CProcOpenCV::CProcOpenCV(void)
{
	m_pBuf_out = NULL;
	m_pReadData = NULL;

	m_ReadingWidth = 0;

	m_bFlagEnable = true;

	m_hGrabThread_RTSP = 0;
	m_dwGrabThreadId_RTSP = 0;

	InterlockedExchange(&m_FlagGrabbing_RTSP, 0);
	InterlockedExchange(&m_FlagStreamIsOpen_RTSP, 0);
	InterlockedExchange(&m_FlagGetParams_RTSP, 0);

	sprintf(m_nm_video_rec, "");
	sprintf(m_nm_video_stream, "");
	
}


CProcOpenCV::~CProcOpenCV(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
// initialization OpenCV using
int CProcOpenCV::Init(	int width_in,
												int height_in,
												int channel_in
											)
{
	m_ReadingWidth = width_in;
	m_ReadingHeight = height_in;
	m_Bit_Pxls = channel_in * 8;

	// create whole data for callback functions
	m_pReadData = ( GraberData* )malloc( sizeof( *m_pReadData ) );
	m_pReadData->image = NULL;
	m_pReadData->mutex = NULL;
	//m_pReadData->mutex = CreateMutex(NULL, FALSE, NULL);

// 	m_pReadData->image = new Mat(m_ReadingWidth, m_ReadingHeight, CV_8UC3);
// 	m_pReadData->pixels = (unsigned char *)m_pReadData->image->data;

	//m_pBuf_out = m_pReadData->pixels;

	m_bFlagEnable = true;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Releasing OpenCV ( context, memory, etc )
void CProcOpenCV::Release(void)
{
	m_bFlagEnable = false;

	// release thread grabbing
	if (m_hGrabThread_RTSP) {
		InterlockedExchange(&m_FlagGrabbing_RTSP, 0);
		InterlockedExchange(&m_FlagStreamIsOpen_RTSP, 0);
		InterlockedExchange(&m_FlagGetParams_RTSP, 0);

		// take time for thread to resume work
		WaitForSingleObject(m_hGrabThread_RTSP, INFINITE);

		CloseHandle(m_hGrabThread_RTSP);

		m_hGrabThread_RTSP = NULL;

		printf("RTSP(%s)->Release_Grab_CameraIP[%d] -> OK!\n", m_nm_video_stream, m_number_Cmr);
	}


	if (m_pReadData){
		if (m_pReadData->image){
			m_pReadData->image->release();

			delete m_pReadData->image;
			m_pReadData->image = NULL;
		}
		if (m_pReadData->mutex){
			ReleaseMutex(m_pReadData->mutex);
			m_pReadData->mutex = NULL;
		}

		free(m_pReadData);
		m_pReadData = NULL;
	}

	m_OCV_capture.release();
	m_frame_Reading.release();

	/////
	//m_OCV_frame_RT.release();
	//m_OCV_Writer.release();
}

//////////////////////////////////////////////////////////////////////////
// create stream IP using path stream of the camera
bool CProcOpenCV::Create_StreamIP(const char *path_strm_in, int nmb_Camera_in)
{
	bool res = false;
	size_t i = 0;
	DWORD get_res_thread = 0;

	//release old memory 
	if (m_hGrabThread_RTSP) {
		Release();
	}

	sprintf(m_nm_video_stream, "%s", path_strm_in);

	m_number_Cmr = nmb_Camera_in;

	/*
	// create event for send PTZ command 
	m_hEvent_InitOCV = CreateEvent(	NULL,		// EventAttributes
																	TRUE,		// ManualReset
																	FALSE,	// InitialState
																	NULL		// Name
																);
																*/
	//res = m_OCV_capture.open(m_nm_video_stream);

	m_hGrabThread_RTSP = CreateThread(	NULL,
																			0,
																			(LPTHREAD_START_ROUTINE)Func_Grab_RTSP_Thread,
																			(void*)this,
																			0,
																			&m_dwGrabThreadId_RTSP
																		);
	
	if (m_hGrabThread_RTSP) {

		// set flag opened stream OpenCV
		InterlockedExchange(&m_FlagStreamIsOpen_RTSP, 1);

		bool err = SetThreadPriority(m_hGrabThread_RTSP, THREAD_PRIORITY_HIGHEST);
		//Sleep(2500);

		return true;
	}
		/*
		////////////////////////////////////////
		DWORD dwWaitResult = WAIT_FAILED;

		// synh. object (set sleep 33 Hz)
		dwWaitResult = WaitForSingleObject(m_hEvent_InitOCV, 5000); //INFINITE

		// release event 
		if (m_hEvent_InitOCV) {
			// rest output event 
			ResetEvent(m_hEvent_InitOCV);// rest output event 

			CloseHandle(m_hEvent_InitOCV);
			m_hEvent_InitOCV = NULL;
		}

		switch (dwWaitResult) {
			// Event object was signaled
			case WAIT_OBJECT_0:
			{
				if (InterlockedCompareExchange(&m_FlagStreamIsOpen_RTSP, 1, 1) == 1) {

					return true;
				}
			}break;

			default: break;
		}
	*/

	return false;
}

//////////////////////////////////////////////////////////////////////////
// is open video stream
bool CProcOpenCV::IsOpenedStream(void) const
{
	//return m_OCV_capture.isOpened();
	return (bool)m_FlagStreamIsOpen_RTSP;
}

//////////////////////////////////////////////////////////////////////////
// get reading Image(buffer) size
int CProcOpenCV::GetBuffer_Size( void )
{
	int out_vl = 0;

	if (m_pReadData){
		//out_vl = m_pReadData->image->elemSize();
		out_vl = m_frame_Reading.elemSize();

		return	out_vl;
	}
	return out_vl;
}

//////////////////////////////////////////////////////////////////////////
// the only step grabber
const void* CProcOpenCV::GrabStep( void )
{
	if (!m_pReadData || !m_bFlagEnable){
		return false;
	}
	//now start grabbing frames and storing into the Mat buffer
	//m_OCV_capture >> m_OCV_frame_RT;
	// OR
	//bool b_flag_grabber = m_OCV_capture.read(*m_pReadData->image);

	//bool b_flag_grabber = m_OCV_capture.read(m_frame_Reading);

//	m_OCV_capture >> m_frame_Reading;		/// torvas [01.08.2018]

	m_SynhCS_RTSP_Frame.Set_Critical_Section();

	bool b_flag_grabber = !m_frame_Reading.empty();	/// torvas [01.08.2018]

	if (b_flag_grabber){
		{
			m_pBuf_out = m_frame_Reading.data;
		}

		m_SynhCS_RTSP_Frame.Leave_Critical_Section();

		return m_pBuf_out;
	}
	
// 	if (m_BitPerPxl_Img == 24){
// 		if (m_pBuf_Bmp && m_OpenCV.m_pBuf_out){
// 			CopyMemory(m_pBuf_Bmp, m_OCV_frame_RT.data, m_Dimension_Bmp_out.cx * m_Dimension_Bmp_out.cy * 3);
// 		}
// 	}else{
// 		// slowly ( for testing )
// 
// 		Mat fr_locSize(Size(m_OCV_frame_RT.cols, m_OCV_frame_RT.rows),CV_8UC4);
// 
// 		cvtColor(m_OCV_frame_RT, fr_locSize, CV_RGB2RGBA);
// 
// 		CopyMemory(m_pBuf_Bmp, fr_locSize.data, m_Dimension_Bmp_out.cx * m_Dimension_Bmp_out.cy * 4);
// 	}

	m_SynhCS_RTSP_Frame.Leave_Critical_Section();

	return m_pBuf_out = NULL;
}

//////////////////////////////////////////////////////////////////////////
// is enable stream opened
bool CProcOpenCV::IsEnable(void) const
{
	if (m_bFlagEnable && IsOpenedStream()/*m_OCV_capture.isOpened()*/){
		return true;
	}
	return false;
};

//////////////////////////////////////////////////////////////////////////
// Get IP Camera parameters(using VLC API)
int CProcOpenCV::GetCameraParams(	T_VideoParams &video_params_out		// video parameters
								)
{
	if (m_bFlagEnable && IsOpenedStream()/*m_OCV_capture.isOpened()*/) {

		// Get Full Information about stream( see: "opencv2refman.pdf")

		//InterlockedExchange(&m_FlagGetParams_RTSP, 1);

		m_SynhCS_RTSP_Frame.Set_Critical_Section();

		CopyMemory((void *)(&video_params_out), (void*)&m_video_params_RTPS, sizeof(T_VideoParams));
		
		m_SynhCS_RTSP_Frame.Leave_Critical_Section();

/*
		// get the main parameters
		Get_3Params(video_params_out.FPS, (UINT&)video_params_out.width_frame, (UINT&)video_params_out.height_frame);

		int codec_code = (int)m_OCV_capture.get(CV_CAP_PROP_FOURCC);	// 4-character code of codec
		// Transform from int to char via Bitwise operators
		//char EXT[] = {(char)(codec_code & 0XFF) , (char)((codec_code & 0XFF00) >> 8),(char)((codec_code & 0XFF0000) >> 16),(char)((codec_code & 0XFF000000) >> 24), 0};

		//	sprintf_s(m_VideoParams.codec_code, 10, "%s%s%s%s", (char)(codec_code & 0XFF) , (char)((codec_code & 0XFF00) >> 8),(char)((codec_code & 0XFF0000) >> 16),(char)((codec_code & 0XFF000000) >> 24));
		
		video_params_out.format_Mat = (int)m_OCV_capture.get(CV_CAP_PROP_FORMAT);		//Format of the Mat objects returned by retrieve() .
		video_params_out.mode_capture = (int)m_OCV_capture.get(CV_CAP_PROP_MODE);		//Backend-specific value indicating the current capture mode
		video_params_out.brightness = m_OCV_capture.get(CV_CAP_PROP_BRIGHTNESS);		//Brightness of the image
		video_params_out.contrast = m_OCV_capture.get(CV_CAP_PROP_CONTRAST);				//Contrast of the image
		video_params_out.saturation = m_OCV_capture.get(CV_CAP_PROP_SATURATION);		//Saturation of the image
		video_params_out.hue = m_OCV_capture.get(CV_CAP_PROP_HUE);									//Hue of the image
		video_params_out.gain = m_OCV_capture.get(CV_CAP_PROP_GAIN);								//Gain of the image
		video_params_out.exposure = m_OCV_capture.get(CV_CAP_PROP_EXPOSURE);				//Exposure
		video_params_out.sharpness = m_OCV_capture.get(CV_CAP_PROP_SHARPNESS);			//Sharpness

// 		if( (m_VideoParams.FPS != video_params_out.FPS)
// 			||
// 			(m_VideoParams.width_frame != video_params_out.width_frame)
// 			||
// 			(m_VideoParams.height_frame != video_params_out.height_frame)
// 			){
// 
// 				CopyMemory((void *)(&video_params_out), (void*)&m_VideoParams, sizeof(T_VideoParams));
// 				return 1;	// there were the changes to IP Cameras
// 		}
//		CopyMemory((void *)(&video_params_out), (void*)&m_VideoParams, sizeof(T_VideoParams));
*/


		return 0;	// there were not the changes to IP Cameras
	}

	return -1;	// the stream is not opened
}

//////////////////////////////////////////////////////////////////////////
// Get IP Camera parameters(using OpenCV API)
int CProcOpenCV::Get_3Params(	float &fps_dev,		// the number frames per second getting from device(CameraIP)
															UINT &img_W_dev,	// width image from device(CameraIP)
															UINT &img_H_dev		// height image from device(CameraIP)
														)
{
	if (IsOpenedStream()){

		m_SynhCS_RTSP_Frame.Set_Critical_Section();

		// Get Information about stream( see: "opencv2refman.pdf")
		fps_dev = 25.0f; // (float)m_OCV_capture.get(CV_CAP_PROP_FPS);	// Frame rate

// 		img_W_dev = (int)m_OCV_capture.get(CV_CAP_PROP_FRAME_WIDTH);		// Width of the frames in the video stream
// 		img_H_dev = (int)m_OCV_capture.get(CV_CAP_PROP_FRAME_HEIGHT);	// Height of the frames in the video stream
		//OR 
		//if (m_pReadData){
		//	 img_W_dev = m_pReadData->image->cols;	// 		img_W_dev = m_OCV_frame_RT.cols;
		//	 img_H_dev = m_pReadData->image->rows;	// 		img_H_dev = m_OCV_frame_RT.rows;
		//}

		if (!m_frame_Reading.empty()){
			img_W_dev = m_frame_Reading.cols;	
			img_H_dev = m_frame_Reading.rows;	
		}else{
			img_W_dev = m_video_params_RTPS.width_frame;	// Width of the frames in the video stream
			img_H_dev = m_video_params_RTPS.height_frame;	// Height of the frames in the video stream

			//img_W_dev = (int)m_OCV_capture.get(CV_CAP_PROP_FRAME_WIDTH);		// Width of the frames in the video stream
			//img_H_dev = (int)m_OCV_capture.get(CV_CAP_PROP_FRAME_HEIGHT);	// Height of the frames in the video stream
		}

		m_SynhCS_RTSP_Frame.Leave_Critical_Section();
		return 0;	// there were not the changes to IP Cameras
	}

	return -1;	// the stream is not opened
}

//////////////////////////////////////////////////////////////////////////
// set name video file for recording
void CProcOpenCV::SetNameVideoRec(char* p_name_video)
{
	sprintf(m_nm_video_rec, "%s", p_name_video);
}

//////////////////////////////////////////////////////////////////////////
// write the single frame
int CProcOpenCV::WriteFrameMPEG(void)
{
	m_SynhCS_RTSP_Frame.Set_Critical_Section();

	bool res = false;
	if (!m_WriterMPEG.IsOpenedStream()){
		// try to create file for writing
		res = m_WriterMPEG.OpenStream2Write(	(const char*)m_nm_video_rec,
												25.0f, //m_OCV_capture.get(CV_CAP_PROP_FPS),
												m_video_params_RTPS.width_frame, //m_OCV_capture.get(CV_CAP_PROP_FRAME_WIDTH),
												m_video_params_RTPS.height_frame //m_OCV_capture.get(CV_CAP_PROP_FRAME_HEIGHT)
											);

		m_SynhCS_RTSP_Frame.Leave_Critical_Section();

		return res;
	}else{
		// the stream for writing was opened
		// => put in the frame to the stream
		if (!m_frame_Reading.empty()){
			res = m_WriterMPEG.WriteFrame( /*m_pReadData->image->data*/ m_frame_Reading.data);

			if (res){
				m_SynhCS_RTSP_Frame.Leave_Critical_Section();

				return res;

// 				if (p_Set_PlayerState){
// 					UINT st = (UINT)T_DeviceState::PLAY;
// 					p_Set_PlayerState(nmb_Cmr, st);
// 				}
			}

		}
	}

	//if (m_OCV_Writer.isOpened()){
	//	m_OCV_Writer.write(m_OCV_frame_RT);
	//}else{
	//	m_OCV_Writer.open(m_nm_video_rec,  
	//															 m_OCV_capture.get(CV_CAP_PROP_FOURCC),
	//															 m_OCV_capture.get(CV_CAP_PROP_FPS), 
	//															 cv::Size(
	//																				m_OCV_capture.get(CV_CAP_PROP_FRAME_WIDTH),
	//																				m_OCV_capture.get(CV_CAP_PROP_FRAME_HEIGHT)
	//																				)
	//															);
	//}
	m_SynhCS_RTSP_Frame.Leave_Critical_Section();

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// writing finish video frame(for correct saving)
int CProcOpenCV::FinishWritingMPEG(void)
{
	m_WriterMPEG.StopStream();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// get state recording
unsigned int CProcOpenCV::GetStateRec(void) const {
	return m_WriterMPEG.GetStateRec();
};

//////////////////////////////////////////////////////////////////////////
// threading function for grabbing frame each time 
DWORD CProcOpenCV::Func_Grab_RTSP_Thread(LPVOID lpParam)
{
	CProcOpenCV *p_ProcOpenCV = (CProcOpenCV *)lpParam;

	// just in case
	if (!p_ProcOpenCV) {
		return 1;
	}
	bool res = false;

	// try open stream by OpenCV
	res = p_ProcOpenCV->m_OCV_capture.open(p_ProcOpenCV->m_nm_video_stream);
	/*
	if (res) {
		InterlockedExchange(&p_ProcOpenCV->m_FlagStreamIsOpen_RTSP, 1);
	} else {
		InterlockedExchange(&p_ProcOpenCV->m_FlagStreamIsOpen_RTSP, 0);
	}

	// set event for testing timeout OpenCV ->open stream
	SetEvent(p_ProcOpenCV->m_hEvent_InitOCV);
	*/
	// check stream OpenCV is opened
	if (!p_ProcOpenCV->m_OCV_capture.isOpened()) {
		printf("RTSP(%s)->Was not opened[%d] -> OK!\n", p_ProcOpenCV->m_nm_video_stream, p_ProcOpenCV->m_number_Cmr);

		InterlockedExchange(&p_ProcOpenCV->m_FlagStreamIsOpen_RTSP, 0);
 		return 2;
	} else {
		InterlockedExchange(&p_ProcOpenCV->m_FlagStreamIsOpen_RTSP, 1);
		InterlockedExchange(&p_ProcOpenCV->m_FlagGrabbing_RTSP, 1);

		printf("RTSP(%s)->Was opened OK!!! -> Start_Grab_CameraIP[%d]\n", p_ProcOpenCV->m_nm_video_stream, p_ProcOpenCV->m_number_Cmr);
	}

	//res = SetThreadPriority(p_ProcOpenCV->m_hGrabThread_RTSP, THREAD_PRIORITY_NORMAL/*THREAD_PRIORITY_ABOVE_NORMAL*/);

	//Mat frame_loc;

	while (InterlockedCompareExchange(&p_ProcOpenCV->m_FlagGrabbing_RTSP, 1, 1)) {

		if (p_ProcOpenCV->m_OCV_capture.isOpened()) {
			InterlockedExchange(&p_ProcOpenCV->m_FlagStreamIsOpen_RTSP, 1);

			/////
			p_ProcOpenCV->m_SynhCS_RTSP_Frame.Set_Critical_Section();
			/////

			// set grabbing frame
			//bool b_flag_grabber = p_ProcOpenCV->m_OCV_capture.read(frame_loc);
			//bool flag_grab = p_ProcOpenCV->m_OCV_capture.grab();

			p_ProcOpenCV->m_OCV_capture.read(p_ProcOpenCV->m_frame_Reading);

			// duplicate
			//p_ProcOpenCV->m_OCV_capture.read(p_ProcOpenCV->m_frame_Reading);
			//

			//if (InterlockedCompareExchange(&p_ProcOpenCV->m_FlagGetParams_RTSP, 1, 1)) {

				// Get Information about stream( see: "opencv2refman.pdf")
				p_ProcOpenCV->m_video_params_RTPS.FPS = 25.0f; // (float)m_OCV_capture.get(CV_CAP_PROP_FPS);	// Frame rate

				if (!p_ProcOpenCV->m_frame_Reading.empty()) {
					p_ProcOpenCV->m_video_params_RTPS.width_frame = p_ProcOpenCV->m_frame_Reading.cols;
					p_ProcOpenCV->m_video_params_RTPS.height_frame = p_ProcOpenCV->m_frame_Reading.rows;
				} else {
					p_ProcOpenCV->m_video_params_RTPS.width_frame = (int)p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_FRAME_WIDTH);		// Width of the frames in the video stream
					p_ProcOpenCV->m_video_params_RTPS.height_frame = (int)p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_FRAME_HEIGHT);	// Height of the frames in the video stream
				}

				int codec_code = (int)p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_FOURCC);	// 4-character code of codec
						// Transform from int to char via Bitwise operators
						//char EXT[] = {(char)(codec_code & 0XFF) , (char)((codec_code & 0XFF00) >> 8),(char)((codec_code & 0XFF0000) >> 16),(char)((codec_code & 0XFF000000) >> 24), 0};

						//	sprintf_s(m_VideoParams.codec_code, 10, "%s%s%s%s", (char)(codec_code & 0XFF) , (char)((codec_code & 0XFF00) >> 8),(char)((codec_code & 0XFF0000) >> 16),(char)((codec_code & 0XFF000000) >> 24));
				p_ProcOpenCV->m_video_params_RTPS.format_Mat = (int)p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_FORMAT);		//Format of the Mat objects returned by retrieve() .
				p_ProcOpenCV->m_video_params_RTPS.mode_capture = (int)p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_MODE);		//Backend-specific value indicating the current capture mode
				p_ProcOpenCV->m_video_params_RTPS.brightness = p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_BRIGHTNESS);		//Brightness of the image
				p_ProcOpenCV->m_video_params_RTPS.contrast = p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_CONTRAST);				//Contrast of the image
				p_ProcOpenCV->m_video_params_RTPS.saturation = p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_SATURATION);		//Saturation of the image
				p_ProcOpenCV->m_video_params_RTPS.hue = p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_HUE);									//Hue of the image
				p_ProcOpenCV->m_video_params_RTPS.gain = p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_GAIN);								//Gain of the image
				p_ProcOpenCV->m_video_params_RTPS.exposure = p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_EXPOSURE);				//Exposure
				p_ProcOpenCV->m_video_params_RTPS.sharpness = p_ProcOpenCV->m_OCV_capture.get(CV_CAP_PROP_SHARPNESS);			//Sharpness

				// set flag default
				//InterlockedExchange(&p_ProcOpenCV->m_FlagGetParams_RTSP, 0);
			//}

			/////
			p_ProcOpenCV->m_SynhCS_RTSP_Frame.Leave_Critical_Section();
			/////

		} else {
			printf("RTSP(%s)->Stop_Grab_CameraIP[%d]\n", p_ProcOpenCV->m_nm_video_stream, p_ProcOpenCV->m_number_Cmr);

			InterlockedExchange(&p_ProcOpenCV->m_FlagStreamIsOpen_RTSP, 0);
			InterlockedExchange(&p_ProcOpenCV->m_FlagGrabbing_RTSP, 0);

			return 3;
		}

		Sleep(1);
	}

	return 0;
}
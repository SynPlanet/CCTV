#pragma once

#include "CommonIP.h"
#include "MetaMoviesHDD.h"


// GPU
// https://github.com/Itseez/opencv/tree/master/samples/gpu/
// https://github.com/Itseez/opencv/blob/master/samples/gpu/video_reader.cpp
using namespace IP_Camera;

class CProcPlayerOCV
{
public:
	CProcPlayerOCV(void);
	~CProcPlayerOCV(void);

	// create stream IP using path stream of the camera
	bool Create_StreamIP(const char *path_strm_in);

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

	// set states for control movie
	bool ControlMovie(	const LONG state_in,							// see T_DeviceState
											const LONGLONG global_time_msec_in		// global time to play
										);

	 bool IsEnable(void) const;

	 // get state playback
	 unsigned int GetStatePlayback(void) const;

	 // create stream file using path stream of the camera
	 bool Create_StreamFile(const CMetaMoviesHDD*	p_metadata_movie_in, const unsigned int nmb_movie_in );
	 
	 // get real time was played (global time)
	 LONGLONG Get_RT_Played(void) const { return m_TotalTimePlayed; }

	 // get state real time
	 LONG Get_RT_State(void) const { return m_State; }

	 // set time initial play
	 //void SetTimeStartMovie(unsigned int tm_start_play_in);

private:
	// read file initialization (get offset duration video from ini file)
	void ReadIniFile( void );

	// open stream file if it is possible
	bool OpenStreamFile(const unsigned int nmb_file_rec_in);

	// get/update metadata rec.file
	bool GetMetaRecFile(	unsigned int nmb_file_rec_in,			// number rec file for searching
												bool index_nmb_flag_in						// the flag define of searching mode (using index number or not)
											);

	// get next rec.file
	unsigned int GetNextRecFile(	unsigned int nmb_rec_file_in			// number rec file for the searching next one
															);

	cv::VideoCapture	m_OCV_capture;	// class for picture grabbing
	//Mat m_OCV_frame_RT;	// frame real Time

//	Cam_Movie *m_pMetaDataMovie;
	Cam_Movie m_md_movie;				// class includes of movie metadata (and information about joined file recording)
	Rec_Info	m_md_rec_file;		// current file recording is playing

	//UINT m_WriteState;	//CommonData::T_DeviceState
	char m_nm_path_video[_MAX_PATH];
	//

	LONGLONG r_t_play_file_msec;	// time playing of file in real time

//	unsigned int m_nmb_CamMovie_RT;	// number Cam_Movie is playing realtime
	LONG m_nmb_RecInfo_RT;	// number Rec_Info is playing realtime

	// pointer for reading data OpenCV using
	GraberData	*m_pReadData;
	Mat					m_frame_Reading;

	int m_ReadingWidth;
	int m_ReadingHeight;
	int m_Bit_Pxls;

	volatile LONG m_State;	//CommonData::T_DeviceState

	volatile LONGLONG m_TotalTimePlayed;	// time movie was played

	volatile LONG m_TotalLengthTrack;	// total length track

	bool m_bFlagEnable;			// flag are used to show enable class

	// time playing offset get from file INI
	volatile LONG m_lOffsetTimePlay_INI;

public:
	BOOL m_bFlag_FirstObj;	// the flag need to control function "p_GetTimeMovie" ( It's for Player!!! see.CProcPlayerOCV)

};


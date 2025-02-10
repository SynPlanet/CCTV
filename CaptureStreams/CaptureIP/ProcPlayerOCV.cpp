#include "StdAfx.h"
#include "ProcPlayerOCV.h"


extern GetTimeMovie	p_GetTimeMovie;

#include "../AudioStreamer/AudioStreamerData.h"
extern AudioDevice::AudioData g_AudioData;

CProcPlayerOCV::CProcPlayerOCV(void)
{
	m_pBuf_out = NULL;
	m_pReadData = NULL;

	m_ReadingWidth = 0;

	r_t_play_file_msec = 0;
//	m_nmb_CamMovie_RT = 0;
	m_nmb_RecInfo_RT = 0;

	sprintf(m_nm_path_video, "");

	m_md_movie.fps_hz =	m_md_movie.frames =	m_md_movie.width = m_md_movie.height = m_md_movie.total_leng_msec = 0;
	sprintf(m_md_movie.name, "");
	m_md_movie.list_nmb_File.clear();

	m_md_rec_file.frames = m_md_rec_file.length_msec = m_md_rec_file.nmb_rec = m_md_rec_file.lng_total_begin_msec = 0;

	m_bFlagEnable = true;

	m_bFlag_FirstObj = FALSE;

	InterlockedExchange(&m_State,  (LONG)T_DeviceState::STOP);
	InterlockedExchange64(&m_TotalTimePlayed, 0);

	InterlockedExchange(&m_TotalLengthTrack, (0xffffffff) -1);

	InterlockedExchange(&m_lOffsetTimePlay_INI, 1000);

	ReadIniFile();
}

CProcPlayerOCV::~CProcPlayerOCV(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
// initialization OpenCV using
int CProcPlayerOCV::Init(	int width_in,
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

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Releasing OpenCV ( context, memory, etc )
void CProcPlayerOCV::Release(void)
{
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

	m_md_movie.list_nmb_File.clear();

	m_bFlagEnable = false;

	/////
	//m_OCV_frame_RT.release();
	//m_OCV_Writer.release();
}

//////////////////////////////////////////////////////////////////////////
// read file initialization (get offset duration video from ini file)
void CProcPlayerOCV::ReadIniFile( void )
{
	LONG val_INI_loc = 0;

	// get offset channel using by Asio Writer/Reader
	val_INI_loc = GetPrivateProfileIntA(NAME_AUDIO_VIDEO_COMMON, PREFIX_TIME_DELAY_AUDIOPLAYER_MS, m_lOffsetTimePlay_INI, FILENAME_INI_DEVICE);

	InterlockedExchange(&m_lOffsetTimePlay_INI, val_INI_loc);

	InterlockedExchange64(&m_TotalTimePlayed, m_lOffsetTimePlay_INI);
	//printf("m_lOffsetTimePlay_INI = %d\n\n\n", m_lOffsetTimePlay_INI);

}

//////////////////////////////////////////////////////////////////////////
// open stream file if it is possible
bool CProcPlayerOCV::OpenStreamFile(const unsigned int nmb_file_rec_in)
{
	if (m_OCV_capture.isOpened()){
		if (nmb_file_rec_in == m_nmb_RecInfo_RT){
			return true;
		}else{
			// close old video file
			m_OCV_capture.set(CV_CAP_PROP_FRAME_COUNT, 0 );
			//m_OCV_capture.set(CV_CAP_PROP_POS_MSEC, 0 );
			m_OCV_capture.release();

			// and open new right stream
			{
				char full_filename[_MAX_PATH];
				sprintf(full_filename, "%s\\%s%s%d%s", m_nm_path_video, m_md_movie.name, NAME_VIDEO_REC_SUFFIX, nmb_file_rec_in, NAME_VIDEO_REC_TAIL);
				if (m_OCV_capture.open(full_filename)){
					m_nmb_RecInfo_RT = nmb_file_rec_in;

					GetMetaRecFile(m_nmb_RecInfo_RT, false);
					return true;
				}
			}
		}
	}else{
		// open new stream
		{
			char full_filename[_MAX_PATH];
			sprintf(full_filename, "%s\\%s%s%d%s", m_nm_path_video, m_md_movie.name, NAME_VIDEO_REC_SUFFIX, nmb_file_rec_in, NAME_VIDEO_REC_TAIL);

			if (m_OCV_capture.open(full_filename)){
				m_nmb_RecInfo_RT = nmb_file_rec_in;

				GetMetaRecFile(m_nmb_RecInfo_RT, false);
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// set states for control movie
bool CProcPlayerOCV::ControlMovie(	const LONG state_in,							// see T_DeviceState
																		const LONGLONG global_time_msec_in		// global time to play
																	)
{
	LONGLONG	new_time_play_msec = global_time_msec_in;

	InterlockedExchange(&m_State, state_in);

	// set state for the player only
	if (global_time_msec_in == -1){
//		InterlockedExchange64(&m_TotalTimePlayed, 1100);

		new_time_play_msec = m_TotalTimePlayed;
	}else{
		// for seek needed frame
		if (m_State == T_DeviceState::PAUSE){
			InterlockedExchange(&m_State,  (LONG)T_DeviceState::SEEK);
		}
	}

	// just in case
	if (!m_bFlagEnable){
		return false;
	}

	// set new time playing
	if ( (m_State == T_DeviceState::PLAY)
				||
			 (m_State == T_DeviceState::PAUSE)
			 ||
			 (m_State == T_DeviceState::SEEK)
		){

		// set PAUSE movie if was gotten the end of movie
		if (new_time_play_msec >= m_md_movie.total_leng_msec){
			InterlockedExchange(&m_State,  (LONG)T_DeviceState::PAUSE);
			return false;
		}

		// get number file which is playing
		IterListNmbRecMovies it_file_rec = m_md_movie.list_nmb_File.begin();
		IterListNmbRecMovies it_file_end = m_md_movie.list_nmb_File.end();

		// find number file which can play
		for (; it_file_rec != it_file_end; ++it_file_rec){
			// right file was found
			if( (it_file_rec->lng_total_begin_msec <= new_time_play_msec) && (new_time_play_msec <= (it_file_rec->lng_total_begin_msec + it_file_rec->length_msec) )){

				// was file opened correctly !?
				if (m_nmb_RecInfo_RT != it_file_rec->nmb_rec){

					// close current video file and open the right
					if (OpenStreamFile(it_file_rec->nmb_rec)){
						m_OCV_capture.set(CV_CAP_PROP_POS_MSEC, abs(new_time_play_msec - it_file_rec->lng_total_begin_msec) );

						return true;
					}
				}else{
					// file is playing correctly => set new time to play
					m_OCV_capture.set(CV_CAP_PROP_POS_MSEC, abs(new_time_play_msec - it_file_rec->lng_total_begin_msec) );
					return true;
				}

				break;
			}
		}//for (int nmb_file_rec)
	}else{
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
// create stream file using path stream of the camera
bool CProcPlayerOCV::Create_StreamFile(const CMetaMoviesHDD*	p_metadata_movie_in, const unsigned int nmb_movie_in )
{
	m_bFlagEnable = false;
	if (p_metadata_movie_in){

		// save folder path for playing movie
		sprintf(m_nm_path_video, "%s", ((CMetaMoviesHDD*)p_metadata_movie_in)->GetFolderPlaying());

		// just in case
		m_md_movie.list_nmb_File.clear();

		// get metadata class for movie and open stream file if it is exist
		if (	((CMetaMoviesHDD*)p_metadata_movie_in)->GetMetaMovie(nmb_movie_in, m_md_movie) ){

			// as it is the first initialization (open) file AVI - set 'm_nmb_RecInfo_RT' by the first element
			if (GetMetaRecFile(0, true)){

				// get total track length from the first track!!!
				InterlockedExchange(&m_TotalLengthTrack, (LONG)((CMetaMoviesHDD*)p_metadata_movie_in)->GetLengthMovie(0));

				//open stream the first file if it is possible
				if (m_bFlagEnable =	OpenStreamFile(m_md_rec_file.nmb_rec)){

					// if it was good opened -> set PAUSE for the play beginning
					InterlockedExchange(&m_State,  (LONG)T_DeviceState::PAUSE);

					// set one of the first frame (by 1000 msec)
					ControlMovie(m_State, m_lOffsetTimePlay_INI);
				}
 			}

			return m_bFlagEnable;
		}
	}
	return m_bFlagEnable;
}

//////////////////////////////////////////////////////////////////////////
// is open video stream
bool CProcPlayerOCV::IsOpenedStream(void) const
{
	return m_OCV_capture.isOpened();
}

//////////////////////////////////////////////////////////////////////////
// get reading Image(buffer) size
int CProcPlayerOCV::GetBuffer_Size( void )
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
const void* CProcPlayerOCV::GrabStep( void )
{
	if (!m_pReadData || !m_bFlagEnable || !p_GetTimeMovie){
		return false;
	}

	///////////////////
	// set Slider of the player to beginning position
	if (m_bFlag_FirstObj){

		// set limit to maximum length track
		LONG total_time_played_loc = m_TotalTimePlayed;
		if (total_time_played_loc >= m_TotalLengthTrack){
			total_time_played_loc = m_TotalLengthTrack -1;
		}

		//sent output by using callback
		p_GetTimeMovie(total_time_played_loc, m_TotalLengthTrack, m_State);
	}
	///////////////////

	// there is not command "PLAY"
	if( !(InterlockedCompareExchange(&m_State, T_DeviceState::PLAY, T_DeviceState::PLAY) == T_DeviceState::PLAY)	//(m_State != T_DeviceState::PLAY)
				&&
			!(InterlockedCompareExchange(&m_State, T_DeviceState::SEEK, T_DeviceState::SEEK) == T_DeviceState::SEEK)	//(m_State != T_DeviceState::SEEK)
		){

		///	except lost frames (f."GetBMP_NmbCamera" could be slow call than f."GrabStep" -> fill frame was kept early)
		if (InterlockedCompareExchange(&m_State, T_DeviceState::PAUSE, T_DeviceState::PAUSE) == T_DeviceState::PAUSE){ //if(m_State == T_DeviceState::PAUSE)
			return m_pBuf_out = m_frame_Reading.data;
		}
		///
		return false;	
	}

	double d_fps_file = 0;
	unsigned int d_frames_cnt_file = 0;

	//now start grabbing frames and storing into the Mat buffer
	bool b_flag_grabber = m_OCV_capture.read(m_frame_Reading);

	if (b_flag_grabber){

		// get parameters for each step
		r_t_play_file_msec = m_OCV_capture.get(CV_CAP_PROP_POS_MSEC);
		d_frames_cnt_file = (unsigned int)m_OCV_capture.get(CV_CAP_PROP_POS_FRAMES);

		///////////////////
		// calculation time was played
		//m_TotalTimePlayed = m_md_rec_file.lng_total_begin_msec + r_t_play_file_msec;
		InterlockedExchange64(&m_TotalTimePlayed, m_md_rec_file.lng_total_begin_msec + r_t_play_file_msec);

		// get only the single frame and drop the state to pause!!!
		if (InterlockedCompareExchange(&m_State, T_DeviceState::SEEK, T_DeviceState::SEEK) == T_DeviceState::SEEK){ //(m_State == T_DeviceState::SEEK){
			InterlockedExchange(&m_State,  (LONG)T_DeviceState::PAUSE);
		}

		// wait last frame(by millisecond)
		if (m_md_rec_file.frames <= d_frames_cnt_file ){

			// get next rec.file
			unsigned int next_rec_file = GetNextRecFile(m_nmb_RecInfo_RT);

			// rec.file was found
			if (next_rec_file != -1){
				// there is final frame of last rec file => try play next file if it's exist
				if (OpenStreamFile(next_rec_file)){
					;//m_State = T_DeviceState::PLAY;
				}else{
					// just in case
					InterlockedExchange(&m_State,  (LONG)T_DeviceState::SEEK);
					InterlockedExchange64(&m_TotalTimePlayed, m_lOffsetTimePlay_INI);
					ControlMovie(m_State, m_TotalTimePlayed);	// seek new frame 
				}
			}else{
				// rec.files was finished => STOP movie playing
				InterlockedExchange(&m_State,  (LONG)T_DeviceState::SEEK);
				InterlockedExchange64(&m_TotalTimePlayed, m_lOffsetTimePlay_INI);
				ControlMovie(m_State, m_TotalTimePlayed);	// seek new frame 
			}
		}

		return m_pBuf_out = m_frame_Reading.data;
	}else{
		// get next rec.file
		unsigned int next_rec_file = GetNextRecFile(m_nmb_RecInfo_RT);

		// rec.file was found
		if (next_rec_file != -1){
			// there is final frame of last rec file => try play next file if it's exist
			if (OpenStreamFile(next_rec_file)){
				;//m_State = T_DeviceState::PLAY;
			}else{
				// just in case
				InterlockedExchange(&m_State,  (LONG)T_DeviceState::SEEK);
				InterlockedExchange64(&m_TotalTimePlayed, m_lOffsetTimePlay_INI);
				ControlMovie(m_State, m_TotalTimePlayed);	// seek new frame 
			}
		}else{
			InterlockedExchange(&m_State,  (LONG)T_DeviceState::SEEK);
			InterlockedExchange64(&m_TotalTimePlayed, m_lOffsetTimePlay_INI);
			ControlMovie(m_State, m_TotalTimePlayed);	// seek new frame 
		}
	}

	return m_pBuf_out = NULL;
}

//////////////////////////////////////////////////////////////////////////
// get/update metadata rec.file
bool CProcPlayerOCV::GetMetaRecFile(	unsigned int nmb_file_rec_in,			// number rec file for searching
																			bool index_nmb_flag_in						// the flag define of searching mode (using index number or not)
																		)
{
	int cnt_files_rec = m_md_movie.list_nmb_File.size();
	IterListNmbRecMovies it_file_rec = m_md_movie.list_nmb_File.begin();

	if (!index_nmb_flag_in){
		// find of file number which could play
		for (int file_rec = 0; file_rec < cnt_files_rec; ++file_rec){
			if (nmb_file_rec_in == it_file_rec->nmb_rec){

				m_md_rec_file.frames = it_file_rec->frames;
				m_md_rec_file.length_msec = it_file_rec->length_msec;
				m_md_rec_file.nmb_rec = it_file_rec->nmb_rec;
				m_md_rec_file.lng_total_begin_msec = it_file_rec->lng_total_begin_msec;

				return true;
			}
			std::advance(it_file_rec, 1);
		}
	}else{
		//searching by index number using
		if (nmb_file_rec_in < cnt_files_rec){

			std::advance(it_file_rec, nmb_file_rec_in);

			m_md_rec_file.frames = it_file_rec->frames;
			m_md_rec_file.length_msec = it_file_rec->length_msec;
			m_md_rec_file.nmb_rec = it_file_rec->nmb_rec;
			m_md_rec_file.lng_total_begin_msec = it_file_rec->lng_total_begin_msec;

			return true;
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
// get next rec.file
unsigned int CProcPlayerOCV::GetNextRecFile(	unsigned int nmb_rec_file_in			// number rec file for the searching next one
																						)
{
	unsigned int res = -1;

	int cnt_files_rec = m_md_movie.list_nmb_File.size();
	IterListNmbRecMovies it_file_rec = m_md_movie.list_nmb_File.begin();

	// find of file number which could play next
	for (int file_rec = 0; file_rec < cnt_files_rec; ++file_rec){
		if (nmb_rec_file_in < it_file_rec->nmb_rec){

			res = it_file_rec->nmb_rec;

			break;
		}
		std::advance(it_file_rec, 1);
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////
// is enable stream opened
bool CProcPlayerOCV::IsEnable(void) const
{
	if (m_bFlagEnable && m_OCV_capture.isOpened()){
		return true;
	}
	return false;
};

//////////////////////////////////////////////////////////////////////////
// Get IP Camera parameters(using VLC API)
int CProcPlayerOCV::GetCameraParams(	T_VideoParams &video_params_out		// video parameters
																)
{
// 	if (m_OCV_capture.isOpened() && m_bFlagEnable){
// 		// Get Full Information about stream( see: "opencv2refman.pdf")
// 
// 		// get the main parameters
// 		Get_3Params(video_params_out.FPS, (UINT&)video_params_out.width_frame, (UINT&)video_params_out.height_frame);
// 
// 		int codec_code = (int)m_OCV_capture.get(CV_CAP_PROP_FOURCC);	// 4-character code of codec
// 		// Transform from int to char via Bitwise operators
// 		//char EXT[] = {(char)(codec_code & 0XFF) , (char)((codec_code & 0XFF00) >> 8),(char)((codec_code & 0XFF0000) >> 16),(char)((codec_code & 0XFF000000) >> 24), 0};
// 
// 		//	sprintf_s(m_VideoParams.codec_code, 10, "%s%s%s%s", (char)(codec_code & 0XFF) , (char)((codec_code & 0XFF00) >> 8),(char)((codec_code & 0XFF0000) >> 16),(char)((codec_code & 0XFF000000) >> 24));
// 
// 		video_params_out.format_Mat = (int)m_OCV_capture.get(CV_CAP_PROP_FORMAT);		//Format of the Mat objects returned by retrieve() .
// 		video_params_out.mode_capture = (int)m_OCV_capture.get(CV_CAP_PROP_MODE);		//Backend-specific value indicating the current capture mode
// 		video_params_out.brightness = m_OCV_capture.get(CV_CAP_PROP_BRIGHTNESS);		//Brightness of the image
// 		video_params_out.contrast = m_OCV_capture.get(CV_CAP_PROP_CONTRAST);				//Contrast of the image
// 		video_params_out.saturation = m_OCV_capture.get(CV_CAP_PROP_SATURATION);		//Saturation of the image
// 		video_params_out.hue = m_OCV_capture.get(CV_CAP_PROP_HUE);									//Hue of the image
// 		video_params_out.gain = m_OCV_capture.get(CV_CAP_PROP_GAIN);								//Gain of the image
// 		video_params_out.exposure = m_OCV_capture.get(CV_CAP_PROP_EXPOSURE);				//Exposure
// 		video_params_out.sharpness = m_OCV_capture.get(CV_CAP_PROP_SHARPNESS);			//Sharpness
// 
// 		return 0;	// there were not the changes to IP Cameras
// 	}

	return -1;	// the stream is not opened
}

//////////////////////////////////////////////////////////////////////////
// Get IP Camera parameters(using OpenCV API)
int CProcPlayerOCV::Get_3Params(	float &fps_dev,		// the number frames per second getting from device(CameraIP)
															UINT &img_W_dev,	// width image from device(CameraIP)
															UINT &img_H_dev		// height image from device(CameraIP)
														)
{
	if (m_OCV_capture.isOpened()){

		// Get Information about stream( see: "opencv2refman.pdf")
		fps_dev = 25.0f;// (float)m_OCV_capture.get(CV_CAP_PROP_FPS);	// Frame rate

		if (!m_frame_Reading.empty()){
			img_W_dev = m_frame_Reading.cols;	
			img_H_dev = m_frame_Reading.rows;	
		}else{
			img_W_dev = (int)m_OCV_capture.get(CV_CAP_PROP_FRAME_WIDTH);		// Width of the frames in the video stream
			img_H_dev = (int)m_OCV_capture.get(CV_CAP_PROP_FRAME_HEIGHT);	// Height of the frames in the video stream
		}

		return 0;	// there were not the changes to IP Cameras
	}

	return -1;	// the stream is not opened
}

//////////////////////////////////////////////////////////////////////////
// get state playback
unsigned int CProcPlayerOCV::GetStatePlayback(void) const {
	return m_State;
};
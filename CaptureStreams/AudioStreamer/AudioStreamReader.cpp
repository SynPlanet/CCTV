#include "StdAfx.h"
#include "AudioStreamReader.h"
#include <mmsystem.h> 

//#include "resource.h"

//http://users.jyu.fi/~vesal/kurssit/winohj/htyot/h93/cdsoitin/cdaudio.cpp
//http://forum.sources.ru/index.php?showtopic=86024
//////////////////////////////////////////////////////////////////////////

// This is the constructor of a class that has been exported.
// see AudioStreamer.h for the class definition
CAudioStreamReader::CAudioStreamReader()
															:m_nmb_devices(MAX_DEVICES)
{
	//mciSendCommand ( MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL );

	//sprintf(m_PathRecord,".//");
	sprintf(m_BasePathPlay,".//");
	sprintf(m_NameFolderPlay,".//");

	InterlockedExchange(&m_flag_Enable_Loop, 0);
	//InterlockedExchange(&m_TotalTimePlayed, 0);
	InterlockedExchange(&m_state_audio_devs, (LONG)T_DeviceState::PAUSE);

	m_hAudioThread = NULL;
	m_dwAudioThreadId = NULL;

	m_list_AudioTracks.clear();
	m_list_AudioPlayDev.clear();

	//////////////////////////////////////////////////////////////////////////
	// fill list audio device states(mute on/off)
	AudioPlayDevice audio_device_loc;
	ZeroMemory((void*)&audio_device_loc, sizeof(AudioPlayDevice));

	for (int nmb_aud_dev = 0; nmb_aud_dev < m_nmb_devices; ++nmb_aud_dev){
		audio_device_loc.index_nmb = nmb_aud_dev + 1;

		m_list_AudioPlayDev.push_back(audio_device_loc);
	}
	//////////////////////////////////////////////////////////////////////////

	m_pSynh_Bar_StCam_1 = NULL;		// sinh. object barrier the state camera
	m_pSynh_Bar_StCam_2 = NULL;		// sinh. object barrier the state camera

/////	m_pProxyVLC = NULL;

	InterlockedExchange64(&m_lTotalTimePlayedCAMs, 0);
	InterlockedExchange(&m_State_CAMs, 0);
	
	//InterlockedExchange64(&m_lCntFrame_CmnTimerCAMs_init, 0);

	InterlockedExchange(&m_flag_Enable_BarrierOut, 0);
	InterlockedExchange(&m_flag_UpdatePath, 0);
	InterlockedExchange(&m_flag_UpdateState, 0);
	InterlockedExchange(&m_flag_UpdateTime, 0);
	InterlockedExchange64(&m_NewTime_Playing, 0);

	InterlockedExchange64(&m_lOffsetTimePlay_INI, 0);

	// set patch for possibility using MCI in supplementary thread
	InitPatch_MCI();

	return;
}

CAudioStreamReader::~CAudioStreamReader()
{
	// release patch MCI
	//ReleasePatch_MCI();

	ReleaseAll();

	/* close sound device */
	//mciSendCommand ( MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL );

	return;
}

//////////////////////////////////////////////////////////////////////////
// set patch for possibility using MCI in supplementary thread
void CAudioStreamReader::InitPatch_MCI(void)
{
	MCIERROR mciError;

	m_Patch_ParmsMCI.lpstrDeviceType = L"MPEGVideo";
	m_Patch_ParmsMCI.lpstrElementName = L".//Default.mp3";
	mciError = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&m_Patch_ParmsMCI);

	// printing error from MCI
	GetErrorMCI(	mciError, "OPEN ./Default.mp3" );
}

//////////////////////////////////////////////////////////////////////////
// release patch MCI
void CAudioStreamReader::ReleasePatch_MCI(void)
{
	MCIERROR mciError;

	// close MCI sound device was opened
	mciError = mciSendCommand ( m_Patch_ParmsMCI.wDeviceID, MCI_CLOSE, MCI_WAIT, NULL );

	// printing error from MCI
	GetErrorMCI(	mciError, "CLOSE ./Default.mp3" );
}

//////////////////////////////////////////////////////////////////////////
// initialization data 
void CAudioStreamReader::Init(void)
{
	ReadIniFile();
}

//////////////////////////////////////////////////////////////////////////
// initialization data for play audio
void CAudioStreamReader::Connect(void)
{
	InterlockedExchange(	&m_flag_Enable_BarrierOut, 1 );

	// set initialization
	if (!LaunchThreadHandlerDevice()){ // audio thread was started
		InterlockedExchange(&m_flag_Enable_Loop, 1);
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// release only necessary member's class
bool CAudioStreamReader::Disconnect(void)
{
	// delete audio thread
	ReleaseAudioThread();

	InterlockedExchange( &m_flag_Enable_BarrierOut, 0 );

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// release all class members
bool CAudioStreamReader::ReleaseAll(void)
{
	// delete audio thread
	ReleaseAudioThread();
	return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// read file initialization (get offset duration audio from ini file)
void CAudioStreamReader::ReadIniFile( void )
{
	LONG val_INI_loc = 0;

	// get offset channel using by Asio Writer/Reader
	val_INI_loc = GetPrivateProfileIntA(NAME_AUDIO_VIDEO_COMMON, PREFIX_TIME_DELAY_AUDIOPLAYER_MS, 0, FILENAME_INI_DEVICE);

	InterlockedExchange64(&m_lOffsetTimePlay_INI, (LONGLONG)val_INI_loc);
}

//////////////////////////////////////////////////////////////////////////
/*
// get VLC proxy(by using module directly)
bool CAudioStreamReader::SetProxyVLC(void* pProxyVLC_in)
{
/////	m_pProxyVLC = (CProxyVLC*)pProxyVLC_in;

	return true;
}
*/

//////////////////////////////////////////////////////////////////////////
// get synchronize objects for output control(camera control)
void CAudioStreamReader::SetBarrierCtrlCam(	void *pSynh_Barrier_StateCam_1_in,
																						void *pSynh_Barrier_StateCam_2_in
																					)
{
	m_pSynh_Bar_StCam_1 = (CSynh_Barrier*)pSynh_Barrier_StateCam_1_in;
	m_pSynh_Bar_StCam_2 = (CSynh_Barrier*)pSynh_Barrier_StateCam_2_in;
}

//////////////////////////////////////////////////////////////////////////
// get count streams reading(using barrier)
unsigned int CAudioStreamReader::GetCountStreams(	void )
{
	if(InterlockedCompareExchange(&m_flag_Enable_Loop, 1,1)){
		return 1;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set need record(by numbers) for each track in input state (corrected all others)
DWORD CAudioStreamReader::PlayAllTracksByTime(	int		nmb_track_rec_in,		// order number of track record to set state
																								DWORD state_in,						// state track in
																								LONGLONG tm_play_own_in			// global time for playing(not local)
																							)
{
	m_Sync_SWMR.WaitToRead();
	{
		// play one by one trackes [0-N]
		int cnt_Tracks = m_list_AudioTracks.size();
		IterAudioTrack it_Track = m_list_AudioTracks.begin();

		int cnt_TrackRecs = 0;
		IterRecAudioInfo it_rec_loc;
		int cnt_recs = 0;

		// search all tracks
		for (int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){

			// searching all recording into track list
			cnt_recs = it_Track->list_nmb_File.size();
			it_rec_loc = it_Track->list_nmb_File.begin();

			// search all recordings in to track
			for (int nmb_rec_loc = 0; nmb_rec_loc < cnt_recs; ++nmb_rec_loc){

				// just in case
				if (!it_rec_loc->mci_real_ID){
					continue;
				}

				//////////////////////////////////////////////////////////////////////////
				if (nmb_track_rec_in == nmb_rec_loc){

					SetVolume(it_rec_loc->mci_real_ID, !it_Track->mute_flag);

					// set new position for play Audio recording
					SetTime_MediaMCI(tm_play_own_in, it_rec_loc, state_in);

				}else{
					SetVolume(it_rec_loc->mci_real_ID, !it_Track->mute_flag);

					SetTime_MediaMCI(-1, it_rec_loc, T_DeviceState::PAUSE);
					//SetState_MediaMCI(T_DeviceState::PAUSE, it_rec_loc);
				}
				//////////////////////////////////////////////////////////////////////////
				std::advance(it_rec_loc, 1);
			}//for(nmb_Track_loc)

			std::advance(it_Track, 1);
		}//for(nmb_Track_loc)
	}
	m_Sync_SWMR.Done(); 

	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CAudioStreamReader::SetState_MediaMCI(	DWORD							state_in,						// state track in
																						IterRecAudioInfo	it_rec_in
																					)
{
	MCIERROR mciError; 
	MCI_PLAY_PARMS mciPlay; 
	memset (&mciPlay, 0x00, sizeof (MCI_PLAY_PARMS));

	switch (state_in){
			case ((LONG)T_DeviceState::PLAY):
				{
					mciError = mciSendCommand (it_rec_in->mci_real_ID, MCI_PLAY, 0, (DWORD) &mciPlay);

					// printing error from MCI
					GetErrorMCI(	mciError, "send MCI_PLAY command failed" );
				}
				break;
			case ((LONG)T_DeviceState::PAUSE):
			case ((LONG)T_DeviceState::SEEK):
				{
					mciError = mciSendCommand (it_rec_in->mci_real_ID, MCI_PAUSE, 0, (DWORD) &mciPlay);

					// printing error from MCI
					GetErrorMCI(	mciError, "send MCI_PAUSE command failed" );
				}
				break;
			case ((LONG)T_DeviceState::STOP):
				{
					mciError = mciSendCommand (it_rec_in->mci_real_ID, MCI_STOP, 0, (DWORD) &mciPlay);

					// printing error from MCI
					GetErrorMCI(	mciError, "send MCI_STOP command failed" );
				}
				break;
	}
}

//////////////////////////////////////////////////////////////////////////
// set state for the recording and time for playing if it has value more than zero
void CAudioStreamReader::SetTime_MediaMCI(	LONGLONG					tm_glb_play_in,						// global time for playing(not local)
																						IterRecAudioInfo	it_rec_in,
																						DWORD							state_in						// state track in
																					)
{

	MCIERROR mciError; 
	MCI_PLAY_PARMS mciPlay;
	MCI_SEEK_PARMS	mciSeek;
	memset (&mciPlay, 0x00, sizeof (MCI_PLAY_PARMS));
	memset (&mciSeek, 0x00, sizeof (MCI_SEEK_PARMS));

	DWORD	tm_glb_play_loc = (DWORD)tm_glb_play_in;
	DWORD	tm_play_begin = 0;

	// just in case
	if (tm_glb_play_loc >= it_rec_in->lng_total_begin_msec){

		// calculate offset time for the playing
		tm_play_begin = tm_glb_play_loc - it_rec_in->lng_total_begin_msec;

		// set new position for play record Audio
		mciSeek.dwCallback = NULL;
		mciSeek.dwTo = tm_play_begin;

		mciError = mciSendCommand (it_rec_in->mci_real_ID, MCI_SEEK, /*MCI_WAIT |*/ MCI_TO, (DWORD) &mciSeek);

		// printing error from MCI
		GetErrorMCI(	mciError, "send MCI_SEEK command failed" );

		SetState_MediaMCI(state_in, it_rec_in);
	}else{
		if (tm_glb_play_in < 0){

			SetState_MediaMCI(state_in, it_rec_in);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// function thread
DWORD CAudioStreamReader::Audio_Data_Thread(LPVOID lpParam)
{
	CAudioStreamReader* p_AudioStreamer = (CAudioStreamReader*)lpParam;

	if (!p_AudioStreamer){
		return -1;
	}

	// set value by atomic operation
	InterlockedExchange(&p_AudioStreamer->m_flag_Enable_Loop, 1);
	
	LONG tm_to_sleep_msec = 5;

	while (InterlockedCompareExchange(&p_AudioStreamer->m_flag_Enable_Loop, 1,1))
	{
		//////////////////////////////////////////////////////////////////////////
		// function for synchronization -> output managment
		p_AudioStreamer->Sync_Function();
		//////////////////////////////////////////////////////////////////////////

		// control change path for reading a new tracks and join their in list
 		if (InterlockedCompareExchange(&p_AudioStreamer->m_flag_UpdatePath, 1, 1)){
//  			p_AudioStreamer->m_SynhCS.Set_Critical_Section();
//  			{
 				// release all data
 				//p_AudioStreamer->ReleaseAll();
 
 				// release memory for all lists audio
 				p_AudioStreamer->ReleaseAudioList();
 
 				// get all files inside folder to play and open by MCI media track for each one
 				if(p_AudioStreamer->ParseFolder(p_AudioStreamer->m_BasePathPlay) > 0 ){
 
 					printf("AudioStreamReader::SetPath: %s \n", p_AudioStreamer->m_BasePathPlay);
 				}

 				// set flag default
 				InterlockedExchange(&p_AudioStreamer->m_flag_UpdatePath, 0);
 			}
//  			p_AudioStreamer->m_SynhCS.Leave_Critical_Section();
//  		}
 		//////////////////////////////////////////////////////////////////////////
 		if (InterlockedCompareExchange(&p_AudioStreamer->m_flag_UpdateState, 1, 1)){
//  			p_AudioStreamer->m_SynhCS.Set_Critical_Section();
//  			{
 				// set start time an audio track using state
 				p_AudioStreamer->Set_TimeStartTrack(p_AudioStreamer->m_lTotalTimePlayedCAMs);

 				// set flag default
 				InterlockedExchange(&p_AudioStreamer->m_flag_UpdateState, 0);
 			}
//  			p_AudioStreamer->m_SynhCS.Leave_Critical_Section();
//  		}
	//////////////////////////////////////////////////////////////////////////
 		if (InterlockedCompareExchange(&p_AudioStreamer->m_flag_UpdateTime, 1, 1)){
//  			p_AudioStreamer->m_SynhCS.Set_Critical_Section();
//  			{
 				int rec_order_nmb__seek = -1;
 
 				// get order number of the recording need set the state
 				rec_order_nmb__seek = p_AudioStreamer->GetRecNmb( p_AudioStreamer->m_NewTime_Playing );
 
 				// just in case
 				if (rec_order_nmb__seek >= 0){
 					if (p_AudioStreamer->PlayAllTracksByTime(rec_order_nmb__seek, p_AudioStreamer->m_state_audio_devs, p_AudioStreamer->m_NewTime_Playing) != -1){
 						;
 					}
 				}
 
 				// set flag default
 				InterlockedExchange(&p_AudioStreamer->m_flag_UpdateTime, 0);
 			}
//  			p_AudioStreamer->m_SynhCS.Leave_Critical_Section();
//  		}
		//////////////////////////////////////////////////////////////////////////
		
 		if (T_DeviceState::PLAY == InterlockedCompareExchange(&p_AudioStreamer->m_state_audio_devs, T_DeviceState::PLAY, T_DeviceState::PLAY)){
 			p_AudioStreamer->Attract_AudioToVideo();
 		}
		Sleep (tm_to_sleep_msec);

	}//while (...)

	// release memory for all lists audio
	p_AudioStreamer->ReleaseAudioList();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// attract audio track to time was got from Camera
int CAudioStreamReader::Attract_AudioToVideo(void)
{
	m_Sync_SWMR.WaitToRead();
	{
		MCI_STATUS_PARMS StatusParams;
		StatusParams.dwItem = MCI_STATUS_POSITION;
		DWORD dur_tm_msec = 0;
		char str [_MAX_PATH] = {0};

		// play one by one trackes [0-N]
		int cnt_Tracks = m_list_AudioTracks.size();
		IterAudioTrack it_Track = m_list_AudioTracks.begin();

		int cnt_TrackRecs = 0;
		IterRecAudioInfo it_rec_loc;
		int rec_order_nmb__seek = -1;

		LONGLONG tm_data_spread_ms = 1500;	// time data spread between Audio and video [ms]

		MCIERROR mciError;

		// change state 
		InterlockedExchange(&m_state_audio_devs, m_State_CAMs);

		// get order number of the recording need set the state
		rec_order_nmb__seek = GetRecNmb( m_lTotalTimePlayedCAMs - m_lOffsetTimePlay_INI );

		if (rec_order_nmb__seek < 0){
			// recording was not found
			m_Sync_SWMR.Done();

			return -1;
		}

		// search all tracks
		for (int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){

			// searching all recording into track list
			it_rec_loc = it_Track->list_nmb_File.begin();

			// get recording was sought
			std::advance(it_rec_loc, rec_order_nmb__seek);

			// just in case
			if (it_rec_loc != it_Track->list_nmb_File.end()){

				if (!it_rec_loc->mci_real_ID){
					continue;
				}
				// get duration time was gone
				mciError = mciSendCommand(it_rec_loc->mci_real_ID, MCI_STATUS,	/*MCI_WAIT | */MCI_STATUS_ITEM, (DWORD)&StatusParams);

				// printing error from MCI
				GetErrorMCI(	mciError, "Get POSITION Track" );

				// calculation time offset between Audio and Cameras time
				dur_tm_msec = (DWORD)StatusParams.dwReturn;

				if( llabs( LONGLONG(it_rec_loc->lng_total_begin_msec + dur_tm_msec) - (m_lTotalTimePlayedCAMs - m_lOffsetTimePlay_INI)  ) > tm_data_spread_ms){
					///
					printf("There is different time between Audio and Video playing!!! -> %lld \n", LONGLONG(it_rec_loc->lng_total_begin_msec + dur_tm_msec) - m_lTotalTimePlayedCAMs - m_lOffsetTimePlay_INI);
					///

					// set time to play for Audio recording
					SetTime_MediaMCI(m_lTotalTimePlayedCAMs - m_lOffsetTimePlay_INI, it_rec_loc, m_state_audio_devs/*T_DeviceState::PLAY*/);
				}

				///
				if (nmb_Track_loc == 0){
					sprintf (str, "play time: %d", dur_tm_msec); 
					printf ("%s \n", str); 
				}
				///
			}

			std::advance(it_Track, 1);
		}//for(nmb_Track_loc)
	}
	m_Sync_SWMR.Done();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set base path for playing audio tracks
bool	CAudioStreamReader::SetPath(	const CHAR *path_play_in		// playing path for audio files	[_MAX_PATH]
																	)
{
// 	m_SynhCS.Set_Critical_Section();
// 	{
		//printf("AudioStreamReader::SetPath: %s \n", path_play_in);
		sprintf(m_BasePathPlay, path_play_in);

		//printf("AudioStreamReader::SetPath: %s \n", path_play_in);
// 	}
// 	m_SynhCS.Leave_Critical_Section();

	// set flag reading a new tracks
	InterlockedExchange(&m_flag_UpdatePath, 1);
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
// get order number of the recording by time from the first track list
int	CAudioStreamReader::GetRecNmb(	LONGLONG tm_search_rec_in // time for searching of number recording [msec] (using the first track list)
																	)
{
	/////////
	m_Sync_SWMR.WaitToRead();
	{
		// play one by one trackes [0-N]
		int cnt_Tracks = m_list_AudioTracks.size();
		IterAudioTrack it_Track = m_list_AudioTracks.begin();

		// find the first track in '0' list
		for (int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){
			it_Track = m_list_AudioTracks.begin();

			std::advance(it_Track, nmb_Track_loc);

			// just in case
			if (it_Track->total_leng_msec < tm_search_rec_in){ // ERROR -> the come time had a mistake

				m_Sync_SWMR.Done(); 
				return -1;
			}
		
			// searching all recording into '0' track list
			int rec_loc = 0;
			IterRecAudioInfo it_rec_loc = it_Track->list_nmb_File.begin();
			IterRecAudioInfo it_rec_end = it_Track->list_nmb_File.end();

			for (; it_rec_loc != it_rec_end; ++it_rec_loc){
				if ( (it_rec_loc->lng_total_begin_msec + it_rec_loc->length_msec) >= tm_search_rec_in){

					m_Sync_SWMR.Done();

					return rec_loc;
				}
				rec_loc++;
			}

			// pass all another tracks
			break;
		}
	}
	m_Sync_SWMR.Done(); 
	/////////	

	return -1;
}
//////////////////////////////////////////////////////////////////////////
// set state for audio device control(see T_DeviceState)
long	CAudioStreamReader::SetStateDevice(	long state_in	//T_DeviceState::(STOP/PLAY/WRITE)
																				)
{
	
	if ( (state_in < T_DeviceState::STOP) || (state_in > T_DeviceState::SEEK)){
		return 1;		// error
	}

	InterlockedExchange(&m_state_audio_devs, (LONG)state_in);

	// set comparison tracks and state mute for them
	SetMute4Tracks();

	// set flag for update state for all tracks 
	InterlockedExchange(&m_flag_UpdateState, 1);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set start time an audio track using state
long	CAudioStreamReader::Set_TimeStartTrack(	LONGLONG tm_start_play_in // time to play audio track [msec]
																						)
{
	// save new time for Audio playing
	InterlockedExchange64(&m_NewTime_Playing, tm_start_play_in - m_lOffsetTimePlay_INI);

	// set flag for update Time for playing tracks
	InterlockedExchange(&m_flag_UpdateTime, 1);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set type for audio streaming thread
long	CAudioStreamReader::LaunchThreadHandlerDevice( void )
{
	ReleaseAudioThread();

	// create a new thread for handling audio streaming
	m_hAudioThread = CreateThread(	NULL,
																0,
																(LPTHREAD_START_ROUTINE)Audio_Data_Thread, 
																(void*)this,
																0,
																&m_dwAudioThreadId
															);
	if (m_hAudioThread){

		BOOL err = SetThreadPriority(m_hAudioThread, THREAD_PRIORITY_NORMAL);

		printf("AudioReader thread enable!\n");

		return 0;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// release audio thread
int CAudioStreamReader::ReleaseAudioThread(void)
{
	// delete thread
	if (m_hAudioThread){
		printf("ReleaseAudioThread()...Begin!\n");

		//SetStateAllRecs((LONG)T_DeviceState::STOP);

		InterlockedExchange(&m_flag_Enable_Loop, 0);

		// take time for thread to resume work
		WaitForSingleObject(m_hAudioThread, INFINITE);

		CloseHandle(m_hAudioThread);

		m_hAudioThread = NULL;

		printf("ReleaseAudioThread()...Finish!\n");
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// release memory for all lists audio
void CAudioStreamReader::ReleaseAudioList(void)
{
	MCIERROR mciError; 

	printf("Release All Records...Begin!\n");

	/////////
	m_Sync_SWMR.WaitToWrite();
	{
		int cnt_Tracks = m_list_AudioTracks.size();
		IterAudioTrack it_Track = m_list_AudioTracks.begin();

		// research all track list
		for (int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){

			/////
			// searching all recording inside each track to stop MCI functionality
			IterRecAudioInfo it_rec_loc = it_Track->list_nmb_File.begin();
			IterRecAudioInfo it_rec_end = it_Track->list_nmb_File.end();

			for (; it_rec_loc != it_rec_end; ++it_rec_loc){

				if (it_rec_loc->mci_real_ID > 0){

					// close MCI sound device was opened
					mciError = mciSendCommand ( it_rec_loc->mci_real_ID, MCI_CLOSE, MCI_WAIT, NULL );

					// printing error from MCI
					GetErrorMCI(	mciError, "Track close" );

					// set ID to default
					it_rec_loc->mci_real_ID = 0;

					printf("MCI track was stopped and release: ID [%d]:%d - OK!\n", it_Track->m_index_nmb, it_rec_loc->nmb_rec);
				}
			}
			/////
			// free the memory
			it_Track->list_nmb_File.clear();

			std::advance(it_Track, 1);
		}
		m_list_AudioTracks.clear();
	}
	m_Sync_SWMR.Done();
	/////////

	printf("Release All Records...Finish!\n");
}

//////////////////////////////////////////////////////////////////////////
// get length track by his number
unsigned int CAudioStreamReader::GetLengthTrack( unsigned int nmb_track_in // number track ( '0' - the first track )
																								)
{
	m_Sync_SWMR.WaitToRead();
	{
		unsigned int cnt_tracks = m_list_AudioTracks.size();
		IterAudioTrack it_track = m_list_AudioTracks.begin();

		if (nmb_track_in < cnt_tracks){
			std::advance(it_track, nmb_track_in);
			return (it_track->total_leng_msec);
		}
	}
	m_Sync_SWMR.Done();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set Audio device using(for the playing and recording)
bool CAudioStreamReader::SetAudioDevice(	UINT	device_nmb_in,	// device number [0-7]
																					BOOL	flag_enable_in	// flag enabling device	(TRUE / FALSE)
																				)
{
	m_Sync_SWMR.WaitToRead();
	{
		unsigned int cnt_aud_dev_play = m_list_AudioPlayDev.size();
		IterAudioPlayDevice it_AudioPlayDev = m_list_AudioPlayDev.begin();

		for (unsigned int nmb_aud_dev = 0; nmb_aud_dev < cnt_aud_dev_play; ++nmb_aud_dev){
			if (it_AudioPlayDev->index_nmb == device_nmb_in){
				it_AudioPlayDev->mute_flag = !flag_enable_in;
				break;
			}
			std::advance(it_AudioPlayDev, 1);
		}
	}
	m_Sync_SWMR.Done();

	return false;
}

//////////////////////////////////////////////////////////////////////////
// set comparison tracks and state mute for them
void CAudioStreamReader::SetMute4Tracks(void)
{
	m_Sync_SWMR.WaitToRead();
	{
		unsigned int cnt_Tracks = m_list_AudioTracks.size();
		IterAudioTrack it_Track = m_list_AudioTracks.begin();

		UINT cnt_aud_dev_play = m_list_AudioPlayDev.size();
		IterAudioPlayDevice it_AudioPlayDev = m_list_AudioPlayDev.begin();

		// research all list tracks
		for (unsigned int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){

			it_AudioPlayDev = m_list_AudioPlayDev.begin();

			// research all states mute for all devices
			for (unsigned int nmb_aud_dev = 0; nmb_aud_dev < m_nmb_devices; ++nmb_aud_dev){

				if (it_AudioPlayDev->index_nmb == it_Track->m_index_nmb){
					it_Track->mute_flag = it_AudioPlayDev->mute_flag;
					break;
				}
				std::advance(it_AudioPlayDev, 1);
			}
			std::advance(it_Track, 1);
		}//for (int nmb_Track_loc)
	}
	m_Sync_SWMR.Done();
}

//////////////////////////////////////////////////////////////////////////
// set pointer to callback function from outside
// void CAudioStreamReader::SetFuncGetTimeMovie(void* pFuncGetTimeAudio_in)
// {
// 	m_pFuncGetTimeAudio = (GetTimeMovie)pFuncGetTimeAudio_in;
// }

//////////////////////////////////////////////////////////////////////////
// function for synchronization( setting and management of working barrier )
void CAudioStreamReader::Sync_Function(void)
{
	// define opportunity to use global barrier
	if (InterlockedCompareExchange(&m_flag_Enable_BarrierOut, 1,1)){

		if (InterlockedCompareExchange(&m_pSynh_Bar_StCam_1->m_InterLock_flag, 1,1)){

			printf("Block begin(Audio)...\n");
			// stop thread to align "the first point"
			m_pSynh_Bar_StCam_1->Block();
			{
				;
			}
			// stop thread to align "second point"   (!!!unblock another threads!!!)
			m_pSynh_Bar_StCam_2->Block();
			printf("...Block end(Audio)\n");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// set states for all records
DWORD CAudioStreamReader::SetStateAllRecs(	DWORD state_in						// state track in
																					)
{
	m_Sync_SWMR.WaitToRead();
	{
		// play one by one trackes [0-N]
		int cnt_Tracks = m_list_AudioTracks.size();
		IterAudioTrack it_Track = m_list_AudioTracks.begin();

		int cnt_TrackRecs = 0;
		IterRecAudioInfo it_rec_loc;
		int cnt_recs = 0;

		// search all tracks
		for (int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){

			// searching all recording into track list
			cnt_recs = it_Track->list_nmb_File.size();
			it_rec_loc = it_Track->list_nmb_File.begin();

			// search all recordings in to track
			for (int nmb_rec_loc = 0; nmb_rec_loc < cnt_recs; ++nmb_rec_loc){

				// just in case
				if (!it_rec_loc->mci_real_ID){
					continue;
				}
				SetState_MediaMCI(state_in, it_rec_loc);

				std::advance(it_rec_loc, 1);
			}//for(nmb_Track_loc)

			std::advance(it_Track, 1);
		}//for(nmb_Track_loc)
	}
	m_Sync_SWMR.Done(); 

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//set mute for the record of the track
void CAudioStreamReader::SetVolume(UINT device_ID_in, BOOL flag_enable_in)
{
	// just in case
	if (!device_ID_in){
		return;
	}

	MCI_SET_PARMS mciSetParams;
	MCIERROR mciError; 
	ZeroMemory((void*)&mciSetParams, sizeof(MCI_SET_PARMS));

	mciSetParams.dwAudio =	MCI_SET_AUDIO_RIGHT;

	if (flag_enable_in){
		mciError = mciSendCommand(device_ID_in, MCI_SET,	MCI_SET_AUDIO | MCI_SET_ON, (DWORD)&mciSetParams);
	}else{
		mciError = mciSendCommand(device_ID_in, MCI_SET,	MCI_SET_AUDIO | MCI_SET_OFF, (DWORD)&mciSetParams);
	}

	// printing error from MCI
	GetErrorMCI(	mciError, "send MCI_SET_AUDIO | MCI_SET*** command failed" );
}

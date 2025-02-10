#include "stdafx.h"
#include "AudioStreamReader.h"


//////////////////////////////////////////////////////////////////////////
// parse all files in list ListAudioTrack by name (return number of files of audio)
int CAudioStreamReader::ParseFolder(const char* search_folder)
{
	HANDLE FindHandle;
	WIN32_FIND_DATAA FindData;
	char	path_cur[_MAX_PATH];
	char	path_old[_MAX_PATH];

	char	file_fullname[_MAX_PATH];

	// release all lists
	ReleaseAudioList();

	sprintf_s(path_cur, _MAX_PATH, search_folder);

	GetCurrentDirectoryA(_MAX_PATH, path_old);

	if (strlen(path_cur)){
		SetCurrentDirectoryA(path_cur);	/* _getcwd(path_reg, LNG_MSG); */
	}else{
		SetCurrentDirectoryA(path_old);
	}

	FindHandle = FindFirstFileA(NAME_AUDIO_REC_FIND, &FindData);

	//searching files in current folder
	do{
		if (strstr(FindData.cFileName, NAME_AUDIO_REC_TAIL)){

			sprintf_s(file_fullname, _MAX_PATH, "%s\\%s", path_cur, FindData.cFileName);

			ParseFileName(file_fullname);
		}

	} while(FindNextFileA( FindHandle, &FindData ));

	//close the specified search handle
	FindClose(FindHandle);

	SetCurrentDirectoryA(path_old);

	///////////////////////////////
	// calculation total time duration before rec.file 
	CalcPostTotalTime();
	///////////////////////////////

	return m_list_AudioTracks.size();
}

//////////////////////////////////////////////////////////////////////////
// calculation total time duration before rec.file 
void CAudioStreamReader::CalcPostTotalTime(void)
{
	m_Sync_SWMR.WaitToRead();
	{
		// find the same Track by name
		int cnt_Tracks = m_list_AudioTracks.size();

		if (cnt_Tracks <= 0){
			m_Sync_SWMR.Done();
			return;
		}

		IterAudioTrack it_Track = m_list_AudioTracks.begin();

		int cnt_fileTrk_ID = it_Track->list_nmb_File.size();
		IterRecAudioInfo it_fileTrk_ID = it_Track->list_nmb_File.begin();
		unsigned int total_time_loc = 0;

		// researching all Track
		for (int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){

			/////////////
			cnt_fileTrk_ID = it_Track->list_nmb_File.size();
			it_fileTrk_ID = it_Track->list_nmb_File.begin();
			total_time_loc = 0;

			// researching all rec.files
			for (int nmb_ID_Trk_loc = 0; nmb_ID_Trk_loc < cnt_fileTrk_ID; ++nmb_ID_Trk_loc){

				//	if (nmb_ID_Trk_loc){
				it_fileTrk_ID->lng_total_begin_msec = total_time_loc;
				//	}
				total_time_loc += it_fileTrk_ID->length_msec;

				std::advance(it_fileTrk_ID, 1);
			}
			std::advance(it_Track, 1);
		}
	}
	m_Sync_SWMR.Done();
}

//////////////////////////////////////////////////////////////////////////
// divide filename by parts and exam their
int CAudioStreamReader::ParseFileName(const char* filename)
{
	Audio_Track audio_Tr_loc;
	Rec_Audio_Info	rec_info_new;
	//Audio_Track	*pTrack_loc = NULL;
	//Rec_Audio_Info	*pRec_Info_loc = NULL;

	audio_Tr_loc.total_leng_msec = 0;
	audio_Tr_loc.device_file_ID = 0;
	audio_Tr_loc.m_type_channel = T_TypeChannel::RIGHT_CNL;
	audio_Tr_loc.mute_flag = TRUE;

	ZeroMemory((void*)&rec_info_new, sizeof(rec_info_new));
	//rec_info_new.pMediaPlayer = NULL;

	char name_AudioDev[_MAX_FNAME/4] = "";

	char *pdest = NULL;
	int  res_find = 0;

	int nm_rec_Sfx_len = strlen(NAME_AUDIO_REC_SUFFIX);
	int nm_rec_Hdr_len = strlen(NAME_AUDIO_REC_HEADER);

	char nm_nmb_Audio_rec[10];
	int nmb_Audio_rec = 0;

	char nm_nmb_Audio_Dvc[_MAX_FNAME/4];
	int nmb_Audio_Dvc = 0;
	///////////////////////////////////
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME/4];
	char ext[_MAX_EXT];

	_splitpath(filename, drive, dir, fname, ext);
	///////////////////////////////////

	pdest = strstr((char*)fname, NAME_AUDIO_REC_SUFFIX);
	if (pdest){
		res_find = pdest - fname +1;

		strncpy(name_AudioDev, fname, res_find -2 );	// "-2" - except "__"

		strcpy(nm_nmb_Audio_rec, pdest+nm_rec_Sfx_len);
		nmb_Audio_rec = atoi(nm_nmb_Audio_rec);

		strcpy(nm_nmb_Audio_Dvc, fname+nm_rec_Hdr_len);
		nmb_Audio_Dvc = atoi(nm_nmb_Audio_Dvc);

		// using only RIGHT channel
		audio_Tr_loc.device_file_ID = nmb_Audio_Dvc;

		//////////////////////////////////////////////////////////////////////////
		/////////
		m_Sync_SWMR.WaitToWrite();
		{
			// find the same Track by name
			int cnt_Tracks = m_list_AudioTracks.size();
			IterAudioTrack it_Track = m_list_AudioTracks.begin();

			bool flag_file_found = false;

			int mci_ID_device = 0;

			// get info from examining file
			//mci_ID_device = Get_TotalTime_One_Tracks("c:\\OpenCV\\FinalResult\\Save_A_V\\AudioDevice_1__Rec_0.mp3", audio_Tr_loc);
			mci_ID_device = Get_TotalTime_One_Tracks(filename, audio_Tr_loc);

			if (mci_ID_device > 0){

				// set new information for recording of the single file
				rec_info_new.nmb_rec = nmb_Audio_rec;
				rec_info_new.length_msec = audio_Tr_loc.total_leng_msec;
				rec_info_new.lng_total_begin_msec = 0;

				rec_info_new.mci_real_ID = mci_ID_device;

				// find the same Track by name
				for (int nmb_Track_loc = 0; nmb_Track_loc < cnt_Tracks; ++nmb_Track_loc){
					it_Track = m_list_AudioTracks.begin();

					std::advance(it_Track, nmb_Track_loc);

					// there is identical Track file
					if (it_Track->device_file_ID == audio_Tr_loc.device_file_ID){
						flag_file_found = true;

						/////////////
						int cnt_fileTrk_ID = it_Track->list_nmb_File.size();
						IterRecAudioInfo it_fileTrk_ID = it_Track->list_nmb_File.begin();

						bool flag_ID_Track_added = false;
						// find nearest number of ID
						for (int nmb_ID_Trk_loc = 0; nmb_ID_Trk_loc < cnt_fileTrk_ID; ++nmb_ID_Trk_loc){

							it_fileTrk_ID = it_Track->list_nmb_File.begin();
							std::advance(it_fileTrk_ID, nmb_ID_Trk_loc);
							//pRec_Info_loc = static_cast<Rec_Info*>(*it_fileTrk_ID);

							if ( it_fileTrk_ID->nmb_rec > rec_info_new.nmb_rec){
								// save total duration all files (within one Track)
								it_Track->total_leng_msec += rec_info_new.length_msec;

								it_Track->list_nmb_File.push_front(rec_info_new);
								flag_ID_Track_added = true;
								break;
							}
						}
						if (!flag_ID_Track_added){
							// save total duration all files (within one Track)
							it_Track->total_leng_msec += rec_info_new.length_msec;

							it_Track->list_nmb_File.push_back(rec_info_new);
						}
						/////////////
					}

					//pTrack_loc = static_cast<Audio_Track>(*it_Track);
				}
			}

			// there is not Track in list -> add it
			if (!flag_file_found){
				// set order number into list of an all tracks
				audio_Tr_loc.m_index_nmb = cnt_Tracks + 1;

				audio_Tr_loc.list_nmb_File.push_back(rec_info_new);
				m_list_AudioTracks.push_back(audio_Tr_loc);
			}
		}
		m_Sync_SWMR.Done();
		/////////
		//////////////////////////////////////////////////////////////////////////

	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// get error from MCI function
DWORD CAudioStreamReader::HandlerError(DWORD mci_err_in)
{
	/*
	char buf_err[_MAX_PATH] = {0};

	mciGetErrorStringA (mci_err_in, buf_err, _MAX_PATH); 
	printf ("CAudioStreamReader: %s \n", buf_err);
	*/
	return mci_err_in;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// get total time from one Audio track
unsigned int CAudioStreamReader::Get_TotalTime_One_Tracks(const char *full_filename, Audio_Track &Audio_track_IO)
{
	// try get total time of file by using VLC

	MCI_OPEN_PARMSA mciOpenParams;
	MCI_SET_PARMS mciSetParams;
	MCI_STATUS_PARMS mciStatusParams;

	ZeroMemory((void*)&mciOpenParams, sizeof(MCI_OPEN_PARMS));
	ZeroMemory((void*)&mciStatusParams, sizeof(MCI_STATUS_PARMS));
	ZeroMemory((void*)&mciSetParams, sizeof(MCI_SET_PARMS));

	MCIERROR mciError; 
	mciOpenParams.lpstrDeviceType = "mpegvideo";
	mciOpenParams.lpstrElementName = full_filename; 

	mciError = mciSendCommandA (0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpenParams);

	if(GetErrorMCI(	mciError, "Track opening" )){
		return 0;
	};

	if (mciOpenParams.wDeviceID > 0){

		// change channel playing
		ZeroMemory((void*)&mciSetParams, sizeof(MCI_SET_PARMS));
		mciSetParams.dwAudio =	MCI_SET_AUDIO_LEFT;
		mciError = mciSendCommand(mciOpenParams.wDeviceID, MCI_SET,	MCI_SET_AUDIO | MCI_SET_OFF, (DWORD)&mciSetParams);

		if(GetErrorMCI(	mciError, "MCI_SET_AUDIO_LEFT" )){
			return 0;
		};

		mciSetParams.dwAudio =	MCI_SET_AUDIO_RIGHT;
		mciError = mciSendCommand(mciOpenParams.wDeviceID, MCI_SET,	MCI_SET_AUDIO | MCI_SET_ON, (DWORD)&mciSetParams);

		if(GetErrorMCI(	mciError, "MCI_SET_AUDIO_RIGHT" )){
			return 0;
		};

		// set millisecond playing
		mciSetParams.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
		mciSetParams.dwAudio =	MCI_SET_AUDIO_ALL;
		mciError = mciSendCommand(mciOpenParams.wDeviceID, MCI_SET,	MCI_SET_TIME_FORMAT, (DWORD)&mciSetParams);

		if(GetErrorMCI(	mciError, "MCI_SET_TIME_FORMAT" )){
			return 0;
		};

		//get audio track length
		mciStatusParams.dwItem = MCI_STATUS_LENGTH;
		mciError = mciSendCommand(mciOpenParams.wDeviceID, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&mciStatusParams);

		if(GetErrorMCI(	mciError, "MCI_STATUS_LENGTH" )){
			return 0;
		};

		if (mciStatusParams.dwReturn > 0){
			Audio_track_IO.total_leng_msec = mciStatusParams.dwReturn;
			return mciOpenParams.wDeviceID;
		}
	}

	return mciOpenParams.wDeviceID;
}


//////////////////////////////////////////////////////////////////////////
// printing error from MCI
unsigned int CAudioStreamReader::GetErrorMCI(	const MCIERROR err_in,		// perhaps error MCI
																							const char* str_proc_in		// name working process
																						)
{
	if (err_in){
		CHAR buf_err [_MAX_PATH] = {0}; 

		mciGetErrorStringA (err_in, buf_err, _MAX_PATH); 

		if(str_proc_in){
			printf ("ERROR(MCI)! %s: %s \n", str_proc_in, buf_err ); 
		}else{
			printf ("ERROR(MCI)! %s \n", buf_err ); 
		}
		
		return err_in;
	}

	return 0;
}

/*
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// get total time from one Audio track (USING MCI)
bool CAudioStreamReader::Get_TotalTime_One_Tracks(const char *full_filename, Audio_Track &Audio_track_IO)
{
	// try get total time of file by using MCI Windows (Media Control Interface)
	MCI_OPEN_PARMSA mciOpen;
	MCI_STATUS_PARMS params_status;
	MCI_SET_PARMS SetParams;

	ZeroMemory((void*)&mciOpen, sizeof(MCI_OPEN_PARMS));
	ZeroMemory((void*)&params_status, sizeof(MCI_STATUS_PARMS));
	ZeroMemory((void*)&SetParams, sizeof(MCI_SET_PARMS));

	mciOpen.lpstrDeviceType = "mpegvideo"; //"waveaudio" / "avivideo" / "sequencer" / "mpegvideo"
	mciOpen.lpstrElementName = full_filename; //L"C:\\OpenCV\\CaptureIP\\Voice_Asio\\Data_Voice\\1.mp3"; 

	//MCI_SET_AUDIO_ALL
	// open device for the work on
	HandlerError(mciSendCommandA (0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen));

	if (mciOpen.wDeviceID > 0){
		SetParams.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
		SetParams.dwAudio =	MCI_SET_AUDIO_ALL;
		HandlerError(mciSendCommandA(mciOpen.wDeviceID, MCI_SET,	MCI_SET_TIME_FORMAT, (DWORD)&SetParams));

		//get audio track length
		params_status.dwItem = MCI_STATUS_LENGTH;
		HandlerError(mciSendCommandA(mciOpen.wDeviceID, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&params_status));

		// close MCI object was opened
		HandlerError(mciSendCommand ( mciOpen.wDeviceID, MCI_CLOSE, MCI_WAIT, NULL ) );

		//MCI_PLAY_PARMS mciPlay; 
		//HandlerError(mciSendCommandA(mciOpen.wDeviceID, MCI_PLAY, 0, (DWORD) &mciPlay)); 
	}

	if (params_status.dwReturn > 0){
		//Audio_track_IO.device_ID = mciOpen.wDeviceID;
		Audio_track_IO.total_leng_msec = params_status.dwReturn;
		return 1;
	}
	return 0;
}
*/
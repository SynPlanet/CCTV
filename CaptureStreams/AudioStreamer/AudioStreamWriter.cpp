#include "StdAfx.h"
#include "AudioStreamWriter.h"
#include <locale.h>

#include <direct.h>
//////////////////////////////////////////////////////////////////////////
//-device="Line 1/2 (M-Audio M-Track Eight" -line="Master Volume" -v="100" -sr="44100"
//////////////////////////////////////////////////////////////////////////
//transform symbol types(CHAR* -> WCHAR*)==> (ANSI -> UNICODE)
//wchar_t device_name[_MAX_PATH] = {L"Line 1/2 (M-Audio M-Track Eight"};
//wchar_t device_name[_MAX_PATH] = {L"Микрофон (2- USB2.0 Camera Audi"};
//wchar_t line_name[_MAX_PATH] = {L"Общая громкость"};
//wchar_t line_name[_MAX_PATH] = {L"Master Volume"};//{"Mono Mix"};//{"Line In"};
//////////////////////////////////////////////////////////////////////////

class CAsioWriter;

//extern CAsioWriter	g_AsioWriter;
CAsioWriter	*g_pAsioWriter = NULL;

CAudioStreamWriter::CAudioStreamWriter()
{
	m_List_Audio_Dev_INI.clear();
	m_List_Audio_Dev_Sys.clear();

	m_pAsioWriter = NULL;

	//PrintAudioDevicesSys();

	m_state = T_DeviceState::STOP;
}

CAudioStreamWriter::~CAudioStreamWriter()
{
	ReleaseAll();
}

//////////////////////////////////////////////////////////////////////////
// initialization data 
void CAudioStreamWriter::Init(void)
{
	g_pAsioWriter = new CAsioWriter();

	if (g_pAsioWriter){
		g_pAsioWriter->Init();
		m_pAsioWriter = g_pAsioWriter;
	}else{
		m_pAsioWriter = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// initialization data for recording audio
bool CAudioStreamWriter::Connect(void)
{
	if (m_pAsioWriter){
		// join opportunity to use global barrier
		InterlockedExchange(	&m_pAsioWriter->m_flag_Enable_BarrierOut, 1 );

		// set default values
		InterlockedExchange( &m_pAsioWriter->m_nmb_rec, 0 );

		return 0;
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// release only necessary member's class
bool CAudioStreamWriter::Disconnect(void)
{
	if (m_pAsioWriter){
		// join opportunity to use global barrier
		InterlockedExchange( &m_pAsioWriter->m_flag_Enable_BarrierOut, 0	);

		return 0;
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// release all audio recording devices
void CAudioStreamWriter::ReleaseAll(void)
{

	m_List_Audio_Dev_INI.clear();
	m_List_Audio_Dev_Sys.clear();

	if (g_pAsioWriter){
		g_pAsioWriter->ReleaseAll();

		delete g_pAsioWriter;
		g_pAsioWriter = NULL;

		m_pAsioWriter = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// get synchronize objects for output control(camera control)
void CAudioStreamWriter::SetBarrierCtrlCam(	void *pSynh_Barrier_StateCam_1_in,
																						void *pSynh_Barrier_StateCam_2_in
																					)
{
	// just in case
	if (!g_pAsioWriter){
		return ;
	}

	g_pAsioWriter->SetBarrierCtrlCam(pSynh_Barrier_StateCam_1_in, pSynh_Barrier_StateCam_2_in);
}

//////////////////////////////////////////////////////////////////////////
// get count streams writing(see size m_List_DevicesAWriter)
unsigned int CAudioStreamWriter::GetCountStreams(	void )
{
	// just in case
	if (!g_pAsioWriter){
		return 0;
	}

	if (g_pAsioWriter->IsAsioEnable()){
		return 1;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set base path for saving audio tracks
bool	CAudioStreamWriter::SetPath(	const CHAR *path_record_in	// audio recording path		[_MAX_PATH]
																	)
{
	if (!g_pAsioWriter){
		return false;
	}

	g_pAsioWriter->m_SynhCS.Set_Critical_Section();
	{
		sprintf(g_pAsioWriter->m_BasePathRecord, path_record_in);

		int err = 0;

		// make directory if no exist
		err = mkdir(g_pAsioWriter->m_BasePathRecord);

		if(!err){
			InterlockedExchange(&g_pAsioWriter->m_nmb_rec, 0);
		}

		// release only necessary member's class
		//Release();
	}
	g_pAsioWriter->m_SynhCS.Leave_Critical_Section();

	return false;
}

//////////////////////////////////////////////////////////////////////////
// set Audio device using(for the recording)
bool CAudioStreamWriter::SetAudioDevice(	UINT	device_nmb_in,	// device number [0-7] ???????
																					BOOL	flag_enable_in	// flag enabling device	(TRUE / FALSE)
																				)
{
	// just in case
	if (!g_pAsioWriter){
		return false;
	}

	// change state file MP3 writers
	g_pAsioWriter->m_SynhCS.Set_Critical_Section();
	{
		unsigned int cnt_units_wr = g_pAsioWriter->m_List_Writers_MP3.size();
		IterListWriterMP3	it_WriterMp3_loc = g_pAsioWriter->m_List_Writers_MP3.begin();

		// exceeding number device
		if (cnt_units_wr < device_nmb_in){
			g_pAsioWriter->m_SynhCS.Leave_Critical_Section();

			return false;
		}

		it_WriterMp3_loc = g_pAsioWriter->m_List_Writers_MP3.begin();

		std::advance(it_WriterMp3_loc, device_nmb_in - 1);		// "device_nmb_in-1" - the beginning has count from 1

		(*it_WriterMp3_loc)->SetEnable(flag_enable_in);
	}
	g_pAsioWriter->m_SynhCS.Leave_Critical_Section();

	return false;
}

//////////////////////////////////////////////////////////////////////////
// set state for audio device control(see T_DeviceState)
long	CAudioStreamWriter::SetStateDevice(	long state_in	//T_DeviceState::(STOP/WRITE)
																				)
{

	if ( (state_in < T_DeviceState::STOP) || (state_in > T_DeviceState::SEEK)){
		return -1;		// error
	}

	if (!g_pAsioWriter){
		return -1;
	}

	// there is the same state
	if(state_in == InterlockedCompareExchange(&g_pAsioWriter->m_state_rec, g_pAsioWriter->m_state_rec, state_in) ){
		return 1;
	}
	//////////////////////////////////////////////////////////////////////////
	bool flag_nmb_rec_new = true;

	// change state ASIO
	switch (state_in){
	case (T_DeviceState::STOP):
		{
			;// see class CAsioWriter
		}
		break;
	case (T_DeviceState::WRITE):
		{
			// create a new files
			g_pAsioWriter->m_SynhCS.Set_Critical_Section();
			{
				unsigned int cnt_units_wr = g_pAsioWriter->m_List_Writers_MP3.size();
				IterListWriterMP3	it_WriterMp3_loc = g_pAsioWriter->m_List_Writers_MP3.begin();

				for (int nmb_unit = 0; nmb_unit < cnt_units_wr; ++nmb_unit){

					if ((*it_WriterMp3_loc)->GetEnable()){

						// the last State was stayed in PAUSE 
						if(T_DeviceState::PAUSE == InterlockedCompareExchange(&g_pAsioWriter->m_state_rec, T_DeviceState::PAUSE, T_DeviceState::PAUSE) ){

							if (	(*it_WriterMp3_loc)->GetState() == T_DeviceState::PAUSE
												||
										(*it_WriterMp3_loc)->GetState() == T_DeviceState::WRITE
										){
								flag_nmb_rec_new = false;
								break;
							}
						}

						(*it_WriterMp3_loc)->CreateAudioFile(g_pAsioWriter->m_nmb_rec, g_pAsioWriter->m_BasePathRecord);
					}

					std::advance(it_WriterMp3_loc, 1);
				}
			}
			g_pAsioWriter->m_SynhCS.Leave_Critical_Section();

			// increase order number record
			if (flag_nmb_rec_new){
				InterlockedIncrement(&g_pAsioWriter->m_nmb_rec);
			}
			
		}
		break;
	case (T_DeviceState::PAUSE):
		{
			;// see class CAsioWriter
		}
		break;

	}
	//////////////////////////////////////////////////////////////////////////
	// change recording state 		
	InterlockedExchange(&g_pAsioWriter->m_state_rec, state_in);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// print in log file the names of all audio devices (was gotten from system)
// and save the same list 
// void CAudioStreamWriter::PrintAudioDevicesSys( void )
// {
// 	UINT nInDev;
// 	WAVEINCAPSA wic;
// 
// 	char str_audio_dev_loc[32];
// 	Audio_Dev_Info	audio_dev_info_loc;
// 
// 	// set locale Operation System
// 	setlocale( LC_ALL, "");
// 
// 	nInDev = waveInGetNumDevs();
// 
// 	printf("Windows Info Wave Device 44.1 kHz, stereo, 16-bit, stereo input\n");
// 	printf(">>\n");
// 
// 	if (nInDev > 0){
// 		printf("WaveDevSys[№]:\n");
// 	}
// 
// 	for (int i = 0; i < nInDev; i++) {
// 		if (!waveInGetDevCapsA(i, &wic, sizeof(WAVEINCAPSA))) {
// 
// 			// We are only interested in devices supporting 44.1 kHz, stereo, 16-bit, stereo input
// 			if (wic.dwFormats & WAVE_FORMAT_4S16) {
// 
// 				audio_dev_info_loc.order_nmb = i;
// 				sprintf_s(audio_dev_info_loc.name_in_system, _MAX_PATH, "%s", wic.szPname);
// 
// 				// save audio device in list
// 				m_List_Audio_Dev_Sys.push_back(audio_dev_info_loc);
// 
// 				// print names in Log file to next preview
// 				printf("%d:%s\n", i, wic.szPname);
// 			}
// 		}
// 	}
// 	printf("<<\n");
// }

//////////////////////////////////////////////////////////////////////////
// get device name by the device number from initialization file
// char* CAudioStreamWriter::GetNameAudioDevINI(	UINT	device_nmb_in	// device number [1-8]
// 																						)
// {
// 	char *nm_Audio_dev = NULL;
// 
// 	IterListAudioDevInfos	it_AudioDevINI_loc = m_List_Audio_Dev_INI.begin();
// 	IterListAudioDevInfos	it_AudioDevINI_end = m_List_Audio_Dev_INI.end();
// 
// 	for (; it_AudioDevINI_loc != it_AudioDevINI_end; ++it_AudioDevINI_loc){
// 		if (it_AudioDevINI_loc->order_nmb == device_nmb_in){
// 			return it_AudioDevINI_loc->name_in_system;
// 		}
// 	}
// 
// 	return NULL;
// }

//////////////////////////////////////////////////////////////////////////
// get approval searching audio device in system(Windows)
// bool CAudioStreamWriter::GetNameAudioDevSys(	char *name_device_in	// name device for the searching
// 																						)
// {
// 	// just in case
// 	if (!name_device_in){
// 		return false;
// 	}
// 
// 	char *nm_Audio_dev = NULL;
// 
// 	IterListAudioDevInfos	it_AudioDevSys_loc = m_List_Audio_Dev_Sys.begin();
// 	IterListAudioDevInfos	it_AudioDevSys_end = m_List_Audio_Dev_Sys.end();
// 
// 	for (; it_AudioDevSys_loc != it_AudioDevSys_end; ++it_AudioDevSys_loc){
// 		if ( !strncmp(it_AudioDevSys_loc->name_in_system, name_device_in, strlen(name_device_in)) ){
// 			return true;
// 		}
// 	}
// 
// 	return false;
// }
//////////////////////////////////////////////////////////////////////////


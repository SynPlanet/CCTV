#include "StdAfx.h"
#include "AsioWriter.h"

//#include "..\CaptureIP\TestTime.h"


bool loadAsioDriver(char *name);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// 
//CAsioWriter	g_AsioWriter;
extern CAsioWriter	*g_pAsioWriter;
extern AsioDrivers* asioDrivers;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//CTestTime	g_TestTime;

CAsioWriter::CAsioWriter(void)
{
	asioDrivers = NULL;

	enable_asio = false;

	// set default parameters fr global value
	m_InBuffer_size = 0;
	m_sectors_number = 2*g_MaxNmbChannels;
	m_sectors_counter = 0;
	m_out_sector_size = 0;

	m_bytes_SzData_real = 0;

	m_max_nmb_LinesIn_dev = 0;
	m_max_nmb_LinesOut_dev = 0;

	//m_hAsioThread = NULL;

	m_pSynh_Bar_StCam_1 = NULL;		// sinh. object barrier the state camera
	m_pSynh_Bar_StCam_2 = NULL;		// sinh. object barrier the state camera

	InterlockedExchange(&m_state_rec, T_DeviceState::STOP);

//	InterlockedExchange(&m_FlagAsio_Thread, 0);

//	InterlockedExchange64(& m_lCntFrame_AsioTimer_All, 0);
//	InterlockedExchange64(&m_lCntFrame_AsioTimer_Wr, 0);

	InterlockedExchange64(&m_lCntFrame_CmnTimerCAMs, 0);
//	InterlockedExchange64(&m_lCntFrame_CmnTimerCAMs_init, 0);
//	InterlockedExchange64(&m_lCmnTimeWrt_CAMs, 0);

	InterlockedExchange(&m_lLastState_CAMs, 0);

	InterlockedExchange(&m_flag_Enable_BarrierOut, 0);

	m_iOffsetChannels_Asio = 0;

	frames_lost_dbl = 0;
	tm_Wr_CAMs_ms = 0.0;

	InterlockedExchange(&m_nmb_rec, 0);

	sprintf(m_BasePathRecord,".//");
}

//////////////////////////////////////////////////////////////////////////
// initialization data
void CAsioWriter::Init(void)
{
	m_List_Writers_MP3.clear();
	m_List_ASIODriverInfo_INI.clear();

	ReadIniFile();

	// ASIO driver MUST be able to initialize in the Main Thread
	if (!InitDriver()){
		CreateBuffers();
	}

	// launch the asio process
	AsioThread_Launch();
}

//////////////////////////////////////////////////////////////////////////
CAsioWriter::~CAsioWriter()
{
	// release all data ASIO
	ReleaseAll();
}

//////////////////////////////////////////////////////////////////////////
// release all data ASIO
void CAsioWriter::ReleaseAll(void)
{
	m_List_ASIODriverInfo_INI.clear();

	// release Asio thread using
	AsioThread_Stop();

	// release and deallocate data files MP3 converter
	ReleaseWriters();

	if (asioDrivers){
		// release ASIO driver's methods
		ASIODisposeBuffers();
		ASIOExit();

		delete asioDrivers;
		asioDrivers = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// create common thread for ASIO processing
bool CAsioWriter::AsioThread_Launch(void)
{
	if (enable_asio){

		// Common function for all audio writers
		if (ASE_OK == ASIOStart()){
			cout << "Asio thread was created..." << endl;
			return true;
		}else{
			enable_asio = false;
		}
	}
	cout << "!!! ERROR !!!" << "Asio start can not be implemented" << endl;

	return false;
}

//////////////////////////////////////////////////////////////////////////
// release Asio thread using
void CAsioWriter::AsioThread_Stop(void)
{
	// terminate thread
	if (enable_asio){

		if (ASE_OK == ASIOStop()){
			cout << "Asio thread was released..." << endl;
		}

		InterlockedExchange(&m_state_rec, T_DeviceState::STOP);

		enable_asio = false;
	}
}

//////////////////////////////////////////////////////////////////////////
void CAsioWriter::ReleaseWriters(void)
{
	m_SynhCS.Set_Critical_Section();
	{
		IterListWriterMP3	it_WriterMp3_loc = m_List_Writers_MP3.begin();
		IterListWriterMP3	it_WriterMp3_end = m_List_Writers_MP3.end();

		for (; it_WriterMp3_loc != it_WriterMp3_end; ++it_WriterMp3_loc){

			CWriter_MP3	*p_Writer_MP3_del = static_cast<CWriter_MP3*>(*it_WriterMp3_loc);
			//p_Writer_MP3_del->ReleaseFileMP3();

			delete p_Writer_MP3_del;
			p_Writer_MP3_del = NULL;
		}

		m_List_Writers_MP3.clear();
	}
	m_SynhCS.Leave_Critical_Section();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// read file initialization(get list audio devices from ini file)
void CAsioWriter::ReadIniFile( void )
{
	char str_loc[32];

	// material number have just read
	int nmb_mat_identical_loc = 0;	

	char str_audio_drv_loc[32];
	ASIO_Driver_Info	asio_drv_info_loc;

	// searching all colors for each read materials
	for(int nmb_mtr_loc = 0; nmb_mtr_loc < COUNT_AUDIO_DEVICE; ++nmb_mtr_loc){

		sprintf(str_audio_drv_loc, "%s%d", PREFIX_ASIO_DRV_MODEL, nmb_mtr_loc);

		GetPrivateProfileStringA( NAME_ASIO_DRV_DEVICE, str_audio_drv_loc, "", str_loc, sizeof(str_loc), FILENAME_INI_DEVICE );

		if (strlen(str_loc) > 0 ){
			asio_drv_info_loc.order_nmb = nmb_mtr_loc;
			sprintf_s(asio_drv_info_loc.name_model, _MAX_PATH, str_loc);

			// save model from INI file to list
			m_List_ASIODriverInfo_INI.push_back(asio_drv_info_loc);
		}
	}

	// get offset channel using by Asio Writer/Reader
	m_iOffsetChannels_Asio = GetPrivateProfileIntA(NAME_ASIO_DRV_DEVICE, PREFIX_ASIO_CHANNEL_OFFSET, 0, FILENAME_INI_DEVICE);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CAsioWriter::bufferSwitch(long index, ASIOBool direct)
{
// 	double msec_double = g_TestTime.GetTimer_MSec();
// 	
// 	if (InterlockedCompareExchange(&g_pAsioWriter->m_state_rec, (LONG)T_DeviceState::WRITE, (LONG)T_DeviceState::WRITE)){
// 		printf("CAsioWriter::bufferSwitch->msec_double: %f\n", msec_double);
// 	}

	if (!g_pAsioWriter){
		return;
	}
	if(index == 0){
			g_pAsioWriter->Sync_Function();
	}

	if ((LONG)T_DeviceState::WRITE == InterlockedCompareExchange(&g_pAsioWriter->m_state_rec, (LONG)T_DeviceState::WRITE, (LONG)T_DeviceState::WRITE)){

		// for test save last STATE
		InterlockedExchange(&g_pAsioWriter->m_lLastState_CAMs, g_pAsioWriter->m_state_rec);

		// get buffer from ASIO and save this ones
		// => !!! SUPPOSE ASIO BUFFER FOR EACH DEVICES ARE NOT CHANGED INSIDE THE SINGLE CALLBACK FUNCTION ('bufferSwitch') !!!
		for(int i=0; i<g_MaxNmbChannels; ++i){
			memcpy(	(void *)(g_pAsioWriter->m_p_out_buffers[i] + g_pAsioWriter->m_out_sector_size * g_pAsioWriter->m_sectors_counter),
							(const void *)g_pAsioWriter->m_p_in_buffers[index][i],
							g_pAsioWriter->m_InBuffer_size*SAMPLE_SIZE
						);
		}

		//////////////////////////////////////////////////////////////////////////
		g_pAsioWriter->m_sectors_counter++;
		if(g_pAsioWriter->m_sectors_counter >= g_pAsioWriter->m_sectors_number)
		{
			///////////////////
			g_pAsioWriter->m_SynhCS.Set_Critical_Section();
			{
				unsigned int cnt_units_wr = g_pAsioWriter->m_List_Writers_MP3.size();
				IterListWriterMP3	it_WriterMp3_loc = g_pAsioWriter->m_List_Writers_MP3.begin();

				DWORD	dwOutSz_Wr_frame = 0;
				double tm_real_MP3_ms_now = 0.0;
				int	frames_lost_int = 0;

				// calculate full time writing
				g_pAsioWriter->tm_Wr_CAMs_ms = g_pAsioWriter->m_lCntFrame_CmnTimerCAMs*40.0; // 40 [ms] -> 1/25 video frame

				for (unsigned int nmb_unit = 0; nmb_unit < cnt_units_wr; ++nmb_unit){

					// it's working unit!
					if ( (*it_WriterMp3_loc)->GetEnable() ){
						if (dwOutSz_Wr_frame = (*it_WriterMp3_loc)->WriteDataMP3( (PSHORT)g_pAsioWriter->m_p_out_buffers[nmb_unit], g_pAsioWriter->tm_Wr_CAMs_ms)){
							//////////////////////////////////////////////////////////////////////////
							// search the lost stream data( time frames ) and save the current frame by several time
							// set synchronization with CAMERAs module

							tm_real_MP3_ms_now = double( (*it_WriterMp3_loc)->m_bytes_FullSize_MP3/* - (*it_WriterMp3_loc)->m_lFileSzMP3_last*/)/BIT_RATE_TO_BYTE;		// 16 - stereo (16 Byte/sec => 128 kBit/sec)

							g_pAsioWriter->frames_lost_dbl = ( g_pAsioWriter->tm_Wr_CAMs_ms - tm_real_MP3_ms_now )
																									/
																								(dwOutSz_Wr_frame/BIT_RATE_TO_BYTE );

							frames_lost_int = (int)g_pAsioWriter->frames_lost_dbl;

							if (frames_lost_int > 0){

								printf("LOST AUDIO FRAMES[%d]:%d;\n", (*it_WriterMp3_loc)->m_port_ID, frames_lost_int);

								// write additional audio frames
								for (int frm_wr = 0; frm_wr < frames_lost_int; ++frm_wr){
									(*it_WriterMp3_loc)->WriteDataMP3( (PSHORT)g_pAsioWriter->m_p_out_buffers[nmb_unit], g_pAsioWriter->tm_Wr_CAMs_ms);
								}
							}
							// 						if (tm_CmnTimerCAMs < double((*it_WriterMp3_loc)->m_bytes_SzData_MP3)/BIT_RATE_TO_BYTE){
							// 							printf("ERROR AUDIO SYNC[%d]->'Audio-Video'[ms]:%f; DataAudioFrame[ms]:%f !!!\n", (*it_WriterMp3_loc)->m_port_ID, double((*it_WriterMp3_loc)->m_bytes_SzData_MP3)/BIT_RATE_TO_BYTE - tm_CmnTimerCAMs, (dwOutSz_Wr_frame/BIT_RATE_TO_BYTE ) );
							// 							printf("ERROR INFO[%d]->TimeCAMsWrt_Whole[ms]:%f; TimeAudioFile[ms]:%f;  \n", (*it_WriterMp3_loc)->m_port_ID, tm_CmnTimerCAMs, double((double)(*it_WriterMp3_loc)->m_bytes_SzData_MP3/BIT_RATE_TO_BYTE) );
							// 						}
							//////////////////////////////////////////////////////////////////////////
						}
					}
					std::advance(it_WriterMp3_loc, 1);
				}
			}
			g_pAsioWriter->m_SynhCS.Leave_Critical_Section();
			////////////////////////////tm_Wr_CAMs_ms//////////////////////////////////////////////

			//
			g_pAsioWriter->m_sectors_counter = 0;

			// AUX information
			//InterlockedIncrement64(&g_pAsioWriter->m_lCntFrame_AsioTimer_Wr);

			// calculation uncompressed data size
			g_pAsioWriter->m_bytes_SzData_real += g_pAsioWriter->m_out_buffer_size/2*SAMPLE_SIZE;	// "/2"- define only one line in buffers
		}
		//////////////////////////////////////////////////////////////////////////

		//InterlockedIncrement64(&g_pAsioWriter-> m_lCntFrame_AsioTimer_All);
	}else{
			if (g_pAsioWriter->m_state_rec != InterlockedCompareExchange(&g_pAsioWriter->m_lLastState_CAMs, g_pAsioWriter->m_state_rec, g_pAsioWriter->m_state_rec) ){

				if (T_DeviceState::WRITE == InterlockedCompareExchange(&g_pAsioWriter->m_lLastState_CAMs, T_DeviceState::WRITE, T_DeviceState::WRITE) ){

					// calculate full time writing
					g_pAsioWriter->tm_Wr_CAMs_ms = g_pAsioWriter->m_lCntFrame_CmnTimerCAMs*40.0; // 40 [ms] -> 1/25 video frame

					// calculate full time writing
					//			g_pAsioWriter->tm_Wr_CAMs_ms = (g_pAsioWriter->m_lCntFrame_CmnTimerCAMs - g_pAsioWriter->m_lCntFrame_CmnTimerCAMs_init)*40.0; // 40 [ms] -> 1/25 video frame
					//	printf("STATE:STOP->m_sectors_counter: %d\n", g_pAsioWriter->m_sectors_counter);
					//	printf("STATE:STOP->tm_Wr_CAMs_ms: %f \n", g_pAsioWriter->tm_Wr_CAMs_ms);

					printf("ASIO->m_lCntFrame_CmnTimerCAMs: %llu \n", g_pAsioWriter->m_lCntFrame_CmnTimerCAMs);

					// the asio buffer begin to fill up ===> calculate finish writing audio file
					unsigned int cnt_units_wr = g_pAsioWriter->m_List_Writers_MP3.size();
					IterListWriterMP3	it_WriterMp3_loc = g_pAsioWriter->m_List_Writers_MP3.begin();
					double tm_real_MP3_ms_now = 0.0;
					int sz_dummy2wr = 0;	// size need to write to the end of file

					for (unsigned int nmb_unit = 0; nmb_unit < cnt_units_wr; ++nmb_unit){

						// it's working unit!
						if ( (*it_WriterMp3_loc)->GetEnable() ){
							tm_real_MP3_ms_now = (double( (*it_WriterMp3_loc)->m_bytes_FullSize_MP3 /*- (*it_WriterMp3_loc)->m_lFileSzMP3_last*/))/BIT_RATE_TO_BYTE;		// 16 - stereo (16 Byte/sec => 128 kBit/sec)
							sz_dummy2wr = (g_pAsioWriter->tm_Wr_CAMs_ms - tm_real_MP3_ms_now)*BIT_RATE_TO_BYTE;

							(*it_WriterMp3_loc)->WriteDummyDataMP3(sz_dummy2wr);

							// set default
							//InterlockedExchange64(&(*it_WriterMp3_loc)->m_bytes_FullSize_MP3, 0);
						}
						std::advance(it_WriterMp3_loc, 1);
					}
				}

				if (T_DeviceState::STOP == InterlockedCompareExchange(&g_pAsioWriter->m_state_rec, T_DeviceState::STOP, T_DeviceState::STOP) ){
					g_pAsioWriter->SetState(T_DeviceState::STOP);
				}
				if (T_DeviceState::PAUSE == InterlockedCompareExchange(&g_pAsioWriter->m_state_rec, T_DeviceState::PAUSE, T_DeviceState::PAUSE) ){
					g_pAsioWriter->SetState(T_DeviceState::PAUSE);
				}

				// set last state camera to a new value
				InterlockedExchange(&g_pAsioWriter->m_lLastState_CAMs, g_pAsioWriter->m_state_rec);
			}
		g_pAsioWriter->m_sectors_counter = 0;
	}
	//g_TestTime.StartTimer();
}

void CAsioWriter::sampleRateDidChange(ASIOSampleRate rate)
{
}

long CAsioWriter::asioMessage(long selector, long value, void* message, double* opt)
{
	return 0;
}

ASIOTime* CAsioWriter::bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool direct)
{
	bufferSwitch(index, direct);
	return params;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// initialization ASIO driver
int CAsioWriter::InitDriver(void)
{
	// just in case
	if (asioDrivers){
		cout << "!!! ERROR !!! Asio driver was initialized already!" << endl;
		return 1;
	}

	asioDrivers = new AsioDrivers();

	if (!asioDrivers->asioGetNumDev()){
		cout << "!!! ERROR !!! there has not Asio driver found in Windows!" << endl;
		return 1;
	}

	cout << "asio drivers found:" << endl << ">>" << endl;
	for(long i = 0; i < asioDrivers->asioGetNumDev(); i++)
	{
		char buffer[32];
		asioDrivers->asioGetDriverName(i, buffer, 32);

		cout << i << " <" << buffer << ">" << endl;
	}
	cout<< ">>" << endl;

	//////////////////////////////////////////////////////////////////////////
	ASIODriverInfo driver_info;
	memset(&driver_info, 0, sizeof(driver_info));
	driver_info.asioVersion = 2;

	bool inited = false;

	// try to initialize ASIO driver model from INI file consecutive
	IterListASIODriverInfo it_ListASIODriverInfo_loc = m_List_ASIODriverInfo_INI.begin();
	IterListASIODriverInfo it_ListASIODriverInfo_end = m_List_ASIODriverInfo_INI.end();
	unsigned int sz_asio_mdl_ini = m_List_ASIODriverInfo_INI.size();
	
	for(; it_ListASIODriverInfo_loc != it_ListASIODriverInfo_end; ++it_ListASIODriverInfo_loc){

		cout << "driver <" << it_ListASIODriverInfo_loc->name_model << ">" << endl;
		cout << "Loading ... " << flush;
		if(loadAsioDriver((char*)it_ListASIODriverInfo_loc->name_model)){
			cout << "->OK" << endl;
			cout << "Initialization ..." << flush;
			if(!ASIOInit(&driver_info)){
				cout << "->OK" << endl << endl;
				inited = true;
				break;
			}
			else{
				cout << "->Error!" << endl << endl;
			}
		}else{
			cout << "->Error!" << endl << endl;
		}
	}

	// if couldn't initialize model ASIO from INI file try to initialize it from embedded models(see namespace AudioDevice->known_drivers[])
	if(!inited){
		for(unsigned int i = 0; i<(sizeof(known_drivers)/sizeof(char *)); i++)
		{
			cout << "driver <" << known_drivers[i] << ">" << endl;
			cout << "Loading ... " << flush;
			if(loadAsioDriver((char*)known_drivers[i])){
				cout << "->OK" << endl;
				cout << "Initialization ..." << flush;
				if(!ASIOInit(&driver_info))
				{
					cout << "->OK" << endl << endl;
					inited = true;
					break;
				}
				else
					cout << "->Error!" << endl << endl;
			}
			else
				cout << "->Error!" << endl << endl;;
			}
	}

	if(!inited)
	{
		cout << "Error: could't initialize Asio driver!" << endl;
		return 1;
	}

	cout << "asio version : " << driver_info.asioVersion << endl;
	cout << "driver version : " << driver_info.driverVersion << endl;
	cout << "driver name : " << driver_info.name << endl;
	cout << endl;

	//////////////////////////////////////////////////////////////////////////
	long channels[2];
	ASIOGetChannels(&channels[0], &channels[1]);

	// save maximum lines using by device
	m_max_nmb_LinesIn_dev = channels[0];
	m_max_nmb_LinesOut_dev = channels[1];

	cout << "Numbers of ASIO device lines in: " << m_max_nmb_LinesIn_dev << endl;
	cout << "Reserve lines in software: " << g_MaxNmbChannels << endl;

	if (m_max_nmb_LinesIn_dev > g_MaxNmbChannels){
		cout << "!!! ERROR !!!" << "There is not corresponding numbers of device lines in and reserve lines in program" << endl;
	}else{
		cout << "!!! Allocation buffers is OK !!!" << endl;
	}

	if (m_iOffsetChannels_Asio){
		cout << "Using offset for input channels(*.INI): "<< m_iOffsetChannels_Asio << endl;
	}

	for(int i=0; i<2; i++)
	{
		if(i == 0)
			cout << "input";
		else
			cout << "output";
		cout << " channels:" << endl;

		for(int j=0; j<channels[i]; j++)
		{

			ASIOChannelInfo channel_info;
			channel_info.channel = j;

			if( (i == 0)
					&&
					(j >= m_iOffsetChannels_Asio)	// skip channels by using INI parameters
				)
				channel_info.isInput = ASIOTrue;
			else
				channel_info.isInput = ASIOFalse;

			ASIOGetChannelInfo(&channel_info);

			cout << j << " <" << channel_info.name << "> <" << channel_info.type << ">";

			if(channel_info.isInput == ASIOTrue){
				cout << ": '+'" << endl;
			}else{
				cout << ": '-'" << endl;
			}
		}
		cout << endl;
	}

	long buf_min, buf_max, buf_pre, buf_grn;
	ASIOGetBufferSize(&buf_min, &buf_max, &buf_pre, &buf_grn);
	m_InBuffer_size = buf_max;
	cout << "minimun buffer size " << buf_min << endl;
	cout << "maximun buffer size " << buf_max << endl;
	cout << "preferred buffer size " << buf_pre << endl;
	cout << "buffer granilarity " << buf_min << endl;
	cout << endl;
	cout << "set buffer size  " << m_InBuffer_size << endl;
	cout << endl;

	ASIOSampleRate rate;
	ASIOGetSampleRate(&rate);
	cout << "default rate : " << rate << endl;
	//	ASIOCanSampleRate(rate);
	ASIOSetSampleRate((ASIOSampleRate)SAMPLE_RATE);
	ASIOGetSampleRate(&rate);
	cout << "set sample rate : " << rate << endl << endl;

	long inputLatency = 0;
	long outputLatency = 0;
	ASIOError err;
	err = ASIOGetLatencies(&inputLatency, &outputLatency);

	ASIOClockSource clocks[16];
	clocks[0].associatedChannel = -1;
	clocks[0].associatedGroup = -1;
	long n_clocks = 16;
	ASIOGetClockSources(clocks, &n_clocks);
	cout << "clock sources number " << n_clocks << endl;
	for(int i=0; i<n_clocks; i++)
		cout << "clock source " << i << " <" << clocks[0].name << ">" << endl;
	ASIOSetClockSource(0);
	cout << endl;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// assignment buffers for Asio
int CAsioWriter::CreateBuffers( void )
{
	ASIOBufferInfo buffer_info[g_MaxNmbChannels];
	for(int i=0; i<g_MaxNmbChannels; i++)
	{
		buffer_info[i].channelNum = i + m_iOffsetChannels_Asio;
		buffer_info[i].isInput = ASIOTrue;
	}

	ASIOCallbacks callbacks;
	callbacks.bufferSwitch = bufferSwitch;
	callbacks.sampleRateDidChange = sampleRateDidChange;
	callbacks.asioMessage = asioMessage;
	callbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo;
	if(ASIOCreateBuffers(buffer_info, g_MaxNmbChannels, m_InBuffer_size, &callbacks))
	{
		cout << "error : can't allocate buffers!" << endl;
		return 1;
	}

	m_out_sector_size = m_InBuffer_size;
	m_out_buffer_size = m_out_sector_size*m_sectors_number;
	m_sectors_counter = 0;

	for(int nmb_port=0; nmb_port < g_MaxNmbChannels; ++nmb_port)
	{
		m_p_in_buffers[0][nmb_port] = (int *)buffer_info[nmb_port].buffers[0];
		m_p_in_buffers[1][nmb_port] = (int *)buffer_info[nmb_port].buffers[1];

		m_p_out_buffers[nmb_port] = (int *)malloc(m_out_buffer_size*SAMPLE_SIZE);

		//////////////////////////////////////////////////////////////////////////
		m_SynhCS.Set_Critical_Section();
		{
			// create a new instance (object class CAudioWriter_MP3)
			CWriter_MP3	*p_Writer_MP3_new = NULL;
			//char *name_dev_loc = NULL;
			long	type_channel_loc = T_TypeChannel::RIGHT_CNL;

			//create class instance to write MP3 file
			p_Writer_MP3_new = new CWriter_MP3(	BIT_RATE, SAMPLE_RATE, SAMPLE_RATE, nmb_port+1, /*name_dev_loc,*/ type_channel_loc );

			p_Writer_MP3_new->SetBufSize(m_out_buffer_size);

			m_List_Writers_MP3.push_back(p_Writer_MP3_new);
		}
		m_SynhCS.Leave_Critical_Section();
		//////////////////////////////////////////////////////////////////////////
	}
	// set flag ASIO driver enable
	enable_asio = true;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// get synchronize objects for output control(camera control)
void CAsioWriter::SetBarrierCtrlCam(	void *pSynh_Barrier_StateCam_1_in,
																			void *pSynh_Barrier_StateCam_2_in
																		)
{
	m_pSynh_Bar_StCam_1 = (CSynh_Barrier*)pSynh_Barrier_StateCam_1_in;
	m_pSynh_Bar_StCam_2 = (CSynh_Barrier*)pSynh_Barrier_StateCam_2_in;
}

//////////////////////////////////////////////////////////////////////////
// function for synchronization( setting and management of working barrier )
void CAsioWriter::Sync_Function(void)
{
	// define opportunity to use global barrier
	if (InterlockedCompareExchange(&m_flag_Enable_BarrierOut, 1,1)){
		//printf("CAsioWriter::Sync_Function!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		
		if (InterlockedCompareExchange(&m_pSynh_Bar_StCam_1->m_InterLock_flag, 1,1)){
			printf("Block begin(Asio)...\n");
			// stop thread to align "the first point"
			m_pSynh_Bar_StCam_1->Block();
			{
				;
			}
			// stop thread to align "second point"   (!!!unblock another threads!!!)
			m_pSynh_Bar_StCam_2->Block();
			printf("...Block end(Asio)\n");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// set state needed
int CAsioWriter::SetState(	long state_in	//T_DeviceState::(...)
													)
{
	//////////////////////////////////////////////////////////////////////////
	// change state ASIO
	switch (state_in){
		case (T_DeviceState::STOP):
			{
				m_SynhCS.Set_Critical_Section();
				{
					unsigned int cnt_units_wr = m_List_Writers_MP3.size();
					IterListWriterMP3	it_WriterMp3_loc = m_List_Writers_MP3.begin();

					for (unsigned int nmb_unit = 0; nmb_unit < cnt_units_wr; ++nmb_unit){

						if ((*it_WriterMp3_loc)->GetEnable()){
							(*it_WriterMp3_loc)->ReleaseFileMP3();
						}

						std::advance(it_WriterMp3_loc, 1);
					}
				}
				m_SynhCS.Leave_Critical_Section();

				// test
		//		printf("CAsioWriter::m_lCntFrame_AsioTimer_All: %llu \n", m_lCntFrame_AsioTimer_All);
		//		printf("CAsioWriter::m_lCntFrame_AsioTimer_Wr: %llu \n", m_lCntFrame_AsioTimer_Wr);
				printf("AsioWriter::Uncompressed audio data: %d \n", m_bytes_SzData_real);

				// set default value
			//	InterlockedExchange64(&m_lCntFrame_AsioTimer_All, 0);
			//	InterlockedExchange64(&m_lCntFrame_AsioTimer_Wr, 0);
				InterlockedExchange64(&m_bytes_SzData_real, 0);
			//	InterlockedExchange64(&m_lCntFrame_CmnTimerCAMs_init, 0);
			}
			break;
		case (T_DeviceState::WRITE):
			{
				;// see class CAudioStreamWriter()
			}
			break;
			case (T_DeviceState::PAUSE):
			{
				;// set default value
				 //	InterlockedExchange64(&m_lCntFrame_CmnTimerCAMs_init, 0);
			}
			break;

	}
	//////////////////////////////////////////////////////////////////////////

	return 0;
}

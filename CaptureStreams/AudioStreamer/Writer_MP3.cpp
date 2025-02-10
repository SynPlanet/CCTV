#include "StdAfx.h"
#include "Writer_MP3.h"

CWriter_MP3::CWriter_MP3(	unsigned int BitRate, unsigned int SampleRateInput, unsigned int SampleRateReq,
													unsigned int nmb_port,
													//char*				 name_audio_dev_sys,
													LONG				 channel_type
												):
										//			m_Enc_MP3(BitRate, SampleRateInput, SampleRateReq),
													m_bitrate(BitRate),
													m_frequency(SampleRateInput),
													m_SimpleRate(SampleRateReq),
													m_port_ID(nmb_port),
													m_channel_type(channel_type)
{
	InterlockedExchange(&m_state_rec, LONG(T_DeviceState::STOP));
	InterlockedExchange64(&m_CntFramesSave, 0);
	InterlockedExchange64(&m_bytes_FullSize_MP3, 0);

	m_out_buffer_size = 0;
	m_dwBytesRecorded = 0;

	m_file_MP3 = NULL;
	m_pBuf_MP3_out = NULL;

	m_pEncording_MP3_AUX = new CMP3Simple(BIT_RATE, SAMPLE_RATE, SAMPLE_RATE);

	m_enable = false;
}

CWriter_MP3::~CWriter_MP3()
{
	// release file MP3
	ReleaseFileMP3();

	// release encoder MP3
	if (m_pEncording_MP3_AUX){
		delete m_pEncording_MP3_AUX;
		m_pEncording_MP3_AUX = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// create audio file by the recording number and the path
int CWriter_MP3::CreateAudioFile(	unsigned int nmb_record_in,				// order record number of device
																	char *path_to_wr_in								// path to write
																)
{
	// create buffer for MP3 safe encode data
	m_pBuf_MP3_out = new BYTE[m_frequency * 4];

	//create a file for audio saving
	char filename_mp3[LENGTH_PATH_STREAM] = {""};
	char fullpath_filename_mp3[LENGTH_PATH_STREAM] = {""};

	if (m_channel_type == T_TypeChannel::RIGHT_CNL){
		sprintf_s(filename_mp3, LENGTH_PATH_STREAM, "%s%d%s%d%s", NAME_AUDIO_REC_HEADER, m_port_ID, NAME_AUDIO_REC_SUFFIX, nmb_record_in, NAME_AUDIO_REC_TAIL);
	}

	sprintf_s(fullpath_filename_mp3, LENGTH_PATH_STREAM, "%s\\%s", path_to_wr_in, filename_mp3);

	m_file_MP3 = fopen(fullpath_filename_mp3/*"music.mp3"*/, "wb");
	////////////////////////////////////////////

	if (m_file_MP3 == NULL){
		throw "Can't create MP3 file.";
		return (-1);
	}else{
		InterlockedExchange(&m_state_rec, LONG(T_DeviceState::PAUSE));
		InterlockedExchange64(&m_CntFramesSave, 0);
		InterlockedExchange64(&m_bytes_FullSize_MP3, 0);

		return (0);
	}
}

int CWriter_MP3::SetBufSize(unsigned int out_buffer_size_in)
{
	m_out_buffer_size = out_buffer_size_in;
	m_dwBytesRecorded = m_out_buffer_size*SAMPLE_SIZE;
	m_dwBytesRecorded /= 2;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// write block memory to file by using converter to MP3
DWORD CWriter_MP3::WriteDataMP3(	PSHORT lpAudioData,				// audio buffer (not compressed)
																	double time_wr_CAMs_ms_in // time[ms] from CAMs need to write
																)
{
	if (m_file_MP3 && m_pBuf_MP3_out&& m_pEncording_MP3_AUX){
		DWORD	dwOutSzWr = 0;	// size in bytes of each element to be written

		// convert block audio data(WAVE) to MP3
		m_pEncording_MP3_AUX->Encode((PSHORT) lpAudioData, m_dwBytesRecorded, m_pBuf_MP3_out, &dwOutSzWr);

		// check exceeding audio file
		if ( time_wr_CAMs_ms_in < ((LONGLONG)dwOutSzWr + m_bytes_FullSize_MP3/* - m_lFileSzMP3_last*/) / BIT_RATE_TO_BYTE ){

			printf("Number_MP3[%d]: ASIO volume > Video volume: -> Audio frame was thrown out\n", m_port_ID);

			// except this audio frame
			return 0;
		}

		// flushing frame MP3 data
		//BE_ERR err = m_pEncording_MP3_AUX->Flush(m_pBuf_MP3_out, &dwOutSzWr);
		size_t sz_elem = fwrite(m_pBuf_MP3_out, dwOutSzWr, 1, m_file_MP3);

		if (sz_elem > 0){
			// flushing file stream
			fflush(m_file_MP3);

			//FlushFileBuffers(m_file_MP3);

			// set real state
			InterlockedExchange(&m_state_rec, LONG(T_DeviceState::WRITE));
			// calculate full size of compressed data (MP3)
			InterlockedExchange64(&m_bytes_FullSize_MP3, (LONGLONG)dwOutSzWr + m_bytes_FullSize_MP3);
			InterlockedIncrement64(&m_CntFramesSave);

			return dwOutSzWr;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// write finish block memory to file
int CWriter_MP3::WriteDummyDataMP3(DWORD sz_dummy_in)
{
	if (m_file_MP3 && (sz_dummy_in > 0) ){

		BYTE	*pBuf_MP3_dummy = (BYTE*)malloc(sz_dummy_in);	// buffer of MP3 encode data

		ZeroMemory((void *)pBuf_MP3_dummy, sz_dummy_in );

		// flushing frame MP3 data
		size_t sz_elem = fwrite(pBuf_MP3_dummy, sz_dummy_in, 1, m_file_MP3);

		if (sz_elem > 0){

		//	printf("Number_MP3[%d]: Write empty data to Audio(%d bytes)\n", m_port_ID, sz_dummy_in );

			// flushing file stream
			fflush(m_file_MP3);

			//FlushFileBuffers(m_file_MP3);

			// set real state
			InterlockedExchange(&m_state_rec, LONG(T_DeviceState::WRITE));
			// calculate full size of compressed data (MP3)
			InterlockedExchange64(&m_bytes_FullSize_MP3, (LONGLONG)sz_dummy_in + m_bytes_FullSize_MP3);
			InterlockedIncrement64(&m_CntFramesSave);

			free( pBuf_MP3_dummy );

			return sz_dummy_in;
		}
		free( pBuf_MP3_dummy );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// release file MP3
int CWriter_MP3::ReleaseFileMP3( void )
{
	if (m_file_MP3){
		fclose(m_file_MP3);
		m_file_MP3 = NULL;
	}

	if (m_pBuf_MP3_out){
		delete m_pBuf_MP3_out;
		m_pBuf_MP3_out = NULL;
	}

	printf("Number_MP3[%d]: Volume MP3: %llu [byte]; Sec: %f\n", m_port_ID, m_bytes_FullSize_MP3, double(m_bytes_FullSize_MP3)/BIT_RATE_TO_BYTE/1000.0 );

	// set default value
	InterlockedExchange(&m_state_rec, LONG(T_DeviceState::STOP));
	InterlockedExchange64(&m_CntFramesSave, 0);
	InterlockedExchange64(&m_bytes_FullSize_MP3, 0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// get real state
LONG CWriter_MP3::GetState(	void ) const
{
	return m_state_rec;
}
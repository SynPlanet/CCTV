#pragma once


#include "../CaptureIP/Misc.h"
#include "../CaptureIP/Synh_Objs.h"
#include "../CaptureIP/CommonData.h"
using namespace CommonData;

#include "./AudioRecording/mp3_simple.h"
#include "./AudioRecording/waveIN_simple.h"

//#include "AudioStreamerData.h"

#include <list>
using namespace std;

// class CWaveINSimple;
// class CMixer;

using namespace AudioDevice;

// class includes management the single recording for getting device
class	CWriter_MP3//: public IReceiver
{
public:
	CWriter_MP3(	unsigned int	BitRate,						// BitRate-says at what bitrate to encode the raw (PCM) sound(e.g. 16, 32, 40, 48, ... 64, ... 96, ... 128, etc), see official LAME documentation for accepted values
								unsigned int	SampleRateInput,		// input frequency of the raw (PCM) sound(e.g. 44100, 32000, 22050, etc), see official LAME documentation for accepted values
								unsigned int	SampleRateReq,			// requested frequency for the encoded/output (MP3) sound
								unsigned int	nmb_port,						// order number port for get record
								//char*					name_audio_dev_sys,	// audio device name for connection the system inside (Windows)
								long					channel_type				// type channel was defined earlier
							);
	/*virtual*/	~CWriter_MP3(void);

	// release file MP3
	int ReleaseFileMP3( void );

	// write block memory to file by using converter to MP3
	DWORD WriteDataMP3(	PSHORT lpAudioData,				// audio buffer (not compressed)
											double time_wr_CAMs_ms_in // time[ms] from CAMs need to write
										);

	// write finish block memory to file
	int WriteDummyDataMP3(DWORD sz_dummy_in);

	// get number port was defined for the record
	unsigned int GetNmbPort(void) const {return m_port_ID;};

	// create audio file by the recording number and the path
	int CreateAudioFile(	unsigned int nmb_record_in,				// order record number of device
												char *path_to_wr_in								// path to write
											);

	int SetBufSize(unsigned int out_buffer_size_in);

	// get real state
	LONG GetState( void ) const;

	// set state needed
	void SetEnable(	bool enable_in ) { m_enable = enable_in; }
	bool GetEnable(	void ) const { return m_enable; }

	volatile LONGLONG m_bytes_FullSize_MP3;	// full size of compressed data [byte]

	unsigned int m_port_ID;		// record port number ID
private:
	unsigned int m_bitrate;
	unsigned int m_frequency;
	unsigned int m_SimpleRate;

	volatile LONGLONG m_CntFramesSave;		// count stream frames was written

	CMP3Simple	*m_pEncording_MP3_AUX;
	
	//unsigned int m_order_num_rec;		// order number of a record

	DWORD m_dwBytesRecorded;
	unsigned int m_out_buffer_size;

	volatile LONG m_state_rec;		// state [T_DeviceState::STOP / T_DeviceState::WRITE]
	
protected:
//public:
	//CMP3Simple	m_Enc_MP3;
	FILE *m_file_MP3;

	LONG	m_channel_type;		// see T_TypeChannel

	BYTE	*m_pBuf_MP3_out;	// buffer of MP3 encode data

	// flag enable examining process
	bool m_enable;
};

typedef list<CWriter_MP3*>						ListWriterMP3;
typedef list<CWriter_MP3*>::iterator	IterListWriterMP3;
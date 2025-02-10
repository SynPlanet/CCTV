#pragma once


// #include <io.h>
// #include <locale.h>
// #include <fcntl.h>
// #include <conio.h>
// #pragma setlocale(".1251")

#include "AsioWriter.h"

#include "../CaptureIP/Misc.h"
#include "../CaptureIP/Synh_Objs.h"
// #include "../CaptureIP/CommonData.h"
// using namespace CommonData;

 #include "./AudioRecording/mp3_simple.h"
 #include "./AudioRecording/waveIN_simple.h"

//#include "AudioStreamerData.h"

#include <list>
using namespace std;

class CWaveINSimple;
class CMixer;

using namespace AudioDevice;

//////////////////////////////////////////////
//the structure includes information about any device that we need to use
struct	Audio_Dev_Info
{
	unsigned int order_nmb;
	CHAR	name_in_system[_MAX_PATH];
	LONG	channel_type;								// see T_TypeChannel
};

typedef list<Audio_Dev_Info>						ListAudioDevInfos;
typedef list<Audio_Dev_Info>::iterator	IterListAudioDevInfos;


//////////////////////////////////////////////
//the structure includes information about any device that we need to use
struct	MAudio_Port_Info
{
	unsigned int	order_nmb;
	bool					enable;
	LONG					channel_type;								// see T_TypeChannel
};

typedef list<MAudio_Port_Info>						ListMAudioInfos;
typedef list<MAudio_Port_Info>::iterator	IterListMAudioInfos;
//////////////////////////////////////////////
//////////////////////////////////////////////
// class CWriter_MP3: public IReceiver
// {
// 
// };

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// class are used for recording streams to MP3 files
class CAudioStreamWriter{
	public:
		CAudioStreamWriter(void);
		virtual ~CAudioStreamWriter(void);

		// initialization data 
		virtual void Init(void);

		// release all audio recording devices
		virtual void ReleaseAll(void);

		// initialization data for recording audio
		virtual bool Connect(void);

		// release only necessary member's class
		virtual bool Disconnect(void);

		// set base path for saving audio tracks
		virtual bool	SetPath(	const CHAR *path_record_in	// audio recording path		[_MAX_PATH]
													);

		// set Audio device using(for the recording)
		virtual bool SetAudioDevice(	UINT	device_nmb_in,	// device number [0-7]
																	BOOL	flag_enable_in	// flag enabling device	(TRUE / FALSE)
																);

		// set state for audio device control(see T_DeviceState)
		virtual long	SetStateDevice(	long state_in	//T_DeviceState::(STOP/WRITE)
																);

		// get synchronize objects for output control(camera control)
		virtual void SetBarrierCtrlCam(	void *pSynh_Barrier_StateCam_1_in,
																		void *pSynh_Barrier_StateCam_2_in
																	);

		// get count streams writing(see size m_List_DevicesAWriter)
		virtual unsigned int GetCountStreams(	void );

private:
		// synchronization object for the writers
		CSynh_CritSect	m_SynhCS_AudioWr;

		// list devices was gotten from ini file (see FILENAME_INI_DEVICE)
		ListAudioDevInfos	m_List_Audio_Dev_INI;

		/////////////////
		// ports list M-Audio device was gotten from ini file (see FILENAME_INI_DEVICE)
		ListMAudioInfos	m_List_MAudio_Ports_INI;

		/////////////////

		// list devices was gotten from the system
		ListAudioDevInfos	m_List_Audio_Dev_Sys;

		// list singles audio recording devices
		//ListDevicesAWriter	m_List_DevicesAWriter;

		volatile LONG m_state;		// state [T_DeviceState::STOP / T_DeviceState::WRITE]

public:
		CAsioWriter	*m_pAsioWriter;		// pointer for output thread using(see class 'CCommonTimerThread', by project 'CaptureIP' )

//private:

	// print in log file the names of all audio devices (was gotten from system) and save the same list 
	//void PrintAudioDevicesSys( void );

	// get device name by device number from initialization file
// 	char* GetNameAudioDevINI(	UINT	device_nmb_in	// device number [1-8]
// 													);

	// get approval searching audio device in system(Windows)
// 	bool GetNameAudioDevSys(	char *name_device_in	// name device for the searching
// 													);
};


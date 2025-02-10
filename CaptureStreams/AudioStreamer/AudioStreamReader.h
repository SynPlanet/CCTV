#pragma once

#include "../CaptureIP/Misc.h"
#include "../CaptureIP/Synh_Objs.h"
/////#include "../CaptureIP/ProxyVLC.h"
#include "../CaptureIP/CommonData.h"

#include "..\CaptureIP\TestTime.h"

using namespace CommonData;
//#include "AudioStreamerData.h"
using namespace AudioDevice;
#include <list>
using namespace std;

//////////////////////////////////////////////
struct	Rec_Audio_Info
{
	//libvlc_media_player_t		*pMediaPlayer;

	//bool	mute_flag;			// flag mute [TRUE / FALSE]

	UINT	mci_real_ID;				// really device ID for using "mciSendCommand"

	unsigned int nmb_rec;				// number recording
	DWORD length_msec;						// time duration of Track
	DWORD lng_total_begin_msec;	// total time duration before rec.file 
};

typedef list<Rec_Audio_Info>						ListNmbRecAudios;
typedef	list<Rec_Audio_Info>::iterator	IterRecAudioInfo;
/*
struct	SyncVLC
{
	volatile LONG flag_NextTrack;		// flag defining escape to next track the playing (USING ONLY THE FIRST TRACK)
	volatile LONG flag_SetChannel;		// flag defining type of the channel(right/left) begin playing
	void* m_pTHIS;
};
*/
struct	AudioPlayDevice
{
	CHAR	index_nmb;		// order number audio device [0...8]
	BOOL	mute_flag;			// mute flag
};

typedef list<AudioPlayDevice>	ListAudioPlayDevice;
typedef	list<AudioPlayDevice>::iterator IterAudioPlayDevice;

struct	Audio_Track
{
	LONG	m_index_nmb;		// order number into list(all tracks)

	//SyncVLC	sync_VLC;	// structure includes of the synchronization signs

	//volatile LONG flag_NextTrack;		// flag defining escape to next track the playing (USING ONLY THE FIRST TRACK)
	//volatile LONG flag_SetChannel;		// flag defining type of the channel(right/left) begin playing

	BOOL	mute_flag;			// flag mute [TRUE / FALSE]

	LONG	total_leng_msec;			// total time 
	UINT	device_file_ID;				// number device ID from file parsing
	LONG	m_type_channel;		// see T_TypeChannel

	ListNmbRecAudios		list_nmb_File;	//  quantity files was included into the one Track audio
};

typedef list<Audio_Track>	ListAudioTrack;
typedef	list<Audio_Track>::iterator IterAudioTrack;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// class managing the reading MP3 files
class CAudioStreamReader {
public:
	CAudioStreamReader(void);
	// TODO: add your methods here.
	virtual ~CAudioStreamReader(void);

	// initialization data 
	virtual void Init(void);

	// release all class members
	virtual bool ReleaseAll(void);

	// initialization data for play audio
	virtual void Connect(void);

	// release only necessary member's class
	virtual bool Disconnect(void);

	// set base path for playing audio tracks
	virtual bool	SetPath(	const CHAR *path_play_in		// playing path for audio files	[_MAX_PATH]
												);

	// set state for audio device(see T_DeviceState)
	virtual long	SetStateDevice(	long state_in	//T_DeviceState::(STOP/PLAY/WRITE)
															);

	// set start time an audio track
	virtual long	Set_TimeStartTrack(	LONGLONG tm_start_play_in // time to play audio track [msec]
																	);

	// get length
	virtual unsigned int GetLengthTrack( unsigned int nmb_track_in // number track ( '0' - the first track )
																			);

	//set Audio device using(for the playing and recording)
	virtual bool SetAudioDevice(	UINT	device_nmb_in,	// device number [0-7]
																BOOL	flag_enable_in	// flag enabling device	(TRUE / FALSE)
															);

	// set pointer to callback function from outside
	//virtual void SetFuncGetTimeMovie(void* pFuncGetTimeAudio_in = NULL);

	// get synchronize objects for output control(camera control)
	virtual void SetBarrierCtrlCam(	void *pSynh_Barrier_StateCam_1_in,
																	void *pSynh_Barrier_StateCam_2_in
																);

	// get count streams reading(see size m_list_AudioTracks)
	virtual unsigned int GetCountStreams(	void );

	// get VLC proxy(by using module directly)
/////	bool SetProxyVLC(void* pProxyVLC_in = NULL);

	// function for synchronization( setting and management of working barrier )
	void Sync_Function(void);

	// set states for all records
	DWORD SetStateAllRecs(	DWORD state_in						// state track in
												);
public:
	/// synchronization data
	volatile LONGLONG m_lTotalTimePlayedCAMs;
	volatile LONG m_State_CAMs;	// state from CAMs

	volatile LONG m_flag_Enable_BarrierOut;	// the flag define enable classCSynh_Barrier on the outside

private:

	// time playing offset get from file INI
	volatile LONGLONG m_lOffsetTimePlay_INI;

	unsigned int m_nmb_devices;

	// synchronization object for the readers
	CSync_SWMR		m_Sync_SWMR;

	//CHAR m_PathRecord[_MAX_PATH];
	CHAR m_BasePathPlay[_MAX_PATH];
	CHAR m_NameFolderPlay[MAX_SIGN_NAME_FOLDER];

	//CStreamPlayAudio	m_ArrStreamPlayAudio[MAX_DEVICES];

	volatile LONG m_flag_Enable_Loop;	// the flag define enable loop thread
	volatile LONG m_state_audio_devs;	// state audio devices needed

	//volatile LONG m_TotalTimePlayed;	// time movie was played

	HANDLE	m_hAudioThread;
	DWORD   m_dwAudioThreadId;

	ListAudioTrack	m_list_AudioTracks;	// list include quantity audio objects(Tracks) folder inside

	ListAudioPlayDevice	m_list_AudioPlayDev; // list audio devices for playing control

//	long m_type_thread;			// type device for using(PLAYING / RECORDING / NONE)

/////	CProxyVLC* m_pProxyVLC;

	// pointer to callback function from outside
//	GetTimeMovie	m_pFuncGetTimeAudio;

	// pointers for barrier state control(from camera (project CaptureIP))
	CSynh_Barrier *m_pSynh_Bar_StCam_1;		// sinh. object barrier the state camera
	CSynh_Barrier *m_pSynh_Bar_StCam_2;		// sinh. object barrier the state camera

	// class for sleeping emulation
	CSleeping	m_Sleeping;

	// synchronization object for the reading
	CSynh_CritSect	m_SynhCS;

	volatile LONG m_flag_UpdatePath;	// the flag set update PATH track lists
	volatile LONG m_flag_UpdateState;	// the flag set update STATE all tracks
	volatile LONG m_flag_UpdateTime;	// the flag set update Time for playing tracks

	volatile LONGLONG m_NewTime_Playing;	// new time playing for all tracks


	MCI_OPEN_PARMS m_Patch_ParmsMCI;		// object(patch) MCI

private:
	// read file initialization (get offset duration audio from ini file)
	void ReadIniFile( void );

	void SetState_MediaMCI(	DWORD							state_in,						// state track in
													IterRecAudioInfo	it_rec_in
												);

	void SetTime_MediaMCI(	LONGLONG tm_glb_play_in,						// global time for playing(not local)
													IterRecAudioInfo	it_rec_in,
													DWORD							state_in = (LONG)T_DeviceState::PAUSE						// state track in
												);

	// attract audio track to time was got from Camera
	int Attract_AudioToVideo(void);

	// set thread for audio stream handler
	long	LaunchThreadHandlerDevice( void );

	//set mute for the all records of the track
	void SetVolume(UINT device_ID_in, BOOL flag_enable_in);

	static DWORD Audio_Data_Thread(LPVOID lpParam);

	// calculation total time duration before rec.file 
	void CalcPostTotalTime(void);

	// release memory for all lists audio
	void ReleaseAudioList(void);

	// release audio thread
	int ReleaseAudioThread(void);

	// parse all files in list ListAudioTrack by name (return number of files of audio)
	int ParseFolder(const char* search_folder);

	// divide filename by parts and exam their
	int ParseFileName(const char* filename);

	// get error from MCI function
	DWORD HandlerError(DWORD mci_err_in = 0);

	// get total time from one Audio track
	unsigned int Get_TotalTime_One_Tracks(const char *full_filename, Audio_Track &Audio_track_IO);

	// get order number of the recording by time from the first track list
	int	GetRecNmb(	LONGLONG tm_search_rec_in // time for searching of number recording [msec] (using the first track list)
								);

	// set need record(by numbers) for each track in input state (corrected all others)
	DWORD PlayAllTracksByTime(	int		nmb_track_rec_in,		// order number of track record to set state
															DWORD state_in,						// state track in
															LONGLONG tm_play_own_in			// base time for playing
														);

	void SetMute4Tracks(void);

	// printing error from MCI
	unsigned int GetErrorMCI(	const MCIERROR err_in,						// perhaps error MCI
														const char* str_proc_in = NULL		// name working process
													);

	// set patch for possibility using MCI in supplementary thread
	void InitPatch_MCI(void);

	// release patch MCI
	void ReleasePatch_MCI(void);
};

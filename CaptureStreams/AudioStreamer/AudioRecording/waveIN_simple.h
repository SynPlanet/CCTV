// #ifndef ___WAVEIN_SIMPLE_H_INCLUDED___
// #define ___WAVEIN_SIMPLE_H_INCLUDED___

#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <vector>

using namespace std;

#define WAIT_SIG 0
#define CONTINUE_SIG 1
#define EXIT_SIG 2

#include "sync_simple.h"

//---------------------------- CLASS -------------------------------------------------------------

// See CWaveINSimple::Start(IReceiver *pReceiver) below.
// Instances of any class extending "IReceiver" will be able to receive raw (PCM)
// sound from an instance of the CWaveINSimple and process sound via own
// implementation of the "ReceiveBuffer" method.

class IReceiver {
public:
	virtual void ReceiveBuffer(LPSTR lpData, DWORD dwBytesRecorded) = 0;
};

class CWaveINSimple;
class CMixer;
///////////////////////////////////////////////////////////////////////////
// Implementation of the Mixer's Line
class CMixerLine {
	friend class CMixer;
private:
	MIXERLINE m_MxLine;
	HMIXER m_MixerHandle;

	// Yep, constructor and destructor are declared private, since only
	// instances of CMixer can create CMixerLine objects (one CMixer can
	// have zero or more CMixerLine's, see CMixer::GetLines()).
	//
	// Constructor receives parameters exactly as they are passed by
	// the CMixer::InitLines(), which is called from CMixer::Open().
	// In fact, MixerHandle and pMxLine (pointer to MIXERLINE which
	// is filled in CMixer::InitLines()) is all we need to know about
	// Mixer's Line.
	CMixerLine(HMIXER MixerHandle, MIXERLINE *pMxLine);
	~CMixerLine() {};

public:

	// Raturns Line's name.
	const TCHAR *GetName() const { return this->m_MxLine.szName; };

	// Un-mute Mixer's Line. I would say this is a useless method, since
	// Mixer Lines for the WaveIN devices doesn't support Mute/Un-mute, at
	// least for the sounds cards I worked with. However, according to
	// http://support.microsoft.com/default.aspx?scid=kb%3Ben-us%3B159753,
	// there may be such sound cards. Anyway, method doesn't throw any
	// exceptions or return any errors if Mixer's Line doesn't support un-muting.
	void UnMute();

	// Set the Mixer's Line volume, nVolumeLevel should be within 0..100,
	// think of it like a percentage.
	void SetVolume(UINT nVolumeLevel);

	// Select the Mixer's Line. Regarding recording Lines, Method selects
	// Mixer's Line as a recording source (same as we would do this
	// manually via "sndvol32 /r").
	// Code for this method is almost an exact copy from the
	// http://support.microsoft.com/default.aspx?scid=kb%3Ben-us%3B159753.
	// I couldn't find a better one.
	void Select();
};
///////////////////////////////////////////////////////////////////////////
// Implementation of the Mixer. Via Mixer we have access to Mixer's Lines.
// Another particularity is that each Wave device (WaveIN in this case) has
// one Mixer.
class CMixer {
	friend class CWaveINSimple;

private:
	UINT m_nWaveDeviceID;

	// Handle to Mixer for WAVE Device
	HMIXER m_MixerHandle;
	QMutex m_qLocalMutex;

	// Here where Mixer keeps its Lines.
	vector<CMixerLine*> m_arrMixerLines;

	// Naturally, Mixer is a property of the Wave device (WaveIN in this case).
	// So, we can't just create (and destroy) a CMixer object from nothing (well,
	// we can but this design says so). See CWaveINSimple::OpenMixer() for more details.
	CMixer(UINT nWaveDeviceID);
	~CMixer();

	// This method initialize Mixer's Lines collection. It is
	// called within CMixer::Open().
	void InitLines();

public:
	// Opens the MIXER and call CMixer::InitLines().
	void Open();

	// Method closes the MIXER and clears the Lines collection.
	void Close();

	// Returns MIXER's WAVEIN lines,
	// call Open() before requesting lines (except cases when
	// Mixer is returned by CWaveINSimple::OpenMixer()).
	const vector<CMixerLine*>& GetLines() { return this->m_arrMixerLines; };

	// Returns a MIXER's WAVEIN line by line's name,
	// call Open() before requesting the line (except cases when
	// Mixer is returned by CWaveINSimple::OpenMixer()).
	CMixerLine& GetLine(const TCHAR *pLineName);

	// Returns a MIXER's WAVEIN line the first("Master Volume") [added torvas]
	CMixerLine& CMixer::GetMainLine(void);
};
///////////////////////////////////////////////////////////////////////////
// Via implementation of the CWaveINSimple we get access to the WaveIN
// devices, or, better saying, to the Wave input devices (capable of
// recording sound).
class CWaveINSimple {
private:
	// Static collection where all WaveIN devices, present in the system,
	// are saved. See CWaveINSimple::GetDevices()
	static vector<CWaveINSimple*> m_arrWaveINDevices;
	static QMutex m_qGlobalMutex;
	static volatile bool m_isDeviceListLoaded;

	// This is thread's routine procedure to which the Windows Low Level
	// WAVE API passes messages regarding digital audio recording (such
	// as MM_WIM_DATA, MM_WIM_OPEN, and MM_WIM_CLOSE). Mind that "waveInOpen"
	// (in CWaveINSimple::_Start()) is called with CALLBACK_THREAD.
	// Thread is created inside the CWaveINSimple::_Start() call.
	static DWORD WINAPI waveInProc(LPVOID arg);

	// WaveIN Device's ID. It is used as input parameter for the CMixer
	// constructor. With this ID we can access Mixer without actually opening
	// the WaveIN device.
	UINT m_nWaveDeviceID;

	// Structure to keep basic details about WaveIN device.
	WAVEINCAPS m_wic;

	// Handle to the WaveIN Device.
	HWAVEIN	m_WaveInHandle;

	// Structure to keep sound's quality settings. Also used when opening WaveIN
	// device, see CWaveINSimple::_Start().
	WAVEFORMATEX m_waveFormat;

	// Two WAVEHDR's are used for recording (ie, double-buffering).
	WAVEHDR	m_WaveHeader[2];

	// Pointer to a IReceiver object, passed via 
	// CWaveINSimple::Start(IReceiver *pReceiver), that will be responsible for 
	// further processing of the sound data.
	IReceiver *m_Receiver;
	CMixer m_Mixer;
	QMutex m_qLocalMutex;

	// These class' attributes are used for communication with thread's routine.
	volatile int m_SIG;
	volatile unsigned char m_BuffersDone;

	// Constructor and destructor are declared private (due design). So, there 
	// is no way to instantiate CWaveINSimple objects directly. To obtain a 
	// CWaveINSimple object, use CWaveINSimple::GetDevices() or CWaveINSimple::GetDevice() 
	// (see below). CWaveINSimple objects are destroyed via CWaveINSimple::CleanUp().
	CWaveINSimple(UINT nWaveDeviceID, WAVEINCAPS *pWIC);
	~CWaveINSimple();

	void Close(int iLevel);

	// This method starts recording sound from the WaveIN device. Passed object (derivate from 
	// IReceiver) will be responsible for further processing of the sound data.
	void _Start(IReceiver *pReceiver);

	// This method stops recording.
	void _Stop();

public:
	// This static method returns a collection of the WaveIN devices (capable of recording), 
	// present in the system.
	static const vector<CWaveINSimple*>& GetDevices();

	// This static method returns an instance of the WaveIN device (capable of recording), 
	// by device's name.
	static CWaveINSimple& GetDevice(const TCHAR *pDeviceName);

	// This static method performs general clean-up, once everything is finished and 
	// no more recording is needed.
	static void CleanUp();

	// Wrapper of the _Start() method, for the multithreading version.
	// This is the actual starter.
	void Start(IReceiver *pReceiver);

	// Wrapper of the _Stop() method, for the multithreading version
	// This is the actual stopper.
	void Stop();

	// Returns name of the Device
	const TCHAR *GetName() const { return this->m_wic.szPname; };

	// This method returns and opens Mixer associated with the Device.
	CMixer& OpenMixer();
};


//#endif
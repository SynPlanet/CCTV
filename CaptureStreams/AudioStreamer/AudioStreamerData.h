#pragma once

#include "AudioStreamReader.h"
#include "AudioStreamWriter.h"




//! type using SDK
enum Type_Thread
{
	NONE =			T_DeviceState::STOP,		// disable thread
	PLAYING =		T_DeviceState::PLAY,		// thread 
	RECORDING = T_DeviceState::WRITE		// Windows media API (MCI (Media Control Interface))
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// the common class for the transfer instance to another module for his managing
class CAudioStreamer {
public:
	CAudioStreamer(void):m_Type_Device_Use(Type_Thread::NONE)
	{
		m_pAStrmReader = NULL;
		m_pAStrmWriter = NULL;

		m_bEnable = false;
	}

	// TODO: add your methods here.
	virtual ~CAudioStreamer()
	{
		Release();
	};

	// initialization class instances
	void Init(void){
		m_pAStrmReader = new CAudioStreamReader();
		m_pAStrmWriter = new CAudioStreamWriter();

		if (m_pAStrmWriter){	// for correct using multi-thread
			m_pAStrmWriter->Init();
			m_bEnable = true;
		}

		if (m_pAStrmReader){	// for correct using multi-thread
			m_pAStrmReader->Init();
			m_bEnable = true;
		}
	};

	// release memory
	void Release(void){
		if (m_pAStrmReader){
			m_pAStrmReader->ReleaseAll();

			delete m_pAStrmReader;
			m_pAStrmReader = NULL;
		}

		if (m_pAStrmWriter){
			m_pAStrmWriter->ReleaseAll();

			delete m_pAStrmWriter;
			m_pAStrmWriter = NULL;
		}
	}

	bool IsEnable(void) const { return m_bEnable; }

	// set type for audio streaming thread 
	long	SetTypeDevice(	long type_device_in	//Type_Thread( PLAYING / RECORDING / NONE(0) )
											)
	{
		m_Type_Device_Use = type_device_in;
		return m_Type_Device_Use;
	}

	CAudioStreamReader	*m_pAStrmReader;
	CAudioStreamWriter	*m_pAStrmWriter;

private:
	long m_Type_Device_Use;	// type device using (PLAY or RECORDING or NONE(0) )

	bool m_bEnable;
};






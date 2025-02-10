// AudioStreamer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "AudioStreamer.h"
#include <mmsystem.h> 

#include "AudioStreamReader.h"

//http://www.codeproject.com/Articles/63094/Simple-MCI-Player

//http://www.codenet.ru/progr/cpp/process-threads-sync.php



#include "../CaptureIP/Log.h"
using namespace Logger;

//class Logger::CLog;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CAudioStreamer	g_AudioStreamer;

//#if defined(_DEBUG)
//Logger::CLog g_LoggerAudio((const char*)NAME_AUDIO_FILE_LOG);
//#endif
//////////////////////////////////////////////////////////////////////////

// This is an example of an exported function.
AUDIOSTREAMER_API CAudioStreamer* GetAudioStreamer(void* pProxyAny = NULL)
{
//	#if defined(_DEBUG)
		// set Logs streaming
//		g_LoggerAudio.SetDirectionLog(0);	// '0' - create log File
//	#endif

	g_AudioStreamer.Init();

/////	g_AudioStreamer.m_pAStrmReader->SetProxyVLC(pProxyVLC);

	return &g_AudioStreamer;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


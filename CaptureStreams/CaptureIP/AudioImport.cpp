// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "../AudioStreamer/AudioStreamerData.h"

//static CAudioStreamer	*g_AudioStreamer = NULL;
using namespace AudioDevice;
extern AudioDevice::AudioData g_AudioData;

//////////////////////////////////////////////////////////////////////////
typedef CAudioStreamer* (__cdecl *_GetAudioStreamer)( void* p_Instance_VLC );
_GetAudioStreamer pFunc_GetAudioStreamer = NULL;
//////////////////////////////////////////////////////////////////////////

// initialization extern Audio module
int Init_AudioImport(void* pProxyVLC)
{
	ZeroMemory((void*)&g_AudioData, sizeof(AudioDevice::AudioData));

	// Get a handle to the DLL module once
	if (!g_AudioData.hAudio_Module){
		g_AudioData.hAudio_Module = (HMODULE)LoadLibrary(TEXT("AudioStreamer.dll"));

		if (g_AudioData.hAudio_Module){
			pFunc_GetAudioStreamer = (_GetAudioStreamer)GetProcAddress(g_AudioData.hAudio_Module, "GetAudioStreamer");

			if (pFunc_GetAudioStreamer){
				g_AudioData.clAudioStreamer = (CAudioStreamer*)pFunc_GetAudioStreamer((void*)pProxyVLC);
			}
		}
	}

	if (g_AudioData.clAudioStreamer){
		//Sleep(500);

		if ( ((CAudioStreamer*)g_AudioData.clAudioStreamer)->IsEnable() ){
				
			g_AudioData.bEnable = TRUE;
			InterlockedExchange(&g_AudioData.lTypeFunc, (LONG)T_DeviceState::PLAY);

			printf("AudioStreamer.dll was initialized!\n");
		}else{
			printf("AudioStreamer.dll was not initialized!\n");
		}

		return 0;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// release extern Audio module
int Release_AudioImport(void)
{
	//printf("Delete->AudioStreamer.dll!\n");

	BOOL res = FALSE;
	// release module
	if (g_AudioData.hAudio_Module){

		// release all class members
		((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->ReleaseAll();

		((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->ReleaseAll();
		// free module memory
		res = FreeLibrary(g_AudioData.hAudio_Module);

		g_AudioData.hAudio_Module = NULL;
		printf("AudioStreamer.dll was uninitialized!\n");

		return 0;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "mp3_simple.h"

// Pointers to LAME API functions
BEINITSTREAM		beInitStream	=NULL;
BEENCODECHUNK		beEncodeChunk	=NULL;
BEDEINITSTREAM		beDeinitStream	=NULL;
BECLOSESTREAM		beCloseStream	=NULL;
BEVERSION		beVersion	=NULL;
BEWRITEVBRHEADER	beWriteVBRHeader=NULL;
BEWRITEINFOTAG		beWriteInfoTag	=NULL;
BEFLUSHNOGAP		beFlushNoGap	=NULL;



//---------------------------- IMPLEMENTATION ----------------------------------------------------
bool CMP3Simple::m_isLibLoaded = false;
QMutex CMP3Simple::m_qMutex;

BE_ERR CMP3Simple::Encode(PSHORT pSamples, DWORD nSamples, PBYTE pOutput, PDWORD pdwOutput) {
	return beEncodeChunk(this->hbeStream, nSamples, pSamples, pOutput, pdwOutput);
}

BE_ERR CMP3Simple::Flush(PBYTE pOutput, PDWORD pdwOutput) {
	return beFlushNoGap(this->hbeStream, pOutput, pdwOutput);
}

CMP3Simple::CMP3Simple(unsigned int nBitRate, unsigned int nInputSampleRate,
						   unsigned int nOutSampleRate) {
	BE_ERR		err = 0;

	try{

		CMP3Simple::LoadLIBS();

		this->dwMP3Buffer = 0;
		this->dwPCMBuffer = 0;
		this->hbeStream = 0;
		memset(&this->beConfig, 0, sizeof(BE_CONFIG));

		this->beConfig.dwConfig = BE_CONFIG_LAME;
		this->beConfig.format.LHV1.dwStructVersion = 1;
		this->beConfig.format.LHV1.dwStructSize = sizeof(BE_CONFIG);

		// OUTPUT IN STREO
		this->beConfig.format.LHV1.nMode = BE_MP3_MODE_STEREO;	//Save in the type: "BE_MP3_MODE_STEREO" !!! left and right channels

		// QUALITY PRESET SETTING, CBR = Constant Bit Rate
		this->beConfig.format.LHV1.nPreset = LQP_CBR;

		// USE DEFAULT PSYCHOACOUSTIC MODEL
		this->beConfig.format.LHV1.dwPsyModel = 0;

		// NO EMPHASIS TURNED ON
		this->beConfig.format.LHV1.dwEmphasis = 0;

		// SET ORIGINAL FLAG
		this->beConfig.format.LHV1.bOriginal = TRUE;

		// Write INFO tag
		this->beConfig.format.LHV1.bWriteVBRHeader = FALSE;

		// No Bit reservoir
		this->beConfig.format.LHV1.bNoRes = TRUE;		

		this->beConfig.format.LHV1.bPrivate = TRUE;
		this->beConfig.format.LHV1.bCopyright = TRUE;

		// MINIMUM BIT RATE
		this->beConfig.format.LHV1.dwBitrate = nBitRate;

		// MPEG VERSION (I or II)
		this->beConfig.format.LHV1.dwMpegVersion = (nBitRate > 32)?MPEG1:MPEG2;

		// INPUT FREQUENCY
		this->beConfig.format.LHV1.dwSampleRate	= nInputSampleRate;

		// OUTPUT FREQUENCY, IF == 0 THEN DON'T RESAMPLE
		this->beConfig.format.LHV1.dwReSampleRate = nOutSampleRate;

  	err = beInitStream(&this->beConfig, &this->dwPCMBuffer, &this->dwMP3Buffer, &this->hbeStream);
		if(err != BE_ERR_SUCCESSFUL) throw "ERRORR in beInitStream.";

	}
	catch(const char *ex_msg){
		printf("CMixer::Open(): %s\n", ex_msg);
	}
}

CMP3Simple::~CMP3Simple() {

	beCloseStream(this->hbeStream);
}

void CMP3Simple::LoadLIBS() {
	HINSTANCE  hDLLlame = NULL;

	try{
		m_qMutex.Lock();
		//if (!m_isLibLoaded) {
			// LAME API wasn't loaded yet, so load it

			hDLLlame = ::LoadLibraryA("lame_enc.dll"/*"c:\\OpenCV\\IPCAMERA\\FinalResult\\lame_enc.dll"*/);

			if( hDLLlame == NULL ) {
				m_qMutex.Unlock();
				throw "Error loading lame_enc.DLL";
			}

			beInitStream	= (BEINITSTREAM) GetProcAddress(hDLLlame, TEXT_BEINITSTREAM);
			beEncodeChunk	= (BEENCODECHUNK) GetProcAddress(hDLLlame, TEXT_BEENCODECHUNK);
			beDeinitStream	= (BEDEINITSTREAM) GetProcAddress(hDLLlame, TEXT_BEDEINITSTREAM);
			beCloseStream	= (BECLOSESTREAM) GetProcAddress(hDLLlame, TEXT_BECLOSESTREAM);
			beVersion      	= (BEVERSION) GetProcAddress(hDLLlame, TEXT_BEVERSION);
			beWriteVBRHeader= (BEWRITEVBRHEADER) GetProcAddress(hDLLlame, TEXT_BEWRITEVBRHEADER);
			beWriteInfoTag  = (BEWRITEINFOTAG) GetProcAddress(hDLLlame, TEXT_BEWRITEINFOTAG);

			beFlushNoGap = (BEFLUSHNOGAP) GetProcAddress(hDLLlame, TEXT_BEFLUSHNOGAP);

			if(!beInitStream || !beEncodeChunk || !beDeinitStream ||
				!beCloseStream || !beVersion || !beWriteVBRHeader || !beFlushNoGap) {

				::FreeLibrary(hDLLlame);
				m_qMutex.Unlock();
				throw "Unable to get LAME interfaces";
			}

			m_isLibLoaded = true;
		//}

		m_qMutex.Unlock();

	}
	catch(const char *ex_msg){
		printf("CMixer::Open(): %s\n", ex_msg);
	}
}

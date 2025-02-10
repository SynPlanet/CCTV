#ifndef ___MP3_SIMPLE_H_INCLUDED___
#define ___MP3_SIMPLE_H_INCLUDED___

#include <windows.h>
#include "BladeMP3EncDLL.h"
#include "sync_simple.h"


//---------------------------- CLASS -------------------------------------------------------------

// Class that implements basic MP3 encoding, by wrapping LAME API.
// This class can be extended to add extra functionality and customization features.
class CMP3Simple {
private:
	static QMutex m_qMutex;
	static bool m_isLibLoaded;

	BE_CONFIG	beConfig;
	HBE_STREAM	hbeStream;
	DWORD		dwMP3Buffer;
	DWORD		dwPCMBuffer;

public:
	// This static method performs LAME API initialization
	static void LoadLIBS();

	// Constructer of the class accepts only three parameters.
	// Feel free to add more constructors with different parameters, if a better
	// customization is necessary.
	//
	// nBitRate - says at what bitrate to encode the raw (PCM) sound
	// (e.g. 16, 32, 40, 48, ... 64, ... 96, ... 128, etc), see official LAME documentation
	// for accepted values.
	//
	// nInputSampleRate - expected input frequency of the raw (PCM) sound
	// (e.g. 44100, 32000, 22050, etc), see official LAME documentation
	// for accepted values.
	//
	// nOutSampleRate - requested frequency for the encoded/output (MP3) sound.
	// If equal with zero, then sound is not
	// re-sampled (nOutSampleRate = nInputSampleRate).
	CMP3Simple(unsigned int nBitRate, unsigned int nInputSampleRate = 44100,
		unsigned int nOutSampleRate = 0);


	~CMP3Simple();

	// This method performs encoding.
	//
	// pSamples - pointer to the buffer containing raw (PCM) sound to be encoded.
	// Mind that buffer must be an array of SHORT (16 bits PCM stereo sound, for
	// mono 8 bits PCM sound better to double every byte to obtain 16 bits).
	//
	// nSamples - number of elements in "pSamples" (SHORT). Not to be confused with
	// buffer size which represents (usually) volume in bytes. See also
	// "MaxInBufferSize" method.
	//
	// pOutput - pointer to the buffer that will receive encoded (MP3) sound, here
	// we have bytes already. LAME says that if pOutput is not cleaned before call,
	// data in pOutput will be mixed with incoming data from pSamples.
	//
	// pdwOutput - pointer to a variable that will receive the number of bytes written
	// to "pOutput". See also "MaxOutBufferSize" method.
	BE_ERR Encode(PSHORT pSamples, DWORD nSamples, PBYTE pOutput, PDWORD pdwOutput);

	// using for flush data to destination (add torvas[28012016])
	BE_ERR Flush(PBYTE pOutput, PDWORD pdwOutput);

	// Returns maximum suggested number of elements (SHORT) to send to "Encode" method.
	// e.g. PSHORT pSamples = (PSHORT) malloc(sizeof(SHORT) * MaxInBufferSize())
	// or PSHORT pSamples = new SHORT[MaxInBufferSize()]
	DWORD MaxInBufferSize() const { return this->dwPCMBuffer; }

	// Returns minimum suggested buffer size (in bytes) for the buffer that will
	// receive encoded (MP3) sound, see "Encode" method for more details.
	DWORD MinOutBufferSize() const { return this->dwMP3Buffer; }

	// Returns requested bitrate for the MP3 sound.
	DWORD BitRate() const { return this->beConfig.format.LHV1.dwBitrate; }

	// Returns expected input frequency of the raw (PCM) sound
	DWORD InSampleRate() const { return this->beConfig.format.LHV1.dwSampleRate; }

	// Returns requested frequency for the encoded/output (MP3) sound.
	DWORD OutSampleRate() const {
		if (this->beConfig.format.LHV1.dwReSampleRate == 0) {
			// If equal with zero, then sound is not
			// re-sampled (nOutSampleRate = nInputSampleRate).
			return this->beConfig.format.LHV1.dwSampleRate;
		}

		return this->beConfig.format.LHV1.dwReSampleRate;
	}
};


#endif
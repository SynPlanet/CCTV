#pragma once

namespace CommonData
{
	//! streaming type
	enum Type_Stream
	{
		FILE_STREAM = 0,	// file(static video(AVI or another) and audio (MP3) ) streaming 
		IP_STREAM	 = 1		// IP streaming (RTSP and other)
	};

	//! type using SDK
	enum Type_SDK_API
	{
		VLC = 0,							// VLC (open source)
		OPEN_CV = 1,					// OpenCV SDK	(open source)
		MCI = 2								// Windows media API (MCI (Media Control Interface))
	};

	//! devices state (for video and audio streaming)
	struct  T_DeviceState
	{
		enum {
			STOP = 0,
			PLAY = 1,
			PAUSE = 2,
			WRITE = 3,	// === RECORDING
			MOUNT = 4,	// mounting(installation) folder for recording from device
			SEEK	=	8,	// the seek frame needed, drawing this frame and set PAUSE
			EXIT	=	9		// flag for application exit
		};
	};

	#define MAX_SIGN_NAME_FOLDER	32
};

//! structures for the work of audio devices on
namespace AudioDevice
{
	// number of devices for using
	#define MAX_DEVICES	8

	struct AudioData
	{
		VOID*			clAudioStreamer;	//see pointer to class CAudioStreamer
		HMODULE			hAudio_Module;
		BOOL			bEnable;					// flag enabling Audio
		volatile LONG	lTypeFunc;				// type functionality of Audio Device (PLAY/RECORD) (see T_DeviceState::(PLAY/WRITE))
	};

	//! the type of audition
	struct  T_TypeChannel
	{
		enum {
			STEREO					= 0x1,
			LEFT_CNL				= 0x2,
			RIGHT_CNL				= 0x4,
			LEFT_RIGHT_CNL	= 0x8
		};
	};

	#define NAME_AUDIO_REC_HEADER "AudioDevice_"
	#define NAME_AUDIO_REC_SUFFIX "__Rec_"
	#define NAME_AUDIO_REC_TAIL ".mp3"
	#define NAME_AUDIO_REC_FIND "***.mp3"
	#define NAME_AUDIO_CHANNEL_ENABLE "__LR"		// using one or two channel (left/ right or both channel)
	#define LENGTH_PATH_STREAM	_MAX_PATH

	//BitRate-says at what bitrate to encode the raw (PCM) sound(e.g. 16, 32, 40, 48, ... 64, ... 96, ... 128, etc), see official LAME documentation for accepted values
	#define	BIT_RATE			128

	//BitRate-says at what bitrate to encode the raw (PCM) sound(e.g. 16, 32, 40, 48, ... 64, ... 96, ... 128, etc), see official LAME documentation for accepted values
	#define	BIT_RATE_TO_BYTE			(double)(BIT_RATE/8.0)

	#define	SAMPLE_SIZE		4

	//SampleRate- frequency(input or requested) of the raw (PCM) sound(e.g. 44100, 32000, 22050, etc), see official LAME documentation for accepted values
	#define	SAMPLE_RATE		44100

	// information for '*.ini" file
	#define FILENAME_INI_DEVICE	"./AV.ini"
	#define NAME_AUDIO_DEVICES	"AudioDevices"
	#define	PREFIX_AUDIO_DEVICE	"Device_"

	#define	NAME_MAUDIO_DEVICE	"M-Audio Eight"
	#define	PREFIX_MAUDIO_PORT	"Port_"

	#define	NAME_ASIO_DRV_DEVICE	"AudioDriverASIO"
	#define	PREFIX_ASIO_DRV_MODEL	"Model_"
	#define	PREFIX_ASIO_CHANNEL_OFFSET	"ChannelsOffset"

	#define	NAME_AUDIO_VIDEO_COMMON					"AV_Common"
	#define	PREFIX_TIME_DELAY_AUDIOPLAYER_MS	"TimeDelay_AudioPlayer_msec"
	
	

	#define	COUNT_AUDIO_DEVICE	99

	const char * const known_drivers[] =
	{
		(char *)"ASIO4ALL v2",
		(char *)"M-Audio M-Track Eight ASIO",
		(char *)"M-Audio ProFire ASIO",
		(char *)"M-Audio M-Track Eight ASIO (64)",
		(char *)"ASIO Echo FireWire"
	};
};
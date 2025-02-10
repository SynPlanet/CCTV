#pragma once
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv/highgui.h>
#include "opencv2/opencv.hpp"
using namespace cv;

#include "Misc.h"

#include "CommonData.h"
using namespace CommonData;
//////////////////////////////////////////////////////////////////////////
// typedef void (__stdcall /*__cdecl __stdcall*/ *Set_PlayerState)( UINT nmb_Cmr, UINT state_out );		//nmb_Cmr: [0...N]; state_out:	T_DeviceState::enum
// 
// typedef void (__stdcall *Joy_IsConnect)( unsigned char flag );											// flag TRUE/FALSE
// typedef void (__stdcall *Joy_Control)( UINT nmb_Cmr, UINT state_show );		// nmb_Cmr: [0...N]; state_show:	bitwise fields T_ControlPTZ::enum
// 
// typedef void (__stdcall *PTZ_GetValue)(INT pan_in, INT tilt_in, INT zoom_in);	

//////////////////////////////////////////////////////////////////////////

namespace JoyDevice
{
	#define MAX_DEVIATION_AXIS		1000	// maximum deviation for each axis [units]
	#define FORSAGE_DEVIATION			975		// forsage speed deviation [units]
	#define HIGH_SPEED_DEVIATION	800		// high speed deviation [units]
	#define DEAD_ZONE_DEVIATION		350		// deadzone deviation

	//! control states (for joystick handler)
	struct  T_ControlPTZ
	{
		enum {
			UP					= 1<<0,	
			DOWN				= 1<<1,	
			LEFT				= 1<<2,
			RIGHT				= 1<<3,
			ZOOM_IN			= 1<<4,
			ZOOM_OUT		= 1<<5,
			FORSAGE			= 1<<6,				// set the best speed
			SPEED_HIGH	= 1<<7,				// set maximum speed
			SPEED_LOW		= 1<<8,				// set minimum speed
			FOCUS_NEAR	= 1<<9,
			FOCUS_FAR		= 1<<10,
			FOCUS_STOP	= 1<<11,
			BASE_POS		= 1<<12			// the flag defining PTZ moving to base coordinates
		};
	};

	//! Type Joystick Device
	struct  T_JoyDevice
	{
		enum {
			REAL		= 0,	
			VIRTUAL	= 1	
		};
	};

	//! Command 1 Pelco-D
	struct  T_Command1_PelcoD
	{
		enum {
			FOCUS_NEAR		= 1<<0,			// bit 0
			IRIS_OPEN			= 1<<1,	
			IRIS_CLOSE		= 1<<2,
			CAM_ON_OFF		= 1<<3,
			SCAN_AUTO_MAN	= 1<<4,
			RESERVED_1		= 1<<5,
			RESERVED_2		= 1<<6,			
			SENSE					= 1<<7			// bit 7
		};
	};

	//! Command 2 Pelco-D
	struct  T_Command2_PelcoD
	{
		enum {
			FIXED_0		= 1<<0,			// bit 0
			PAN_RIGHT	= 1<<1,	
			PAN_LEFT	= 1<<2,
			TILT_UP		= 1<<3,
			TILT_DOWN	= 1<<4,
			ZOOM_TELE	= 1<<5,
			ZOOM_WIDE	= 1<<6,			
			FOCUS_FAR	= 1<<7			// bit 7
		};
	};
};

namespace IP_Camera
{
	#define MAX_VIDEO_WIDTH	1920
	#define MAX_VIDEO_HEIGHT	1080
	#define MAX_VIDEO_4_BYTE_PXL		32/8
	#define MAX_VIDEO_3_BYTE_PXL		24/8

	#define IMAGE_DEFAULT "./Backdrop.bmp"

	#define NAME_VIDEO_REC_HEADER "Cam_"
	#define NAME_VIDEO_REC_SUFFIX "___Rec_"
	#define NAME_VIDEO_REC_TAIL ".avi"
	#define NAME_VIDEO_REC_FIND "***.avi"
	#define LENGTH_PATH_STREAM	_MAX_PATH

	#define SAVE_VIDEO_MINUTES 45

	#define MAX_COUNT_EVENTS_GRAB_THREADS 32

	#define TIME_CONTROL_HDD_MSEC (5*60*1000)
	#define SIZE_HDD_STOP_GRAB_VIDEO_MB		300

// 	//! net address info 
// 	struct  T_NetAddress
// 	{
// 		unsigned char IP[4];
// 		unsigned int Port;
// 		char Login[_MAX_PATH/4];
// 		char Password[_MAX_PATH/4];
// 	};
	
	//! the type packet for control video writing from outside( the header byte in packet )
	struct  T_TypePacket
	{
		enum {
			STRING = 1,
			COMMAND = 2
		};
	};
	
	//! data from joystick(use for send protocol)
	struct  T_DataPTZ
	{
		long speed_moving;
		//long speed_LeftRight;
		bool cam_OnOff;
		volatile LONG state_controle;		// joystick state (see T_ControlPTZ::enum)
		volatile LONG cur_cam;						// camera  was chosen
		unsigned int dev_type;					// device type ( enum T_JoyDevice )
	};

	struct T_VideoParams
	{
		float FPS;							// Frame rate 
		int width_frame;			// Width of the frames in the video stream
		int height_frame;			// Height of the frames in the video stream
		//int codec_code;				// 4-character code of codec
		char codec_code[10];	// 4-character code of codec
		int format_Mat;				//Format of the Mat objects returned by retrieve() .
		int mode_capture;				//Backend-specific value indicating the current capture mode
		double brightness;		//Brightness of the image
		double contrast;			//Contrast of the image
		double saturation;		//Saturation of the image
		double hue;									//Hue of the image
		double gain;								//Gain of the image
		double exposure;			//Exposure
		double sharpness;			//Sharpness
	};

	//////////////////////////////////////////////////////////////////////////
	struct GraberData
	{
		Mat*		image;
		HANDLE	mutex;
		uchar*	pixels;
	};
};
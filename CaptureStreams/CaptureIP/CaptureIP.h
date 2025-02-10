// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CAPTUREIP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CAPTUREIP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CAPTUREIP_EXPORTS
#define CAPTUREIP_API __declspec(dllexport)
#else
#define CAPTUREIP_API __declspec(dllimport)
#endif

//////////////////////////////////////////////////////////////////////////
#include "CommonIP.h"
//////////////////////////////////////////////////////////////////////////
//! initialization general modules
/*!
\param[in] PTZ_enable [unsigned int] flag enable PTZ control(launch COM object)

\return return value( '0' - initialization was correct; '1' - initialization have initialized already; '-1' - error of initialization )
*/
CAPTUREIP_API	int Initialize_ALL(const UINT PTZ_enable); 

//! release modules was initialized
/*!
\return return value( '0' - initialization was correct; '1' - initialization have initialized already; '-1' - error of initialization )
*/
CAPTUREIP_API int Release_ALL(void);

//! create IP stream for recording video
/*!
\param[in] nmb_Cmr_in [unsigned int]	number of current camera
\param[in] net_addr_in [unsigned int]	number of current camera
\param[out] path_stream_out [char *]	IP video stream path (was opened) ( copy path to buffer [_MAX_PATH] )
\return return value( '0' - the joining to IP camera was correct; '1' - this one camera was already initialized; '-1' - error of the streaming path(CameraIP is not opened) )
*/
CAPTUREIP_API int CreateStreamIP(	UINT					cmr_ID_in,				// camera number
																	T_NetAddress	*net_addr_in,			// net address
																	char*					path_stream_out		// video stream path (was opened)
																);

//! create file stream(for play video)
/*!
\param[in] nmb_Cmr_in [unsigned int]	number of current camera
\param[in] nmb_file_in [unsigned int] number file for the playing
\param[out] path_stream_out [char *]	file video stream path (was opened) ( copy path to buffer [_MAX_PATH] )
\param[out] total_time_out [unsigned int]	total time video files for playing
\return return video files was found(from camera) inside folder ("N" number files was found;  "-1" - error)
*/

CAPTUREIP_API int CreateStreamFile(	UINT					cmr_ID_in,				// camera number
																		UINT					nmb_file_in,			// number file for the playing
																		char*					path_stream_out		// video stream path (was opened)
																	);

//! getting screenshot of current camera( using BMP format )
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera
\param[out] size_Bmp_W [unsigned int]	bitmap width
\param[out] size_Bmp_H [unsigned int]	bitmap height
\param[out] bit_pxl_Bmp [unsigned int]	bitmap per pixel
\param[out] buf_Bmp [void *]	bitmap buffer output
\return return value( '0' - image buffer was get correctly; '-1' - IP camera was not found; '-2' - error )
*/
CAPTUREIP_API const int GetBMP_NmbCamera(	const UINT nmb_Cmr,				// number of current camera
																					UINT &size_Bmp_W,			// bitmap width
																					UINT &size_Bmp_H,			// bitmap height
																					UINT &bit_pxl_Bmp,		// bitmap per pixel
																					VOID **buf_Bmp				// bitmap buffer 
																				);

//! update state video stream for each device
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera ( "-1" - it will be control for all cameras )
\param[in] new_state [unsigned int] new state for further control audio-video streaming (see: VideoState{} )
\return return value( '0' - parameters was set correctly; '-1' - IP camera was not found; '-2' - error )
*/
CAPTUREIP_API const int UpdataStateStream(	const UINT nmb_Cmr,		// number of current camera [-1 ... N]
																						const UINT new_state	// new state for further control audio-video streaming(see T_DeviceState)
																					);

//! set parameters for streaming camera
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera
\param[in] dim_img_W [unsigned int]	 width image CameraIP(if exist)
\param[in] dim_img_H [unsigned int]	height image CameraIP(if exist)
\return return value( '0' - parameters was changed correctly; '-1' - IP camera was not found; '-2' - error )
*/
// CAPTUREIP_API int SetParamsCamera(	const UINT nmb_Cmr,		// number of current camera
// 																		const UINT dim_img_W,		// width image CameraIP(if exist)
// 																		const UINT dim_img_H		// height image CameraIP(if exist)
// 																	);

//! set new parameter bit per pixel for getting image
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera
\param[in] bit_pxl_img [unsigned int]	bit per pixel for getting image
\return return value( '0' - set new parameter a bit/pxl was changed correctly; '-1' - IP camera was not found; '-2' - unsupported parameter;)
*/
CAPTUREIP_API int SetBitImgCamera(	const UINT nmb_Cmr,			// number of current camera
																		const UINT bit_pxl_img	// bit per pixel for getting image
																	);

//! release stream IP camera(include allocated memory) by number
/*!
\param[in] nmb_Cmr_del [unsigned int]	number of camera for delete
\return return value( '0' - cameara IP was deleted correctly; '-1' - IP camera was not found; '-2' - error )
*/
CAPTUREIP_API int ReleaseCameraIP_Nmb(const UINT nmb_Cmr_del	// number camera IP for delete
																		);

//! get parameters from camera
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera
\param[out] fps_dev [unsigned int] the number frames per second getting from device(CameraIP)
\param[out] dim_img_W [unsigned int] width image from device(CameraIP)
\param[out] dim_img_H [unsigned int] height image from device(CameraIP)
\return return value( '0' - parameters was get correctly; '-1' - IP camera was not found; '-2' - error )
*/
CAPTUREIP_API const int GetParamsCamera(	const UINT nmb_Cmr,		// number of current camera
																					UINT &fps_dev,				// the number frames per second getting from device(CameraIP)
																					UINT &img_W_dev,	// width image from device(CameraIP)
																					UINT &img_H_dev		// height image from device(CameraIP)
																				);

//! Set Flag Camera IP support rotation (360)
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera
\param[in] flag_360_in [unsigned char]	definition flag of control using CameraIP 360
\return return value ("TRUE" - parameters was set correctly; 'FALSE' - IP camera was not found or any error;)
*/

CAPTUREIP_API BOOL SetCamera_360(	const UINT nmb_Cmr,				// number of current camera
																	const UCHAR flag_360_in		// definition flag of control using CameraIP 360
																);


//! Get Flag Camera IP support rotation (360)
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera
\return return value ("TRUE" - parameters was set correctly; 'FALSE' - IP camera was not found or any error;)
*/

CAPTUREIP_API BOOL GetCamera_360(	const UINT nmb_Cmr				// number of current camera
																);

//! set joystick states for camera
/*!
\param[in] nmb_Cmr [unsigned int]	number of current camera
\param[in] state_in [unsigned int] camera states for the move (T_ControlPTZ::enum)
\return return
*/
CAPTUREIP_API BOOL Joystick_Control(	const UINT nmb_Cmr,		// number of current camera
																			const UINT state_in		// camera states for the move (T_ControlPTZ::enum)
																		);

// bind callback function for sending out confirmation of the recording state 
CAPTUREIP_API void CallBackFunc_PlayerState(void* ptr_CBFunc_RecState);

// bind callback function for sending out information about joystick connection and control ones
CAPTUREIP_API void CallBackFunc_JoyControl(void* ptr_CBFunc_JoyIsConnect, void* ptr_CBFunc_JoyControl);

// bind callback function for getting out count time from player
CAPTUREIP_API void CallBackFunc_GetTimePlayer(void* ptr_CBFunc_GetTimePlayer);

// bind callback function for getting out folder for the movie playing
CAPTUREIP_API void CallBackFunc_SetMovieFolder(void* ptr_CBFunc_SetMovieFolder);

//! set path recording video
/*!
\param[in] path_in [const CHAR*]	path recording video
\return return value ("TRUE" - parameters was set correctly; "FALSE" - error)
*/
// CAPTUREIP_API BOOL SetPathRec(	const CHAR *path_in		// path recording video
// 															);

//! set folder path to play video files in it
/*!
\param[in] path_in [const CHAR*] folder path recording video
\return return video files was found (from camera)inside folder ("N" number files was found;  "-1" - error)
*/
// CAPTUREIP_API INT SetPathPlay(	const CHAR *path_in		// path playing video files
// 															);

//! set paths for Video playing and recording (except folder(get from INET) and except filename)
/*!
\param[in] path_record_in [const CHAR*]	video recording path				(length = [_MAX_PATH])
\param[in] path_play_in [const CHAR*]	playing path for video files	(length = [_MAX_PATH])
\return return video files was found (from camera)inside folder ("N" number files was found;  "-1" - error)
*/
CAPTUREIP_API INT SetPathsVideo(	const CHAR *path_record_in,	// video recording path		[_MAX_PATH]
																	const CHAR *path_play_in		// playing path for video files	[_MAX_PATH]
																);

//! set paths for Audio playing and recording (except folder(get from INET) and except filename)
/*!
\param[in] path_record_in [const CHAR*]	audio recording path				(length = [_MAX_PATH])
\param[in] path_play_in [const CHAR*]	playing path for audio files	(length = [_MAX_PATH])
\return return audio files was found inside folder ("N" number files was found;  "-1" - error)
*/
CAPTUREIP_API INT SetPathsAudio(	const CHAR *path_record_in,	// audio recording path		[_MAX_PATH]
																	const CHAR *path_play_in		// playing path for audio files	[_MAX_PATH]
																);

//! set the listening port (TCP protocol) for the control writing IP video
/*!
\param[in] port_in [const UINT]	port for the listening
\return return value ("TRUE" - parameters was set correctly; "FALSE" - error)
*/
CAPTUREIP_API BOOL SetPort_VideoControl(	const UINT port_in		//  port for the listening
																				);

//! set current camera(for right control (joystick/mouse))
/*!
\param[in] nmb_Cmr_in [const UINT]	the current number of camera
\return return value ("TRUE" - camera was found; "FALSE" - error)
*/
CAPTUREIP_API BOOL SetCurrentCamera(	const UINT nmb_cur_Cmr_in		// the current number of camera [0...N]
																		);

//! set maximum time for recording all movies
/*!
\param[in] mimutes_in [unsigned int]	time [min] for recording single movie
\return return value ("TRUE" - parameters was set correctly; 'FALSE' - any error)
*/

CAPTUREIP_API BOOL SetTimeRecMinutes(	const UINT mimutes_in				// time [min] for recording single movie
																		);

//! set beginning time to play for file movie (for all objects camera)
/*!
\param[in] time_play [unsigned int]	set time for playing video [sec]
\return return value ("TRUE" - playing was set correctly; 'FALSE' - any error)
*/
CAPTUREIP_API BOOL SetTimePlay(	const UINT time_play_msec	// set time for playing video	[msec]
															);

//! get playing time for file movie outside(for all objects camera)
/*!
\param[out] total_time_out [unsigned int]	get outside total time for playing video [sec]
\return return value ("TRUE" - parameters was get correctly; 'FALSE' - any error)
*/
CAPTUREIP_API BOOL GetTotalTimePlay(	UINT &total_time_out 	// total time video files for playing	[msec]
																		);

 //! set manual control for player (use Slider or TCP client)
 /*!
 \param[in] flag [bool]	('1' - manual control, '0' - control from client(TCP/IP) )
 \return return value("TRUE" - parameters was set correctly; "FALSE" - any error;)
 */
CAPTUREIP_API const INT SetPlayerManualControl(	const BOOL flag	// state flag
																							);

 //! get state video stream for each device
 /*!
	\param[in] nmb_Cmr [unsigned int]	number of current camera
	\return return of state value(see: T_DeviceState)
 */
 //! get state video stream for each device
 CAPTUREIP_API	const LONG GetStateCamera(	const UINT nmb_Cmr		// number of current camera[0 ... N]
																					);

//! set Audio device using(for the playing and recording)
 /*!
\param[in] device_nmb_in [int]	device number [0-7]; "-1"- all device was chosen
\param[in] flag_enable_in [bool]	flag enabling device	(TRUE / FALSE)
\return return number device was examining correctly ([0-7] or "-1" parameters was set correctly; "-2" - any error )
*/
CAPTUREIP_API INT SetAudioDevice(	INT		device_nmb_in,	// device number [0-7]; "-1"- all device was chosen
																	BOOL	flag_enable_in	// flag enabling device	(TRUE / FALSE)
																);

//! set type interface (playing or recording)
 /*!
\param[in] type_intarface [UINT]	(T_DeviceState:: (PLAY/WRITE)
\return return type of interface was gotten( "-1" - any error )
*/
CAPTUREIP_API INT SetTypeInterface(	UINT	type_intarface	// type interface (T_DeviceState::PLAY === 1) || (T_DeviceState::WRITE === 3)
																	);

 BOOL SetFolderNet(	const CHAR *name_fld_in	// name folder [32]
									);
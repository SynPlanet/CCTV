#pragma once

//////////////////////////////////////////////////////////////////////////
typedef void (__stdcall /*__cdecl __stdcall*/ *Set_PlayerState)( UINT nmb_Cmr, UINT state_out );		//nmb_Cmr: [0...N]; state_out:	T_DeviceState::enum

typedef void (__stdcall *Joy_IsConnect)( unsigned char flag );											// flag TRUE/FALSE
typedef void (__stdcall *Joy_Control)( UINT nmb_Cmr, UINT state_show );		// nmb_Cmr: [0...N]; state_show:	bitwise fields T_ControlPTZ::enum

typedef void (__stdcall *PTZ_GetValue)(INT pan_in, INT tilt_in, INT zoom_in);

typedef void (__stdcall *GetTimeMovie)( UINT cnt_time_msec, UINT max_lng_track_msec, UINT state_real );
typedef void (__stdcall *SetMovieFolder)( CHAR *folder_movie_play, UINT cnt_chars );
//////////////////////////////////////////////////////////////////////////

inline bool Path_File_Exists(const char *path_filename);

//////////////////////////////////////////////////////////////////////////


//! net address info 
struct  T_NetAddress
{
	char Header[_MAX_PATH/4];		// include protocol
	char Tail[_MAX_PATH/4];			// 
	char Login[_MAX_PATH/4];
	char Password[_MAX_PATH/4];
	unsigned char IP[4];
	unsigned int Port;
};

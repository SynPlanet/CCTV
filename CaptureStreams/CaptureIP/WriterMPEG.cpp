#include "StdAfx.h"
#include "WriterMPEG.h"

#include "Synh_Objs.h"

#include "CommonIP.h"
using namespace IP_Camera;

//#include "c:\OpenCV\opencv\sources\modules\highgui\src\cap_ffmpeg_impl.hpp"

static CSynh_CritSect g_Synh_CS_CWriterMPEG;



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef void* (__cdecl *CvCreateFileCapture_Plugin)( const char* filename );
typedef void* (__cdecl *CvCreateCameraCapture_Plugin)( int index );
typedef int (__cdecl *CvGrabFrame_Plugin)( void* capture_handle );
typedef int (__cdecl *CvRetrieveFrame_Plugin)( void* capture_handle, unsigned char** data, int* step,	int* width, int* height, int* cn );
typedef int (__cdecl *CvSetCaptureProperty_Plugin)( void* capture_handle, int prop_id, double value );
typedef double (__cdecl *CvGetCaptureProperty_Plugin)( void* capture_handle, int prop_id );
typedef void (__cdecl *CvReleaseCapture_Plugin)( void** capture_handle );
typedef void* (__cdecl *CvCreateVideoWriter_Plugin)( const char* filename, int fourcc, double fps, int width, int height, int iscolor );
typedef int (__cdecl *CvWriteFrame_Plugin)( void* writer_handle, const unsigned char* data, int step,	int width, int height, int cn, int origin);
typedef void (__cdecl *CvReleaseVideoWriter_Plugin)( void** writer );

CvCreateFileCapture_Plugin icvCreateFileCapture_FFMPEG_p = 0;
CvReleaseCapture_Plugin icvReleaseCapture_FFMPEG_p = 0;
CvGrabFrame_Plugin icvGrabFrame_FFMPEG_p = 0;
CvRetrieveFrame_Plugin icvRetrieveFrame_FFMPEG_p = 0;
CvSetCaptureProperty_Plugin icvSetCaptureProperty_FFMPEG_p = 0;
CvGetCaptureProperty_Plugin icvGetCaptureProperty_FFMPEG_p = 0;
CvCreateVideoWriter_Plugin icvCreateVideoWriter_FFMPEG_p = 0;
CvReleaseVideoWriter_Plugin icvReleaseVideoWriter_FFMPEG_p = 0;
CvWriteFrame_Plugin icvWriteFrame_FFMPEG_p = 0; 
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CWriterMPEG::CWriterMPEG(void)
{
	m_hMPEG_CV = NULL;
 	h_File_Wr = NULL;

	m_flag_Enable = false;

	m_width_frame = 0;
	m_height_frame = 0;
	m_FPS = 0;
	m_step_frame = 0;

	m_WriteState = CommonData::T_DeviceState::STOP;

	Init();
}

CWriterMPEG::~CWriterMPEG(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
// initialization of outside module
bool CWriterMPEG::Init(	void )
{
	// Get a handle to the DLL module
	m_hMPEG_CV = LoadLibrary(TEXT("opencv_ffmpeg2413.dll"/*"opencv_ffmpeg342.dll" " opencv_ffmpeg2411.dll"  opencv_ffmpeg331.dll*/));

	if( m_hMPEG_CV ){
		icvCreateFileCapture_FFMPEG_p = (CvCreateFileCapture_Plugin)GetProcAddress(m_hMPEG_CV, "cvCreateFileCapture_FFMPEG");
		icvReleaseCapture_FFMPEG_p =		(CvReleaseCapture_Plugin)GetProcAddress(m_hMPEG_CV, "cvReleaseCapture_FFMPEG");
		icvGrabFrame_FFMPEG_p =					(CvGrabFrame_Plugin)GetProcAddress(m_hMPEG_CV, "cvGrabFrame_FFMPEG");
		icvRetrieveFrame_FFMPEG_p =			(CvRetrieveFrame_Plugin)GetProcAddress(m_hMPEG_CV, "cvRetrieveFrame_FFMPEG");
		icvSetCaptureProperty_FFMPEG_p =(CvSetCaptureProperty_Plugin)GetProcAddress(m_hMPEG_CV, "cvSetCaptureProperty_FFMPEG");
		icvGetCaptureProperty_FFMPEG_p =(CvGetCaptureProperty_Plugin)GetProcAddress(m_hMPEG_CV, "cvGetCaptureProperty_FFMPEG");
		icvCreateVideoWriter_FFMPEG_p =	(CvCreateVideoWriter_Plugin)GetProcAddress(m_hMPEG_CV, "cvCreateVideoWriter_FFMPEG");
		icvReleaseVideoWriter_FFMPEG_p =(CvReleaseVideoWriter_Plugin)GetProcAddress(m_hMPEG_CV, "cvReleaseVideoWriter_FFMPEG");
		icvWriteFrame_FFMPEG_p =				(CvWriteFrame_Plugin)GetProcAddress(m_hMPEG_CV, "cvWriteFrame_FFMPEG");

		if( (icvCreateFileCapture_FFMPEG_p != 0) &&
				(icvReleaseCapture_FFMPEG_p != 0) &&
				(icvGrabFrame_FFMPEG_p != 0) &&
				(icvRetrieveFrame_FFMPEG_p != 0) &&
				(icvSetCaptureProperty_FFMPEG_p != 0) &&
				(icvGetCaptureProperty_FFMPEG_p != 0) &&
				(icvCreateVideoWriter_FFMPEG_p != 0) &&
				(icvReleaseVideoWriter_FFMPEG_p != 0) &&
				(icvWriteFrame_FFMPEG_p != 0 )
			 ){
			//printf("Successfully initialized ffmpeg plugin!\n");
			m_flag_Enable = true;
		}else{
			printf("Failed to load FFMPEG plugin: module handle=%p\n", m_hMPEG_CV);
		}
	}

// icvCreateFileCapture_FFMPEG_p = (CvCreateFileCapture_Plugin)cvCreateFileCapture_FFMPEG;
// icvReleaseCapture_FFMPEG_p = (CvReleaseCapture_Plugin)cvReleaseCapture_FFMPEG;
// icvGrabFrame_FFMPEG_p = (CvGrabFrame_Plugin)cvGrabFrame_FFMPEG;
// icvRetrieveFrame_FFMPEG_p = (CvRetrieveFrame_Plugin)cvRetrieveFrame_FFMPEG;
// icvSetCaptureProperty_FFMPEG_p = (CvSetCaptureProperty_Plugin)cvSetCaptureProperty_FFMPEG;
// icvGetCaptureProperty_FFMPEG_p = (CvGetCaptureProperty_Plugin)cvGetCaptureProperty_FFMPEG;
// icvCreateVideoWriter_FFMPEG_p = (CvCreateVideoWriter_Plugin)cvCreateVideoWriter_FFMPEG;
// icvReleaseVideoWriter_FFMPEG_p = (CvReleaseVideoWriter_Plugin)cvReleaseVideoWriter_FFMPEG;
// icvWriteFrame_FFMPEG_p = (CvWriteFrame_Plugin)cvWriteFrame_FFMPEG; 

	return m_flag_Enable;
}

//////////////////////////////////////////////////////////////////////////
void CWriterMPEG::Release(void)
{
	BOOL res = FALSE;

	//set finish video recording if it needed
	if (m_WriteState == CommonData::T_DeviceState::WRITE){

		if (h_File_Wr){
			icvReleaseVideoWriter_FFMPEG_p((void **)&h_File_Wr);
			h_File_Wr = NULL;
		}
		m_WriteState = CommonData::T_DeviceState::STOP;
	}

	// release library
	if(m_hMPEG_CV){
		res = FreeLibrary(m_hMPEG_CV); 
		m_hMPEG_CV = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// open stream to write file using coder
bool CWriterMPEG::OpenStream2Write(	const char*	filename_in,
																		const double		fps_in,							// fps
																		const int		width_in,						// width frame
																		const int		height_in						// height frame
																	)
{
	// just in case
	if (!m_flag_Enable){
		return m_flag_Enable;
	}



	//int code_rec = CV_FOURCC('D', 'I', 'V', '3');	// ?????

	//	int code_rec = CV_FOURCC('M','J','P','G');	//motion-jpeg === > !!! OK !!!
	int code_rec = CV_FOURCC('X','V','I','D');	// # MPEG-4 Part 2 === > !!! OK !!!
	//int code_rec = CV_FOURCC('H','F','Y','U');// # HuffYUV === > !!! OK !!! -> heavy volume file (VLC watching)
	//int code_rec = CV_FOURCC('I', 'Y', 'U', 'V');	// # AVI === > !!! OK !!!	-> heavy volume file 
	//int code_rec = CV_FOURCC('Y', 'V', '1', '2');	// # AVI === > !!! OK !!!	-> heavy volume file 
	//!!! does not work !!!
	
	/////////int code_rec = CV_FOURCC('N','V','1','2');	// !!! Not Support !!!
	/////////int code_rec = CV_FOURCC('P','I','M','1');	//MPEG-1 ===> !!! Not Support !!!
	/////////int code_rec = CV_FOURCC('X','2','6','4'); //# MPEG-4 Part 10 (aka. H.264 or AVC)	=== > !!! Not Support !!!
	/////////int code_rec = CV_FOURCC('M','P','1','V'); //# MPEG-1 video	=== > !!! Not Support !!!
	/////////int code_rec = CV_FOURCC('D','R','A','C'); // # BBC Dirac === > !!! Not Support !!!

	//	int code_rec = AVCodecContext;
	//void* capture_handle = NULL;
	if( (m_width_frame != width_in) || (m_height_frame != height_in) || (m_FPS != fps_in)	){
		// just in case
		StopStream();

		// set new parameters
		m_width_frame = width_in;
		m_height_frame = height_in;
		m_FPS = fps_in;
		m_step_frame = m_width_frame * 3;	// 3 ===> colors step for 1 line

	}else{
		if (h_File_Wr){
			// the stream had been already opened
			return m_flag_Enable;
		}
	}

	///
	// lock code part for except repetition of the writing file
 	g_Synh_CS_CWriterMPEG.Set_Critical_Section();
 	{
		// check file exist => create file the one
		if (!Path_File_Exists(filename_in)){
				CreateDummyFile(filename_in);

 				//printf("CreateDummyFile()\n");

				// MessageBoxA(0, "PathFileExistsA", "Error", 0);
				h_File_Wr = (HMODULE)icvCreateVideoWriter_FFMPEG_p(	(const char*)filename_in /*"123.avi"*/,
																														code_rec,
																														fps_in,								// fps
																														width_in,							// width frame
																														height_in,						// height frame
																														true									// is color enable flag
																													);
				//Flushes the buffer of a file
				FlushFileBuffers(h_File_Wr);

			//	printf("icvCreateVideoWriter_FFMPEG_p()\n");
		}

		if (h_File_Wr){
			m_flag_Enable = true;

			m_WriteState = CommonData::T_DeviceState::WRITE;

			//printf("Create_File: %s\n", filename_in);
		}else{
			m_flag_Enable = false;

			m_WriteState = CommonData::T_DeviceState::STOP;

			printf("!!! Error Create_File: %s\n", filename_in);
		}
	}
	g_Synh_CS_CWriterMPEG.Leave_Critical_Section();
	///

	return m_flag_Enable;
}

// write the single frame
bool CWriterMPEG::WriteFrame(const unsigned char* wr_data_in)
{
	int res = 0;
	if (h_File_Wr){
 		//	capture_handle = icvCreateFileCapture_FFMPEG_p("123.mpg");
		res = icvWriteFrame_FFMPEG_p(	h_File_Wr,
																	wr_data_in,
																	m_step_frame,			// m_width_frame*3 === colors step for 1 line
																	m_width_frame,
																	m_height_frame,
																	3,								// number colors ===> 3
																	0									// origin --- ?
																);

		//Flushes the buffer of a file
		FlushFileBuffers(h_File_Wr);

		if (res){
			m_WriteState = CommonData::T_DeviceState::WRITE;
		}
	}
	return m_flag_Enable;
}

// release streaming
void CWriterMPEG::StopStream(void)
{
	if (h_File_Wr){
		icvReleaseVideoWriter_FFMPEG_p((void **)&h_File_Wr);

		//Flushes the buffer of a file
		FlushFileBuffers(h_File_Wr);

		//printf("ReleaseVideoWriter\n");

		h_File_Wr = NULL;

		//MessageBoxA(0, "m_WriteState = CommonData::T_DeviceState::STOP", "StopStream", 0);

		m_WriteState = CommonData::T_DeviceState::STOP;
	}
}

//////////////////////////////////////////////////////////////////////////
// create dummy file for the trick of OpenCV SDK
void CWriterMPEG::CreateDummyFile(const char* filename_in)
{
	HANDLE hFile = NULL;
	hFile = CreateFileA(	filename_in,                // name of the write
											GENERIC_WRITE,          // open for writing
											0,                      // do not share
											NULL,                   // default security
											CREATE_NEW,             // create new file only
											FILE_ATTRIBUTE_NORMAL,  // normal file		(FILE_FLAG_NO_BUFFERING || FILE_FLAG_OVERLAPPED)	//https://msdn.microsoft.com/ru-ru/library/windows/desktop/aa363858(v=vs.85).aspx
											NULL
										);
	//Flushes the buffer of a file
	FlushFileBuffers(hFile);

	if (hFile == INVALID_HANDLE_VALUE) 
	{ 
		return;
	}

	CloseHandle(hFile);
}
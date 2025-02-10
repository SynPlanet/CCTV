#pragma once
//#include "opencv2/highgui/highgui_c.h"

//http://mathnathan.com/2010/07/error1/
//http://recog.ru/blog/ffmpeg/69.html
class CWriterMPEG
{
public:
	CWriterMPEG(void);
	~CWriterMPEG(void);

	// open stream to write file using coder
	bool OpenStream2Write(	const  char*	filename_in,
													const double		fps_in,							// fps
													const int		width_in,						// width frame
													const int		height_in						// height frame
												);

	// write the single frame
	bool WriteFrame(const unsigned char* wr_data_in);

	// release streaming
	void StopStream(void);

	bool IsOpenedStream(void) const
	{
		if (h_File_Wr){
			return true;
		}else{
			return false;
		}
	};

	// get state recording
	unsigned int GetStateRec(void) const {
		return m_WriteState;
	};


private:
	// initialization of outside module
	bool Init( void );

	// Releasing this module
	void Release(void);

	// create dummy file for the trick of OpenCV SDK
	void CreateDummyFile(const char* filename_in);

	HMODULE h_File_Wr; // void*
	HMODULE m_hMPEG_CV;	//  handler of loading module

	bool m_flag_Enable;	// enable flag to work correctly

	// frame info
	int m_width_frame;		// width frame for writing
	int m_height_frame;		// height frame for writing
	double m_FPS;					// FPS needed for writing
	int m_step_frame;

	UINT m_WriteState;	//CommonData::T_DeviceState
};


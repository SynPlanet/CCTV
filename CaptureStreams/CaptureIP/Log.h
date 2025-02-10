#pragma once
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>

namespace Logger{
//	#define NAME_CAMS_FILE_LOG "CmrLogs.txt"
//	#define NAME_AUDIO_FILE_LOG "AudioLogs.txt"
	#define NAME_CAMS_AUDIO_FILE_LOG "CmrAudioLogs.txt"

	#define LOG_TO_CONSOLE_ARG_1 "-console"
	#define LOG_TO_CONSOLE_ARG_2 "-c"
	#define LOG_TO_FILE_ARG_1 "-file"
	#define LOG_TO_FILE_ARG_2 "-f"

	#define LENGTH_FILENAME	64

	enum Out_Dir{
		File = 0,
		Console
	};

// 	enum Type_Msg{
// 		Err = 0,
// 		Info
// 	};

	class CLog
	{
	public:
		CLog(const char *filename = NULL)
		{
			m_bEnable = false;
			m_lOutDir = -1;

			m_File_Stream = NULL;

			if (filename){
				sprintf(m_FilenameLog, "%s", filename);
			}else{
				sprintf(m_FilenameLog, "%s",NAME_CAMS_AUDIO_FILE_LOG);
			}

			InitArg();
		};

		virtual ~CLog(void)
		{
			if (m_File_Stream){
				fclose( m_File_Stream );
				m_File_Stream = NULL;
			}
		};

		// set direction stream output
		void SetDirectionLog(long dir_out)
		{
			if( !((dir_out == Out_Dir::File) || (dir_out == Out_Dir::Console)) ){
				return;
			}

			if (dir_out == Out_Dir::File){

				// there was chosen Console stream output => delete!!!
				if (m_lOutDir == Out_Dir::Console){
					FreeConsole();
				}

				m_File_Stream = freopen(m_FilenameLog, "w", stdout);
				::setvbuf(stdout, NULL, _IONBF, 0 );
		
				//fflush(m_File_Stream);	// force writing to stream
				m_lOutDir = dir_out;
				m_bEnable = true;
			}

			if (dir_out == Out_Dir::Console){

				// there was chosen File stream output => delete!!!
				if (m_lOutDir == Out_Dir::File){
					if (m_File_Stream){
						fclose( m_File_Stream );
						m_File_Stream = NULL;
					}
				}

				FreeConsole();        //just in case
				if (AllocConsole()){
					long lStdHandle = (long) GetStdHandle( STD_OUTPUT_HANDLE );
					int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

					// redirect to base filestream ("stdout")
					*stdout = *(::_fdopen(hConHandle, "w"));
					::setvbuf(stdout, NULL, _IONBF, 0 );

					m_lOutDir = dir_out;
					m_bEnable = true;
				}
			}
		};

		// send messages to the predefine stream
		void PrintMessage(char *msg)
		{
			if (!m_bEnable){
				return;
			}

			// print message to the selected stream
			printf("%s\n", msg);
		};

	private:
		// init log file or console using arguments
		void InitArg(void)
		{
			int cnt_arg = __argc;
			char* nm_arg = NULL;

			// research all arguments
			for(int nmb_arg = 1; nmb_arg < cnt_arg; ++nmb_arg){

				nm_arg = __argv[nmb_arg];

				// define output message to the file
				if (!strcmp(LOG_TO_FILE_ARG_1, nm_arg) || !strcmp(LOG_TO_FILE_ARG_2, nm_arg) ){
					SetDirectionLog(Out_Dir::File);
				}else{
					// define output message to the console
					if (!strcmp(LOG_TO_CONSOLE_ARG_1, nm_arg) || !strcmp(LOG_TO_CONSOLE_ARG_2, nm_arg)){
						SetDirectionLog(Out_Dir::Console);
					}
				}
			}
		};

	private:
		bool m_bEnable;	// enable Log messages
		long m_lOutDir;	// output direction

		FILE *m_File_Stream;

		char	m_FilenameLog[LENGTH_FILENAME];
	};
};

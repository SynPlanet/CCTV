#include "StdAfx.h"
#include "MetaMoviesHDD.h"

#include "CommonIP.h"


CMetaMoviesHDD::CMetaMoviesHDD(void)
{
	sprintf(m_path_movies, "");

	m_bFlagEnableMCI = Init_MCI_AVI();
}

CMetaMoviesHDD::~CMetaMoviesHDD(void)
{
	Release();

	Term_MCI_AVI();
}

// parse all files in list ListCam_Movie by name (return number of camera)
int CMetaMoviesHDD::ParseFolder(const char* search_folder)
{
	HANDLE FindHandle;
	WIN32_FIND_DATAA FindData;
	char	path_cur[_MAX_PATH];
	char	path_old[_MAX_PATH];

	// release all lists
	Release();

	sprintf_s(m_path_movies, _MAX_PATH, search_folder);

	sprintf_s(path_cur, _MAX_PATH, search_folder);

	GetCurrentDirectoryA(_MAX_PATH, path_old);

	if (strlen(path_cur)){
		SetCurrentDirectoryA(path_cur);	/* _getcwd(path_reg, LNG_MSG); */
	}else{
		SetCurrentDirectoryA(path_old);
	}

	FindHandle = FindFirstFileA(NAME_VIDEO_REC_FIND, &FindData);

	//searching files in current folder
	do{
		if (strstr(FindData.cFileName, NAME_VIDEO_REC_TAIL)){

			ParseFileName(search_folder, FindData.cFileName);
		}

	} while(FindNextFileA( FindHandle, &FindData ));

	//close the specified search handle
	FindClose(FindHandle);

	SetCurrentDirectoryA(path_old);

	///////////////////////////////
	// calculation total time duration before rec.file 
	CalcPostTotalTime();
	///////////////////////////////

	return m_list_CamMovies.size();
}

//////////////////////////////////////////////////////////////////////////
// divide filename by parts and exam their
int CMetaMoviesHDD::ParseFileName(const char* search_folder, const char* filename)
{
	Cam_Movie cam_Mv_loc;
	Cam_Movie cam_Mv_Info_loc;
	Rec_Info	rec_info_new;
	Cam_Movie	*pMovie_loc = NULL;
	Rec_Info	*pRec_Info_loc = NULL;

	cam_Mv_loc.fps_hz = cam_Mv_loc.frames = cam_Mv_loc.width = cam_Mv_loc.height = cam_Mv_loc.total_leng_msec = 0;
	sprintf(cam_Mv_loc.name, "");

	cam_Mv_Info_loc.fps_hz = cam_Mv_Info_loc.frames = cam_Mv_Info_loc.width = cam_Mv_Info_loc.height = cam_Mv_Info_loc.total_leng_msec = 0;
	sprintf(cam_Mv_Info_loc.name, "");

	ZeroMemory((void*)&rec_info_new, sizeof(rec_info_new));

	char name_cam_IP[_MAX_FNAME/4] = "";

	char *pdest = NULL;
	int  res_find = 0;

	char nm_rec[10] = "__Rec_";
	int nm_rec_len = strlen(nm_rec);

	char nm_nmb_camIP_rec[5];
	int nmb_camIP_rec = 0;

	///////////////////////////////////
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME/4];
	char ext[_MAX_EXT];

	_splitpath(filename, drive, dir, fname, ext);
	///////////////////////////////////

	pdest = strstr((char*)fname, nm_rec);
	if (pdest){
		res_find = pdest - fname +1;

		strncpy(name_cam_IP, fname, res_find -2 );	// "-2" - except "__"

		strcpy(nm_nmb_camIP_rec, pdest+nm_rec_len);
		nmb_camIP_rec = atoi(nm_nmb_camIP_rec);

		// fill the structure
		sprintf(cam_Mv_loc.name, name_cam_IP);

		//////////////////////////////////////////////////////////////////////////
		// find the same movie by name
		int cnt_movies = m_list_CamMovies.size();
		IterCamMovie it_movie = m_list_CamMovies.begin();

		bool flag_file_found = false;
		char full_file_name[_MAX_FNAME];

		sprintf(full_file_name, "%s\\%s", search_folder, filename);
		
		// get info from examining file
		Get_TotalTime_One_Movies(/*filename*/full_file_name, cam_Mv_Info_loc);

		// set new information for recording of the single file
		rec_info_new.nmb_rec = nmb_camIP_rec;
		rec_info_new.length_msec = cam_Mv_Info_loc.total_leng_msec;
		rec_info_new.lng_total_begin_msec = 0;
		rec_info_new.frames = cam_Mv_Info_loc.frames;

		// find the same movie by name
		for (int nmb_movie_loc = 0; nmb_movie_loc < cnt_movies; ++nmb_movie_loc){
			it_movie = m_list_CamMovies.begin();

			std::advance(it_movie, nmb_movie_loc);

			// there is identical movie file
			if (!strcmp(it_movie->name, cam_Mv_loc.name)){
				flag_file_found = true;

				/////////////
				int cnt_fileMv_ID = it_movie->list_nmb_File.size();
				IterListNmbRecMovies it_fileMv_ID = it_movie->list_nmb_File.begin();

				bool flag_ID_movie_added = false;
				// find nearest number of ID
				for (int nmb_ID_Mv_loc = 0; nmb_ID_Mv_loc < cnt_fileMv_ID; ++nmb_ID_Mv_loc){

					it_fileMv_ID = it_movie->list_nmb_File.begin();
					std::advance(it_fileMv_ID, nmb_ID_Mv_loc);
					//pRec_Info_loc = static_cast<Rec_Info*>(*it_fileMv_ID);

					if ( it_fileMv_ID->nmb_rec > rec_info_new.nmb_rec){
						// save total duration all files (within one movie)
						it_movie->total_leng_msec += rec_info_new.length_msec;
						it_movie->frames += rec_info_new.frames;

						it_movie->list_nmb_File.push_front(rec_info_new);
						flag_ID_movie_added = true;
						break;
					}
				}
				if (!flag_ID_movie_added){
					// save total duration all files (within one movie)
					it_movie->total_leng_msec += rec_info_new.length_msec;
					it_movie->frames += rec_info_new.frames;

					it_movie->list_nmb_File.push_back(rec_info_new);
				}
				/////////////
			}

			//pMovie_loc = static_cast<Cam_Movie>(*it_movie);
		}

		// there is not movie in list -> add it
		if (!flag_file_found){
			cam_Mv_loc.fps_hz = cam_Mv_Info_loc.fps_hz;
			cam_Mv_loc.height = cam_Mv_Info_loc.height;
			cam_Mv_loc.width = cam_Mv_Info_loc.width;
			cam_Mv_loc.frames = cam_Mv_Info_loc.frames;
			cam_Mv_loc.total_leng_msec = cam_Mv_Info_loc.total_leng_msec;

			cam_Mv_loc.list_nmb_File.push_back(rec_info_new);
			m_list_CamMovies.push_back(cam_Mv_loc);
		}
		//////////////////////////////////////////////////////////////////////////
		
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// calculation total time duration before rec.file 
void CMetaMoviesHDD::CalcPostTotalTime(void)
{
	//////////////////////////////////////////////////////////////////////////
	// find the same movie by name
	int cnt_movies = m_list_CamMovies.size();

	if (cnt_movies <= 0){
		return;
	}
	IterCamMovie it_movie = m_list_CamMovies.begin();

	int cnt_fileMv_ID = it_movie->list_nmb_File.size();
	IterListNmbRecMovies it_fileMv_ID = it_movie->list_nmb_File.begin();
	unsigned int total_time_loc = 0;

	// researching all movie
	for (int nmb_movie_loc = 0; nmb_movie_loc < cnt_movies; ++nmb_movie_loc){

		/////////////
		cnt_fileMv_ID = it_movie->list_nmb_File.size();
		it_fileMv_ID = it_movie->list_nmb_File.begin();
		total_time_loc = 0;

		// researching all rec.files
		for (int nmb_ID_Mv_loc = 0; nmb_ID_Mv_loc < cnt_fileMv_ID; ++nmb_ID_Mv_loc){
			

		//	if (nmb_ID_Mv_loc){
				it_fileMv_ID->lng_total_begin_msec = total_time_loc;
		//	}
			total_time_loc += it_fileMv_ID->length_msec;

			std::advance(it_fileMv_ID, 1);
		}
		std::advance(it_movie, 1);
	}
}
//////////////////////////////////////////////////////////////////////////
bool CMetaMoviesHDD::DirectoryExists(const char* dirName) {
	DWORD attribs = GetFileAttributesA(dirName);
	if (attribs == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

//////////////////////////////////////////////////////////////////////////
// release memory for all lists
void CMetaMoviesHDD::Release(void)
{
	int cnt_movies = m_list_CamMovies.size();
	IterCamMovie it_movie = m_list_CamMovies.begin();

	Cam_Movie	*pMovie_loc = NULL;
	bool flag_file_found = false;

	// find the same movie by name
	for (int nmb_movie_loc = 0; nmb_movie_loc < cnt_movies; ++nmb_movie_loc){
		it_movie = m_list_CamMovies.begin();

		std::advance(it_movie, nmb_movie_loc);
		it_movie->list_nmb_File.clear();
	}
	m_list_CamMovies.clear();
}

//////////////////////////////////////////////////////////////////////////
// get numbers movies
unsigned int CMetaMoviesHDD::GetNumbersMovies( void )
{
	return	m_list_CamMovies.size();
}

//////////////////////////////////////////////////////////////////////////
// get name movie by number
const char* CMetaMoviesHDD::GetNameMovie( unsigned int nmb_movies_in // number movie ( '0' - the first movie )
																		)
{
	int cnt_movies = m_list_CamMovies.size();
	IterCamMovie it_movie = m_list_CamMovies.begin();

	if (nmb_movies_in < cnt_movies){
		std::advance(it_movie, nmb_movies_in);
		return it_movie->name;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////
// get length movies by second
LONGLONG CMetaMoviesHDD::GetLengthMovie( unsigned int nmb_movies_in // number movie ( '0' - the first movie )
																				)
{
	int cnt_movies = m_list_CamMovies.size();
	IterCamMovie it_movie = m_list_CamMovies.begin();

	if (nmb_movies_in < cnt_movies){
			std::advance(it_movie, nmb_movies_in);
			return (it_movie->total_leng_msec);
	}

	return (-1);
}

// get metadata of the single movie (!!! memory allotment for <list> - need release after using !!!)
bool CMetaMoviesHDD::GetMetaMovie(	unsigned int nmb_movies_in, // number movie ( '0' - the first movie )
																		Cam_Movie	&meta_movie_out		// movie metadata
																	)
{
	Rec_Info	_md_file_loc;

	int cnt_movies = m_list_CamMovies.size();
	IterCamMovie it_movie = m_list_CamMovies.begin();

	if (nmb_movies_in < cnt_movies){
		std::advance(it_movie, nmb_movies_in);

		// copy data for output (Cam_Movie)
		meta_movie_out.fps_hz = it_movie->fps_hz;
		meta_movie_out.frames = it_movie->frames;
		meta_movie_out.width = it_movie->width;
		meta_movie_out.height = it_movie->height;
		meta_movie_out.total_leng_msec = it_movie->total_leng_msec;
		sprintf(meta_movie_out.name, it_movie->name);

		IterListNmbRecMovies	it_rec_loc = it_movie->list_nmb_File.begin();
		IterListNmbRecMovies	it_rec_end = it_movie->list_nmb_File.end();

		for (;it_rec_loc != it_rec_end; ++it_rec_loc){
			_md_file_loc = static_cast<Rec_Info>(*it_rec_loc);
			meta_movie_out.list_nmb_File.push_back(_md_file_loc);
		}

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// get total time from one movie
void CMetaMoviesHDD::Get_TotalTime_One_Movies(const char *full_filename, Cam_Movie &cam_movie_IO)
{
// 	unsigned int len_file = 0;
// 	unsigned int fps_file = 0;
// 	unsigned int width_file = 0;
// 	unsigned int height_file = 0;

	cv::VideoCapture	m_OCV_file;	// OpenCV class for picture showing

	////////////////////////////////////
	// try get total time of file by using OpenCV
	if (m_OCV_file.open(full_filename)){

		double d_fps_file = 0;
		unsigned int d_frames_cnt_file = 0;

		cam_movie_IO.width = (unsigned int)m_OCV_file.get(CV_CAP_PROP_FRAME_WIDTH);
		cam_movie_IO.height = (unsigned int)m_OCV_file.get(CV_CAP_PROP_FRAME_HEIGHT);

		d_fps_file = 25.0f;// m_OCV_file.get(CV_CAP_PROP_FPS);
		d_frames_cnt_file = (unsigned int)m_OCV_file.get(CV_CAP_PROP_FRAME_COUNT);
		
		if (d_fps_file){ // just in case (check divide by zero)
			cam_movie_IO.total_leng_msec = (LONGLONG)(d_frames_cnt_file * 1000.0 / d_fps_file);
		}else{
			cam_movie_IO.total_leng_msec = 0;
		}

		cam_movie_IO.frames = (unsigned int)d_frames_cnt_file;
		cam_movie_IO.fps_hz = (unsigned int)d_fps_file;
		
		m_OCV_file.release();
		////////////////////////////////////
	}else{
		// try get total time of file by using MCI Windows (Media Control Interface)
		char cmd_req[_MAX_PATH] = "";
		char str_VideoLen_out[_MAX_PATH] = "";
		char str_FPS_out[_MAX_PATH] = "";
		char str_Width_out[_MAX_PATH] = "";
		char str_Height_out[_MAX_PATH] = "";

		// create request for get duration video file
		sprintf(cmd_req, "open %s alias video", full_filename);
		int err = 0;

		// open video at mci
		if ( err = mciSendStringA(cmd_req, NULL, 0, 0) ){
			showError(err);
		}

		// set time format, you can see other formats 
		if ( err =  mciSendStringA("set video time format ms", NULL, 0, 0) ){
			showError(err);
		}

		//get video length into mssg
		if ( err =  mciSendStringA("status video length", str_VideoLen_out, _MAX_PATH, 0) ){
			showError(err);
		}
		cam_movie_IO.total_leng_msec = atoi(str_VideoLen_out);
		//cam_movie_IO.total_leng_msec /= 1000;

		//get video FPS into mssg
		if ( err =  mciSendStringA("status video frame rate", str_FPS_out, _MAX_PATH, 0) ){
			showError(err);
		}
		cam_movie_IO.fps_hz = atoi(str_FPS_out);

		//get video width into mssg
		if ( err =  mciSendStringA("status video width", str_Width_out, _MAX_PATH, 0) ){
			showError(err);
		}
		cam_movie_IO.width = atoi(str_Width_out);

		//get video height into mssg
		if ( err =  mciSendStringA("status video width", str_Height_out, _MAX_PATH, 0) ){
			showError(err);
		}
		cam_movie_IO.height = atoi(str_Height_out);

		//Close video stream and the MCIAVI driver
		if ( err =  mciSendStringA("close video", NULL, 0, 0) ){
			showError(err);
		}
	}
}

// Use mciGetErrorString to get a textual description of an MCI error.Display the error description using MessageBox.
void CMetaMoviesHDD::showError(DWORD dwError)
{
	/*
	char szErrorBuf[MAXERRORLENGTH];
	MessageBeep(MB_ICONEXCLAMATION);
	if(mciGetErrorStringA(dwError, (LPSTR) szErrorBuf, MAXERRORLENGTH))
	{
		MessageBoxA(NULL, szErrorBuf, "Get data VideoFile by MCI: Error",	MB_ICONEXCLAMATION);
	}
	else
	{
		MessageBoxA(NULL, "Unknown Error", "Get data VideoFile by MCI",		MB_ICONEXCLAMATION);
	}
	*/
}


// Initialize the MCIAVI driver. This returns TRUE if OK, FALSE on error. 
bool CMetaMoviesHDD::Init_MCI_AVI(VOID) 
{ 
	// Perform additional initialization before loading first file. 
	return mciSendStringA("open digitalvideo", NULL, 0, NULL);
}

// Close the MCIAVI driver. 
void CMetaMoviesHDD::Term_MCI_AVI(VOID) 
{ 
	if (m_bFlagEnableMCI){
		mciSendStringA("close digitalvideo", NULL, 0, NULL); 
	}
}
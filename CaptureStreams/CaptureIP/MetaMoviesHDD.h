#pragma once

//////////////////////////////////////////////
struct	Movie_Info
{
	char name[_MAX_PATH];		// name movie
	LONGLONG length_msec;
};

typedef list<Movie_Info>	ListMovieInfo;
typedef	list<Movie_Info>::iterator IterMovieInfo;
//////////////////////////////////////////////

//////////////////////////////////////////////
struct	Rec_Info
{

	unsigned int nmb_rec; // number recording
	LONG length_msec;	// time duration of movie
	LONGLONG lng_total_begin_msec; // total time duration before rec.file 
	unsigned int	frames;			// number frames of movie
};

typedef list<Rec_Info>	ListNmbRecMovies;
typedef	list<Rec_Info>::iterator IterListNmbRecMovies;

struct	Cam_Movie
{
	char name[_MAX_PATH/4];		// name movie except tail (Sample: "File_Test_1" => "File_Test"

	LONG	total_leng_msec;
	unsigned int	fps_hz;
	unsigned int	width;
	unsigned int	height;
	unsigned int	frames;

	ListNmbRecMovies		list_nmb_File;	//  quantity files was included into the one movie
};

typedef list<Cam_Movie>	ListCamMovie;
typedef	list<Cam_Movie>::iterator IterCamMovie;
//////////////////////////////////////////////

class CMetaMoviesHDD	
{
public:
	CMetaMoviesHDD(void);
	~CMetaMoviesHDD(void);

	// parse all files in list ListCam_Movie by name (return number of camera)
	int ParseFolder(const char* search_folder);

	bool DirectoryExists(const char* dirName);

	// get numbers movies
	unsigned int GetNumbersMovies( void );

	// get metadata of the single movie
	bool GetMetaMovie(	unsigned int nmb_movies_in, // number movie ( '0' - the first movie )
											Cam_Movie	&meta_movie_out		// movie metadata
										);

	// get length movies by second
	LONGLONG GetLengthMovie( unsigned int nmb_movies_in // number movie ( '0' - the first movie )
													);

	// get name movie by number
	const char* GetNameMovie( unsigned int nmb_movies_in // number movie ( '0' - the first movie )
													);

	// get folder path for playing
	const char* GetFolderPlaying( void )
	{
		return m_path_movies;
	}
private:

	bool m_bFlagEnableMCI;	// the flag MCI enable

	// Initialize the MCIAVI driver. This returns TRUE if OK, FALSE on error. 
	bool Init_MCI_AVI(VOID);

	// Close the MCIAVI driver. 
	void Term_MCI_AVI(VOID);

	// divide filename by parts and exam their
	int ParseFileName(const char* search_folder, const char* filename);

	// get total time from one movie
	void Get_TotalTime_One_Movies(const char *full_filename, Cam_Movie &cam_movie_IO);

	// calculation total time duration before rec.file 
	void CalcPostTotalTime(void);

	// Use mciGetErrorString to get a textual description of an MCI error.Display the error description using MessageBox.
	void showError(DWORD dwError);

	ListCamMovie	m_list_CamMovies;	// list include quantity objects(movies) folder inside

	char	m_path_movies[_MAX_PATH];

	// release memory for all lists
	void Release(void);
};


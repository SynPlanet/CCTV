// stdafx.cpp : source file that includes just the standard includes
// CaptureIP.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

// check file exist (analog PathFileExistsA)
bool Path_File_Exists(const char *path_filename)
{
	return GetFileAttributesA(path_filename) != INVALID_FILE_ATTRIBUTES;
}



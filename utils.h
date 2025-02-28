
#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#else 
#include <windows.h>
#endif

//-----------------------------------------------------------------------------
inline int xfn_getFullPath(const char *name, char *path)
{
#ifdef __linux__
	char* ret = realpath(name, path);
#else
	char *fchar;
	DWORD ret = GetFullPathName(name, MAX_PATH, path, &fchar);
#endif
	if (!ret)
		return -1;
	return 0;
}

//-----------------------------------------------------------------------------
inline int xfn_getCurrentDir(char *buf, int size)
{
#ifdef __linux__
	char* ret = getcwd(buf, size);
#else
	DWORD ret = GetCurrentDirectory(size, buf);
#endif
	if (!ret)
		return -1;
	return 0;
}

//-----------------------------------------------------------------------------
inline int xfn_isDir(const char* dirname)
{
	int ret = 0;
#ifdef __linux__
	DIR* dir = opendir(dirname);
	if (dir)
	{ // Directory exists
		closedir(dir); ret = 1;
	}
	else
		if (ENOENT == errno) { // Directory does not exist.
			ret = 0;
		}
		else {  // opendir() failed for some other reason.
			ret = -1;
		}
#else
	DWORD dwAttrib = GetFileAttributes(dirname);
	ret = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
	return ret;
}

//-----------------------------------------------------------------------------
inline void xfn_createDir(const char* dirname)
{
#ifdef __linux__
	struct stat st; // = { 0 };
	if (stat(dirname, &st) == -1) {
		mkdir(dirname, 0755);
	}
#else
	DWORD dwAttrib = GetFileAttributes(dirname);
	int isdir = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	if(!isdir)
		CreateDirectory(dirname, NULL);
#endif
}

#endif	// _UTILS_H_

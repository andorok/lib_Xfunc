
#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __linux__
#include <unistd.h>
#else 
#include <windows.h>
#endif

//-----------------------------------------------------------------------------
int getFullPath(const char *name, char *path)
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
int getCurrentDir(char *buf, int size)
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

#endif	// _UTILS_H_

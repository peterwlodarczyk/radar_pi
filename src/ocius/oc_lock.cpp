#include <stdio.h>
#include "oc_lock.h"
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>
#else
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

//#include "oc_fileio.h"

#define MAX_TRIES 3001

////////////////////////////////////////////////////////////////////////////////
//new FileLock

bool
FileLock::obtainLock()
{
#ifdef WIN32
	SECURITY_ATTRIBUTES MutexAttributes;
	ZeroMemory( &MutexAttributes, sizeof(MutexAttributes) );
	MutexAttributes.nLength = sizeof( MutexAttributes );
	MutexAttributes.bInheritHandle = FALSE; // object uninheritable

	// declare and initialize a security descriptor
	SECURITY_DESCRIPTOR SD;
	BOOL bInitOk = InitializeSecurityDescriptor( &SD, SECURITY_DESCRIPTOR_REVISION );
	if ( bInitOk )
	{
		// give the security descriptor a Null Dacl
		// done using the  "TRUE, (PACL)NULL" here
		BOOL bSetOk = SetSecurityDescriptorDacl( &SD,
                                            TRUE,
                                            (PACL)NULL,
                                            FALSE );
		if (bSetOk)
			MutexAttributes.lpSecurityDescriptor = &SD;
	}

	std::string strName = "Global\\";
	char ach[512];
	strcpy(ach, strFile.c_str());
	char *p = ach;
	while (p && *p)
	{
		if (*p == '\\')
			*p = '/';
		p++;
	}

	p = ach;
	while (p && *p)
	{
		if (*p == '/')
		{
			p++;
			break;
		}
		p++;
	}
	strName += p;

	int iTries = 0;
	while (iTries < 2000)
	{
		hMutex = CreateMutexA(&MutexAttributes, FALSE, strName.c_str());
		if (hMutex)
		{
			DWORD dwCount=0, dwWaitResult;
			while( !b_locked && dwCount < 20 )
			{
				dwCount++;
				dwWaitResult = WaitForSingleObject(hMutex, 100);
				switch (dwWaitResult)
				{
					case WAIT_OBJECT_0:
						b_locked = true;
						break;
					case WAIT_ABANDONED:
						break;
				}
			}
			//if (b_locked)
			//	if (b_debug)
			//		DebugLog(DEBUG_LOCKING, "Lock for %s created after %d tries", strFile.c_str(), iTries);
			break;
		}
		else
		{
			DWORD dwError = GetLastError();
			Sleep(1);
			iTries++;
		}
	}
	if (!b_locked)
	{
		//if (b_debug)
		//	DebugLog(DEBUG_LOCKING, "Lock for %s still not created after %d attempts", strFile.c_str(), iTries);
	}
#else
	struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
	fl.l_pid = getpid();
	if (access(strFile.c_str(), 0))
	{
		//does not exist yet
		b_locked = true;
		return b_locked;

		//create it first
		//FILE *fp = fopen(strFile.c_str(), "w");
		//if (fp)
		//{
		//	fclose(fp);
		//}
	}
	bool done = false;
	int iCount = 0;
	while (!done && iCount < 1000)
	{
		if ((fd = open(strFile.c_str(), O_RDWR)) != -1)
		{
			if (fcntl(fd, F_SETLKW, &fl) != -1)
			{
				b_locked = true;
				return b_locked;
			}
		}
		usleep(1000);

		iCount++;
	}
#endif
	return b_locked;
}

FileLock::FileLock(const char *pchFile) :
	strFile(pchFile ? pchFile : "")
	,b_locked(false)
	,b_debug(false)
#ifdef WIN32
	,hMutex(0)
#else
	,fd(0)
#endif
{
	obtainLock();
}

FileLock::FileLock(std::string &strFile_) :
	strFile(strFile_)
	,b_locked(false)
	,b_debug(false)
#ifdef WIN32
	,hMutex(0)
#else
	,fd(0)
#endif
{
	obtainLock();
}

FileLock::~FileLock()
{
#ifdef WIN32
	if (hMutex)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		hMutex = 0;
		//if (b_debug)
		//	DebugLog(DEBUG_LOCKING, "Lock for %s released", strFile.c_str());
	}
#else
	if (fd)
	{
		struct flock fl = {F_UNLCK, SEEK_SET,   0,      0,     0 };
		fl.l_pid = getpid();
		if (fcntl(fd, F_SETLKW, &fl) == -1)
		{
		}
		close(fd);
		fd = 0;
		//if (b_debug)
		//	DebugLog(DEBUG_LOCKING, "Lock for %s released", strFile.c_str());
	}
#endif
}

void
RemoveLock(FileLock *&pfl)
{
	if (pfl)
		delete pfl;
	pfl = 0;
}

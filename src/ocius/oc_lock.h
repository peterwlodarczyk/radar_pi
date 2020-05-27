#ifndef __OC_LOCK_H
#define __OC_LOCK_H
#include <string>

class FileLock
{
private:
	std::string	strFile;
	std::string	strError;
	bool		b_locked;
	bool		b_debug;
#ifdef WIN32
  void *hMutex;
#else
	int		fd;
#endif

	bool		obtainLock();
public:
	FileLock(const char *pchFile);
	FileLock(std::string &strFile);
	~FileLock();

	bool	locked() const { return b_locked; }
	const char *getError() const { return strError.c_str(); }
};

void	RemoveLock(FileLock *&pfl);

#endif

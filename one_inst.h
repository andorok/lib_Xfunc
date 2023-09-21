#ifndef _ONEINSTANCE_H
#define _ONEINSTANCE_H

#include <string>

class OneInstance
{
public:
	OneInstance(const std::string &name);
    ~OneInstance();

    bool operator()();

    std::string getLockName();

private:
    std::string m_name;
#ifdef __linux__
    int m_fd;
	int m_rc;
#else
    HANDLE m_hMutex;
#endif
};

#endif // _ONEINSTANCE_H

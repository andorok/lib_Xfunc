#ifdef __linux__
#include <sys/file.h>
#include <errno.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "one_inst.h"

OneInstance::OneInstance(const std::string &name):
    m_name( name )
{
#ifdef __linux__
	m_fd = -1;
	m_rc = -1;
#else
	m_hMutex = reinterpret_cast<HANDLE>(-1);
#endif
}

OneInstance::~OneInstance()
{
#ifdef __linux__
	if(m_fd >= 0 )
		::close( m_fd );
#else
	::CloseHandle(m_hMutex); // close handle before terminating
#endif
}

#ifdef __linux__
bool OneInstance::operator()()
{
    if(m_fd == -1 )
		m_fd = ::open(m_name.c_str(), O_CREAT | O_RDWR, 0666);

    if(m_fd >= 0 && m_rc == -1 )
        m_rc = ::flock(m_fd, LOCK_EX | LOCK_NB);

    return m_fd != -1 && m_rc == 0;
}
#else
bool OneInstance::operator()()
{
	if (m_hMutex == reinterpret_cast<HANDLE>(-1))
	{
		m_hMutex = ::CreateMutex(NULL, FALSE, m_name.c_str());

		if (ERROR_ALREADY_EXISTS == GetLastError())
		{ // Program already running somewhere
			return false;
		}
	}
	return true;
}
#endif

std::string OneInstance::getLockName()
{
    return m_name;
}

// 1
//#define APPLICATION_INSTANCE_MUTEX_NAME "{BA49C45E-B29A-4359-A07C-51B65B5571AD}"
//
////Make sure at most one instance of the tool is running
//HANDLE hMutexOneInstance(::CreateMutex(NULL, TRUE, APPLICATION_INSTANCE_MUTEX_NAME));
//bool bAlreadyRunning((::GetLastError() == ERROR_ALREADY_EXISTS));
//if (hMutexOneInstance == NULL || bAlreadyRunning)
//{
//    if (hMutexOneInstance)
//    {
//        ::ReleaseMutex(hMutexOneInstance);
//        ::CloseHandle(hMutexOneInstance);
//    }
//    throw std::exception("The application is already running");
//}

// 2
// HANDLE hMutex = CreateMutexA(NULL, FALSE, "my mutex");
//DWORD dwMutexWaitResult = WaitForSingleObject(hMutex, 0);
//if (dwMutexWaitResult != WAIT_OBJECT_0)
//{
//    MessageBox(HWND_DESKTOP, TEXT("This application is already running"), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
//    CloseHandle(hMutex);
//}

// 3
//int WINAPI WinMain(...)
//{
//    const char szUniqueNamedMutex[] = "com_mycompany_apps_appname";
//    HANDLE hHandle = CreateMutex(NULL, TRUE, szUniqueNamedMutex);
//    if (ERROR_ALREADY_EXISTS == GetLastError())
//    {
//        // Program already running somewhere
//        return(1); // Exit program
//    }
//
//    // Program runs...
//
//    // Upon app closing:
//    ReleaseMutex(hHandle); // Explicitly release mutex
//    CloseHandle(hHandle); // close handle before terminating
//    return(1);
//}

// 4
//HANDLE hMutexOneInstance = ::CreateMutex(NULL, FALSE, "MYAPPNAME-088FA840-B10D-11D3-BC36-006067709674");
//
//bool AlreadyRunning = (::GetLastError() == ERROR_ALREADY_EXISTS ||
//    ::GetLastError() == ERROR_ACCESS_DENIED);

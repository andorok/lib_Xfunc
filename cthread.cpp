
#include "cthread.h"

CThread::CThread()
{
}

CThread::~CThread()
{
}

#ifdef __linux__

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//-----------------------------------------------------------------------------
int CThread::create(void* (*function)(void *), void* param)
{
	int res;

	//pthread_attr_t attr; // поток можно создать с заданием ему некоторых атрибутов
	//res = pthread_attr_init(&attr); // Инициализация атрибутов по-умолчанию
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // 
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	//res = pthread_create(&thread_id, &attr, function, param);

	res = pthread_create(&m_thread_id, NULL, function, param);

	//Destroy the thread attributes object, since it is no longer needed
	//pthread_attr_destroy(&attr);

	if (res != 0) {
		printf("%s(): error create thread. %s\n", __FUNCTION__, strerror(errno));
		return 0;
	}

	//printf("%s(): thread was created\n", __FUNCTION__);

	return 0;
}

//-----------------------------------------------------------------------------
int CThread::wait(int timeout)
{
	void *retval = NULL;
	int res = 0;

	if (timeout < 0)
	{
		//printf("%s(): Start waiting...\n", __FUNCTION__);
		res = pthread_join(m_thread_id, &retval);
	}
	else
	{
		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
			//printf("%s(): pthread_join() error int clock_gettime(). Try again.\n", __FUNCTION__);
			return -EAGAIN;
		}
		ts.tv_nsec += (timeout * 1000000);
		res = pthread_timedjoin_np(m_thread_id, NULL, &ts);
	}

	if (res != 0)
	{
		if (res == ETIMEDOUT) {
			printf("%s(): pthread_join() error. retval = %p, ETIMEDOUT\n", __FUNCTION__, retval);
			return -ETIMEDOUT;
		}
		else
			if (res == EBUSY) {
				printf("%s(): pthread_join() error. retval = %p, EBUSY\n", __FUNCTION__, retval);
				return -EBUSY;
			}
	}

	//printf("%s(): thread was finished. retval = %p. OK\n", __FUNCTION__, retval);

	return 0;
}

//-----------------------------------------------------------------------------

int CThread::close()
{
	void *ret = 0;

	int res = pthread_join(m_thread_id, &ret);
	if (res != 0) {
		if (res != ESRCH) {
			printf("%s(): thread was error %s\n", __FUNCTION__, strerror(errno));
			return -ESRCH;
		}
	}

	//DEBUG_PRINT("%s(): threadwas deleted\n", __FUNCTION__);

	return 0;
}

#else

#include <process.h> 

int CThread::create(unsigned int (*function)(void *), void* param)
{
	unsigned threadID;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, function, param, 0, &(threadID));
	return 0;
}

//-----------------------------------------------------------------------------
int CThread::wait(int timeout)
{
	ULONG status = WaitForSingleObject(m_hThread, timeout);
	if (status == WAIT_TIMEOUT)
		return WAIT_TIMEOUT;
	else if (status == WAIT_ABANDONED)
		return WAIT_ABANDONED;
	else if (status == WAIT_OBJECT_0)
		return 0;
	else
		return -1;

	return 0;
}

//-----------------------------------------------------------------------------
int CThread::close()
{
	int ret = CloseHandle(m_hThread);
	if (!ret)
		return -1;

	return 0;
}

#endif // __linux__

CSharedMem::CSharedMem()
{
	m_ptr = nullptr;
#ifdef __linux__
	m_fd = 0;
#else
    m_hMem = NULL;
#endif
}

CSharedMem::~CSharedMem()
{
	close();
}

//-----------------------------------------------------------------------------
void* CSharedMem::create(const char* name, size_t size)
{
	m_size = size;
#ifdef __linux__
	m_fd = shm_open(name, O_CREAT | O_RDWR, 0777);
	if(m_fd == -1)
		return nullptr;
	ftruncate(m_fd, size);
	m_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
	if (m_ptr == MAP_FAILED)
		return nullptr;
#else
	m_hMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)size, name);
	if (m_hMem == NULL)
		return nullptr;
	//*alreadyCreated = (GetLastError() == ERROR_ALREADY_EXISTS) ? 1 : 0;
	m_ptr = MapViewOfFile(m_hMem, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (m_ptr == NULL)
		return nullptr;
#endif
	return m_ptr;
}

//-----------------------------------------------------------------------------
int CSharedMem::close()
{
#ifdef __linux__
	if (m_ptr != nullptr) {
		munmap(m_ptr, m_size);
		m_ptr = nullptr;
	}
	if (m_fd != 0) {
		::close(m_fd);
		m_fd = 0;
	}
#else
	int ret = 0;
	if (m_ptr != nullptr) {
		ret = UnmapViewOfFile(m_ptr);
		if (!ret)
			return -1;
		m_ptr = nullptr;
	}
	if (m_hMem != NULL) {
		ret = CloseHandle(m_hMem);
		if (!ret)
			return -1;
        m_hMem = NULL;
	}
#endif
	return 0;
}

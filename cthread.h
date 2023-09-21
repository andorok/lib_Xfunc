//=************************* CThread  - the thread class *************************

#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef __linux__
#include <pthread.h>
#define INFINITE -1
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#else
#include "windows.h"
#endif
#include	<vector>

class CThread
{

#ifdef __linux__
	pthread_t m_thread_id;
#else
	HANDLE m_hThread;
#endif

protected:

public:

#ifdef __linux__
	int create(void* (*function)(void *), void* param);
#else
	int create(unsigned int(*function)(void *), void* param);
#endif

	int wait(int timeout);
	int close();

	CThread();
	~CThread();

};

//int const MAX_NSYNC = 64;

class CBarrier
{

#ifdef __linux__
	pthread_barrier_t m_barrier;
#else
	int m_nsync;
	int m_cnt;
	//HANDLE m_hSyncEvent[MAX_NSYNC];
	std::vector <HANDLE> m_hSyncEvents;
	HANDLE* m_pEvents;
#endif

protected:

public:

	int init(int count)
	{
		int ret = 0;
#ifdef __linux__
		pthread_barrier_init(&m_barrier, NULL, count);
#else
		m_nsync = 0;
		m_cnt = count;
		for (int i = 0; i < count; i++)
		{
			HANDLE hSyncEvent = CreateEvent(
									NULL,   // default security attributes
									FALSE,  // auto-reset event object
									FALSE,  // initial state is nonsignaled
									NULL);  // unnamed object
			m_hSyncEvents.push_back(hSyncEvent);
		}
		m_pEvents = &m_hSyncEvents[0];
#endif
		return ret;
	}
	void wait()
	{
#ifdef __linux__
		pthread_barrier_wait(&m_barrier);
#else
		SetEvent(m_hSyncEvents[m_nsync++]); // установить в состояние Signaled
		//size_t count = m_hSyncEvents.size();
		WaitForMultipleObjects(m_cnt, m_pEvents, TRUE, INFINITE);
#endif
	}
	int close()
	{
		int ret = 0;
#ifdef __linux__
		pthread_barrier_destroy(&m_barrier);
#else
		for (auto& event : m_hSyncEvents)
				CloseHandle(event);
#endif
		return ret;
	}

	//CBarrier();
	//~CBarrier();

};

class CMutex
{

#ifdef __linux__
	pthread_mutex_t m_mutex;
#else
	HANDLE m_hMutex;
#endif

protected:

public:

	inline void lock();
	inline void unlock();
	inline void sync();

	CMutex()
	{
#ifdef __linux__
		pthread_mutex_init(&m_mutex, NULL);//Инициализация мьютекса
#else
		m_hMutex = CreateMutex(NULL, FALSE, NULL);
#endif
	}

	~CMutex()
	{
#ifdef __linux__
		pthread_mutex_destroy(&m_mutex); //Уничтожение мьютекса
#else
		CloseHandle(m_hMutex);
#endif
	}

};

//CMutex::CMutex()
//{
//#ifdef __linux__
//	pthread_mutex_init(&m_mutex, NULL);//Инициализация мьютекса
//#else
//	m_hMutex = CreateMutex(NULL, FALSE, NULL);
//#endif
//}
//
//CMutex::~CMutex()
//{
//#ifdef __linux__
//	pthread_mutex_destroy(&m_mutex); //Уничтожение мьютекса
//#else
//	CloseHandle(m_hMutex);
//#endif
//}

void CMutex::lock()
{
#ifdef __linux__
	pthread_mutex_lock(&m_mutex);
#else
	WaitForSingleObject(m_hMutex, INFINITE);
#endif
}

void CMutex::unlock()
{
#ifdef __linux__
	pthread_mutex_unlock(&m_mutex);
#else
	ReleaseMutex(m_hMutex);
#endif
}

void CMutex::sync()
{
#ifdef __linux__
	pthread_mutex_lock(&m_mutex);
	pthread_mutex_unlock(&m_mutex);
#else
	WaitForSingleObject(m_hMutex, INFINITE);
	ReleaseMutex(m_hMutex);
#endif
}

class CSharedMem
{

#ifdef __linux__
	int m_fd;
#else
	HANDLE m_hMem;
#endif
	size_t m_size;
	void* m_ptr;

protected:

public:

	void* create(const char* name, size_t size);
	int close();

	CSharedMem();
	~CSharedMem();

};

#endif	// _THREAD_H_

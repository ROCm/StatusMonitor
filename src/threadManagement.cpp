#include "threadManagement.h"

#if defined( WIN32 )
//----------------------------------------------------------------------------------------------
//  Windows implementation
//----------------------------------------------------------------------------------------------

Thread::Thread() : m_bRunning(false), m_hThread(NULL), m_uiThreadId(0)
{
}

Thread::~Thread()
{
    if (m_bRunning)
    {
        join();
    }
}

bool Thread::create(THREAD_PROC pThreadFunc, void *pArg)
{
    if (pThreadFunc == NULL || m_hThread != NULL)
    {
        return false;
    }

    m_hThread = CreateThread(NULL, 0, pThreadFunc, pArg, 0, &((DWORD)m_uiThreadId));

    if (m_hThread == NULL)
    {
        return false;
    }

    m_bRunning = true;

    return true;
}


void Thread::join()
{
    if (m_hThread && m_bRunning)
    {
        WaitForSingleObject(m_hThread, INFINITE);
    }

    m_bRunning = false;


	m_hThread = NULL;
}

#else

//----------------------------------------------------------------------------------------------
//  Linux implementation of Thread
//----------------------------------------------------------------------------------------------
#include <pthread.h>

Thread::Thread() : m_bRunning(false), m_uiThreadId(0)
{
}

Thread::~Thread()
{
    if (m_bRunning)
    {
        join();
    }
}

bool Thread::create(THREAD_PROC pThreadFunc, void *pArg)
{
    if (pThreadFunc == NULL)
    {
        return false;
    }

    int res = pthread_create(&m_uiThreadId, NULL, pThreadFunc, pArg);

    if (res != 0)
    {
        return false;
    }

    m_bRunning = true;

    return true;
}

void Thread::join()
{
	struct timespec timeout;

	timeout.tv_sec  = 2;
	timeout.tv_nsec = 0;

	if (m_bRunning)
		pthread_join(m_uiThreadId, NULL);
//		pthread_timedjoin_np(m_uiThreadId, NULL, &timeout);

    m_bRunning = false;
}
#endif
//Thread g_threads[MAX_DEVICE];
//
//
//unsigned int CRC = 0;
//char* devicesName[MAX_DEVICE];
//int   deviceNum[MAX_DEVICE];
//int   deviceAll = 1;
//int   deviceUsed = 0;




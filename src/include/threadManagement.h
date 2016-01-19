//no c++ 11 for the moment
#ifndef THREADMANAGEMENT
#define THREADMANAGEMENT


#include <stdlib.h>

#if defined (WIN32)
#include <windows.h>

typedef LPTHREAD_START_ROUTINE  THREAD_PROC;
#define CALLAPI WINAPI

#else

typedef void *(*THREAD_PROC)(void*);
#define INFINITE            0xFFFFFFFF 
#define CALLAPI
#endif

class Thread
{
public:
    Thread();
    virtual ~Thread();

    bool            create(THREAD_PROC pThreadFunc, void* pArg);
    void            join();

    //HANDLE          getThreadHandle()   { return m_hThread; };
    unsigned int    getThreadId()       { return m_uiThreadId; };

private:

    bool            m_bRunning;
    unsigned long   m_uiThreadId;

#if defined ( WIN32 )
    HANDLE          m_hThread;
#endif

};


#endif
// 
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

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

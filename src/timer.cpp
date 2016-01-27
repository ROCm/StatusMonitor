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

#include "Timer.h"




/*
After hours of perusing different answers, blogs, and headers, I found a portable way to get the current time:

struct timespec ts;

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
clock_serv_t cclock;
mach_timespec_t mts;
host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
clock_get_time(cclock, &mts);
mach_port_deallocate(mach_task_self(), cclock);
ts.tv_sec = mts.tv_sec;
ts.tv_nsec = mts.tv_nsec;

#else
clock_gettime(CLOCK_REALTIME, &ts);
#endif

or check out this gist: https://gist.github.com/1087739

Hope this saves someone time. Cheers!

Comment by jbenet — July 17, 2011 @ 12:29 pm

Actually, if you just want to stay in straight C/C++ and link with a minimum of additional libraries, you’re better off using mach_absolute_time(), as described here: http://developer.apple.com/library/mac/qa/qa1398
*/



CPerfCounter::CPerfCounter() : _clocks(0), _start(0)
{

#ifdef _WIN32
    QueryPerformanceFrequency((LARGE_INTEGER *)&_freq);
#else
    _freq = 1000;
#endif

}

CPerfCounter::~CPerfCounter()
{
    // EMPTY!
}

void
CPerfCounter::Start(void)
{

#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER *)&_start);
    
#elif __APPLE__
    // Convert to nanoseconds.
    
    // Have to do some pointer fun because AbsoluteToNanoseconds 
    // works in terms of UnsignedWide, which is a structure rather 
    // than a proper 64-bit integer.
//    elapsedNano = AbsoluteToNanoseconds( *(AbsoluteTime *) &elapsed );
    
    _start = mach_absolute_time();
    

#else
    struct timespec s;
    clock_gettime( CLOCK_REALTIME, &s );
    _start = (i64)s.tv_sec * 1e9 + (i64)s.tv_nsec;
#endif

}

void
CPerfCounter::Stop(void)
{
    i64 n;

#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER *)&n);
#elif __APPLE__
    n = mach_absolute_time();
    
#else
    struct timespec s;
    clock_gettime( CLOCK_REALTIME, &s );
    n = (i64)s.tv_sec * 1e9 + (i64)s.tv_nsec;
#endif

    n -= _start;
    _start = 0;
    _clocks += n;
}

void
CPerfCounter::Reset(void)
{

    _clocks = 0;
}

double
CPerfCounter::GetElapsedTime(void)
{
#if _WIN32
    return (double)_clocks / (double) _freq;
#else
    return (double)_clocks / (double) 1e9;
#endif

}


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

#ifndef _TIMER_H_
#define _TIMER_H_
/**
 * \file Timer.h
 * \brief A timer class that provides a cross platform timer for use
 * in timing code progress with a high degree of accuracy.
 */
#ifdef _WIN32
/**
 * \typedef __int64 i64
 * \brief Maps the windows 64 bit integer to a uniform name
 */
#if defined(__MINGW64__) || defined(__MINGW32__)
typedef long long i64;
#else
typedef __int64 i64;
#endif
#else
/**
 * \typedef long long i64
 * \brief Maps the linux 64 bit integer to a uniform name
 */
typedef long long i64;
#endif

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

/**
 * \class CPerfCounter
 * \brief Counter that provides a fairly accurate timing mechanism for both
 * windows and linux. This timer is used extensively in all the samples.
 */
class CPerfCounter {

public:
    /**
     * \fn CPerfCounter()
     * \brief Constructor for CPerfCounter that initializes the class
     */
    CPerfCounter();
    /**
     * \fn ~CPerfCounter()
     * \brief Destructor for CPerfCounter that cleans up the class
     */
    ~CPerfCounter();
    /**
     * \fn void Start(void)
     * \brief Start the timer
     * \sa Stop(), Reset()
     */
    void Start(void);
    /**
     * \fn void Stop(void)
     * \brief Stop the timer
     * \sa Start(), Reset()
     */
    void Stop(void);
    /**
     * \fn void Reset(void)
     * \brief Reset the timer to 0
     * \sa Start(), Stop()
     */
    void Reset(void);
    /**
     * \fn double GetElapsedTime(void)
     * \return Amount of time that has accumulated between the \a Start()
     * and \a Stop() function calls
     */
    double GetElapsedTime(void);

private:

    i64 _freq;
    i64 _clocks;
    i64 _start;
};

#endif // _TIMER_H_


/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/



/**
 *@file  Time_unix.cc
 *@brief Generic unix implementation of the Time class
 *
 *@author
 *   Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *@date: June 1997
 *
 */


#include <Core/Thread/Time.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/ThreadError.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <iostream>
#ifdef __linux
#include <time.h>
#endif
#include <string.h>

static bool initialized=false;
static struct timeval start_time;

using namespace SCIRun;

void
Time::initialize()
{
    initialized=true;
    if(gettimeofday(&start_time, 0) != 0)
	throw ThreadError(std::string("gettimeofday failed: ")
			  +strerror(errno));
}

double
Time::secondsPerTick()
{
    return 1.e-6;
}

double
Time::currentSeconds()
{
    if(!initialized)
	initialize();
    struct timeval now_time;
    if(gettimeofday(&now_time, 0) != 0)
	throw ThreadError(std::string("gettimeofday failed: ")
			  +strerror(errno));

    return (now_time.tv_sec-start_time.tv_sec)+(now_time.tv_usec-start_time.tv_usec)*1.e-6;
}

Time::SysClock
Time::currentTicks()
{ 
    if(!initialized) initialize();
    struct timeval now_time;
    if(gettimeofday(&now_time, 0) != 0)
	throw ThreadError(std::string("gettimeofday failed: ")
			  +strerror(errno));

    Time::SysClock msec = static_cast<Time::SysClock>(now_time.tv_sec-start_time.tv_sec);
    msec = msec * 1000000;
    msec = msec + static_cast<Time::SysClock>(now_time.tv_usec-start_time.tv_usec);
    if (msec < 0) msec = 0;
 
    return msec;
}

double
Time::ticksPerSecond()
{
    return 1000000;
}

void
Time::waitUntil(double seconds)
{
    waitFor(seconds-currentSeconds());
}

void
Time::waitFor(double seconds)
{
  if(!initialized) 
    initialize();

  if(seconds<=0)
    return;

  struct timespec ts;
  ts.tv_sec=(int)seconds;
  ts.tv_nsec=(int)(1.e9*(seconds-ts.tv_sec));

  int oldstate=Thread::couldBlock("Timed wait");
  nanosleep(&ts, &ts);
  //  while (nanosleep(&ts, &ts) == 0) /* Nothing */ ;
  Thread::couldBlockDone(oldstate);
}

void
Time::waitUntil(SysClock time)
{
    waitFor(time-currentTicks());
}

void
Time::waitFor(SysClock time)
{
    if(!initialized)
	initialize();
    if(time<=0)
	return;
    struct timespec ts;
    ts.tv_sec=(int)(time*1.e-6);
    ts.tv_nsec=(int)(1.e9*(time*1.e-6-ts.tv_sec));
    int oldstate=Thread::couldBlock("Timed wait");
    nanosleep(&ts, &ts);
    //while (nanosleep(&ts, &ts) == 0) /* Nothing */ ;
    Thread::couldBlockDone(oldstate);
}


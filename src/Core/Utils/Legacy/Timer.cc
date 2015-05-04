#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
/*
  For more information, please see: http://software.sci.utah.edu

  The MIT License

  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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



/*
 *@file  Timer.h
 *@brief Implementation of portable timer utility classes
 *
 *@author
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *@date  Feb. 1994
 *
 */


#include <Core/Utils/Legacy/Timer.h>
#include <Core/Thread/Legacy/Time.h>
#include <Core/Utils/Legacy/Assert.h>

#include <iostream>

#ifndef _WIN32
#include <sys/times.h>
// some older linux systems look for sysconf 
// in the following include...
#include <unistd.h>
#else
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#include <time.h>
#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CLOCK_T_DEFINED
#define _CLOCK_T_DEFINED
typedef long clock_t;
#endif

struct tms
{
  clock_t tms_utime;
  clock_t tms_stime;
  clock_t tms_cutime;
  clock_t tms_cstime;
};

#ifdef __cplusplus
}
#endif

clock_t times(struct tms* buffer)
{
#ifdef _WIN32
  static double cps = (double)(CLOCKS_PER_SEC/1000);
#else
  static double cps = (double)(sysconf(_SC_CLK_TCK)/1000);
#endif

  timeb curtime;
  ftime(&curtime);
  
  clock_t ticks = static_cast<clock_t>(((curtime.time*1000)+curtime.millitm)*cps);
  
  buffer->tms_utime = buffer->tms_cutime = ticks;
  buffer->tms_stime = buffer->tms_cstime = 0;
  
  return ticks;
}
#endif


namespace SCIRun {

Timer::Timer()
{
  state_      = Stopped;
  total_time_ = 0;
}

Timer::~Timer()
{
#if SCI_ASSERTION_LEVEL >=1  
  if(state_ != Stopped) {
    std::cerr << "Warning: Timer destroyed while it was running" << std::endl;
  }
#endif
}

void 
Timer::start()
{
  if(state() == Stopped) {
    start_time_ = get_time();
    state_ = Running;
  } else {
    std::cerr << "Warning: Timer started while it was already running" << std::endl;
  }
}

void 
Timer::stop()
{
  if(state() == Stopped) {
    std::cerr << "Warning: Timer stopped while it was already stopped" << std::endl;
  } else {
    state_ = Stopped;
    double t = get_time();
    total_time_ += t - start_time_;
  }
}

void 
Timer::add(double t) {
  start_time_ -= t;
}

void 
Timer::clear()
{
  if(state() == Stopped){
    total_time_ = 0;
  } else {
    std::cerr << "Warning: Timer cleared while it was running" << std::endl;
    total_time_ = 0;
    start_time_ = get_time();
  }
}

double 
Timer::time()
{
  if(state() == Running){
    double t = get_time();
    return t - start_time_ + total_time_;
  } else {
    return total_time_;
  }
}


double 
CPUTimer::get_time()
{
#ifdef _WIN32
  static double cps = (double)CLOCKS_PER_SEC;
#else
  static double cps = (double)sysconf(_SC_CLK_TCK);
#endif
    //cerr << "CLOCKS_PER_SEC:" << CLOCKS_PER_SEC << "  cps:" << cps << std::endl;
  struct tms buffer;
  times(&buffer);
  double cpu_time = 
    double(buffer.tms_utime + buffer.tms_stime) / (double)cps;
  return cpu_time;
}

WallClockTimer::WallClockTimer()
{
}

double 
WallClockTimer::get_time()
{
  return Time::currentSeconds();
}

WallClockTimer::~WallClockTimer()
{
}

CPUTimer::~CPUTimer()
{
}

TimeThrottle::TimeThrottle()
{
}

TimeThrottle::~TimeThrottle()
{
}

void 
TimeThrottle::wait_for_time(double endtime)
{
  if(endtime==0)
      return;
  double time_now=time();
  double delta=endtime-time_now;
  if(delta <=0)
      return;
  Time::waitFor(delta);
}

} // end namespace SCIRun

#endif

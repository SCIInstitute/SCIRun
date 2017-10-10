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



///
///@file  Timer.h
///@brief Interface to portable timer utility classes
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  Feb. 1994
///

#if !defined(Timer_h)
#define Timer_h

#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

class SCISHARE Timer {
public:
  enum timer_state_e {
    Stopped,
    Running
  };

  Timer();
  virtual ~Timer();

  void               start();
  void               stop();
  void               clear();
  double             time();
  void               add(double t);
  timer_state_e      state() { return state_; }
protected:
  virtual double get_time() = 0;
private:
  double total_time_;
  double start_time_;
  timer_state_e state_;
};

class SCISHARE CPUTimer : public Timer {
public:
  virtual ~CPUTimer();
protected:
  virtual double get_time();
};

class SCISHARE WallClockTimer : public Timer {
public:
  WallClockTimer();
  virtual ~WallClockTimer();
protected:
  virtual double get_time();
};

class SCISHARE TimeThrottle : public WallClockTimer {
public:
  TimeThrottle();
  virtual ~TimeThrottle();

  void wait_for_time(double time);
};

} // end namespace SCIRun

#endif /* Timer_h */

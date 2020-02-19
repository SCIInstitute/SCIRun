/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
///@file   Time.h
///@brief  Utility class for dealing with time
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_Time_h
#define Core_Thread_Time_h

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {
/**************************************

@class
 Time

 KEYWORDS
 Thread, Time

@details
 Utility class to manage Time.  This class is implemented using
 high precision counters on the SGI, and standard unix system calls
 on other machines.

****************************************/
class SCISHARE Time {
public:
  typedef long long SysClock;

  //////////
  /// Return the current system time, in terms of clock ticks.
  /// Time zero is at some arbitrary point in the past.
  static SysClock currentTicks();

  //////////
  /// Return the current system time, in terms of seconds.
  /// This is slower than currentTicks().  Time zero is at
  /// some arbitrary point in the past.
  static double currentSeconds();

  //////////
  /// Return the conversion from seconds to ticks.
  static double ticksPerSecond();

  //////////
  /// Return the conversion from ticks to seconds.
  static double secondsPerTick();

  //////////
  /// Wait until the specified time in clock ticks.
  static void waitUntil(SysClock ticks);

  //////////
  /// Wait until the specified time in seconds.
  static void waitUntil(double seconds);

  //////////
  /// Wait for the specified time in clock ticks
  static void waitFor(SysClock ticks);

  //////////
  /// Wait for the specified time in seconds
  static void waitFor(double seconds);

private:
  Time();
  static void initialize();
};
} // End namespace SCIRun

#endif

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
///@file   Guard.h
///@brief  Automatically lock/unlock a mutex or crowdmonitor.
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_Guard_h
#define Core_Thread_Guard_h

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

class Mutex;
class RecursiveMutex;
class CrowdMonitor;

/**************************************

@class
  Guard

  KEYWORDS
  Thread

@details
  Utility class to lock and unlock a <b>Mutex</b> or a <b>CrowdMonitor</b>.
  The constructor of the <b>Guard</b> object will lock the mutex
  (or <b>CrowdMonitor</b>), and the destructor will unlock it.
  <p>
  This would be used like this:
  <blockquote><pre>
  {
  <blockquote>Guard mlock(&mutex);  // Acquire the mutex
  ... critical section ...</blockquote>
  } // mutex is released when mlock goes out of scope
  </pre></blockquote>

****************************************/
class SCISHARE Guard {
public:
  //////////
  /// Attach the <b>Guard</b> object to the <i>mutex</i>, and
  /// acquire the mutex.
  Guard(Mutex* mutex);

  Guard(RecursiveMutex* rmutex);

  enum Which {
    Read,
    Write
  };

  //////////
  /// Attach the <b>Guard</b> to the <i>CrowdMonitor</pre> and
  /// acquire one of the locks.  If <i>action</i> is
  /// <b>Guard::Read</b>, the read lock will be acquired, and if
  /// <i>action</i> is <b>Write</b>, then the write lock will be
  /// acquired.  The appropriate lock will then be released by the
  /// destructor
  Guard(CrowdMonitor* crowdMonitor, Which action);

  //////////
  /// Release the lock acquired by the constructor.
  ~Guard();
private:
  Mutex* mutex_;
  RecursiveMutex* rmutex_;
  CrowdMonitor* monitor_;
  Which action_;

  // Cannot copy them
  Guard(const Guard&);
  Guard& operator=(const Guard&);
};
} // End namespace SCIRun

#endif

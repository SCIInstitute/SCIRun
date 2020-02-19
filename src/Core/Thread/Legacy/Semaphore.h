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
///@file   Semaphore
///@breif  Basic semaphore primitive
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_Semaphore_h
#define Core_Thread_Semaphore_h

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

class Semaphore_private;

/**************************************

@class
 Semaphore

 KEYWORDS
 Thread

@detials
 Counting semaphore synchronization primitive.  A semaphore provides
 atomic access to a special counter.  The <i>up</i> method is used
 to increment the counter, and the <i>down</i> method is used to
 decrement the counter.  If a thread tries to decrement the counter
 when the counter is zero, that thread will be blocked until another
 thread calls the <i>up</i> method.

****************************************/
class SCISHARE Semaphore {
public:
  //////////
  /// Create the semaphore, and setup the initial <i>count.name</i>
  /// should be a static string which describes the primitive for
  /// debugging purposes.
  Semaphore(const char* name, int count);

  //////////
  /// Destroy the semaphore
  ~Semaphore();

  //////////
  /// Increment the semaphore count, unblocking up to <i>count</i>
  /// threads that may be blocked in the <i>down</i> method.
  void up(int count=1);

  //////////
  /// Decrement the semaphore count by <i>count</i>.  If the
  /// count is zero, this thread will be blocked until another
  /// thread calls the <i>up</i> method. The order in which
  /// threads will be unblocked is not defined, but implementors
  /// should give preference to those threads that have waited
  /// the longest.
  void down(int count=1);

  //////////
  /// Attempt to decrement the semaphore count by one, but will
  /// never block. If the count was zero, <i>tryDown</i> will
  /// return false. Otherwise, <i>tryDown</i> will return true.
  bool tryDown();

private:
  Semaphore_private* priv_;
  const char* name_;

  // Cannot copy them
  Semaphore(const Semaphore&);
  Semaphore& operator=(const Semaphore&);
};
} // End namespace SCIRun

#endif

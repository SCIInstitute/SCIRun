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
///@file  Barrier
///@brief Barrier synchronization primitive
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_RecursiveMutex_h
#define Core_Thread_RecursiveMutex_h

#include <Core/Thread/Legacy/Mutex.h>

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

class RecursiveMutex_private;
/**************************************

@class
 RecursiveMutex

 KEYWORDS
 Thread

@details
 Provides a recursive <b>Mut</b>ual <b>Ex</b>clusion primitive.  Atomic
 <b>lock()</b> and <b>unlock()</b> will lock and unlock the mutex.
 Nested calls to <b>lock()</b> by the same thread are acceptable,
 but must be matched with calls to <b>unlock()</b>.  This class
 may be less efficient that the <b>Mutex</b> class, and should not
 be used unless the recursive lock feature is really required.

****************************************/
class SCISHARE RecursiveMutex {
public:
  //////////
  /// Create the Mutex.  The Mutex is allocated in the unlocked
  /// state. <i>name</i> should be a static string which describe
  /// the primitive for debugging purposes.
  explicit RecursiveMutex(const char* name);

  //////////
  /// Destroy the Mutex.  Destroying a Mutex in the locked state
  /// has undefined results.
  ~RecursiveMutex();

  //////////
  /// Acquire the Mutex.  This method will block until the Mutex
  /// is acquired.
  void lock();

  //////////
  /// Attempt to acquire the Mutex without blocking.  Returns
  /// true if the mutex was available and actually acquired.
  bool tryLock();

  //////////
  /// Release the Mutex, unblocking any other threads that are
  /// blocked waiting for the Mutex.
  void unlock();

  friend class ConditionVariable;
private:
  const char* name_;
  RecursiveMutex_private* priv_;

  // Cannot copy them
  RecursiveMutex(const RecursiveMutex&);
  RecursiveMutex& operator=(const RecursiveMutex&);
};

} // End namespace SCIRun

#endif

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
///@file   Mutex.h
///@brief  Standard locking primitive
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_Mutex_h
#define Core_Thread_Mutex_h

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

struct Mutex_private;

/**************************************

@class
 Mutex

 KEYWORDS
 Thread

@details
 Provides a simple <b>Mut</b>ual <b>Ex</b>clusion primitive.  Atomic
 <b>lock()</b> and <b>unlock()</b> will lock and unlock the mutex.
 This is not a recursive Mutex (See <b>RecursiveMutex</b>), and calling
 lock() in a nested call will result in an error or deadlock.

****************************************/
class SCISHARE Mutex {
public:
  //////////
  /// Create the mutex.  The mutex is allocated in the unlocked
  /// state. <i>name</i> should be a static string which describes
  /// the primitive for debugging purposes.  
  explicit Mutex(const char* name);

  //////////
  /// Destroy the mutex.  Destroying the mutex in the locked state
  /// has undefined results.
  ~Mutex();

  //////////
  /// Acquire the Mutex.  This method will block until the mutex
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
  Mutex_private* priv_;
  const char* name_;

  // Cannot copy them
  Mutex(const Mutex&);
  Mutex& operator=(const Mutex&);
};

} // End namespace SCIRun

#endif



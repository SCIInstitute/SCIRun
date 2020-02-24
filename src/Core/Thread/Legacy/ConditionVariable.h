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
///@file  ConditionVariable
///@brief Condition variable primitive
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///@date  June 1997
///

#ifndef Core_Thread_ConditionVariable_h
#define Core_Thread_ConditionVariable_h

#include <Core/Thread/Legacy/Semaphore.h>
#include <Core/Thread/Legacy/Mutex.h>
#include <Core/Thread/Legacy/RecursiveMutex.h>

#include <Core/Thread/Legacy/share.h>

struct timespec;

namespace SCIRun {

class ConditionVariable_private;
struct CrowdMonitor_private;

/**************************************

 @class
  ConditionVariable

  KEYWORDS
  Thread

 @details
  Condition variable primitive.  When a thread calls the
  <i>wait</i> method,which will block until another thread calls
  the <i>conditionSignal</i> or <i>conditionBroadcast</i> methods.  When
  there are multiple threads waiting, <i>conditionBroadcast</i> will unblock
  all of them, while <i>conditionSignal</i> will unblock only one (an
  arbitrary one) of them.  This primitive is used to allow a thread
  to wait for some condition to exist, such as an available resource.
  The thread waits for that condition, until it is unblocked by another
  thread that caused the condition to exist (<i>i.e.</i> freed the
  resource).

****************************************/

class SCISHARE ConditionVariable {
public:
  //////////
  /// Create a condition variable. <i>name</i> should be a static
  /// string which describes the primitive for debugging purposes.
  ConditionVariable(const char* name);

  //////////
  /// Destroy the condition variable
  ~ConditionVariable();

  //////////
  /// Wait for a condition.  This method atomically unlocks
  /// <b>mutex</b>, and blocks.  The <b>mutex</b> is typically
  /// used to guard access to the resource that the thread is
  /// waiting for.
  void wait(Mutex& m);
  void wait(RecursiveMutex& m);

  //////////
  /// Wait for a condition.  This method atomically unlocks
  /// <b>mutex</b>, and blocks.  The <b>mutex</b> is typically
  /// used to guard access to the resource that the thread is
  /// waiting for.  If the time abstime is reached before
  /// the ConditionVariable is signaled, this will return
  /// false.  Otherewise it will return true.
  bool timedWait(Mutex& m, const struct ::timespec* abstime);
  bool timedWait(RecursiveMutex& m, const struct ::timespec* abstime);

  //////////
  /// Signal a condition.  This will unblock one of the waiting
  /// threads. No guarantee is made as to which thread will be
  /// unblocked, but thread implementations typically give
  /// preference to the thread that has waited the longest.
  void conditionSignal();

  //////////
  /// Signal a condition.  This will unblock all of the waiting
  /// threads. Note that only the number of waiting threads will
  /// be unblocked. No guarantee is made that these are the same
  /// N threads that were blocked at the time of the broadcast.
  void conditionBroadcast();

private:
  const char* name_;
  ConditionVariable_private* priv_;

  // Cannot copy them
  ConditionVariable(const ConditionVariable&);
  ConditionVariable& operator=(const ConditionVariable&);
};
} // End namespace SCIRun

#endif

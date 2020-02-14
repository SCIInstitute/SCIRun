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
///@file   Barrier.h
///@brief  Barrier synchronization primitive
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_Barrier_h
#define Core_Thread_Barrier_h

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

class Barrier_private;

/**************************************

@class
 Barrier

 KEYWORDS
 Thread

@details
 Barrier synchronization primitive.  Provides a single wait
 method to allow a set of threads to block at the barrier until all
 threads arrive.

 WARNING
 When the ThreadGroup semantics are used, other threads outside of the
 ThreadGroup should not access the barrier, or undefined behavior will
 result. In addition, threads should not be added or removed from the
 ThreadGroup while the Barrier is being accessed.

****************************************/

class SCISHARE Barrier {
public:
  //////////
  /// Create a barrier which will be used by a variable number
  /// of threads.   <tt>name</tt> should be a static string
  /// which describes the primitive for debugging purposes.
  explicit Barrier(const char* name);

  //////////
  /// Destroy the barrier
  virtual ~Barrier();

  //////////
  /// This causes all of the threads to block at this method
  /// until all numThreads threads have called the method.
  /// After all threads have arrived, they are all allowed
  /// to return.
  void wait(int numThreads);

protected:
private:
  Barrier_private* priv_;
  const char* name_;

  // Cannot copy them
  Barrier(const Barrier&);
  Barrier& operator=(const Barrier&);
};
} // End namespace SCIRun

#endif

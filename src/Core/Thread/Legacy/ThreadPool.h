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
///@file   ThreadPool
///@brief  A pool of threads
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   January 2000
///

#ifndef Core_Thread_ThreadPool_h
#define Core_Thread_ThreadPool_h

#include <Core/Thread/Legacy/Barrier.h>
#include <Core/Thread/Legacy/Mutex.h>
#include <Core/Thread/Legacy/Parallel.h>
#include <Core/Thread/Legacy/Parallel1.h>
#include <Core/Thread/Legacy/Parallel2.h>
#include <Core/Thread/Legacy/Parallel3.h>

#include <vector>


namespace SCIRun {
  class ThreadGroup;
class 	ThreadPoolHelper;
/**************************************

@class
 ThreadPool

 KEYWORDS
 ThreadPool

@details
 The ThreadPool class groups a bunch of worker threads.

****************************************/
class ThreadPool {
public:
  //////////
  /// Create a thread pool.  <tt>name</tt> should be a static
  /// string which describes the primitive for debugging purposes.
  ThreadPool(const char* name);

  //////////
  /// Destroy the pool and shutdown all threads
  ~ThreadPool();

  //////////
  /// Start up several threads that will run in parallel.
  /// The caller will block until all of the threads return.
  void parallel(const ParallelBase& helper, int nthreads);

  //////////
  /// Start up several threads that will run in parallel.
  /// The caller will block until all of the threads return.
  template<class T>
  void parallel(T* ptr, void (T::*pmf)(int), int numThreads) {
    parallel(Parallel<T>(ptr, pmf),
	     numThreads);
  }

  //////////
  /// Another overloaded version of parallel that passes 1 argument
  template<class T, class Arg1>
  void parallel(T* ptr, void (T::*pmf)(int, Arg1),
		int numThreads,
		Arg1 a1) {
    parallel(Parallel1<T, Arg1>(ptr, pmf, a1),
	     numThreads);
  }

  //////////
  /// Another overloaded version of parallel that passes 2 arguments
  template<class T, class Arg1, class Arg2>
  void parallel(T* ptr, void (T::* pmf)(int, Arg1, Arg2),
		int numThreads,
		Arg1 a1, Arg2 a2) {
    parallel(Parallel2<T, Arg1, Arg2>(ptr, pmf, a1, a2),
	     numThreads);
  }

  //////////
  /// Another overloaded version of parallel that passes 3 arguments
  template<class T, class Arg1, class Arg2, class Arg3>
  void parallel(T* ptr, void (T::* pmf)(int, Arg1, Arg2, Arg3),
		int numThreads,
		Arg1 a1, Arg2 a2, Arg3 a3) {
    parallel(Parallel3<T, Arg1, Arg2, Arg3>(ptr, pmf, a1, a2, a3),
	     numThreads);
  }

private:
  const char* name_;
  ThreadGroup* group_;
  Mutex lock_;
  std::vector<ThreadPoolHelper*> threads_;
  int numThreads_;
  Barrier barrier;
  friend class ThreadPoolHelper;
  void wait();

  // Cannot copy them
  ThreadPool(const ThreadPool&);
  ThreadPool& operator=(const ThreadPool&);
};
} // End namespace SCIRun

#endif

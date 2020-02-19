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
///@file  ThreadPool.cc
///@brief A pool of threads
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///@date  January 2000
///

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Thread/Legacy/ThreadPool.h>
#include <Core/Thread/Legacy/Barrier.h>
#include <Core/Thread/Legacy/Runnable.h>
#include <Core/Thread/Legacy/Semaphore.h>
#include <Core/Thread/Legacy/Thread.h>
#include <Core/Thread/Legacy/ThreadGroup.h>
#include <stdio.h>

namespace SCIRun {

class ThreadPoolHelper : public Runnable {
  const ParallelBase* helper;
  int proc;
  friend class ThreadPool;
  ThreadPool* pool;
  //Semaphore start_sema;
  //Semaphore done_sema;
public:
  ThreadPoolHelper(int proc, ThreadPool* pool)
    : helper(0), proc(proc), pool(pool)
    //start_sema("ThreadPool helper startup semaphore", 0),
    //done_sema("ThreadPool helper completion semaphore", 0)
  {
  }
  virtual ~ThreadPoolHelper() {}
  virtual void run() {
    for(;;){
      //start_sema.down();
      pool->wait();
      ParallelBase* cheat=(ParallelBase*)helper;
      cheat->run(proc);
      //done_sema.up();
      pool->wait();
    }
  }
};

ThreadPool::ThreadPool(const char* name)
  : name_(name), lock_("ThreadPool lock"), barrier("ThreadPool barrier")
{
  group_ = 0;
}

ThreadPool::~ThreadPool()
{
  // All of the threads will go away with this
  delete group_;
}

void ThreadPool::wait()
{
  barrier.wait((int)threads_.size()+1);
}

void
ThreadPool::parallel(const ParallelBase& helper, int nthreads)
{
  lock_.lock();
  if(nthreads >= (int)threads_.size()){
    if(!group_)
      group_=new ThreadGroup("Parallel group");
    int oldsize = (int)threads_.size();
    threads_.resize(nthreads);
    for(int i=oldsize;i<nthreads;i++){
      char buf[50];
      sprintf(buf, "Parallel thread %d of %d", i, nthreads);
      threads_[i] = new ThreadPoolHelper(i, this);
      Thread* t = new Thread(threads_[i], buf, group_,
			     Thread::Stopped);
      t->setDaemon(true);
      t->detach();
      t->migrate(i);
      t->resume();
    }
  }
  Thread::self()->migrate(nthreads%Thread::numProcessors());
  for(int i=0;i<nthreads;i++){
    threads_[i]->helper = &helper;
    //	threads_[i]->start_sema.up();
  }
  barrier.wait(nthreads+1);
  barrier.wait(nthreads+1);
  for(int i=0;i<nthreads;i++){
    //threads_[i]->done_sema.down();
    threads_[i]->helper = 0;
  }
  lock_.unlock();
}

} // End namespace SCIRun
#endif

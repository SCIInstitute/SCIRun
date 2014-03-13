/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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



/**
 *@class CrowdMonitor
 *@brief Multiple reader/single writer locks, implementation
 *   for pthreads simply wrapping pthread_rwlock_t
 *
 *@author
 *   Wayne Witzel
 *   Department of Computer Science
 *   University of Utah
 *@date March 2002
 *
 */

#include <Core/Thread/CrowdMonitor.h>
#include <pthread.h>

/*** WARNING: THIS IMPLEMENTATION HAS NOT BEEN TESTED - 3/6/2002 ***/

namespace SCIRun {

struct CrowdMonitor_private {
  pthread_rwlock_t lock_;
  CrowdMonitor_private();
  ~CrowdMonitor_private();
};

CrowdMonitor_private::CrowdMonitor_private()
{
  pthread_rwlock_init(&lock_, 0);
}

CrowdMonitor_private::~CrowdMonitor_private()
{
  pthread_rwlock_destroy(&lock_);
}

CrowdMonitor::CrowdMonitor(const char* name)
  : name_(name)
{
  if(!Thread::isInitialized()){
    if(getenv("THREAD_SHOWINIT"))
      fprintf(stderr, "CrowdMonitor: %s\n", name);
    Thread::initialize();
  }
  priv_=new CrowdMonitor_private;
}

CrowdMonitor::~CrowdMonitor()
{
  delete priv_;
  priv_=0;
}

void
CrowdMonitor::readLock()
{
  int oldstate=Thread::couldBlock(name_);
  pthread_rwlock_rdlock(&priv_->lock_);
  Thread::couldBlockDone(oldstate);
}

void
CrowdMonitor::readUnlock()
{
  pthread_rwlock_unlock(&priv_->lock_);  
}

void
CrowdMonitor::writeLock()
{
  int oldstate=Thread::couldBlock(name_);
  pthread_rwlock_wrlock(&priv_->lock_);
  Thread::couldBlockDone(oldstate);
} // End namespace SCIRun

void
CrowdMonitor::writeUnlock()
{
  pthread_rwlock_unlock(&priv_->lock_);  
}

} // End namespace SCIRun

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



/*
 *  CrowdMonitor: Multiple reader/single writer locks
 *
 *  Written by:
 *   Author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   Date: June 1997
 *
 */

#include <iostream>
#include <Core/Thread/Legacy/CrowdMonitor.h>
#include <Core/Thread/Legacy/Mutex.h>
#include <Core/Thread/Legacy/Thread.h>
#include <Core/Thread/Legacy/ConditionVariable.h>

namespace SCIRun {

struct CrowdMonitor_private {
  ConditionVariable write_waiters;
  ConditionVariable read_waiters;
  Mutex lock;
  int num_readers_waiting;
  int num_writers_waiting;
  int num_readers;
  int num_writers;
  CrowdMonitor_private();
  ~CrowdMonitor_private();
};

CrowdMonitor_private::CrowdMonitor_private()
  : write_waiters("CrowdMonitor write condition"),
    read_waiters("CrowdMonitor read condition"),
    lock("CrowdMonitor lock")
{
  num_readers_waiting=0;
  num_writers_waiting=0;
  num_readers=0;
  num_writers=0;
}

CrowdMonitor_private::~CrowdMonitor_private()
{
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
  priv_->lock.lock();
  while(priv_->num_writers > 0){
    priv_->num_readers_waiting++;
    int s=Thread::couldBlock(name_);
    priv_->read_waiters.wait(priv_->lock);
    Thread::couldBlockDone(s);
    priv_->num_readers_waiting--;
  }
  priv_->num_readers++;
  priv_->lock.unlock();
  Thread::couldBlockDone(oldstate);
}

void
CrowdMonitor::readUnlock()
{
  priv_->lock.lock();
  priv_->num_readers--;
  if(priv_->num_readers == 0 && priv_->num_writers_waiting > 0)
    priv_->write_waiters.conditionSignal();
  priv_->lock.unlock();
}

void
CrowdMonitor::writeLock()
{
  int oldstate=Thread::couldBlock(name_);
  priv_->lock.lock();
  while(priv_->num_writers || priv_->num_readers){
    // Have to wait...
    priv_->num_writers_waiting++;
    int s=Thread::couldBlock(name_);
    priv_->write_waiters.wait(priv_->lock);
    Thread::couldBlockDone(s);
    priv_->num_writers_waiting--;
  }
  priv_->num_writers++;
  priv_->lock.unlock();
  Thread::couldBlockDone(oldstate);
} // End namespace SCIRun

void
CrowdMonitor::writeUnlock()
{
  priv_->lock.lock();
  priv_->num_writers--;
  if(priv_->num_writers_waiting)
    priv_->write_waiters.conditionSignal(); // Wake one of them up...
  else if(priv_->num_readers_waiting)
    priv_->read_waiters.conditionBroadcast(); // Wake all of them up...
  priv_->lock.unlock();
}

} // End namespace SCIRun

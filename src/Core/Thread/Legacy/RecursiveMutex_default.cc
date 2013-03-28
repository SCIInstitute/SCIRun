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
 *  Barrier: Barrier synchronization primitive (default implementation)
 *
 *  Written by:
 *   Author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   Date: June 1997
 *
 */

#include <Core/Thread/Legacy/RecursiveMutex.h>
#include <Core/Thread/Legacy/Thread.h>

#include <stdio.h>

namespace SCIRun {

struct RecursiveMutex_private {
  Mutex mylock;
  Thread* owner;
  int lock_count;
  RecursiveMutex_private(const char* name);
  ~RecursiveMutex_private();
};

RecursiveMutex_private::RecursiveMutex_private(const char* name)
  : mylock(name)
{
  owner=0;
  lock_count=0;
}

RecursiveMutex_private::~RecursiveMutex_private()
{
}

RecursiveMutex::RecursiveMutex(const char* name)
  : name_(name)
{
  if(!Thread::isInitialized()){
    Thread::initialize();
  }
  priv_=new RecursiveMutex_private(name);
}

RecursiveMutex::~RecursiveMutex()
{
  delete priv_;
  priv_=0;
}

void
RecursiveMutex::lock()
{
  Thread* me=Thread::self();
  if(priv_->owner == me)
  {
    priv_->lock_count++;
    return;
  }
  priv_->mylock.lock();
  priv_->owner=me;
  priv_->lock_count=1;
}

bool
RecursiveMutex::tryLock()
{
  Thread* me=Thread::self();
  if(priv_->owner == me)
  {
    priv_->lock_count++;
    return (true);
  }
  
  if (priv_->mylock.tryLock())
  {
    priv_->owner=me;
    priv_->lock_count=1;
    return (true);
  }
  return (false);
}

void
RecursiveMutex::unlock()
{
  if(--priv_->lock_count == 0)
  {
    priv_->owner=0;
    priv_->mylock.unlock();
  }
}

} // End namespace SCIRun


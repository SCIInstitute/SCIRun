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
 *  AtomicCounter: Thread-safe integer variable
 *
 *  Written by:
 *   Author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   Date: June 1997
 *
 */

#include <Core/Thread/Legacy/Thread.h>
#include <Core/Thread/Legacy/ConditionVariable.h>
#include <Core/Thread/Legacy/Mutex.h>
#include <Core/Thread/Legacy/AtomicCounter.h>

namespace SCIRun {
struct AtomicCounter_private {
  Mutex lock;
  int value;
  AtomicCounter_private();
  ~AtomicCounter_private();
};
}

using SCIRun::AtomicCounter_private;
using SCIRun::AtomicCounter;

AtomicCounter_private::AtomicCounter_private()
    : lock("AtomicCounter lock")
{
}

AtomicCounter_private::~AtomicCounter_private()
{
}

AtomicCounter::AtomicCounter(const char* name)
    : name_(name)
{
  if(!Thread::isInitialized())
  {
    Thread::initialize();
  }
  priv_=new AtomicCounter_private;
}

AtomicCounter::AtomicCounter(const char* name, int value)
    : name_(name)
{
  priv_=new AtomicCounter_private;
  priv_->value=value;
}

AtomicCounter::~AtomicCounter()
{
  delete priv_;
  priv_=0;
}

AtomicCounter::operator int() const
{
    return priv_->value;
}

int
AtomicCounter::operator++()
{
  int oldstate = Thread::couldBlock(name_);
  priv_->lock.lock();
  int ret=++priv_->value;
  priv_->lock.unlock();
  Thread::couldBlockDone(oldstate);
  return ret;
}

int
AtomicCounter::operator++(int)
{
  int oldstate = Thread::couldBlock(name_);
  priv_->lock.lock();
  int ret=priv_->value++;
  priv_->lock.unlock();
  Thread::couldBlockDone(oldstate);
  return ret;
}

int
AtomicCounter::operator--()
{
  int oldstate = Thread::couldBlock(name_);
  priv_->lock.lock();
  int ret=--priv_->value;	
  priv_->lock.unlock();
  Thread::couldBlockDone(oldstate);
  return ret;
}

int
AtomicCounter::operator--(int)
{
  int oldstate = Thread::couldBlock(name_);
  priv_->lock.lock();
  int ret=priv_->value--;
  priv_->lock.unlock();
  Thread::couldBlockDone(oldstate);
  return ret;
} 

void
AtomicCounter::set(int v)
{
  int oldstate=Thread::couldBlock(name_);
  priv_->lock.lock();
  priv_->value=v;
  priv_->lock.unlock();
  Thread::couldBlockDone(oldstate);
}

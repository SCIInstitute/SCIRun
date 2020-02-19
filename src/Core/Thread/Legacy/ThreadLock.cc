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
///@file  TCLTask.cc:
///@brief Mutex that is lockable multiple times within the same thread
///@author
///       McKAy Davis / Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  August 1994
///

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Thread/Legacy/ThreadLock.h>
#include <Core/Utils/Legacy/Assert.h>

namespace SCIRun {

ThreadLock::ThreadLock(const char *name) :
  mutex_(name),
  owner_(0),
  count_(0)
{
}

void
ThreadLock::lock()
{
  Thread *self = Thread::self();
  ASSERT(self != 0);
  if(owner_ == self) {
    count_++;
    return;
  }
  mutex_.lock();
  owner_ = self;
  count_ = 1;
}

void
ThreadLock::unlock()
{
  ASSERT(count_ > 0);
  ASSERT(Thread::self() == owner_);

  if(--count_ == 0) {
    owner_ = 0;
    mutex_.unlock();
  }
}

int
ThreadLock::try_lock()
{
  Thread *self = Thread::self();
  if(owner_ == self) {
    count_++;
    return 1;
  }
  if(mutex_.tryLock()) {
    owner_ = self;
    count_ = 1;
    return 1;
  }
  return 0;
}


} // End namespace SCIRun

#endif

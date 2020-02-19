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
///@class Guard
///@brief Automatically lock/unlock a mutex or crowdmonitor.
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///@date  June 1997
///

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#ifndef WIN32

#include <Core/Thread/Legacy/Guard.h>
#include <Core/Thread/Legacy/CrowdMonitor.h>
#include <Core/Thread/Legacy/Mutex.h>
#include <Core/Thread/Legacy/RecursiveMutex.h>


namespace SCIRun {

Guard::Guard(Mutex* mutex)
    : mutex_(mutex), rmutex_(0), monitor_(0)
{
  mutex_->lock();
}

Guard::Guard(RecursiveMutex* rmutex)
    : mutex_(0), rmutex_(rmutex), monitor_(0)
{
  rmutex_->lock();
}

Guard::Guard(CrowdMonitor* crowd_monitor, Which action)
    : mutex_(0), monitor_(crowd_monitor), action_(action)
{
    if (action_ == Read) {
      monitor_->readLock();
    }
    else {
      monitor_->writeLock();
    }
}

Guard::~Guard()
{
    if (mutex_) {
      mutex_->unlock();
    }
    else if (rmutex_) {
      rmutex_->unlock();
    }
    else if (action_ == Read) {
      monitor_->readUnlock();
    }
    else {
      monitor_->writeUnlock();
    }
}


} // End namespace SCIRun

#endif
#endif

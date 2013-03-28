/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
 *  Barrier: Barrier synchronization primitive
 *
 *  Written by:
 *   Author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   Date: June 1997
 *
 */


#include <Core/Thread/Legacy/Thread.h>
#include <Core/Thread/Legacy/Barrier.h>
#include <Core/Thread/Legacy/ConditionVariable.h>
#include <Core/Thread/Legacy/Mutex.h>

namespace SCIRun {
struct Barrier_private {
  Mutex mutex;
  ConditionVariable cond;
  int nwait;
  
  Barrier_private();
  ~Barrier_private();
};
}

using SCIRun::Barrier_private;
using SCIRun::Barrier;

Barrier_private::Barrier_private()
    : mutex("Barrier lock"),
      cond("Barrier condition"),
      nwait(0)
{
}

Barrier_private::~Barrier_private()
{
}

Barrier::Barrier(const char* name)
    : name_(name)
{
  if(!Thread::isInitialized())
  {
    Thread::initialize();
  }
  priv_=new Barrier_private;
}

Barrier::~Barrier()
{
  delete priv_;
  priv_=0;
}

void
Barrier::wait(int n)
{
  int oldstate=Thread::couldBlock(name_);

  // Lock the counter and add one until
  // we reached the number of processes
  priv_->mutex.lock();
  priv_->nwait++;
  if(priv_->nwait == n)
  {
    // We are the last thread 
    // now wake everybody up...
    priv_->nwait=0;
    priv_->cond.conditionBroadcast();
  } 
  else 
  {
    // Wait until the last one signals us
    priv_->cond.wait(priv_->mutex);
  }
  priv_->mutex.unlock();
  Thread::couldBlockDone(oldstate);
}

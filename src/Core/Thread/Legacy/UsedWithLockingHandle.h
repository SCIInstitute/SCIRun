/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Core/Thread/Legacy/ UsedWithLockingHandle.h
#ifndef Core_Thread_UsedWithLockingHandle_h
#define Core_Thread_UsedWithLockingHandle_h

#include <Core/Thread/Legacy/Mutex.h>
#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

class SCISHARE UsedWithLockingHandleBase 
{
public:
  virtual ~UsedWithLockingHandleBase();
};

template <class LockType>
class UsedWithLockingHandle : public UsedWithLockingHandleBase
{
public:
  explicit UsedWithLockingHandle(const char* name) : ref_cnt(0), lock(name) {}
  int ref_cnt;
  LockType lock;
};

class SCISHARE UsedWithLockingHandleAndMutex : public UsedWithLockingHandleBase
{
public:
  explicit UsedWithLockingHandleAndMutex(const char* name) : ref_cnt(0), lock(name) {}
  int ref_cnt;
  Mutex lock;
};

template <class LockType>
class UsedWithLockingHandle<LockType&> : public UsedWithLockingHandleBase
{
public:
  explicit UsedWithLockingHandle(LockType& l) : ref_cnt(0), lock(l) {}
  int ref_cnt;
  LockType& lock;
};


} // End namespace SCIRun

#endif



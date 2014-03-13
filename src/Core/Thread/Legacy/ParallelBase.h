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
 *@file  ParallelBase.h
 *@brief Helper class to instantiate several threads
 *
 *   @author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   @date: June 1997
 *
 */

#ifndef Core_Thread_ParallelBase_h
#define Core_Thread_ParallelBase_h

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

class Semaphore;
/**************************************

@class
 ParallelBase

 KEYWORDS
 Thread

@details
 Helper class for Parallel class.  This will never be used
 by a user program.  See <b>Parallel</b> instead.
   
****************************************/
class SCISHARE ParallelBase {
public:
  //////////
  /// <i>The thread body</i>
  virtual void run(int proc)=0;

protected:
  ParallelBase();
  virtual ~ParallelBase();
  mutable Semaphore* wait_; // This may be modified by Thread::parallel
  friend class Thread;

private:
  // Cannot copy them
  ParallelBase(const ParallelBase&);
  ParallelBase& operator=(const ParallelBase&);
};
} // End namespace SCIRun

#endif




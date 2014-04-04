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



///
///@file   Parallel1.h
///@brief  Automatically instantiate several threads, with 1 argument
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 2000
///

#ifndef Core_Thread_Parallel1_h
#define Core_Thread_Parallel1_h

#include <Core/Thread/Legacy/ParallelBase.h>
#include <Core/Thread/Legacy/Semaphore.h>

namespace SCIRun {
/**************************************

@class
 Parallel1

 KEYWORDS
 Thread

@details
 Helper class to make instantiating threads to perform a parallel
 task easier.
   
****************************************/
template<class T, class Arg1> class Parallel1  : public ParallelBase {
public:
  //////////
  /// Create a Parallel1 object, using the specified member
  /// function instead of <i>Parallel1</i>.  This will
  /// typically be used like:
  /// <b><pre>Thread::Parallel1(Parallel1&lt;MyClass&gt;(this, &amp;MyClass::mymemberfn), nthreads);</pre></b>
  Parallel1(T* obj, void (T::*pmf)(int, Arg1), Arg1 a1);
	    
  //////////
  /// Destroy the Parallel1 object - the threads will remain alive.
  virtual ~Parallel1();
  T* obj_;
  void (T::*pmf_)(int, Arg1);
  Arg1 a1;
protected:
  virtual void run(int proc);
private:
  // Cannot copy them
  Parallel1(const Parallel1&);
  Parallel1<T, Arg1>& operator=(const Parallel1<T, Arg1>&);
};

template<class T, class Arg1>
void
Parallel1<T, Arg1>::run(int proc)
{
    // Copy out do make sure that the call is atomic
    T* obj=obj_;
    void (T::*pmf)(int, Arg1) = pmf_;
    if(wait_)
	wait_->up();
    (obj->*pmf)(proc, a1);
    // Cannot do anything here, since the object may be deleted by the
    // time we return
}

template<class T, class Arg1>
Parallel1<T, Arg1>::Parallel1(T* obj,
			      void (T::*pmf)(int, Arg1),
			      Arg1 a1)
  : obj_(obj), pmf_(pmf), a1(a1)
{
  wait_=0; // This may be set by Thread::parallel
} // End namespace SCIRun

template<class T, class Arg1>
Parallel1<T, Arg1>::~Parallel1()
{
}

} // End namespace SCIRun

#endif



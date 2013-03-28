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
 *  Parallel3: Automatically instantiate several threads, with 3 arguments
 *
 *  Written by:
 *   Author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   Date: June 3997
 *
 */

#ifndef Core_Thread_Parallel3_h
#define Core_Thread_Parallel3_h

#include <Core/Thread/Legacy/ParallelBase.h>
#include <Core/Thread/Legacy/Semaphore.h>

namespace SCIRun {
/**************************************

 CLASS
 Parallel3

 KEYWORDS
 Thread

 DESCRIPTION
 Helper class to make instantiating threads to perform a parallel
 task easier.
   
****************************************/
template<class T, class Arg1, class Arg2, class Arg3> class Parallel3  : public ParallelBase {
public:
  //////////
  // Create a Parallel3 object, using the specified member
  // function instead of <i>Parallel3</i>.  This will
  // typically be used like:
  // <b><pre>Thread::Parallel3(Parallel3&lt;MyClass&gt;(this, &amp;MyClass::mymemberfn), nthreads);</pre></b>
  Parallel3(T* obj, void (T::*pmf)(int, Arg1, Arg2, Arg3), Arg1 a1, Arg2 a2, Arg3 a3);
	    
  //////////
  // Destroy the Parallel3 object - the threads will remain alive.
  virtual ~Parallel3();
  T* obj_;
  void (T::*pmf_)(int, Arg1, Arg2, Arg3);
  Arg1 a1;
  Arg2 a2;
  Arg3 a3;
protected:
  virtual void run(int proc);
private:
  // Cannot copy them
  Parallel3(const Parallel3&);
  Parallel3<T, Arg1, Arg2, Arg3>& operator=(const Parallel3<T, Arg1, Arg2, Arg3>&);
};

template<class T, class Arg1, class Arg2, class Arg3>
void
Parallel3<T, Arg1, Arg2, Arg3>::run(int proc)
{
    // Copy out do make sure that the call is atomic
    T* obj=obj_;
    void (T::*pmf)(int, Arg1, Arg2, Arg3) = pmf_;
    if(wait_)
	wait_->up();
    (obj->*pmf)(proc, a1, a2, a3);
    // Cannot do anything here, since the object may be deleted by the
    // time we return
}

template<class T, class Arg1, class Arg2, class Arg3>
Parallel3<T, Arg1, Arg2, Arg3>::Parallel3(T* obj,
				         void (T::*pmf)(int, Arg1, Arg2, Arg3),
					 Arg1 a1, Arg2 a2, Arg3 a3) : 
  obj_(obj), 
  pmf_(pmf), 
  a1(a1), 
  a2(a2), 
  a3(a3)
{
  wait_=0; // This may be set by Thread::parallel
} // End namespace SCIRun

template<class T, class Arg1, class Arg2, class Arg3>
Parallel3<T, Arg1, Arg2, Arg3>::~Parallel3()
{
}

} //End namespace SCIRun

#endif



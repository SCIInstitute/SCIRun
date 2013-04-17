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
 *  Parallel: Automatically instantiate several threads
 *
 *  Written by:
 *   Author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   Date: June 1997
 *
 */

#ifndef Core_Thread_Parallel_h
#define Core_Thread_Parallel_h

#include <Core/Thread/Legacy/ParallelBase.h>
#include <Core/Thread/Legacy/Semaphore.h>

namespace SCIRun {
/**************************************
 
  CLASS
  Parallel
   
  KEYWORDS
  Thread
   
  DESCRIPTION
  Helper class to make instantiating threads to perform a parallel
  task easier.
   
****************************************/
template<class T> class Parallel  : public ParallelBase {
public:
  //////////
  // Create a parallel object, using the specified member
  // function instead of <i>parallel</i>.  This will
  // typically be used like:
  // <b><pre>Thread::parallel(Parallel&lt;MyClass&gt;(this, &amp;MyClass::mymemberfn), nthreads);</pre></b>
  Parallel(T* obj, void (T::*pmf)(int));
	    
  //////////
  // Destroy the Parallel object - the threads will remain alive.
  virtual ~Parallel();
  T* obj_;
  void (T::*pmf_)(int);
protected:
  virtual void run(int proc);
private:
  // Cannot copy them
  Parallel(const Parallel&);
  Parallel<T>& operator=(const Parallel<T>&);
};

template<class T>
void
Parallel<T>::run(int proc)
{
    // Copy out do make sure that the call is atomic
    T* obj=obj_;
    void (T::*pmf)(int) = pmf_;
    if(wait_)
	wait_->up();
    (obj->*pmf)(proc);
    // Cannot do anything here, since the object may be deleted by the
    // time we return
}

template<class T>
Parallel<T>::Parallel(T* obj, void (T::*pmf)(int))
    : obj_(obj), pmf_(pmf)
{
    wait_=0; // This may be set by Thread::parallel
} // End namespace SCIRun

template<class T>
Parallel<T>::~Parallel()
{
}

} //End namespace SCIRun

#endif





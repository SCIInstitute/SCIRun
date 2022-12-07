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
///@file   Reducer.h
///@brief  A barrier with reduction operations
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_Reducer_h
#define Core_Thread_Reducer_h

#include <Core/Thread/Legacy/Barrier.h>

namespace SCIRun {

/**************************************

@class
   Reducer

KEYWORDS
   Thread

@details
   Perform reduction operations over a set of threads.  Reduction
   operations include things like global sums, global min/max, etc.
   In these operations, a local sum (operation) is performed on each
   thread, and these sums are added together.

****************************************/
	template<class T> class Reducer : public Barrier {
	public:
	    //////////
	    /// The function that performs the reduction
	    typedef T (*ReductionOp)(const T&, const T&);

	    //////////
	    /// Create a <b> Reducer</i>.
	    /// At each operation, a barrier wait is performed, and the
	    /// operation will be performed to compute the global balue.
	    /// <i>name</i> should be a static string which describes
	    /// the primitive for debugging purposes.
	    /// op is a function which will compute a reduced value from
	    /// a pair of values.  op should be associative and commutative,
	    /// even up to floating point errors.
	    Reducer(const char* name, ReductionOp);

	    //////////
	    /// Destroy the Reducer and free associated memory.
	    virtual ~Reducer();

	    //////////
	    /// Performs a global reduction over all of the threads.  As
	    /// soon as each thread has called reduce with their local value,
	    /// each thread will return the same global reduced value.
	    T reduce(int myrank, int numThreads, const T& value);

	private:
	    T (*f_op)(const T&, const T&);
	    struct DataArray {
		// We want this on it's own cache line
		T data_;
		// Assumes 128 bytes in a cache line...
		char filler_[128];
	    };
	    DataArray* join_[2];

	    struct BufArray {
		int which;
		char filler_[128-sizeof(int)];
	    };
	    BufArray* p_;

	    int array_size_;
	    void collectiveResize(int proc, int numThreads);
	    void allocate(int size);

	    // Cannot copy them
	    Reducer(const Reducer<T>&);
	    Reducer<T>& operator=(const Reducer<T>&);
	};
    }
}

template<class T>
Reducer<T>::Reducer(const char* name, ReductionOp op)
    : Barrier(name), f_op(op)
{
    array_size_=-1;
    p_=0;
}

template<class T>
void
Reducer<T>::allocate(int n)
{
    join_[0]=new DataArray[2*numThreads+2]-1;
    join_[1]=join_[0]+numThreads;
    p_=new BufArray[num_threads_+2]+1;
    for(int i=0;i<num_threads_;i++)
        p_[i].whichBuffer_=0;
    array_size_=n;
}

template<class T>
Reducer<T>::~Reducer()
{
    if(p_){
	delete[] (void*)(join_[0]-1);
	delete[] (void*)(p_-1);
    }
}

template<class T>
void
Reducer<T>::collectiveResize(int proc, int n)
{
    // Extra barrier here to change the array size...

    // We must wait until everybody has seen the array size change,
    // or they will skip down too soon...
    wait(n);
    if(proc==0){
	delete[] (void*)(join_[0]-1);
	delete[] (void*)(p_-1);
	allocate(n);
	array_size_=n;
    }
    wait(n);
}

template<class T>
T
Reducer<T>::reduce(int proc, int n, const T& myresult)
{
    if(n != array_size_){
        collectiveResize(proc, n);
} // End namespace SCIRun
    if(n<=1)
	return myresult;

    int buf=p_[proc].whichBuffer_;
    p_[proc].whichBuffer_=1-buf;

    dataArray* j=join_[buf];
    j[proc].data_=myresult;
    wait(n);
    T red=j[0].data_;
    for(int i=1;i<n;i++)
        red=(*f_op)(red, j[i].data_);
    return red;

#endif

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
///@file   SimpleReducer.h
///@brief  A barrier with reduction operations
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_SimpleReducer_h
#define Core_Thread_SimpleReducer_h

#include <Core/Thread/Legacy/Barrier.h>

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

/**************************************

@class
   SimpleReducer

KEYWORDS
   Thread

@details
   Perform reduction operations over a set of threads.  Reduction
   operations include things like global sums, global min/max, etc.
   In these operations, a local sum (operation) is performed on each
   thread, and these sums are added together.

****************************************/
	class SCISHARE SimpleReducer : public Barrier {
	public:
	    //////////
	    /// Create a <b> SimpleReducer</i>.
	    /// At each operation, a barrier wait is performed, and the
	    /// operation will be performed to compute the global balue.
	    /// <i>name</i> should be a static string which describes
	    /// the primitive for debugging purposes.
	    SimpleReducer(const char* name);

	    //////////
	    /// Destroy the SimpleReducer and free associated memory.
	    virtual ~SimpleReducer();

	    //////////
	    /// Performs a global sum over all of the threads.  As soon as each
	    /// thread has called sum with their local sum, each thread will
	    /// return the same global sum.
	    double sum(int myrank, int numThreads, double mysum);

	    //////////
	    /// Performs a global max over all of the threads.  As soon as each
	    /// thread has called max with their local max, each thread will
	    /// return the same global max.
	    double max(int myrank, int numThreads, double mymax);

	    //////////
	    /// Performs a global min over all of the threads.  As soon as each
	    /// thread has called min with their local max, each thread will
	    /// return the same global max.
	    double min(int myrank, int numThreads, double mymax);

	private:
	    struct data {
		double d_;
	    };
	    struct joinArray {
		data d_;
		// Assumes 128 bytes in a cache line...
		char filler_[128-sizeof(data)];
	    };
	    struct pdata {
		int buf_;
		char filler_[128-sizeof(int)];
	    };
	    joinArray* join_[2];
	    pdata* p_;
	    int array_size_;
	    void collectiveResize(int proc, int numThreads);

	    // Cannot copy them
	    SimpleReducer(const SimpleReducer&);
	    SimpleReducer& operator=(const SimpleReducer&);
	};
} // End namespace SCIRun

#endif

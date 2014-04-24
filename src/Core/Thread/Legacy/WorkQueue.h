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
///@file   WorkQueue.h
///@brief  Manage assignments of work
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_WorkQueue_h
#define Core_Thread_WorkQueue_h

#include <Core/Thread/Legacy/AtomicCounter.h>

#include <vector>


#include <Core/Thread/Legacy/share.h>
namespace SCIRun {
	struct WorkQueue_private;
	
/**************************************
 
@class
   WorkQueue
   
KEYWORDS
   Thread, Work
   
@details
   Doles out work assignment to various worker threads.  Simple
   attempts are made at evenly distributing the workload.
   Initially, assignments are relatively large, and will get smaller
   towards the end in an effort to equalize the total effort.
   
****************************************/
	class SCISHARE WorkQueue {
	public:
	    //////////
	    /// Make an empty work queue with no assignments.
	    WorkQueue(const char* name);
	    
	    //////////
	    /// Fill the work queue with the specified total number of work
	    /// assignments.  <i>num_threads</i> specifies the approximate
	    /// number of threads which will be working from this queue.
	    /// The optional <i>granularity</i> specifies the degree to
	    /// which the tasks are divided.  A large granularity will
	    /// create more assignments with smaller assignments.  A
	    /// granularity of zero will recieve a single assignment of
	    /// approximately uniform size.  <i>name</i> should be a static
	    /// string which describes the primitive for debugging purposes.
	    void refill(int totalAssignments, int nthreads,
			int granularity=5);
	    
	    //////////
	    /// Destroy the work queue.  Any unassigned work will be lost.  
	    ~WorkQueue();
	    
	    //////////
	    /// Called by each thread to get the next assignment.  If
	    /// <i>nextAssignment</i> returns true, the thread has a valid
	    /// assignment, and then would be responsible for the work 
	    /// from the returned <i>start</i> through <i>end-l</i>.
	    /// Assignments can range from 0 to  <i>totalAssignments</i>-1.
	    /// When <i>nextAssignment</i> returns false, all work has
	    /// been assigned.
	    bool nextAssignment(int& start, int& end);
	    
	private:
	    const char* name_;
	    int num_threads_;
	    int total_assignments_;
	    int granularity_;
	    std::vector<int> assignments_;
	    AtomicCounter current_assignment_;
	    bool dynamic_;
	    
	    void fill();

	    // Cannot copy them
	    WorkQueue(const WorkQueue& copy);
	    WorkQueue& operator=(const WorkQueue&);
	};
} // End namespace SCIRun

#endif


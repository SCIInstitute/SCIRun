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
///@file   ThreadGroup.h
///@brief  A set of threads
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_ThreadGroup_h
#define Core_Thread_ThreadGroup_h

#include <Core/Thread/Legacy/Mutex.h>

#include <vector>

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {

class Thread;

/**************************************

@class
   ThreadGroup

KEYWORDS
   Thread

@details
   A group of threads that are linked together for scheduling
   and control purposes.  The threads may be stopped, resumed
   and alerted simultaneously.

****************************************/
	class SCISHARE ThreadGroup {
	public:

	    //////////
	    /// Create a thread group with the specified <i>name</i>.
	    /// <i>parentGroup</i> specifies the parent <b>ThreadGroup</b>
	    /// which defaults to the default top-level group.
	    ThreadGroup(const char* name, ThreadGroup* parentGroup=0);

	    //////////
	    /// Destroy the thread group.  All of the running threads
	    /// should be stopped before the <b>ThreadGroup</b> is destroyed.
	    ~ThreadGroup();

	    //////////
	    /// Return a snapshot of the number of living threads.  If
	    /// <i>countDaemon</i> is true, then daemon threads will be
	    /// included in the count.
	    int numActive(bool countDaemon);

	    //////////
	    /// Stop all of the threads in this thread group
	    void stop();

	    //////////
	    /// Resume all of the threads in this thread group
	    void resume();

	    //////////
	    /// Wait until all of the threads have completed.
	    void join();

	    //////////
	    /// Detach the thread, joins are no longer possible.
	    void detach();

	    //////////
	    /// Return the parent <b>ThreadGroup.</b>  Returns null if
	    /// this is the default threadgroup.
	    ThreadGroup* parentGroup();

	    //////////
	    /// Arrange to have the threadGroup gang scheduled, so that
	    /// all of the threads will be executing at the same time if
	    /// multiprocessing resources permit.  This interface will
	    /// typically be employed by the <i>Thread::parallel</i>
	    /// static method, and will typically not be called directly
	    /// by user code.  Threads added to the group after this call
	    /// may or may not be included in the schedule gang.
	    void gangSchedule();

	protected:
	    friend class Thread;
	    static ThreadGroup* s_default_group;

	private:
	    Mutex lock_;
	    const char* name_;
	    ThreadGroup* parent_;
	    std::vector<ThreadGroup*> groups_;
	    std::vector<Thread*> threads_;
	    void addme(ThreadGroup* t);
	    void addme(Thread* t);

	    // Cannot copy them
	    ThreadGroup(const ThreadGroup&);
	    ThreadGroup& operator=(const ThreadGroup&);
	};
} // End namespace SCIRun

#endif

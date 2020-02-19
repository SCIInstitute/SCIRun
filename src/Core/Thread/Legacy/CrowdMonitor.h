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
///@file   CrowdMonitor.h
///@brief  Multiple reader/single writer locks
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_CrowdMonitor_h
#define Core_Thread_CrowdMonitor_h

#include <Core/Thread/Legacy/share.h>

namespace SCIRun {
  struct CrowdMonitor_private;
/**************************************

@class
   CrowdMonitor

KEYWORDS
   Thread

@details
   Multiple reader, single writer synchronization primitive.  Some
   applications do not need the strict level of synchronization
   provided by the <b>Mutex</b>.  The <b>CrowdMonitor</b> relaxes
   the synchronization by allowing multiple threads access to a
   resource (usually a data area), on the condition that the thread
   will only read the data.  When a thread needs to write the data,
   it can access the monitor in write mode (using <i>writeLock</i>).
   At any given time, one writer thread can be active, or multiple
   reader threads can be active.  <b>CrowdMonitor</b> guards against
   multiple writers accessing a data, and against a thread writing
   to the data while other threads are reading it.

WARNING
   <p> Calling <i>readLock</i> within a <i>writeLock/write_unlock</i>
   section may result in a deadlock.  Likewise, calling <i>writeLock</i>
   within a <i>readLock/readUnlock</i> section may result in a deadlock.
   Calling <i>readUnlock</i> or <i>writeUnlock</i> when the lock is
   not held is not legal and may result in undefined behavior.

****************************************/

	class SCISHARE CrowdMonitor {
	public:
	    //////////
	    /// Create and initialize the CrowdMonitor. <i>name</i> should
	    /// be a static which describes the primitive for debugging
	    /// purposes.
	    CrowdMonitor(const char* name);

	    //////////
	    /// Destroy the CrowdMonitor.
	    ~CrowdMonitor();

	    //////////
	    /// Acquire the read-only lock associated with this
	    /// <b>CrowdMonitor</b>. Multiple threads may hold the
	    /// read-only lock simultaneously.
	    void readLock();

	    //////////
	    /// Release the read-only lock obtained from <i>readLock</i>.
	    /// Undefined behavior may result when <i>readUnlock</i> is
	    /// called and a <i>readLock</i> is not held by the calling
	    /// Thread.
	    void readUnlock();

	    //////////
	    /// Acquire the write lock associated with this
	    /// <b>CrowdMonitor</b>. Only one thread may hold the write
	    /// lock, and during the time that this lock is not held, no
	    /// threads may hold the read-only lock.
	    void writeLock();

	    //////////
	    /// Release the write-only lock obtained from <i>writeLock</i>.
	    /// Undefined behavior may result when <i>writeUnlock</i> is
	    /// called and a <i>writeLock</i> is not held by the calling
	    /// Thread.
	    void writeUnlock();

	private:
	    const char* name_;
	    CrowdMonitor_private* priv_;

	    // Cannot copy them
	    CrowdMonitor(const CrowdMonitor&);
	    CrowdMonitor& operator=(const CrowdMonitor&);
	};
} // End namespace SCIRun

#endif

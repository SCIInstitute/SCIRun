/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// @file  Thread.h
/// @brief The thread class
/// 
/// @author Steve Parker
///    Department of Computer Science
///    University of Utah
/// @date June 1997
/// 

#ifndef Core_Thread_Thread_h
#define Core_Thread_Thread_h

//#include <sci_defs/bits_defs.h>
#include <Core/Thread/Legacy/Parallel.h>
#include <Core/Thread/Legacy/Parallel1.h>
#include <Core/Thread/Legacy/Parallel2.h>
#include <Core/Thread/Legacy/Parallel3.h>
#include <Core/Thread/Legacy/Parallel4.h>
#include <Core/Thread/Legacy/share.h>

#ifndef _WIN32
#include <signal.h>
#endif

namespace SCIRun {

struct Thread_private;
class ParallelBase;
class Runnable;
class ThreadGroup;
	

/// The Thread class provides a new context in which to run.  A single
/// Runnable class is attached to a single Thread class, which are
/// executed in another thread.   
class SCISHARE Thread {
#ifdef SCI_64BITS
  static const unsigned long long DEFAULT_STACKSIZE  = 256 * 1024; // 256 KB
#else
  static const unsigned long long DEFAULT_STACKSIZE  = 128 * 1024; // 128 KB
#endif
public:
  /// Possible thread start states
  enum ActiveState {
    Activated,
    Stopped,
    NotActivated
  };
	    
  /// Create a thread, which will execute the <b>run()</b>
  /// method in the <b>runner</b> object. The thread <b>name</b>
  /// is used for identification purposes, and does not need to
  /// be unique with respect to other threads.  <b>Group</b>
  /// specifies the ThreadGroup that to which this thread
  /// should belong.  If no group is specified (group==0),
  /// the default group is used.
  Thread(Runnable* runner,
	 const char* name,
	 ThreadGroup* group=0, 
	 ActiveState state=Activated,
	 unsigned long long stack_size = DEFAULT_STACKSIZE);

 
  /// Return the <b>ThreadGroup</b> associated with this thread.
  ThreadGroup* getThreadGroup();
	    
 
  /// Return the <b>Runnable</b> associated with this thread.
  Runnable* getRunnable();
	    
 
  /// Flag the thread as a daemon thread.  When all non-deamon
  /// threads exit, the program will exit.
  void setDaemon(bool to=true);
	    
 
  /// Returns true if the thread is tagged as a daemon thread.
  bool isDaemon() const;
	    
 
  /// If the thread is started in the the NotActivated state,
  /// use this to activate the thread (the argument should be
  /// false).
  void activate(bool stopped);
	    
 
  /// Arrange to have the thread deleted automatically at exit.
  /// The pointer to the thread should not be used by any other
  /// threads once this has been called.
  void detach();
	    
 
  /// Returns true if the thread is detached
  bool isDetached() const;
	    
 
  /// Set the stack size for a particular thread.  In order
  /// to use this thread, you must create the thread in the
  /// NotActivated state, set the stack size, and then start
  /// the thread using activate(false).  Setting the stack
  /// size for a thread that is running or has ever been run,
  /// will throw an exception.  The units are in bytes.
  void setStackSize(unsigned long long stackSize);
	    
 
  /// Returns the stack size for the thread
  unsigned long long getStackSize() const;
	    
 
  /// Kill all threads and exit with <b>code</b>.
  static void exitAll(int code);
	    
 
  /// Exit the currently running thread
  static void exit();
	    
 
  /// Returns a pointer to the currently running thread.
  static Thread* self();
	    
 
  /// Stop the thread.
  void stop();
	    
 
  /// Resume the thread
  void resume();
	    
 
  /// Blocks the calling thread until this thead has finished
  /// executing. You cannot join detached threads or daemon threads.
  void join();
	    
 
  /// Returns the name of the thread
  const char* getThreadName() const;
	    
 
  /// Returns the number of processors on the system
  static int numProcessors();
	    
 
  /// Request that the thread migrate to processor <i>proc</i>.
  /// If <i>proc</i> is -1, then the thread is free to run
  /// anywhere.
  void migrate(int proc);
	    
 
  /// Start up several threads that will run in parallel.  A new
  /// <b>ThreadGroup</b> is created as a child of the optional parent.
  /// If <i>block</i> is true, then the caller will block until all
  /// of the threads return.  Otherwise, the call will return
  /// immediately.
  static ThreadGroup* parallel(ParallelBase& helper,
			       int nthreads, bool block,
			       ThreadGroup* threadGroup=0);

 
  /// Start up several threads that will run in parallel.
  /// If <i>block</i> is true, then the caller will block until all
  /// of the threads return.  Otherwise, the call will return
  /// immediately.
  template<class T>
  static void parallel(T* ptr, void (T::*pmf)(int),
		       int numThreads)
  {
    if (numThreads <= 1) { (ptr->*pmf)(0); }
    else
    {
      Parallel<T> p(ptr, pmf);
      parallel(p, numThreads, true);
    }
  }

 
  /// Another overloaded version of parallel that passes 1 argument
  template<class T, class Arg1>
  static void parallel(T* ptr, void (T::*pmf)(int, Arg1),
		       int numThreads, Arg1 a1)
  {
    if (numThreads <= 1) { (ptr->*pmf)(0, a1); }
    else
    {
      Parallel1<T, Arg1> p(ptr, pmf, a1);
      parallel(p, numThreads, true);
    }
  }

 
  /// Another overloaded version of parallel that passes 2 arguments
  template<class T, class Arg1, class Arg2>
  static void parallel(T* ptr, void (T::* pmf)(int, Arg1, Arg2),
		       int numThreads, Arg1 a1, Arg2 a2)
  {
    if (numThreads <= 1) { (ptr->*pmf)(0, a1, a2); }
    else
    {
      Parallel2<T, Arg1, Arg2> p(ptr, pmf, a1, a2);
      parallel(p, numThreads, true);
    }
  }

 
  /// Another overloaded version of parallel that passes 3 arguments
  template<class T, class Arg1, class Arg2, class Arg3>
  static void parallel(T* ptr, void (T::* pmf)(int, Arg1, Arg2, Arg3),
		       int numThreads, Arg1 a1, Arg2 a2, Arg3 a3)
  {
    if (numThreads <= 1) { (ptr->*pmf)(0, a1, a2, a3); }
    else
    {
      Parallel3<T, Arg1, Arg2, Arg3> p(ptr, pmf, a1, a2, a3);
      parallel(p, numThreads, true);
    }
  }

  /// Another overloaded version of parallel that passes 4 arguments
  template<class T, class Arg1, class Arg2, class Arg3, class Arg4>
  static void parallel(T* ptr, void (T::* pmf)(int, Arg1, Arg2, Arg3, Arg4),
		       int numThreads, Arg1 a1, Arg2 a2, Arg3 a3, Arg4 a4)
  {
    if (numThreads <= 1) { (ptr->*pmf)(0, a1, a2, a3, a4); }
    else
    {
      Parallel4<T, Arg1, Arg2, Arg3, Arg4> p(ptr, pmf, a1, a2, a3, a4);
      parallel(p, numThreads, true);
    }
  }

 
  /// Abort the current thread, or the process.  Prints a message on
  /// stderr, and the user may choose one of:
  /// <pre>continue(c)/dbx(d)/cvd(v)/kill thread(k)/exit(e)</pre>
  /// context is necesary on Windows to catch a segfault
  static void niceAbort(void* Context = 0);
	    
 
  /// Mark a section as one that could block for debugging purposes.
  /// The <b>int</b> that is returned should be passed into
  /// <i>couldBlockDone(int)</i> when the section has completed.  This
  /// will typically not be used outside of the thread implementation.
  static int couldBlock(const char* why);
	    
 
  /// Mark the end of a selection that could block.
  /// <i>restore</i> was returned from a previous invocation
  /// of the above <b>couldBlock</b>.
  static void couldBlockDone(int restore);
	    
 
  /// The calling process voluntarily gives up time to another process
  static void yield();

 
  /// Return true if the thread library has been initialized. This
  /// will typically not be used outside of the thread implementation.
  static bool isInitialized();

  /// set to "exit" (or something else) so we don't have to always 
  /// wait for the user to input something
  static void setDefaultAbortMode(const char* abortMode);

  static void initialize();

  friend class Runnable;	    
  friend class ConditionVariable;
  friend class RecursiveMutex;
  friend class Mutex;
  friend struct Thread_private;
  friend class SystemCallManager;

  friend void Thread_run(Thread* t);
  friend void Thread_shutdown(Thread*,bool);
#ifdef _WIN32
  friend unsigned long run_threads(void* priv_v);
#else
  friend void* run_threads(void* priv_v);
  friend void handle_abort_signals(int, siginfo_t*, void*);
  friend void handle_quit(int);
  friend void handle_siguser2(int);
  friend void install_signal_handlers();
  friend void exit_handler();
#endif

private:

  Runnable*     runner_;
  const char*   threadname_;
  ThreadGroup*  group_;
  unsigned long long stacksize_;
  bool          daemon_;
  bool          detached_;
  bool          activated_;
  
  void os_start(bool stopped);
  Thread(ThreadGroup* g, const char* name);

  static bool   initialized;
  static void   checkExit();

  static const char* defaultAbortMode;
  int cpu_;
  ~Thread();

  Thread_private* priv_;

  static int id();
  void run_body();	    	    
  enum ThreadState {
    STARTUP,
    RUNNING,
    IDLE,
    SHUTDOWN,
    DIED,
    PROGRAM_EXIT,
    JOINING,
    BLOCK_ANY,
    BLOCK_BARRIER,
    BLOCK_MUTEX,
    BLOCK_SEMAPHORE,
    BLOCK_CONDITIONVARIABLE
  };

  static const char* getStateString(ThreadState);
  
  static int push_bstack(Thread_private*, Thread::ThreadState s, const char* why);
  static void pop_bstack(Thread_private*, int oldstate);
  
  static void print_threads();

  /// Cannot copy them
  Thread(const Thread&);
  Thread& operator=(const Thread&);
};	
} // End namespace SCIRun

#endif



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
 *@file  Thread_pthreads.cc
 *@brief Posix threads implementation of the thread library
 *
 *@authof
 *   Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *@date: June 1997
 *
 */
#include <algorithm>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include <sci_debug.h>
#include <sci_defs/bits_defs.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define __USE_UNIX98
#include <pthread.h>
#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

#include <Core/Thread/Thread.h>
#include <Core/Thread/Mutex.h> // So ConditionVariable can get to Mutex::priv_

#include <Core/Thread/Thread.h>
#include <Core/Thread/AtomicCounter.h>
#include <Core/Thread/Barrier.h>
#include <Core/Thread/CleanupManager.h>
#include <Core/Thread/ConditionVariable.h>
#include <Core/Thread/RecursiveMutex.h>
#include <Core/Thread/Semaphore.h>
#include <Core/Thread/ThreadError.h>
#include <Core/Thread/ThreadGroup.h>
#include <Core/Thread/WorkQueue.h>
#include <Core/Thread/Thread_unix.h>
#include <Core/Thread/Time.h>

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

/*
 * The pthread implementation uses the default version of AtomicCounter,
 * Barrier, and CrowdMonitor.  It provides native implementations of
 * of ConditionVariable, Mutex, RecursiveMutex and Semaphore.
 *
 */


#include <Core/Thread/CrowdMonitor_default.cc>

using std::string;
using SCIRun::ConditionVariable;
using SCIRun::Mutex;
using SCIRun::RecursiveMutex;
using SCIRun::Semaphore;
using SCIRun::Thread;
using SCIRun::ThreadError;
using SCIRun::ThreadGroup;

static bool exiting = false;

#define MAXBSTACK 10
#define MAXTHREADS 4000

namespace SCIRun {

class Thread_private {
  public:
    Thread_private(bool stopped);

    // The thread that owns this structure
    Thread* thread;
    // The pthread id
    pthread_t threadid;

    //-----------------------------------------
    // Debugging information
    // Is this thread waiting for for something
    Thread::ThreadState state;
    // Where is the thread waiting for
    std::vector<const char *> blockstack;
    std::vector<const char *> mutexlist;

    //-----------------------------------------
    // For exit code
    Semaphore done;
    Semaphore delete_ready;
    Semaphore block_sema;

    bool is_blocked;
    bool exiting;
    bool ismain;
};

using SCIRun::Thread_private;
static Thread_private* active[MAXTHREADS];
static int numActive = 0;
static pthread_mutex_t sched_lock;
static pthread_key_t   thread_key;
static Semaphore main_sema("main",0);
static Semaphore control_c_sema("control-c",1);
static Thread*  mainthread = 0;

Thread_private::Thread_private(bool stopped) :
  done("done",0),
  delete_ready("delete_ready",0),
  block_sema("block_sema",stopped?0:1),
  exiting(false)
{}

static
void
lock_scheduler()
{
  const int status = pthread_mutex_lock(&sched_lock);
  if (status)
  {
    switch (status)
    {
    case EINVAL:
      throw ThreadError("pthread_mutex_lock:  Uninitialized lock.");
      break;

    case EDEADLK:
      throw ThreadError("pthread_mutex_lock:  Calling thread already holds this lock.");
      break;

    default:
      throw ThreadError("pthread_mutex_lock:  Unknown error.");
    }
  }
}

static
void
unlock_scheduler()
{
  const int status = pthread_mutex_unlock(&sched_lock);
  if (status)
  {
    switch (status)
    {
    case EINVAL:
      throw ThreadError("pthread_mutex_unlock:  Uninitialized lock.");
      break;

    case EPERM:
      throw ThreadError("pthread_mutex_unlock:  Unlocker did not lock.");
      break;

    default:
      throw ThreadError("pthread_mutex_unlock:  Unknown error.");
    }
  }
}

int
Thread::push_bstack(Thread_private* p, Thread::ThreadState state,
                    const char* name)
{
  int oldstate = p->state;

  p->state = state;
  p->blockstack.push_back(name);

  return oldstate;
}

void
Thread::pop_bstack(Thread_private* p, int oldstate)
{
  p->blockstack.pop_back();
  p->state = (ThreadState)oldstate;
}

void
ThreadGroup::gangSchedule()
{
  // Cannot do this on pthreads unfortunately
}

void
Thread_shutdown(Thread* thread, bool /*dummy*/)
{
  Thread_private* priv = thread->priv_;

  priv->done.up();
  if (!priv->ismain) priv->delete_ready.down();

  // Allow this thread to run anywhere...
  if (thread->cpu_ != -1)
    thread->migrate(-1);

  lock_scheduler();
  /* Remove it from the active queue */
  int i;
  for (i = 0;i<numActive;i++){
    if (active[i]==priv)
      break;
  }
  for (i++;i<numActive;i++){
    active[i-1]=active[i];
  }
  numActive--;

  // This can't be done in checkExit, because of a potential race
  // condition.
  int done = true;
  for (int i = 0;i<numActive;i++){
    Thread_private* p = active[i];
    if (!p->thread->isDaemon()){
      done = false;
      break;
    }
  }
  unlock_scheduler();

  bool wait_main = priv->ismain;
  delete thread;
  if (pthread_setspecific(thread_key, 0) != 0)
    std::cerr << "Warning: pthread_setspecific failed." << std::endl;
  priv->thread = 0;
  delete priv;
  if (done)
  {
    Thread::exitAll(0);
  }
  if (wait_main) {
    main_sema.down();
  }
  pthread_exit(0);
}

void
Thread::exit()
{
  Thread* self = Thread::self();
  Thread_shutdown(self,true);
}

void
Thread::checkExit()
{
  lock_scheduler();
  int done = true;
  for (int i = 0;i<numActive;i++)
  {
    Thread_private* p = active[i];
    if (!p->thread->isDaemon())
    {
      done = false;
      break;
    }
  }
  unlock_scheduler();

  if (done)
    Thread::exitAll(0);
}

Thread*
Thread::self()
{
  void* p = pthread_getspecific(thread_key);
  return (Thread*)p;
}

void
Thread::join()
{
  pthread_t id = priv_->threadid;

  priv_->delete_ready.up();

  const int status = pthread_join(id, 0);
  if (status)
  {
    switch (status)
    {
    case ESRCH:
      throw ThreadError("pthread_join:  No such thread.");
      break;

    case EINVAL:
      throw ThreadError("pthread_join:  Joining detached thread or joining same thread twice.");
      break;

    case EDEADLK:
      throw ThreadError("pthread_join:  Joining self, deadlock.");
      break;

    default:
      throw ThreadError("pthread_join:  Unknown error.");
    }
  }
}

int
Thread::numProcessors()
{
  static int np = 0;

  if (np == 0) {
#ifdef __APPLE__
    size_t len = sizeof(np);
    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    sysctl(mib, 2, &np, &len, NULL, 0); 
#else
    // Linux
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo) {
      int count = 0;
      while (!cpuinfo.eof()) 
      {
        std::string str;
        cpuinfo >> str;
        if (str == "processor") {
          ++count;
        } 
      }
      np = count;
    }
#endif
    if (np <= 0) np = 1;
  }
  return np;
}

void
Thread_run(Thread* t)
{
  t->run_body();
}

void*
run_threads(void* priv_v)
{
  Thread_private* priv = (Thread_private*)priv_v;
  if (pthread_setspecific(thread_key, priv->thread))
  {
    throw ThreadError("pthread_setspecific: Unknown error.");
  }
  priv->is_blocked = true;

  priv->block_sema.down();
  priv->is_blocked = false;
  priv->state = Thread::RUNNING;
  Thread_run(priv->thread);
  priv->state = Thread::SHUTDOWN;
  Thread_shutdown(priv->thread,true);
  return (0);
}

void
Thread::os_start(bool stopped)
{
  if (!initialized)
    Thread::initialize();

  priv_ = new Thread_private(stopped);

  priv_->state = STARTUP;

  priv_->thread = this;
  priv_->threadid = 0;
  priv_->is_blocked = false;
  priv_->ismain = false;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, stacksize_);
		
  lock_scheduler();
  active[numActive]=priv_;
  numActive++;
  
  if (pthread_create(&priv_->threadid, &attr, run_threads, priv_) != 0)
  {
    // Always EAGAIN
    throw ThreadError("pthread_create:  Out of thread resources.");
  }
  unlock_scheduler();
}

void
Thread::stop()
{
  lock_scheduler();
  if (priv_->block_sema.tryDown() == false)
  {
    if (this == self())
    {
      priv_->block_sema.down();
    }
    else
    {
      pthread_kill(priv_->threadid, SIGUSR2);
    }
  }
  unlock_scheduler();
}

void
Thread::resume()
{
  lock_scheduler();
  priv_->block_sema.up();
  unlock_scheduler();
}

void
Thread::detach()
{
  detached_=true;
  pthread_t id = priv_->threadid;

  priv_->delete_ready.up();

  const int status = pthread_detach(id);
  if (status)
  {
    switch (status)
    {
    case ESRCH:
      throw ThreadError("pthread_detach:  Thread does not exist.");
      break;

    case EINVAL:
      throw ThreadError("pthread_detach:  Thread is already detached.");
      break;

    default:
      throw ThreadError("pthread_detach:  Unknown error.");
    }
  }
}

void
Thread::exitAll(int code)
{
  // We should not need this
  if (getenv("SCIRUN_EXIT_CRASH_WORKAROUND"))
  {
    raise(SIGKILL);
  }
  
  if (Thread::isInitialized() && !exiting)
  {
    lock_scheduler(); // we are about to quit don't unlock.
    exiting = true; // Indicate that we are exiting
    
    CleanupManager::call_callbacks();
    
    // Stop all of the other threads before we die, because
    // global destructors may destroy primitives that other
    // threads are using...
    Thread* me = Thread::self();
    for (int i = 0;i<numActive;i++)
    {
      Thread_private* t = active[i];
      if (t->thread != me)
      {
        t->exiting = true;
        // send signal to thread.
        pthread_kill(t->threadid, SIGUSR2);
        // wait for signal handler to finish with the thread.
        int numtries = 100;
        while (--numtries && !t->is_blocked) 
        {
          Time::waitFor(0.01);
        }
        if (!numtries)
        {
          std::cerr << std::endl << std::endl
              << "Thread id: " << t->threadid << " named: " 
              << t->thread->getThreadName() 
              << "is slow to stop, giving up." << std::endl;
        }
      }
    }
  }
  ::_Exit(code);
}

/*
 * Handle an abort signal - like segv, bus error, etc.
 */

void
handle_abort_signals(int sig, siginfo_t* info, void* /*context*/)
{
  if (Thread::isInitialized())
  {
    // Set default handler.
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_handler = SIG_DFL;
    action.sa_flags = 0;
    if (sigaction(sig, &action, NULL) == -1)
      throw ThreadError(std::string("sigaction failed")
            +strerror(errno));

    Thread* self = Thread::self();
    const char* tname = self?self->getThreadName():"idle or main";
    string signam = Core_Thread_signal_name(sig, info->si_addr);

    std::cerr << std::endl
        << "Thread \"" << tname << "\" (id: " << self->priv_->threadid 
        << ", pid: " << getpid() << ") " << std::endl 
        << "\tcaught signal " << signam 
        << std::endl << "\t...handle_abort_signals..." << std::endl;

    Thread::niceAbort();

    // Restore this handler.
    action.sa_sigaction = handle_abort_signals;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(sig, &action, NULL) == -1)
      throw ThreadError(std::string("sigaction failed")
            +strerror(errno));
  }
}


void
Thread::print_threads()
{
  for (int i = 0;i < numActive; i++)
  {
    Thread_private* p = active[i];
   
    const char* tname = p->thread?p->thread->getThreadName():"???";
    std::cerr << p->threadid << ":" << tname << "(";
    if (p->thread)
    {
      if (p->thread->isDaemon())
        std::cerr << "daemon, ";
      if (p->thread->isDetached())
        std::cerr << "detached, ";
    }
    std::cerr << " " << Thread::getStateString(p->state);
    for (size_t i = 0;i<p->blockstack.size();i++)
    {
      std::cerr << ", " << p->blockstack[i];
    }

    std::cerr << ", List: ";
    for (size_t i = 0;i<p->mutexlist.size();i++)
    {
      std::cerr << p->mutexlist[i] << "| ";
    }
    std::cerr << ")" << std::endl;
  }
}


/*
 * Handle sigquit - usually sent by control-C
 */

void
handle_quit(int sig)
{
  if (Thread::isInitialized())
  {
    // Try to acquire a lock.  If we can't, then assume that somebody
    // else already caught the signal...
    Thread* self = Thread::self();
    if (self == 0)
      return; // This is an idle thread...
      
    if (!(control_c_sema.tryDown()))
    {
      control_c_sema.down();
      control_c_sema.up();
      return;
    }

    // Otherwise, we got the semaphore and handle the interrupt
    const char* tname = self?self->getThreadName():"main?";

    // Kill all of the threads.
    string signam = Core_Thread_signal_name(sig, 0);
    std::cerr << std::endl << std::endl
        << "Thread \"" << tname << "\" (id: " << self->priv_->threadid 
        << ", pid: " << getpid() << ") " << std::endl 
        << "\tcaught signal " << signam 
        << std::endl << "\t...handle_quit..." << std::endl;

    Thread::niceAbort(); // Enter the monitor
    control_c_sema.up();
  }
}


/*
 * Handle siguser1 - for stop/resume
 */

void
handle_siguser2(int)
{
  Thread* self = Thread::self();
  if (!self)
  {
    // This can happen if the thread is just started and hasn't had
    // the opportunity to call setspecific for the thread id yet
    for (int i = 0;i<numActive;i++) 
    {
      if (pthread_self() == active[i]->threadid) 
      {
        self = active[i]->thread;
        break;
      }
    }
  }

  //! Make sure the exiting information is stored on the local stack
  //! In case we cannot access the structures any more
  //  bool exiting = self->priv_->exiting;
  self->priv_->is_blocked = true;
  self->priv_->block_sema.down();
  
  //   //! Make sure the thread will not revive
  //   if (exiting) for (;;) sched_yield();
  
  //! If not exiting tell we are running again
  self->priv_->is_blocked = false;
}


/*
 * Setup signals for the current thread
 */

void
install_signal_handlers()
{
  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_SIGINFO;

  action.sa_sigaction = handle_abort_signals;
  if (sigaction(SIGILL, &action, NULL) == -1)
    throw ThreadError(std::string("SIGILL failed") + strerror(errno));
  if (sigaction(SIGABRT, &action, NULL) == -1)
    throw ThreadError(std::string("SIGABRT failed") + strerror(errno));
  if (sigaction(SIGTRAP, &action, NULL) == -1)
    throw ThreadError(std::string("SIGTRAP failed") + strerror(errno));
  if (sigaction(SIGBUS, &action, NULL) == -1)
    throw ThreadError(std::string("SIGBUS failed") + strerror(errno));
  if (sigaction(SIGSEGV, &action, NULL) == -1)
    throw ThreadError(std::string("SIGSEGV failed") + strerror(errno));


  action.sa_flags = 0;
  action.sa_handler = handle_quit;
  if (sigaction(SIGQUIT, &action, NULL) == -1)
    throw ThreadError(std::string("SIGQUIT failed") + strerror(errno));
  if (sigaction(SIGINT, &action, NULL) == -1)
    throw ThreadError(std::string("SIGINT failed") + strerror(errno));


  struct sigaction actionu2;
  sigemptyset(&actionu2.sa_mask);
  actionu2.sa_flags = 0;
  actionu2.sa_handler = handle_siguser2;
  if (sigaction(SIGUSR2, &actionu2, NULL) == -1)
    throw ThreadError(std::string("SIGUSR2 - intentional kill") + 
		      strerror(errno));
}


void
exit_handler()
{
  if (exiting)
    return;
  Thread_shutdown(Thread::self(),true);
}


void
Thread::initialize()
{

  if (initialized)
    return;
  if (exiting)
    abort(); // Something really weird happened!

  CleanupManager::initialize();

  atexit(exit_handler);
  pthread_mutex_init(&sched_lock, NULL);

  if (pthread_key_create(&thread_key, NULL) != 0)
  {
    throw ThreadError("pthread_key_create:  Out of resources.");
  }

  initialized = true;
  ThreadGroup::s_default_group = new ThreadGroup("default group", 0);
  mainthread = new Thread(ThreadGroup::s_default_group, "main");
  mainthread->priv_ = new Thread_private(true);
  mainthread->priv_->thread = mainthread;
  mainthread->priv_->state = RUNNING;
  mainthread->priv_->is_blocked = false;
  mainthread->priv_->threadid = pthread_self();
  mainthread->priv_->ismain = true;

  if (pthread_setspecific(thread_key, mainthread) != 0)
  {
    throw ThreadError("pthread_setspecific:  Failed.");
  }

  lock_scheduler();
  active[numActive]=mainthread->priv_;
  numActive++;
  unlock_scheduler();
  if (!getenv("THREAD_NO_CATCH_SIGNALS"))
    install_signal_handlers();
  numProcessors();  //initialize the processor count;
}


void
Thread::yield()
{
  sched_yield();
}


void
Thread::migrate(int /*proc*/)
{
  // Nothing for now...
}


class Mutex_private {
  public:
    pthread_mutex_t mutex_;
};


Mutex::Mutex(const char* name)
  : name_(name)
{
  priv_ = new Mutex_private;

#ifdef PTHREAD_MUTEX_ERRORCHECK_NP
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr); // always returns zero

  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP) != 0)
    throw ThreadError("pthread_mutexattr_settype:  Bad kind.");

  pthread_mutex_init(&priv_->mutex_, &attr); // always returns zero
  pthread_mutexattr_destroy(&attr);  // usually noop
#else
  pthread_mutex_init(&priv_->mutex_, 0); // always returns zero
#endif
}


Mutex::~Mutex()
{
  if (pthread_mutex_destroy(&priv_->mutex_) != 0)
  {
    priv_ = 0;
    return;
  }
  
  delete priv_;
  priv_ = 0;
}

void
Mutex::unlock()
{

#ifdef DEBUG
  // Remove this lock from the list, so it is not marked as owning a certain
  // lock
  Thread* t = Thread::isInitialized()?Thread::self():0;
  if (t)
  {
    // Remove mutex from the list
    t->priv_->mutexlist.erase(remove(t->priv_->mutexlist.begin(),
                                     t->priv_->mutexlist.end(),name_),
                                     t->priv_->mutexlist.end());
  }
#endif  

  int status = pthread_mutex_unlock(&(priv_->mutex_));

  if (status)
  {
    switch (status)
    {
    case EINVAL:
      ThreadError("pthread_mutex_unlock:  Uninitialized lock.");
      break;

    case EPERM:
      ThreadError("pthread_mutex_unlock:  Calling thread did not lock.");
      break;

    default:
      ThreadError("pthread_mutex_unlock:  Unknown error.");
    }
  }
}

void
Mutex::lock()
{
#ifdef DEBUG
  // Denoting that the thread may block any moment
  int oldstate = -1;

  Thread* t = Thread::isInitialized()?Thread::self():0;
  if (t)
  {
    oldstate = t->priv_->state;
    t->priv_->state = Thread::BLOCK_MUTEX;
    t->priv_->blockstack.push_back(name_);
  }
#endif

  int status = pthread_mutex_lock(&priv_->mutex_);

#ifdef DEBUG
  // Reporting that the thread wil continue to run
  if (t)
  {
    t->priv_->state = static_cast<Thread::ThreadState>(oldstate);
    t->priv_->blockstack.pop_back();
  }

  // Telling as well that it owns this mutex
  if (t)
  {
    t->priv_->mutexlist.insert(t->priv_->mutexlist.end(),name_);
  }
#endif

  if (status)
  {
    switch (status)
    {
    case EINVAL:
      throw ThreadError("pthread_mutex_lock:  Uninitialized lock.");
      break;

    case EDEADLK:
      throw ThreadError("pthread_mutex_lock:  Calling thread already holds this lock.");
      break;

    default:
      throw ThreadError("pthread_mutex_lock:  Unknown error.");
    }
  }


  
}


bool
Mutex::tryLock()
{
  const int status = pthread_mutex_trylock(&priv_->mutex_);
  
  switch (status)
  {
    case 0:
    {
#ifdef DEBUG
      // Denote that it now owns the mutex
      Thread* t = Thread::isInitialized()?Thread::self():0;
      if (t)
      {
        t->priv_->mutexlist.insert(t->priv_->mutexlist.end(),name_);
      }
#endif    
      return true;
    }
    case EBUSY:
    {
      return false;
    }
    default: // EINVAL
      throw ThreadError("pthread_mutex_trylock:  Uninitialized lock.");
  }
}


class RecursiveMutex_private {
  public:
    pthread_mutex_t mutex_;
    int             cnt_;
};

RecursiveMutex::RecursiveMutex(const char* name)
  : name_(name)
{
  if (!Thread::initialized) Thread::initialize();

  priv_ = new RecursiveMutex_private;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
    throw ThreadError("pthread_mutexattr_settype: unknown kind.");
  pthread_mutex_init(&priv_->mutex_, &attr);
  pthread_mutexattr_destroy(&attr);
  
  priv_->cnt_ = 0;
}


RecursiveMutex::~RecursiveMutex()
{
  if (pthread_mutex_destroy(&priv_->mutex_))
  {
    priv_ = 0;
    return;
  }
  delete priv_;
  priv_=0;
}


void
RecursiveMutex::unlock()
{
  priv_->cnt_--;
#ifdef DEBUG
  // Remove this lock from the list, so it is not marked as owning a certain
  // lock
  Thread* t = Thread::isInitialized()?Thread::self():0;
  if (t && priv_->cnt_==0)
  {
    // Remove mutex from the list
    t->priv_->mutexlist.erase(remove(t->priv_->mutexlist.begin(),
                                     t->priv_->mutexlist.end(),name_),
                                     t->priv_->mutexlist.end());
  }
#endif  

  const int status = pthread_mutex_unlock(&priv_->mutex_);
  if (status)
  {
    switch (status)
    {
    case EINVAL:
      ThreadError("pthread_mutex_unlock:  Uninitialized lock.");
      break;

    case EPERM:
      ThreadError("pthread_mutex_unlock:  Calling thread did not lock.");
      break;

    default:
      ThreadError("pthread_mutex_unlock:  Unknown error.");
    }
  }
}


void
RecursiveMutex::lock()
{
#ifdef DEBUG
  // Denoting that the thread may block any moment
  int oldstate = -1;

  Thread* t = Thread::isInitialized()?Thread::self():0;
  if (t)
  {
    oldstate = t->priv_->state;
    t->priv_->state = Thread::BLOCK_MUTEX;
    t->priv_->blockstack.push_back(name_);
  }
#endif  
  
  const int status = pthread_mutex_lock(&priv_->mutex_);
  priv_->cnt_++;

#ifdef DEBUG
  // Reporting that the thread wil continue to run
  if (t)
  {
    t->priv_->state = static_cast<Thread::ThreadState>(oldstate);
    t->priv_->blockstack.pop_back();
  }

  // Telling as well that it owns this mutex
  if (t && priv_->cnt_ == 1)
  {
    t->priv_->mutexlist.insert(t->priv_->mutexlist.end(),name_);
  }
#endif

  if (status)
  {
    switch (status)
    {
      case EINVAL:
      {
        throw ThreadError("pthread_mutex_lock:  Uninitialized lock.");
        break;
      }
      case EDEADLK:
      {
        throw ThreadError("pthread_mutex_lock:  Calling thread already holds this lock.");
        break;
      }
      default:
      {
        throw ThreadError("pthread_mutex_lock:  Unknown error.");
      }
    }
  }
}



bool
RecursiveMutex::tryLock()
{  
  const int status = pthread_mutex_trylock(&priv_->mutex_);
  priv_->cnt_++;

  switch (status)
  {
    case 0:
    {    
#ifdef DEBUG
      // Denote that it now owns the mutex
      Thread* t = Thread::isInitialized()?Thread::self():0;
      if (t && priv_->cnt_ == 1)
      {
        t->priv_->mutexlist.insert(t->priv_->mutexlist.end(),name_);
      }
#endif
      return (true);
    }
    case EBUSY:
    {
      return (false);
    }
    
    case EINVAL:
    {
      throw ThreadError("pthread_mutex_lock:  Uninitialized lock.");
      break;
    }
    
    case EDEADLK:
    {
      throw ThreadError("pthread_mutex_lock:  Calling thread already holds this lock.");
      break;
    }
    
    default:
    {
      throw ThreadError("pthread_mutex_lock:  Unknown error.");
    }
  }

  return (true);
}


class Semaphore_private {
  public:
    Semaphore_private(const char *name, int value);
    Mutex             mutex_;
    int               cnt_;
    ConditionVariable cv_;
};


Semaphore_private::Semaphore_private(const char *name, int value) :
  mutex_(name),
  cnt_(value),
  cv_(name)
{
}


Semaphore::Semaphore(const char *name, int value)
  : name_(name)
{
  priv_ = new Semaphore_private(name,value);
}


Semaphore::~Semaphore()
{
  if (priv_)
  {
    delete priv_;
    priv_ = 0;
  }
}


void
Semaphore::down(int count)
{
  for (int p = 0 ; p < count; p++)
  {
    priv_->mutex_.lock();
    priv_->cnt_--;
    if (priv_->cnt_ < 0) priv_->cv_.wait(priv_->mutex_);
    priv_->mutex_.unlock();
  }
}


bool
Semaphore::tryDown()
{
  priv_->mutex_.lock();
  if (priv_->cnt_ > 0)
  {
    priv_->cnt_--;
    priv_->mutex_.unlock();
    return (true);
  }
  priv_->mutex_.unlock();
  return (false);
}


void
Semaphore::up(int count)
{
  for (int p = 0;p < count; p++)
  {
    priv_->mutex_.lock();
    priv_->cv_.conditionBroadcast();
    priv_->cnt_++;
    priv_->mutex_.unlock();
  }
}

class ConditionVariable_private {
  public:
    pthread_cond_t cond_;
};

ConditionVariable::ConditionVariable(const char* name)
  : name_(name)
{
  if (!Thread::initialized) Thread::initialize();

  priv_ = new ConditionVariable_private;
  pthread_cond_init(&priv_->cond_, 0);
}


ConditionVariable::~ConditionVariable()
{
  if (pthread_cond_destroy(&priv_->cond_) != 0)
  {
    ThreadError("pthread_cond_destroy:  Threads are currently waiting on this condition.");
  }
  delete priv_;
  priv_ = 0;
}


void
ConditionVariable::wait(Mutex& m)
{
#ifdef DEBUG
  // Mark that the thread will wait
  Thread* t = Thread::self();
  int oldstate = 0;
  
  if (t) 
  {
    oldstate = t->priv_->state;
    t->priv_->state = Thread::BLOCK_MUTEX;
    t->priv_->blockstack.push_back(name_);

    // Remove mutex from the list
    t->priv_->mutexlist.erase(remove(t->priv_->mutexlist.begin(),
                                     t->priv_->mutexlist.end(),m.name_),
                                     t->priv_->mutexlist.end());  }
#endif
  
  pthread_cond_wait(&priv_->cond_, &m.priv_->mutex_);

#ifdef DEBUG  
  if (t) 
  {
    t->priv_->state = static_cast<Thread::ThreadState>(oldstate);
    t->priv_->blockstack.pop_back();

    // Add mutex to the list
    t->priv_->mutexlist.push_back(m.name_);
  }
#endif
}


void
ConditionVariable::wait(RecursiveMutex& m)
{
#ifdef DEBUG
  // Mark that the thread will wait
  Thread* t = Thread::self();
  int oldstate = 0;
  
  if (t) 
  {
    oldstate = t->priv_->state;
    t->priv_->state = Thread::BLOCK_MUTEX;
    t->priv_->blockstack.push_back(name_);

    // Remove mutex from the list
    t->priv_->mutexlist.erase(remove(t->priv_->mutexlist.begin(),
                                     t->priv_->mutexlist.end(),m.name_),
                                     t->priv_->mutexlist.end());
  }
#endif
  
  pthread_cond_wait(&priv_->cond_, &m.priv_->mutex_);

#ifdef DEBUG  
  if (t) 
  {
    t->priv_->state = static_cast<Thread::ThreadState>(oldstate);
    t->priv_->blockstack.pop_back();

    // Add mutex to the list
    t->priv_->mutexlist.push_back(m.name_);
  }
#endif
}



bool
ConditionVariable::timedWait(Mutex& m, const struct timespec* abstime)
{
#ifdef DEBUG
  // Mark that the thread will wait
  Thread* t = Thread::self();
  int oldstate = 0;
  
  if (t) 
  {
    oldstate = t->priv_->state;
    t->priv_->state = Thread::BLOCK_MUTEX;
    t->priv_->blockstack.push_back(name_);

    // Remove mutex from the list
    t->priv_->mutexlist.erase(remove(t->priv_->mutexlist.begin(),
                                     t->priv_->mutexlist.end(),m.name_),
                                     t->priv_->mutexlist.end());  }
#endif  
  
  bool success;
  if (abstime)
  {
    int err = pthread_cond_timedwait(&priv_->cond_, &m.priv_->mutex_, abstime);
    if (err != 0)
    {
      if (err == ETIMEDOUT)
      {
        success = false;
      }
      else
      {
        throw ThreadError("pthread_cond_timedwait:  Interrupted by a signal.");
      }
    } 
    else 
    {
      success = true;
    }
  } 
  else 
  {
    pthread_cond_wait(&priv_->cond_, &m.priv_->mutex_);
    success = true;
  }
    
#ifdef DEBUG  
  if (t) 
  {
    t->priv_->state = static_cast<Thread::ThreadState>(oldstate);
    t->priv_->blockstack.pop_back();

    // Add mutex to the list
    t->priv_->mutexlist.push_back(m.name_);
  }
#endif
  
  return (success);
}


bool
ConditionVariable::timedWait(RecursiveMutex& m, const struct timespec* abstime)
{
#ifdef DEBUG
  // Mark that the thread will wait
  Thread* t = Thread::self();
  int oldstate = 0;
  
  if (t) 
  {
    oldstate = t->priv_->state;
    t->priv_->state = Thread::BLOCK_MUTEX;
    t->priv_->blockstack.push_back(name_);

    // Remove mutex from the list
    t->priv_->mutexlist.erase(remove(t->priv_->mutexlist.begin(),
                                     t->priv_->mutexlist.end(),m.name_),
                                     t->priv_->mutexlist.end());  }
#endif 
  
  bool success;
  if (abstime)
  {
    int err = pthread_cond_timedwait(&priv_->cond_, &m.priv_->mutex_, abstime);
    if (err != 0)
    {
      if (err == ETIMEDOUT)
      {
        success = false;
      }
      else
      {
        throw ThreadError("pthread_cond_timedwait:  Interrupted by a signal.");
      }
    } 
    else 
    {
      success = true;
    }
  } 
  else 
  {
    pthread_cond_wait(&priv_->cond_, &m.priv_->mutex_);
    success = true;
  }

#ifdef DEBUG  
  if (t) 
  {
    t->priv_->state = static_cast<Thread::ThreadState>(oldstate);
    t->priv_->blockstack.pop_back();

    // Add mutex to the list
    t->priv_->mutexlist.push_back(m.name_);
  }
#endif
  
  return (success);
}


void
ConditionVariable::conditionSignal()
{
  pthread_cond_signal(&priv_->cond_);
}


void
ConditionVariable::conditionBroadcast()
{
  pthread_cond_broadcast(&priv_->cond_);
}

} // end namespace

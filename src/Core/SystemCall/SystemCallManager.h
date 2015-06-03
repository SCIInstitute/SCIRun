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
 *  SystemCallManager.h
 *
 *  The classes in this file represent the low level interface to calling unix 
 *  programs outside of SCIRun. The idea of this class is to have a separate
 *  SCIRun process from which we can fork external processes. The advantage of
 *  having a separate process over a separate thread is that we have full 
 *  control over the STDIN/STDOUT/STDERR of a process. In other words we can
 *  fully wrap SCIRun around an external program and use a SCIRun module to
 *  give an external program an interface. This class is mainly meant to wrap
 *  the STDIO handling and error handling and hide the existance of a separate
 *  thread from the developer by wrapping it in a class.
 *  The SystemCallManager class also keeps track of which processes have been 
 *  launched, so they can adequately be killed once SCIRun exits. Especially
 *  with separate processes it is important to keep track of what is out there
 *  For internal communication a combination of fifos and pipes is used. These
 *  are obviously all UNIX entities and thus there is not yet a Windows implementation
 *  for this class.
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER


#ifndef CORE_SYSTEMCALL_SYSTEMCALLMANAGER_H
#define CORE_SYSTEMCALL_SYSTEMCALLMANAGER_H 1

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/signal.h>
#include <unistd.h>
#else
typedef int pid_t;
#endif
#include <fcntl.h>


#include <list>
#include <string>
#include <iostream>
#include <sstream>


#include <stdio.h>
#include <stdlib.h>

#include <Core/Thread/Mutex.h>
#include <Core/Thread/Runnable.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/CleanupManager.h>
#include <Core/SystemCall/SystemCallError.h>
#include <Core/SystemCall/TempFileManager.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Handle.h>

#include <Core/SystemCall/share.h>

namespace SCIRun {

class SystemCallProcess : public SystemCallBase {

  public:
  
    // The SystemCallProcess class is an internal mechanism of the SystemCallManager to keep
    // track of each external process being launched. This class is for internal use only
    
    SystemCallProcess(int processid);
    ~SystemCallProcess();

    // Lock and unlock the class. As processes launched from the process manager can be shared
    // by multiple threads. There needs to be a way of sharing information. Hence a Mutex is
    // needed. For example the matlab interface allows multiple modules to use the same matlab 
    // thread. When this is the case each module thread needs to lock and unlock shared resources
    // All functions in this class automatically make sure that shared resources cannot be used 
    // at the same time.
    void    dolock();
    void    unlock();
    
    // Wait: wait for a process to end (wait until a message is written on the exit fifo)
    // Kill: kill a process before it ended (the unix kill instruction is used for this purpose)
    // Close: Close all file descriptors that are still open. Although the application may have
    //        ended some data may be pending on the fifo pipeline. To close this side of the
    //        fifos close must be called
    void    wait();
    // The last parameter informs the process that we already know that the process ended
    // Hence in that case it only needs to clean up. Otherwise it will do a detect on the
    // exit channel whether data has been written to this channel, if not it will kill the process
    void    kill(int secs = 0,bool processexit = false);
    void    close();
    
  public:    
    int        processid_;        // Processid, to keep the different processes separated
    pid_t      pid_;              // The PID of the process
    
    int        fd_stdin_;        // STDIN -> file descriptor to the STDIN channel to the external program
    int        fd_stdout_;       // STDOUT
    int        fd_stderr_;       // STDERR
    int        fd_exit_;         // EXIT fifo, this fifo will only receive a message when the program 
                            // ended. In the future this will be used to return an exit code
                            // The latter has not been implemented yet
    
    std::string    fifo_stdin_;    // Names of the fifos used in the process
    std::string    fifo_stdout_;   // We need to keep these, so we can effectively close and unlink
    std::string    fifo_stderr_;   // these, so there is no trace of the communication in the filesystem
    std::string    fifo_exit_;

  private:
    Mutex    lock_;                // Private lock, in this case we do not use handles. Not needed since this
                                // object is embedded into the SystemCallManager


};


// Shorts for locking and unlocking the structure  
inline void    SystemCallProcess::dolock()
{
    lock_.lock();
}
  
inline void    SystemCallProcess::unlock()
{
    lock_.unlock();
}
  
  

class SCISHARE SystemCallManager : public SystemCallBase {

  public:

    SystemCallManager();
    ~SystemCallManager();

    // Create the systemcallmanager by forking a new process which will
    // be used to create new forked processes. The reason for this two step
    // creation process, is the problem that fork will duplicate the current
    // process including all its shared memory. Which in the case of SCIRun 
    // can be pretty huge. Hence we fork at the start of SCIRun and create a
    // child connected by a simple pipe. This separate child process will be
    // used to fork external applications. As it is created before any dynamically
    // loaded libraries and processes and as well beore a lot of dynamic allocations
    // have been made, the core of this fork should be small. Hence any process we fork
    // from this process is small in memory terms as well.
    // The reason we want to fork processes is to be able to fully capture STDIN, STDOUT and
    // STDERR. So the external program is not aware that it is running inside of SCIRun
    // Create does the first fork and thus needs to be called at the start of main.

    void    create();
    
    // For creating shells in which to execute, use the following four set of commands
    
    int        exec(std::string command);
    void       wait(int processid);    // wait for process to finish
    void       kill(int processid,int secs=0,bool processexit=false);    // for the impatient: kill the process prematurely
                                                                         // In case we only want to clean up and know the process
                                                                         // is not running anymore, put processexit to true
     void       close(int processid);    // close the communication descriptors

    // Cleanup routine
    static void cleanup(void *data);

    // Get the descriptors to the communication channels
    int        getstdin(int processid);    // Receives the stdin
    int        getstdout(int processid);    // Receives the stdout
    int        getstderr(int processid);    // Receives the stderr
    int        getexit(int processid);        // Gets the return value

    void    dolock();    // Unfortunately in SCIRun a lock is called lock so I have to give the function a different name
    void    unlock();

  private:

    // Function that is being run as the child process.
    void            childmain();
    bool            readline(int fd, std::string &str); // For receiving data from the child process that will fork the external application
    bool            writeline(int fd, std::string str); // For sending data to that same child process
    void            killall();  // Kill all external processes. This is in fact the destructor.
                                // Since not all destructors are called properly at the time this function was written, it has been put in
                                // a separate function to be called when exiting SCIRun
                    
  public:            // This needs to be public for the LockingHandle class
    // Communication channel with child process
    Mutex    lock;                        // Lock communication with child, parent is a multi threaded application
    int      ref_cnt;
    bool     exit_;
    
  private:
    pid_t    childpid_;                    // Pid for process launching the children
    int      child_in_;                    // input to child stream
    int      child_out_;                   // output from child stream
    
    int                              processidcnt_;  // Each processes gets a new internal process ID
    std::list<SystemCallProcess*>    processlist_;   // Keep track of all processes launched
    
    TempFileManager    tfm_;
    std::string        tempdir_;
};
  
  
// Inline functions to speed up class performance
inline void    SystemCallManager::dolock()
{
    lock.lock();
}
  
inline void    SystemCallManager::unlock()
{
    lock.unlock();
}

// The systemcallmanager_ will be allocated in main and will be used by 
// classes making use of this class.

extern SCISHARE SystemCallManager* systemcallmanager_;

}

#endif
#endif
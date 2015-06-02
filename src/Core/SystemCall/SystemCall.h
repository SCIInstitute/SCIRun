#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

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
 *  SystemCall.h
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */



#ifndef CORE_SYSTEMCALL_SYSTEMCALL_H
#define CORE_SYSTEMCALL_SYSTEMCALL_H 1


#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>


#include <stdio.h>
#include <stdlib.h>

#include <Core/Thread/Time.h>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/Runnable.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/ConditionVariable.h>
#include <Core/SystemCall/SystemCallError.h>
#include <Core/SystemCall/SystemCallManager.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/ICom/IComSocket.h>

#include <Core/SystemCall/share.h>
namespace SCIRun {

class SystemCallHandler;
class SystemCallThread;
class SystemCall;

/////////////////////////////////////////////////////////////////////
// SystemCallHandle:
// Use this object to create a system call. Since a separate thread is
// launched, data is shared and it is not clear whether the calling
// thread exits first or the newly created thread. Hence we use smart
// pointers to keep track of the memory requirements

typedef LockingHandle<SystemCall> SystemCallHandle;

typedef LockingHandle<SystemCallHandler> SystemCallHandlerHandle;

// Hence to make a call to the system use:
// SystemCallHandle handle = new SystemCall;

//////////////////////////////////////////////////////////////////////
// SystemCallHandler, this is a template for handling the stdout and
// stderr, so we can have asynchrone handling of data
// Currently a line buffer is used and the data is forwarded line by
// line to the handler. Multiple handlers can exist and share data
// There is always a single thread buffering and sorting out the data

class SCISHARE SystemCallHandler : public SystemCallBase, public UsedWithLockingHandle<Mutex>
{
  public:
  SystemCallHandler();
  virtual ~SystemCallHandler();
  
  virtual void start(std::list<std::string> &buffer);
  virtual bool execute(std::string line);
  virtual void end();
  
  void dolock();
  void unlock();
  
  virtual SystemCallHandler* clone();
  bool end_;
};

inline void SystemCallHandler::start(std::list<std::string> &/*buffer*/)
{
  // Handle a the start of the process. 
  // If present all data in the buffer is send 
}

inline SystemCallHandler* SystemCallHandler::clone()
{
  return(new SystemCallHandler());
}

inline bool SystemCallHandler::execute(std::string /*line*/)
{
  // Handle a line of input from the external program
  return(true);
}

inline void SystemCallHandler::end()
{
  // This function is called to signal the end-of-file
  // marker in the stream of data
}


//////////////////////////////////////////////////////////////////////
// SystemCallThread, this is a thread that will be created when
// a systemcall is made. It is created as soon as execute is called
// on the main SystemCall object.

class SystemCallThread : public Runnable, public SystemCallBase {

  public:
  
    SystemCallThread(SystemCallHandle syscall_);
	virtual ~SystemCallThread();

	// Entry point for the thread
	void run();
	
  private:
	// Store the handle to the object
	// so it will not be destroyed before
	// the destructor of this object is called
	
	SystemCallHandle syscall_;
};


//////////////////////////////////////////////////////////////////////
// SystemCall:  Main object for handling calls to unix

class SCISHARE SystemCall : public SystemCallBase, public UsedWithLockingHandle<Mutex>
{
public:
	SystemCall();
	virtual ~SystemCall();
	
	// implementation for the SystemCall class
	void	execute(std::string command);
	void	wait();
	void	wait_eof();
	void	kill(int secs=0);

	bool	isrunning();

	// STDIN-FUNCTIONS
	
	void	put_stdin(std::vector<std::string> &instructions, bool raw = false);
	void	put_stdin(std::list<std::string> &instructions, bool raw = false);
	void	put_stdin(std::string &instruction, bool raw = false);	
	
	void	put_stdin_int(std::string &str); // no locking is done
	
	// STDOUT-FUNCTIONS

	void	add_stdout_handler(SystemCallHandlerHandle handle, bool front = false);
	void	rem_stdout_handler(SystemCallHandlerHandle handle);
	
	// STDERR-FUNCTIONS

	void	add_stderr_handler(SystemCallHandlerHandle handle, bool front = false);
	void	rem_stderr_handler(SystemCallHandlerHandle handle);

	// EXIT-FUNCTIONS
	
	void	add_exit_handler(SystemCallHandlerHandle handle);
	void	rem_exit_handler(SystemCallHandlerHandle handle);

  	// LOG FILE
	
	bool	create_logfile(std::string filename);
	bool	append_logfile(std::string filename);

	////////////////
	// MULTI THREADING FUNCTIONS
	// the object will invoke these functions for
	// almost every function call. So the object
	// should be thread safe. 

	void	set_stdout_buffersize(int size);
	void	set_stderr_buffersize(int size);
	
	void	dolock();		// This will lock the object
	void	unlock();

    void    use_stdout_timeout(bool use_timeout);
    
    bool    getexitcode(int &exitcode);


  ///////////////////////
  // The next set of functions is used by the thread to insert
  // data into the SystemCall object.

  friend class SystemCallThread;
  protected:

	// functions called by thread
	void	insert_stdout_line(std::string &line);
	void	insert_stderr_line(std::string &line);

	bool	signal_stdout_eof();
	bool	signal_stderr_eof();

	void	signal_eof();
	void	signal_exit();

	int		get_stdin();
	int		get_stdout();
	int		get_stderr();
	int		get_exit();

private:
	bool				isrunning_;			// Is there a process running ?

	int					fd_stdin_;
	int					fd_stdout_;
	int					fd_stderr_;
	int					fd_exit_;

	int					processid_;
	
	std::ofstream		file_;
	bool				isfile_;

	std::list<SystemCallHandlerHandle>   stdouthandler_;
	std::list<SystemCallHandlerHandle>   stderrhandler_; 
	std::list<SystemCallHandlerHandle>   exithandler_; 

	bool				stdout_eof_;
	bool				stderr_eof_;
	
	bool				iseof_;
	ConditionVariable	eof_;			// signal of reading eof on stdout and stderr
	
	bool				isexit_;
	ConditionVariable	exit_;			// signal of reading eof on the exit channel

	std::list<std::string> stdoutbuffer_;
	std::list<std::string> stderrbuffer_;
	int					stdoutbuffersize_;
	int					stderrbuffersize_;
        
    bool                use_timeout_;
    
    protected:
    int                 exitcode_;
    bool                hasexitcode_;
    
};

// If there is a long inactivity, pending data will be forwarded
// And line buffering will be overruled
inline void SystemCall::use_stdout_timeout(bool use_timeout)
{
    use_timeout_ = use_timeout;
}

inline void SystemCall::dolock()
{
	lock.lock();
}

inline void	SystemCall::unlock()
{
	lock.unlock();
}

inline bool	SystemCall::isrunning()
{
	return(isrunning_);
}

inline int	SystemCall::get_stdin()
{
	return(fd_stdin_);
}

inline int	SystemCall::get_stdout()
{
	return(fd_stdout_);
}

inline int	SystemCall::get_stderr()
{
	return(fd_stderr_);
}

inline int	SystemCall::get_exit()
{
	return(fd_exit_);
}

inline void	SystemCall::set_stdout_buffersize(int size)
{
	stdoutbuffersize_ = size;
}
inline void	SystemCall::set_stderr_buffersize(int size)
{
	stderrbuffersize_ = size;
}

inline bool SystemCall::getexitcode(int &exitcode)
{
    exitcode = exitcode_;
    return(hasexitcode_);
}

} // end namespace

#endif






#endif
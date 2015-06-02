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
 *  SystemCall.cc
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */


#include <Core/SystemCall/SystemCall.h>
#ifdef _WIN32
#include <io.h>
#endif

namespace SCIRun {

///////////////////////////////////////////////

SystemCallHandler::SystemCallHandler() :
  UsedWithLockingHandle<Mutex>("system_call_handler"),
  end_(false)
{
}

SystemCallHandler::~SystemCallHandler()
{
}


///////////////////////////////////////////////


SystemCallThread::SystemCallThread(SystemCallHandle syscall) :
  syscall_(syscall)
{
}

SystemCallThread::~SystemCallThread()
{
  // force LockingHandle to destroy the
  // object if we are the only one still using
  // the object.
  syscall_ = 0;
}

void	SystemCallThread::run()
{
  int fd_stderr;
  int fd_stdout;
  int fd_exit;

  syscall_->dolock();

  fd_stderr = syscall_->get_stderr();
  fd_stdout = syscall_->get_stdout();
  fd_exit   = syscall_->get_exit();
	
  syscall_->unlock();
	
  // Here the thread reading the stdout and stderr starts
	
  std::string stdout_buffer;
  std::string stderr_buffer;

  std::string	read_buffer;
  int	read_buffer_length = 512;
  read_buffer.resize(read_buffer_length);
	
  size_t  bytesread;
  size_t  linestart;
  size_t  lineend;
  size_t	buffersize;
	
  int maxfd = fd_stderr;
  if (fd_stdout > maxfd) maxfd = fd_stdout;
  if (fd_exit   > maxfd) maxfd = fd_exit;

  maxfd++;

  fd_set	selectset;
	
  bool dostdout = true;
  bool dostderr = true;
  bool doexit   = true;
  bool need_new_read = true;

  bool done = false;
	
  Time::SysClock lastsend = Time::currentTicks();
  Time::SysClock tickstowait = static_cast<Time::SysClock>(0.3*Time::ticksPerSecond());
  Time::SysClock curtime = lastsend;
    
  while(!done)
  {
    bool timeout = false;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 300000;
        
    FD_ZERO(&selectset);
    if (dostderr) FD_SET(fd_stderr,&selectset);	// Read from the stderr channel
    if (dostdout) FD_SET(fd_stdout,&selectset);	// Read from the stdout channel
    if (doexit)   FD_SET(fd_exit,&selectset);	// Read from the stdout channel

    if ((!dostdout)&&(!dostderr)) break;
		
    if (syscall_->use_timeout_)
    {
      int ret;
      while((ret = ::select(maxfd,&selectset,0,0,&tv)) < 0)
      {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) continue;
                      
        syscall_->signal_exit();
        syscall_->signal_eof();
        return;
      }      
            
      if (ret == 0) timeout = true; else timeout = false; 
      curtime = Time::currentTicks();
    }
    else
    {
      while( ::select(maxfd,&selectset,0,0,0) < 0)
      {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) continue;
                      
        std::cerr << "SystemCall: Detected an error during select" << std::endl;
                      
        syscall_->signal_exit();
        syscall_->signal_eof();
        return;
      }
    }
        
    if ((timeout == true)&&(stdout_buffer.size()))
    {
      // For some applications you want to have feedback quicker
      // So if there is data pending and there hasn't been new data
         
      linestart = 0;
      buffersize = stdout_buffer.size();
      while((linestart < buffersize)&&((stdout_buffer[linestart]=='\n')||(stdout_buffer[linestart]=='\r')||(stdout_buffer[linestart]=='\0'))) linestart++;			
      std::string newline = stdout_buffer.substr(linestart);
      stdout_buffer = "";
            
      syscall_->insert_stdout_line(newline);
            
      lastsend = Time::currentTicks();                           
    }
        
    if (timeout == false)
    {
      if (FD_ISSET(fd_exit,&selectset))
      {
        bytesread = read(fd_stdout,&(read_buffer[0]),read_buffer_length);
        if (bytesread > 0) 
        {
          std::string exitcode = read_buffer.substr(0,bytesread);	
          std::istringstream iss(exitcode);
          iss >> syscall_->exitcode_;
          syscall_->hasexitcode_ = true;
        }
                     
        syscall_->signal_exit();
        doexit = false;
      }

      if (FD_ISSET(fd_stdout,&selectset))
      {
        bytesread = read(fd_stdout,&(read_buffer[0]),read_buffer_length);

        if (bytesread < 0)
        {
          // I need to add a better error handling routine here
          if (errno == EAGAIN) continue;	// Unix requests that we try once more
          if (errno == EINTR) continue;	// The process got interupted somehow, so we just need to try again
          std::cerr << "SystemCall: Detected error while reading from STDOUT of a process running in an internal shell" << std::endl;
          return;
        }
        if (bytesread > 0) stdout_buffer += read_buffer.substr(0,bytesread);	// add the newly read buffer to the buffer remaining from the previoud read operation
            
        need_new_read = false;
                    
        while (!need_new_read)
        {
          // figure out where the new line starts
          // we discard any \n, \r and \0
          linestart = 0;	
          buffersize = stdout_buffer.size();
          while((linestart < buffersize)&&((stdout_buffer[linestart]=='\n')||(stdout_buffer[linestart]=='\r')||(stdout_buffer[linestart]=='\0'))) linestart++;			
                
          // newline will contain the newly read line from the data
          std::string newline;
          // if bytesread is 0, it indicates an EOF, hence we just need to add the remainder
          // of what is in the buffer. The program did not send us nicely terminated strings....
          if (bytesread == 0)
          {	
            if(linestart < stdout_buffer.size()) newline = stdout_buffer.substr(linestart);
            syscall_->insert_stdout_line(newline);
            lastsend = Time::currentTicks();
            dostdout = false;
            if(syscall_->signal_stdout_eof()) { done = true;  break; }
            break;   // Force exit out of lookp
          }
          else
          {
            // Detect whether we already read the end of line. If the line is huge it will not fit in the buffer and hence we need to do a econd read
            // or it could be that the message was split into multiple messages.
            lineend = linestart;
            while((lineend < buffersize)&&(stdout_buffer[lineend]!='\n')&&(stdout_buffer[lineend]!='\r')&&(stdout_buffer[lineend]!='\0')) lineend++;
            if (lineend == buffersize)
            {	// end of line not yet read
              need_new_read = true;
            }
            else
            {	// split of the latest line read
              newline = stdout_buffer.substr(linestart,(lineend-linestart)) + std::string("\n");
              stdout_buffer = stdout_buffer.substr(lineend+1);
              need_new_read = false;
              lastsend = Time::currentTicks();;
                                
              syscall_->insert_stdout_line(newline);
            }
          }
        }
                      
                      
        if ((lastsend + tickstowait < curtime)&&(syscall_->use_timeout_ == true))
        {
          linestart = 0;
          buffersize = stdout_buffer.size();
          while((linestart < buffersize)&&((stdout_buffer[linestart]=='\n')||(stdout_buffer[linestart]=='\r')||(stdout_buffer[linestart]=='\0'))) linestart++;			
          std::string newline = stdout_buffer.substr(linestart);
          stdout_buffer = "";
                        
          syscall_->insert_stdout_line(newline);
          lastsend = curtime;                        
        }
        
      }
    
      if (FD_ISSET(fd_stderr,&selectset))
      {
        bytesread = ::read(fd_stderr,&(read_buffer[0]),read_buffer_length);

        if (bytesread < 0)
        {
          // I need to add a better error handling routine here
          if (errno == EAGAIN) continue;	// Unix requests that we try once more
          if (errno == EINTR) continue;	// The process got interupted somehow, so we just need to try again
          std::cerr << "Detected error while reading from STDERR of a process running in an internal shell" << std::endl;
          return;
        }
        if (bytesread > 0) stderr_buffer += read_buffer.substr(0,bytesread);	// add the newly read buffer to the buffer remaining from the previoud read operation

        need_new_read = false;
        while (!need_new_read)
        {
          // figure out where the new line starts
          // we discard any \n, \r and \0
          linestart = 0;	
          buffersize = stderr_buffer.size();
          while((linestart < buffersize)&&((stderr_buffer[linestart]=='\n')||(stderr_buffer[linestart]=='\r')||(stderr_buffer[linestart]=='\0'))) linestart++;			
                
          // newline will contain the newly read line from the data
          std::string newline;
          // if bytesread is 0, it indicates an EOF, hence we just need to add the remainder
          // of what is in the buffer. The program did not send us nicely terminated strings....
          if (bytesread == 0)
          {	
            if(linestart < stderr_buffer.size()) newline = stderr_buffer.substr(linestart);
            syscall_->insert_stderr_line(newline);
            dostderr = false;
            if(syscall_->signal_stderr_eof()) { done = true; break; }
            break;

          }
          else
          {
            // Detect whether we already read the end of line. If the line is huge it will not fit in the buffer and hence we need to do a econd read
            // or it could be that the message was split into multiple messages.
            lineend = linestart;
            while((lineend < buffersize)&&(stderr_buffer[lineend]!='\n')&&(stderr_buffer[lineend]!='\r')&&(stderr_buffer[lineend]!='\0')) lineend++;
            if (lineend == stderr_buffer.size())
            {	// end of line not yet read
              need_new_read = true;
            }
            else
            {	// split of the latest line read
              newline = stderr_buffer.substr(linestart,(lineend-linestart)) + std::string("\n");
              stderr_buffer = stderr_buffer.substr(lineend+1);
              need_new_read = false;
            }
            syscall_->insert_stderr_line(newline);
          }
        }
      }
    }
  }
  // Signal that we have an end-of-file
  // for both the stderr and stdout
	
  syscall_->signal_eof();
}





SystemCall::SystemCall() :
  UsedWithLockingHandle<Mutex>("system_call_lock"),
  isrunning_(false),
  fd_stdin_(-1),
  fd_stdout_(-1),
  fd_stderr_(-1),
  fd_exit_(-1),
  processid_(0),
  isfile_(false),
  stdout_eof_(false),
  stderr_eof_(false),
  iseof_(false),
  eof_("systemcall_eof_condition"),
  isexit_(false),
  exit_("systemcall_exit_condition"),
  stdoutbuffersize_(0),
  stderrbuffersize_(0),
  exitcode_(0),
  hasexitcode_(false)
{
}


SystemCall::~SystemCall()
{

  // Kill any remaining process
  // This function returns immediately if there is nothing to kill
  // It will dolock() inside the function
  try
  {
        
    if (processid_)
    {
      systemcallmanager_->kill(processid_);
      systemcallmanager_->close(processid_);
    }
        
    // Make sure every handler got an end signal
    std::list<SystemCallHandlerHandle>::iterator it;
    for (it = stdouthandler_.begin(); it != stdouthandler_.end(); it++)
    {
      if ((*it)->end_ == false) (*it)->end();
    }

    stdouthandler_.clear();

    for (it = stderrhandler_.begin(); it != stderrhandler_.end(); it++)
    {
      if ((*it)->end_ == false) (*it)->end();
    }
     
    stderrhandler_.clear();

    for (it = exithandler_.begin(); it != exithandler_.end(); it++)
    {
      if ((*it)->end_ == false) (*it)->end();
    }

    exithandler_.clear();     
        
    if (isfile_) file_.close();

    isfile_    = false;
    isrunning_ = false;
    fd_stdin_  = -1;
    fd_stdout_ = -1;
    fd_stderr_ = -1;
    fd_exit_   = -1;
    processid_ = 0;
        
    stdoutbuffer_.clear();
    stderrbuffer_.clear();
       
  }
  catch (...)
  {
    std::cerr << "Caught an exception in the destructor of SystemCall class" << std::endl;
  }
}


void SystemCall::execute(std::string command)
{

  dolock();
  // If the SystemCall is already handling another call
  if (isrunning_) throw(SystemCallError("Object still running another shell command",0,SCE_INUSE));

  // If no command is specified throw an error
  if(command == "") throw SystemCallError("No command to execute",0,SCE_NOCMD);

  // Store the instuctions in the object
  if (!(systemcallmanager_))  throw SystemCallError("The systemcallmanager has not been forked",0,SCE_NOSYSMANAGER);
  unlock();

  Thread				*thread = 0;
  SystemCallThread	*systemcallprivatethread = 0;

  try
  {
    processid_ = systemcallmanager_->exec(command);
    fd_stdin_  = systemcallmanager_->getstdin(processid_);
    fd_stdout_ = systemcallmanager_->getstdout(processid_);
    fd_stderr_ = systemcallmanager_->getstderr(processid_);
    fd_exit_   = systemcallmanager_->getexit(processid_);
		
    SystemCallHandle myhandle = this;
    systemcallprivatethread = new SystemCallThread(myhandle);
    if (systemcallprivatethread == 0) throw SystemCallError("Could not create systemcall thread private object",0,SCE_THREADERROR);
	
    // we need to call this before starting the thread
    isrunning_ = true;
			
    thread = new Thread(systemcallprivatethread,"system_call_private_thread");
    if (thread == 0) throw SystemCallError("Could not launch thread",0,SCE_THREADERROR);
    thread->detach();

  }
  catch(...)
  {
    std::cerr << "SystemCall: Caught an error\n";
    systemcallmanager_->kill(processid_);
    systemcallmanager_->close(processid_);
		
    isrunning_ = false;
    fd_stdin_  = -1;
    fd_stdout_ = -1;
    fd_stderr_ = -1;
    fd_exit_   = -1;
    processid_ = 0;
		
    throw;
  }
	
}

void SystemCall::wait_eof()
{
  dolock();
  if (iseof_)
  {
    unlock();
    return;
  }
  eof_.wait(lock);
  isrunning_ = false;
  unlock();
  return;
}

void SystemCall::signal_eof()
{
  dolock();
  iseof_ = true;
  eof_.conditionBroadcast();
  unlock();
}

void SystemCall::signal_exit()
{
  dolock();
  isexit_ = true;
  std::list<SystemCallHandlerHandle>::iterator it = exithandler_.begin();
  for (; it != exithandler_.end(); it++)  { (*it)->end(); (*it)->end_ = true; }
  exit_.conditionBroadcast();
  unlock();
}


void SystemCall::wait()
{
  dolock();
  if (isexit_)
  {
    unlock();
    return;
  }
  exit_.wait(lock);
  isrunning_ = false;
  unlock();
  return;
}


void SystemCall::kill(int secs)
{
  dolock();
  if (processid_) 
  {
    if (isrunning_) systemcallmanager_->kill(processid_,secs);
  }
  isrunning_ = false;				// set state to not running
  unlock();
}


bool SystemCall::signal_stdout_eof()
{
  dolock();
  stdout_eof_ = true;
	
  // signal the handlers that we read the end of file
  std::list<SystemCallHandlerHandle>::iterator it = stdouthandler_.begin();
  for (; it != stdouthandler_.end(); it++)  { (*it)->end(); (*it)->end_ = true; }
	
  bool retval = stderr_eof_;
  unlock();
  return(retval);		
}

bool SystemCall::signal_stderr_eof()
{
  dolock();
  stderr_eof_ = true;

  // signal the handlers that we read the end of file
  std::list<SystemCallHandlerHandle>::iterator it = stderrhandler_.begin();
  for (; it != stderrhandler_.end(); it++) { (*it)->end(); (*it)->end_ = true; }
	
  bool retval = stdout_eof_;
  unlock();
  return(retval);
}

void SystemCall::put_stdin(std::list<std::string> &vec, bool raw)
{
  dolock();
    
  if (!isrunning_) 
  { 
    unlock(); 
    throw(SystemCallError("No process is running",0,SCE_NOCMD));
  }
	
  try
  {
    std::list<std::string>::iterator p;
    for (; p!= vec.end(); p++)	
    {
      if (raw == false)
      {
        if ((*p).size() == 0)
        {
          std::string empty = "\n";
          put_stdin_int(empty);
        }
        else
        {
          if ((*p)[(*p).size()-1] != '\n') 
          {
            std::string newstring = (*p) + std::string("\n");
            put_stdin_int(newstring);
          }
          else
          {
            put_stdin_int((*p));
          }
        }
      }
      else
      {
        put_stdin_int((*p));
      }
    }
  }
  catch(...)
  {
    unlock();
    throw;
  }
  unlock();	
}




void SystemCall::put_stdin(std::vector<std::string> &vec, bool raw)
{
  dolock();
	
  if (!isrunning_) 
  { 
    unlock(); 
    throw(SystemCallError("No process is running",0,SCE_NOCMD)); 
  }
	
  try
  {
    for (int p = 0; p < (int)vec.size(); p++)	
    {
      if (raw == false)
      {
        if (vec[p].size() == 0)
        {
          std::string empty = "\n";
          put_stdin_int(empty);
        }
        else
        {
          if (vec[p][vec[p].size()-1] != '\n') 
          {
            std::string newstring = vec[p] + std::string("\n");
            put_stdin_int(newstring);
          }
          else
          {
            put_stdin_int(vec[p]);
          }
        }
      }
      else
      {
        put_stdin_int(vec[p]);
      }
    }
  }
  catch (...)
  {
    unlock();
    throw;
  }
  unlock();	
}


void SystemCall::put_stdin(std::string &str, bool raw)
{
  dolock();

  if (!isrunning_) 
  { 
    unlock(); 
    throw(SystemCallError("No process is running",0,SCE_NOCMD));  
  }
	
  try
  {
    if (raw == false)
    {
      if (str.size() == 0)
      {
        std::string empty = "\n";
        put_stdin_int(empty);
      }
      else
      {
        if (str[str.size()-1] != '\n') 
        {
          std::string newstring = str + std::string("\n");
          put_stdin_int(newstring);
        }
        else
        {
          put_stdin_int(str);
        }
      }
    }
    else
    {
      put_stdin_int(str);
    }
  }
  catch(...)
  {
    unlock();
    throw;
  }
  unlock();	
}


void SystemCall::put_stdin_int(std::string &str)
{
  // internal function, locking is done in 
  // function that calls this function
	
  if (systemcallmanager_->exit_ == true) return;
    
  size_t bytestowrite = str.size();
  size_t byteswritten = 0;
  size_t len = 0;
  while (byteswritten < bytestowrite)
  {
    len = ::write(fd_stdin_,&(str[byteswritten]),bytestowrite-byteswritten);
    if (len < 0)
    {
      if (errno == EINTR) continue;		// Function was interupted by a signal
      if (errno == EAGAIN) continue;		// Kernel wants us to try once more
			
      // No hope left, so just generate an error
      // throw SystemCallError("Error writing to stdin",errno,SCE_IOERROR);
    }
    byteswritten += len;
  }
}




bool SystemCall::create_logfile(std::string filename)
{
  dolock();
  try
  {
    file_.open(filename.c_str(),std::ios::out);
    isfile_	= true;
  }
  catch (...)
  {
    isfile_ = false;
    unlock();
    return(false);
  }
  unlock();
  return(true);
}

bool SystemCall::append_logfile(std::string filename)
{
  dolock();
  try
  {
    file_.open(filename.c_str(),std::ios::app);
    isfile_	= true;
  }
  catch (...)
  {
    isfile_ = false;
    unlock();
    return(false);
  }
	
  unlock();
  return(true);
}


void SystemCall::insert_stdout_line(std::string &line)
{
  // Add the line to the file log;
  dolock();

  if (isfile_) file_ << line;
	
  if (stdoutbuffersize_ != 0)
  {
    stdoutbuffer_.push_back(line);
    if ((stdoutbuffersize_ > 0)&&(stdoutbuffersize_ < (int)stdoutbuffer_.size())) stdoutbuffer_.pop_front();
  }
	
  std::list<SystemCallHandlerHandle>::iterator it = stdouthandler_.begin();
  for (; it != stdouthandler_.end(); it++)
  {
    if(!((*it)->execute(line))) break;
  }
	
  unlock();
}

void SystemCall::insert_stderr_line(std::string &line)
{
  dolock();

  if (stderrbuffersize_ != 0)
  {
    stderrbuffer_.push_back(line);
    if ((stderrbuffersize_ > 0)&&(stderrbuffersize_ < (int)stderrbuffer_.size())) stderrbuffer_.pop_front();
  }

  if (isfile_) file_ << "STD_ERROR_OUTPUT: " <<line;
	
  std::list<SystemCallHandlerHandle>::iterator it = stderrhandler_.begin();
  for (; it != stderrhandler_.end(); it++)
  {
    if(!((*it)->execute(line))) break;
  }

  unlock();
}


void SystemCall::add_stdout_handler(SystemCallHandlerHandle handle, bool front)
{
  dolock();

  if (front)
  {
    stdouthandler_.push_front(handle);
  }
  else
  {
    stdouthandler_.push_back(handle);
  }
  handle->start(stdoutbuffer_);
	
  if (isexit_) { handle->end(); handle->end_ = true; }

  unlock();
}

void SystemCall::rem_stdout_handler(SystemCallHandlerHandle handle)
{
  dolock();
  stdouthandler_.remove(handle);
  unlock();
}

void SystemCall::add_stderr_handler(SystemCallHandlerHandle handle, bool front)
{
  dolock();
	
  if (front)
  {
    stderrhandler_.push_front(handle);
  }
  else
  {
    stderrhandler_.push_back(handle);
  }
  handle->start(stderrbuffer_);
  if (isexit_) { handle->end(); handle->end_ = true; }
	
  unlock();
}

void SystemCall::rem_stderr_handler(SystemCallHandlerHandle handle)
{
  dolock();
  stderrhandler_.remove(handle);
  unlock();
}

void SystemCall::add_exit_handler(SystemCallHandlerHandle handle)
{
  dolock();
	
  exithandler_.push_back(handle);
  // for this one we don not call start, it is an exit handler
  if (isexit_) { handle->end(); handle->end_ = true; }
  unlock();
}

void SystemCall::rem_exit_handler(SystemCallHandlerHandle handle)
{
  dolock();
  exithandler_.remove(handle);
  unlock();
}

} // namespace

#endif
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


#include <Core/Services/Service.h>
#include <Core/Services/SimpleService.h>
#include <Core/SystemCall/SystemCall.h>
#include <Packages/MatlabInterface/Services/MatlabEngine.h>
#include <Core/Util/Environment.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

namespace SCIRun {

class MatlabCall;
class MatlabEngine;

typedef LockingHandle<MatlabCall> MatlabCallHandle;


class   MatlabCallHandler : public SystemCallHandler {
  public:

        MatlabCallHandler(MatlabCall* handle);
        virtual bool execute(std::string line);
        
  public:
        MatlabCall* handle_;
};

typedef LockingHandle<MatlabCallHandler> MatlabCallHandlerHandle;


class   MatlabCallErrorHandler : public SystemCallHandler {
  public:

        MatlabCallErrorHandler(MatlabCall* handle);
        virtual bool execute(std::string line);
        
  public:
        MatlabCall* handle_;
};

typedef LockingHandle<MatlabCallErrorHandler> MatlabCallErrorHandlerHandle;


class MatlabCall : public SystemCall {
  public:
    MatlabCall();
    virtual ~MatlabCall();
    
    void                start_engine(std::string command);
    void                close_engine();
    void                lock_engine(MatlabEngine* ptr);
    void                unlock_engine();
    
    SystemCallHandle    getsyscallhandle();

    void                obtain();
    bool                release();
    int                 engine_count;

    bool                engine_in_use_;
    bool                detected_error_;
    bool                passed_test_;
    bool                start_test_;
    MatlabEngine*       engine_ptr_;
    ConditionVariable   wait_in_use_;
    ConditionVariable   wait_test_;
    MatlabCallHandlerHandle         handler_;
    MatlabCallErrorHandlerHandle    error_handler_;
};

// global to register all engines running

std::map<int,MatlabCallHandle>  matlab_processes_;
Mutex matlab_processes_lock_("matlab processes lock");

// Create a class for the remote service
class MatlabEngine : public SimpleService {

  public:
        // constructor
        MatlabEngine(ServiceContext &ctx);

        // executable code
        bool init_service(IComPacketHandle& packet);
        void handle_service(IComPacketHandle& packet);
        void close_service();

        void send_end_command(bool detected_error);
                                        
  private:
        std::string     addcode(std::string &mfile);
         
        MatlabCallHandle matlab_handle_;        
};

// Mark the class for the dynamic loader
DECLARE_SERVICE_MAKER(MatlabEngine)

MatlabCallHandler::MatlabCallHandler(MatlabCall* handle) :
        handle_(handle)
{
}

bool
MatlabCallHandler::execute(std::string line)
{
  if (line == "SCIRUN-MATLABINTERFACE-MATLABENGINE-END\n") 
    {
      if (handle_->engine_ptr_)
        {
          handle_->engine_ptr_->send_end_command(handle_->detected_error_);
          handle_->engine_ptr_ = 0;
        }

      handle_->detected_error_ = false;
      handle_->engine_in_use_ = false;
      handle_->wait_in_use_.conditionBroadcast();
    }
  if (line == "SCIRUN-MATLABINTERFACE-MATLABENGINE-ERROR\n") 
    {
      handle_->detected_error_ = true;
    }
        
  if (!(handle_->passed_test_))
    {
      if (line == "MATLABENGINE-PASSED-TEST\n")
        {   
          handle_->passed_test_ = true;
          handle_->wait_test_.conditionBroadcast();
          return(false); // Don't forward this to the user
        }
      return(true); // Don't forward this to the user
    }
  return(true);   
}


MatlabCallErrorHandler::MatlabCallErrorHandler(MatlabCall* handle) :
        handle_(handle)
{
}


bool MatlabCallErrorHandler::execute(std::string /*line*/)
{
 // The following piece of code quits the matlab engine and reports an
 // error as soon as matlab reports an error on the stderr channel
 //
 // However under OSX Tiger Matlab always reports errors on stderr as
 // some code is probably not fully compatible
 // Hence we disable this security feature here

 /*
  if (!handle_->passed_test_)
  {
    std::cout << "ERROR DETECTED : " << line << "\n";
    handle_->wait_test_.conditionBroadcast();
  }

  // release engine if data is send to stderr

  if (handle_->engine_ptr_)
    {
      handle_->engine_ptr_->send_end_command(true);
      handle_->engine_ptr_ = 0;
    }

  handle_->detected_error_ = false;
  handle_->engine_in_use_ = false;
  handle_->wait_in_use_.conditionSignal();
*/
        
  return(true);
}


MatlabCall::MatlabCall() :
  engine_count(0),
  engine_in_use_(false),
  detected_error_(false),
  passed_test_(false),
  start_test_(false),
  engine_ptr_(0),
  wait_in_use_("wait for matlab to finish"),
  wait_test_("wait for matlab engine to pass test")
{
  handler_ = new MatlabCallHandler(this);
  error_handler_ = new  MatlabCallErrorHandler(this);
  add_stdout_handler(dynamic_cast<SystemCallHandler*>(handler_.get_rep()));
  add_stderr_handler(dynamic_cast<SystemCallHandler*>(error_handler_.get_rep()));
  use_stdout_timeout(true);
}

MatlabCall::~MatlabCall()
{
  close_engine();
  SystemCallHandler *handle;
  handle = dynamic_cast<SystemCallHandler*>(handler_.get_rep());
  if (handle) rem_stdout_handler(handle);
  handle = dynamic_cast<SystemCallHandler*>(error_handler_.get_rep());
  if (handle) rem_stderr_handler(handle);
}

void
MatlabCall::obtain()
{
  dolock();
  engine_count++;
  unlock();
}

bool
MatlabCall::release()
{
  dolock();
  engine_count--;
  if (engine_count == 0) 
    {
      unlock();
      close_engine();
      return(true);
    }
  else
    {
      unlock();
      return(false);
    }
}

void
MatlabCall::lock_engine(MatlabEngine* ptr)
{
  dolock();
  while (engine_in_use_)
    {
      wait_in_use_.wait(lock);
    }
  engine_in_use_ = true;
  engine_ptr_ = ptr;
  unlock();
}

void
MatlabCall::unlock_engine()
{
  dolock();
  engine_in_use_ = false;
  engine_ptr_ = 0;
  unlock();
}


inline SystemCallHandle MatlabCall::getsyscallhandle()
{
  SystemCallHandle handle = dynamic_cast<SystemCall *>(this);
  return(handle);
}

inline void MatlabCall::start_engine(std::string command)
{
  execute(command);
}

inline void MatlabCall::close_engine()
{
  if (isrunning())
    {
      std::string exitcmd = "exit\n";
      put_stdin_int(exitcmd);
      // Give the engine 20 seconds before we pull the plug
      kill(20);
    }
}




// Constructor
MatlabEngine::MatlabEngine(ServiceContext &ctx) :
  SimpleService(ctx)
{
}

bool MatlabEngine::init_service(IComPacketHandle &packet)
{
  int session = getsession();
 
  putmsg("MatlabEngine: Got session number");
 
  matlab_processes_lock_.lock();
  matlab_handle_ = matlab_processes_[session];
    
  if (session == 0)
  {
    // Create new unique session
    int p = 1;
    while(1)
      {
        if(matlab_processes_[p].get_rep() == 0) break;
      }
      session = p;
      p++;
  }
    
  setsession(session);
    
  putmsg("MatlabEngine: getting matlab process handle");
        
  if (matlab_handle_.get_rep() == 0)
    {   // start engine
      putmsg("MatlabEngine: trying to start matlab engine");
			updateparameters();
      std::string startmatlab = getparameter("startmatlab");
      
      std::string startmatlab2 = getstartcommand();
      if (startmatlab2.size() != 0) startmatlab = startmatlab2;
      
      putmsg("MatlabEngine: Get matlab startup command:" + startmatlab);
      if (startmatlab == "")
      {
        putmsg("MatlabEngine: Could not read startmatlab from matlabengine.rc file");
        packet->settag(TAG_MERROR);
        packet->setstring("The matlabengine.rc file does not specify how to start matlab");
        matlab_processes_lock_.unlock();
        return(false);
      }  
      try
      {
        putmsg("MatlabEngine: initializing matlab call");
        matlab_handle_ = new MatlabCall;
        if (matlab_handle_.get_rep() == 0) throw SystemCallError("Could not create Systemcall",0,SystemCallBase::SCE_BADALLOC);

        putmsg("MatlabEngine: Start forwarding stdio");
        forward_stdout(matlab_handle_->getsyscallhandle());
        forward_stderr(matlab_handle_->getsyscallhandle());
        forward_exit(matlab_handle_->getsyscallhandle());            
          
        putmsg("MatlabEngine: start matlab engine");
        matlab_handle_->start_engine(startmatlab);
        matlab_handle_->obtain();
        matlab_processes_[session] = matlab_handle_;
        matlab_processes_lock_.unlock();
        putmsg("MatlabEngine: Started matlab engine successfully");
      }
      catch(SystemCallError& error)
      {
        putmsg("MatlabEngine: Could not launch matlab process");
        putmsg("MatlabEngine: systemcall return following error: " + error.geterror());
        matlab_processes_lock_.unlock();
        packet->settag(TAG_MERROR);
        packet->setstring("Could not launch matlab");
        return(false);                  
      }
      catch(std::exception& exp)
      {
        putmsg("MatlabEngine: Could not launch matlab process");
        putmsg("MatlabEngine: std-lib-exception: " + std::string(exp.what()));
        matlab_processes_lock_.unlock();
        packet->settag(TAG_MERROR);
        packet->setstring("Could not launch matlab");
        return(false);  
      }
      catch(...)
      {
        putmsg("MatlabEngine: Could not launch matlab process");
        putmsg("MatlabEngine: unknown cause");
        matlab_processes_lock_.unlock();
        packet->settag(TAG_MERROR);
        packet->setstring("Could not launch matlab");
        return(false);
      }
    }
  else
  {   // If another module already started this matlab engine session
    putmsg("MatlabEngine: Matlab is already running");
    matlab_handle_->obtain();
    matlab_processes_lock_.unlock();                
          
    putmsg("MatlabEngine: Start forwarding stdio");
    forward_stdout(matlab_handle_->getsyscallhandle());
    forward_stderr(matlab_handle_->getsyscallhandle());
    forward_exit(matlab_handle_->getsyscallhandle());
  }

  putmsg("MatlabEngine: Send test to matlab to see whether it is working");
  matlab_handle_->dolock();
  if (!(matlab_handle_->start_test_))
  {
    // We print it backwards in case there is a loop back, this way
    // it tests matlab engine properly
    std::string testcode = "fprintf(1,fliplr('n\\n\\TSET-DESSAP-ENIGNEBALTAMn\\'))\n";
    matlab_handle_->put_stdin_int(testcode);
    matlab_handle_->start_test_ = true;
  }
        
  if (!(matlab_handle_->passed_test_))
  {
    int secondstowait = 180;        // built in  default time we should wait before matlab is launched
    std::string timetowait = getparameter("matlabtimeout");

    if (timetowait!="")
    {
      std::istringstream iss(timetowait);
      iss >> secondstowait;
      if (secondstowait  < 1) secondstowait = 180;
    }
    
    if (gettimeout() > 0) secondstowait = gettimeout();
    
#ifndef _WIN32
      // FIX
      struct timeval  tv;
      struct timespec tm;
      gettimeofday(&tv,0);
      tm.tv_nsec = 0;
      tm.tv_sec = tv.tv_sec + secondstowait;
      matlab_handle_->wait_test_.timedWait( matlab_handle_->lock, &tm);
#endif

    if (!(matlab_handle_->passed_test_))
    {
      matlab_handle_->unlock();
      packet->settag(TAG_MERROR);
      packet->setstring("Matlab did not launch properly, it failed in the testing stage");
                                            
      return(false);
    }      
  }
  matlab_handle_->unlock();


  // Need to add some code to validate test
        
  putmsg("MatlabEngine: Success, send success signal to client");
  packet->settag(TAG_MSUCCESS);
  packet->setparam1(session);    
  return(true);
}

void MatlabEngine::close_service()
{
  try
    {
      int session = getsession();

      if (matlab_handle_.get_rep() != 0)
        {        
          try
            {
              stop_forward_stdout();
              stop_forward_stderr();
              stop_forward_exit();
            }
          catch(SystemCallError& error)
            {
              std::cerr << "MatlabEngine Caught an exception:" << std::endl;
              std::cerr << "Error: " << error.geterror() << std::endl;
            }
          matlab_processes_lock_.lock();
          try
            {
              if (matlab_handle_->release())
                {
                  matlab_processes_[session] = 0;
                }
            }
          catch(SystemCallError& error)
            {
              std::cerr << "MatlabEngine Caught an exception:" << std::endl;
              std::cerr << "Error: " << error.geterror() << std::endl;
            }        
          matlab_processes_lock_.unlock();
          matlab_handle_ = 0;
        }
    }
  catch (...)
    {
      // This process may have crashed but we want others to continue
      // Hence unlocking this lock is important
      matlab_processes_lock_.unlock(); 
      throw;
    } 
}


// Executable piece of code
void MatlabEngine::handle_service(IComPacketHandle &packet)
{
  switch (packet->gettag())
    {
    case TAG_INPUT:
    {
      if (packet->getelsize() != 1)
        {
          packet->settag(TAG_MERROR);
          packet->setstring("Code needs to be of character size");
          send_packet(packet);
          break;
        }
      if (packet->getdatasize() == 0)
        {
          packet->settag(TAG_MERROR);
          packet->setstring("No matlab code was send");
        }

      std::string str = packet->getstring();
      matlab_handle_->put_stdin(str,true);
    }                    
    break;

    case TAG_MCODE:
    {
      if (packet->getelsize() != 1)
        {
          packet->settag(TAG_MERROR);
          packet->setstring("Code needs to be of character size");
          send_packet(packet);
          break;
        }
      if (packet->getdatasize() == 0)
        {
          packet->settag(TAG_MERROR);
          packet->setstring("No matlab code was send");
        }

      std::string mfile = packet->getstring();
      std::string code = addcode(mfile);
                        
      // Add one line that puts an unique message on the output so we know that the
      // engine is done
                        
      matlab_handle_->lock_engine(this);
      matlab_handle_->put_stdin(code);
                        
      // unlocking of the engine will be done by the stdout/stderr handler
      // as soon as they recognise an end or error message on the output
      // the engine will be unlocked for the next job
    }
    break;
    }
}

std::string     MatlabEngine::addcode(std::string &mfile)
{
  // This function wraps an error detection mechanism around the m-code the
  // user supplied

  size_t lastslash = mfile.size()+1;
  for (size_t p = 0; p < mfile.size(); p++)
    {
      if (mfile[p] == '/') lastslash = p;
    }
  std::string path;
  std::string command;
  if (lastslash <= mfile.size())
    {
      path = mfile.substr(0,lastslash+1);
      command = mfile.substr(lastslash+1);
    } 
  else
    {
      path = "";
      command = mfile;
    }

  const char *srcdir = sci_getenv("SCIRUN_SRCDIR");
  std::string matlablibrarypath = srcdir + std::string("/Packages/MatlabInterface/MatlabLibrary");

  std::string newcode;
  newcode += "\nfprintf(1,'\\nSCIRUN-MATLABINTERFACE-MATLABENGINE-START\\n');\n";

  newcode += "try\n";
  newcode += "addpath('" + matlablibrarypath + "')\n";
  newcode += "scirunMountLibrary\n";
  newcode += "\ncatch\n";
  newcode += "disp('Failed to mount SCIRun/Matlab function library')\n";
  newcode += "end\n";
  newcode += "try\n";
  
  if(path != "") newcode += "addpath('" + path + "')\n";
  newcode += command;
  newcode += "\ncatch\n";
  newcode += "fprintf(1,'\\nSCIRUN-MATLABINTERFACE-MATLABENGINE-ERROR\\n');\n";
  newcode += "disp(lasterr)\n";
  newcode += "end\n";
  newcode += "fprintf(1,'\\n\\nSCIRUN-MATLABINTERFACE-MATLABENGINE-END\\n');\n";
  return(newcode);
}

void MatlabEngine::send_end_command(bool detected_error)
{
  IComPacketHandle packet = new IComPacket;
  if (detected_error)
  {
    packet->settag(TAG_MCODE_ERROR);
    packet->setstring("Detected an error in the matlab code, could not fully execute code");
    send_packet(packet);
  }
  else
  {
    packet->settag(TAG_MCODE_SUCCESS);
    send_packet(packet);
  }
}

}

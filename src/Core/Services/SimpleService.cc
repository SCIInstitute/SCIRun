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


// SimpleService.cc

#include <Core/Services/SimpleService.h>
#include <sstream>

namespace SCIRun {

SimpleServiceOutputInfo::SimpleServiceOutputInfo(IComSocket socket,ServiceLogHandle log) :
  UsedWithLockingHandle<Mutex>("SimpleServiceOutputThread Lock"),
  packet_list_(0),
  wait_("SimpleServiceOutputThread Condition Variable"),
  socket_(socket),
  exit_(false),
  log_(log)
{
}

SimpleServiceOutputInfo::~SimpleServiceOutputInfo()
{
  delete packet_list_;
}



SimpleServiceOutputThread::SimpleServiceOutputThread(SimpleServiceOutputInfoHandle handle) :
  handle_(handle)
{
}

SimpleServiceOutputThread::~SimpleServiceOutputThread()
{
}

void SimpleServiceOutputThread::run()
{
  try
  {
      std::list<IComPacketHandle> *plist;
      handle_->lock.lock();
      while(!handle_->exit_)
      {
          if (handle_->packet_list_)
          {
              plist = handle_->packet_list_;
              handle_->packet_list_ = 0;
              handle_->lock.unlock();
              std::list<IComPacketHandle>::iterator it;
              for (it = plist->begin(); it != plist->end(); it++)
              {
                  if(!(handle_->socket_.send((*it))))
                  {
                     if (!(handle_->socket_.isconnected()))
                     {  // remote connection was closed
                        delete plist;
                        handle_->lock.lock();
                        break;
                     }
                     else
                     {
                        std::cerr << "SimpleServiceOutputThread: error sending packet" << std::endl;
                     }
                  }
              }	
              delete  plist;
              handle_->lock.lock();	
           }
           handle_->wait_.wait(handle_->lock);
      }
	
      if (handle_->exit_)
      {
          IComPacketHandle packet = new IComPacket;
          packet->settag(TAG_END_);
          handle_->socket_.send(packet);
      }
      handle_->lock.unlock();
  }
  catch(...)
  {
      handle_->log_->putmsg("SimpleServiceOutputThread: thread caught an exception");
      handle_->lock.unlock();
  }
	
  // In case we caught an exception wait until we receive an exit signal
  // from the main task. It is using this threads resources so we cannot exit
	
  handle_->lock.lock();

  while (!handle_->exit_)
  {
      handle_->wait_.wait(handle_->lock);
      // delete all incoming traffic
      // This process stalled and we cannot relay
      // output. To keep the program from crashing
      // immediately we do not forward any messages
      if (handle_->packet_list_) delete handle_->packet_list_;
      handle_->packet_list_ = 0;
  }

  handle_->lock.unlock();
  
  // Let's clean up here, so SCIRun does not have to do it.
  // It does not always invoke the destructors.
  handle_ = 0;  
  return;
}

///////////////////////////////////////////////////


SimpleServiceOutputHandler::SimpleServiceOutputHandler(SimpleServiceOutputInfoHandle handle,int tag) :
  handle_(handle),
  count_(0),
  tag_(tag)
{
}

SimpleServiceOutputHandler::~SimpleServiceOutputHandler()
{
}

void SimpleServiceOutputHandler::start(std::list<std::string> &buffer)
{
  std::list<std::string>::iterator it;
  for (it = buffer.begin(); it != buffer.end(); it++)
  {
      IComPacketHandle handle = new IComPacket();
      handle->settag(tag_);
      handle->setparam1(count_++);
      handle->setstring((*it));
      handle_->add_packet(handle);
  }
}

bool SimpleServiceOutputHandler::execute(std::string line)
{

  IComPacketHandle handle = new IComPacket();
  handle->settag(tag_);
  handle->setparam1(count_++);
  handle->setstring(line);
  handle_->add_packet(handle);
  return(true);
}

void SimpleServiceOutputHandler::end()
{
  IComPacketHandle handle = new IComPacket();
  handle->settag(tag_);
  handle->setparam1(-1);
  handle_->add_packet(handle);
}

///////////////////////////////////////////////////

void SimpleService::forward_stdout(SystemCallHandle syscall)
{
  stdout_syscall_ = syscall;
  stdout_handler_ = new SimpleServiceOutputHandler(info_handle_,TAG_STDO);
  SystemCallHandlerHandle handle = dynamic_cast<SystemCallHandler*>(stdout_handler_.get_rep());
  if(handle.get_rep()) syscall->add_stdout_handler(handle);
}

void SimpleService::forward_stderr(SystemCallHandle syscall)
{
  stderr_syscall_ = syscall;
  stderr_handler_ = new SimpleServiceOutputHandler(info_handle_,TAG_STDE);
  SystemCallHandlerHandle handle = dynamic_cast<SystemCallHandler*>(stderr_handler_.get_rep());
  if(handle.get_rep()) syscall->add_stderr_handler(handle);
}

void SimpleService::forward_exit(SystemCallHandle syscall)
{
  exit_syscall_ = syscall;
  exit_handler_ = new SimpleServiceOutputHandler(info_handle_,TAG_EXIT);
  SystemCallHandlerHandle handle = dynamic_cast<SystemCallHandler*>(exit_handler_.get_rep());
  if(handle.get_rep()) syscall->add_exit_handler(handle);
}

void SimpleService::stop_forward_stdout()
{
  if (stdout_syscall_.get_rep() != 0)
  {
      if (stdout_handler_.get_rep() != 0) 
      {
          SystemCallHandlerHandle handle = dynamic_cast<SystemCallHandler*>(stdout_handler_.get_rep());
          stdout_syscall_->rem_stdout_handler(handle);
      }
      stdout_handler_ = 0;
      stdout_syscall_ = 0;
  }
}

void SimpleService::stop_forward_stderr()
{
  if (stderr_syscall_.get_rep() != 0)
  {
      if (stderr_handler_.get_rep() != 0) 
      {
          SystemCallHandlerHandle handle = dynamic_cast<SystemCallHandler*>(stderr_handler_.get_rep());
          stderr_syscall_->rem_stderr_handler(handle);
      }
      stderr_handler_ = 0;
      stderr_syscall_ = 0;
  }
}

void SimpleService::stop_forward_exit()
{
  if (exit_syscall_.get_rep() != 0)
  {
      if (exit_handler_.get_rep() != 0)
      {
          SystemCallHandlerHandle handle = dynamic_cast<SystemCallHandler*>(exit_handler_.get_rep());
          exit_syscall_->rem_exit_handler(handle);
      }
      exit_handler_ = 0;
      exit_syscall_ = 0;
  }
}


SimpleService::SimpleService(ServiceContext &ctx) :
  Service(ctx),
  output_thread_(0)  
{
}


SimpleService::~SimpleService()
{
  // Stop all the forwarding of output to the output thread
  stop_forward_stdout();
  stop_forward_stderr();
  stop_forward_exit();

  // Make sure the thread knows we are exiting
  kill_output_thread();

}


void SimpleService::create_service_info()
{
    
    // if it is already running
    if (info_handle_.get_rep() != 0) return;

    try
    {
        info_handle_ = new SimpleServiceOutputInfo(getsocket(),getlog());
    }
    catch (...)
    {
        // Just inform of what went wrong
        // This is a crucial error, so we do not recover from this one
        errormsg("Could not launch object for handling output");
        throw;
    }

}

void SimpleService::create_output_thread()
{
    // launch output thread
    // Since communication over socket may be interupted by small pauses we do not
    // want the input handling and buffering of stdout/stderr to be affected. Hence
    // output is generate asynchronely

    if (info_handle_.get_rep() == 0) return;
    
    try
    {
        SimpleServiceOutputThread* ssot = new SimpleServiceOutputThread(info_handle_);
        
        output_thread_ = new Thread(ssot,"SimpleServiceOutputThread");
        output_thread_->detach();
        output_thread_ = 0;

    }
    catch (...)
    {
        // Just inform of what went wrong
        // This is a crucial error, so we do not recover from this one
        errormsg("Could not launch a thread for handling output");
        throw;
    }
}


void SimpleService::kill_output_thread()
{
  if (info_handle_.get_rep()) info_handle_->signal_exit();
  info_handle_ = 0;
}

void SimpleService::execute()
{
    putmsg("SimpleService: Create service info object");
    create_service_info();
    
    IComPacketHandle packet = new IComPacket;
    
    putmsg("SimpleService: Initiate service");
    bool success = init_service(packet);

    if (success)
    {
        info_handle_->add_packet(packet,true);    // Let the output thread deal with this
        putmsg("SimpleService: Starting output thread after successfull initialization");
        create_output_thread();
        // Main loop of handling packages
        putmsg("SimpleService: Main loop starts here");
        handle_input();
        putmsg("SimpleService: Main loop ends here");
        putmsg("SimpleService: closing down sevice");
        // we need to end the stream of output first before
        // ending the service and sending an end message
        kill_output_thread();
        close_service();
    }
    else
    {
        putmsg("SimpleService: Initialization failure, sending out one more packet  to report failure");    
        send(packet);
        putmsg("SimpleService: closing down sevice");
        close_service();
        // no thread to kill
    }
}

void SimpleService::handle_input()
{
  IComPacketHandle	packet;
  bool				done = false;

  while(!done)
  {
      if(!(recv(packet))) 
      {
          std::ostringstream oss;
          oss << "Error receiving message from input queue (errno=" << geterrno() << ")";
          errormsg(oss.str());
          break;
      }

      if (packet->gettag() == TAG_END_STREAM) 
      { 
          done = true; 
          continue;
		
      }
      handle_service(packet);
  }
}


bool SimpleService::init_service(IComPacketHandle& /*packet*/)
{
  // Overwrite with user defined function.
  // This is the default function that does nothing
  return(true);
}   

void SimpleService::close_service()
{
  // Overwrite with user defined function.
  // This is the default function that does nothing
}


void SimpleService::handle_service(IComPacketHandle& /*packet*/)
{
  // Overwrite with user defined function.
  // This is the default function that does nothing
}


} // end namespace


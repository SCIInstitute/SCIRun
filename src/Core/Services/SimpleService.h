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


// SimpleService.h

#ifndef CORE_SERVICES_SIMPLESERVICE_H
#define CORE_SERVICES_SIMPLESERVICE_H 1

#include <Core/Services/Service.h>
#include <Core/SystemCall/SystemCall.h>
#include <Core/ICom/IComSocket.h>

#include <Core/Services/share.h>
namespace SCIRun {

class	SimpleService;
typedef LockingHandle<SimpleService> SimpleServiceHandle;


/////////////////////////////////////////////////////////////
//  Class for handling output of the Service
//  This classes uses the underlying Service class
//  to define any functionality

class SCISHARE SimpleServiceOutputInfo : public ServiceBase, public UsedWithLockingHandle<Mutex>
{
  public:
	SimpleServiceOutputInfo(IComSocket socket,ServiceLogHandle log);
	SimpleServiceOutputInfo* clone();
	virtual ~SimpleServiceOutputInfo();

	// Communication with main thread
	void signal_exit();  
	void add_packet(IComPacketHandle& packet, bool atfront = false);

  public:
	friend class SimpleServiceOutputThread;
  protected:
	std::list<IComPacketHandle>*  packet_list_;
 	ConditionVariable		wait_;
	IComSocket				socket_;	  
	bool							exit_;
	ServiceLogHandle		log_;
};

typedef LockingHandle<SimpleServiceOutputInfo> SimpleServiceOutputInfoHandle;

inline SimpleServiceOutputInfo* SimpleServiceOutputInfo::clone()
{
	return(new SimpleServiceOutputInfo(socket_,log_));
}

inline void SimpleServiceOutputInfo::signal_exit()
{
	lock.lock();
	exit_ = true;
	wait_.conditionBroadcast();
	lock.unlock();
}

inline void SimpleServiceOutputInfo::add_packet(IComPacketHandle &packet,bool atfront)
{
    lock.lock();
    if (packet_list_ == 0) packet_list_ = new std::list<IComPacketHandle>;
    if (atfront)
    {
        packet_list_->push_front(packet);
    }
    else
    {
        packet_list_->push_back(packet);
    }
    wait_.conditionSignal();    
    lock.unlock();
}

class SCISHARE SimpleServiceOutputThread : public Runnable, public ServiceBase
{
public:
	explicit SimpleServiceOutputThread(SimpleServiceOutputInfoHandle handle);
	virtual ~SimpleServiceOutputThread();

	// Entry point for runnable
	void run();
	  
  private:
 	  SimpleServiceOutputInfoHandle handle_;
  	// STDOUT STREAMING
};




//////////////////////////////////////////////////////////////////////

class SCISHARE SimpleServiceOutputHandler : public SystemCallHandler {
public:
	SimpleServiceOutputHandler(SimpleServiceOutputInfoHandle handle, int tag);
	virtual ~SimpleServiceOutputHandler();

	virtual	void start(std::list<std::string> &buffer);
	virtual bool execute(std::string line);
	virtual void end();
	
  private:
    SimpleServiceOutputInfoHandle handle_;
	int count_;
	int tag_;
	
};

typedef LockingHandle<SimpleServiceOutputHandler> SimpleServiceOutputHandlerHandle;

///////////////////////////////////////////////////////////

class SCISHARE SimpleService : public Service {

  public:
	SimpleService(ServiceContext &ctx);
	virtual ~SimpleService();
  
	void			execute();						// Main loop
	void      create_service_info();
	void			create_output_thread();			// Create a separate thread for handling sending over socket
	void			kill_output_thread();			// Close the thread by signalling that it should exit
													// After this the packages in the buffer are still being send

	// User defined functions
	virtual bool	init_service(IComPacketHandle& packet);		// User function to initiate stuff
	virtual void	close_service();				// Function called at end of communication
													// EG when an END_ signal is send to the service
	virtual void	handle_service(IComPacketHandle& packet);	// Handle service specific packets
	
	
	void			handle_input();
	void			send_packet(IComPacketHandle& packet);

	void			forward_stdout(SystemCallHandle syscall);
	void			forward_stderr(SystemCallHandle syscall);
	void			forward_exit(SystemCallHandle syscall);
	void			stop_forward_stdout();
	void			stop_forward_stderr();
	void			stop_forward_exit();

  private:
  
	// Data needed for forwarding messages to client
	SystemCallHandle					stdout_syscall_;
	SimpleServiceOutputHandlerHandle	stdout_handler_;
	SystemCallHandle					stderr_syscall_;
	SimpleServiceOutputHandlerHandle	stderr_handler_;
	SystemCallHandle					exit_syscall_;
	SimpleServiceOutputHandlerHandle	exit_handler_;

	Thread*                         output_thread_;
	// Output thread data
	SimpleServiceOutputInfoHandle   info_handle_;	
	// File forwarding functions	
    
};


inline void SimpleService::send_packet(IComPacketHandle& packet)
{
	info_handle_->add_packet(packet);
}


} // end namespace

#endif

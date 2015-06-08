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


// ServiceManager.cc

#include <Core/Services/ServiceManager.h>

namespace SCIRun {

ServiceManager::ServiceManager(ServiceDBHandle db, IComAddress address,
                               ServiceLogHandle log) :
  Runnable(true),
  log_(log),
  db_(db),
  address_(address)
{
  // create an empty log
  if (log_.get_rep() == 0) log_ = new ServiceLog("");
}

ServiceManager::~ServiceManager()
{
  log_->putmsg("ServiceManager: Terminating ServiceManager");
}

// Multiple ServiceManager can be launched but they need to be in
// different objects: e.g. one for IPv4, one for IPv6, and an internal
// one Having multiple means that one can always launch an internal
// one without compromising security...

void ServiceManager::run()
{
  log_->putmsg("ServiceManager: Starting ServiceManager");
  IComSocket serversocket;
	
  if (!address_.isvalid()) 
  {
    log_->putmsg("ServiceManager: invalid address for server, shutting down service manager");
    std::cerr << "-------------------------------------------------------" << std::endl;
    std::cerr << "Error detected when launching the service manager" << std::endl;
    std::cerr << "Invalid address for server, shutting down service manager" << std::endl;
    std::cerr << "-------------------------------------------------------" << std::endl;

    return;		
  }
  
  if(!(serversocket.create(address_.getprotocol()))) 
  {
    log_->putmsg("ServiceManager: socket.create() failed, shutting down service manager");
    log_->putmsg(std::string("ServiceFrame: socket error = ") + serversocket.geterror());

    std::cerr << "-------------------------------------------------------" << std::endl;
    std::cerr << "Error detected when launching the service manager" << std::endl;
    std::cerr << std::string("ServiceFrame: socket error = ") + serversocket.geterror() << std::endl;
    std::cerr << "Please check the service manager settings" << std::endl;
    std::cerr << "-------------------------------------------------------" << std::endl;

    return;	
  }
	
  if(!(serversocket.bind(address_))) 
  {
    log_->putmsg("ServiceManager: socket.bind() failed, shutting down service manager");
    std::string errormsg = "ServiceManager: socket error = " + serversocket.geterror();
    log_->putmsg(errormsg);

    std::cerr << "-------------------------------------------------------" << std::endl;
    std::cerr << "Error detected when launching the service manager" << std::endl;
    std::cerr << errormsg << std::endl;
    std::cerr << "Please check the service manager port settings" << std::endl;
    std::cerr << "-------------------------------------------------------" << std::endl;

    return;	
  }

  std::string status = "ServiceManager: servicemanager address: " + address_.geturl();
  log_->putmsg(status);
  log_->putmsg("ServiceManager: listening...");

  if (!(serversocket.listen())) 
  {
    log_->putmsg("ServiceManager: socket.listen() failed, shutting down service manager");

    std::cerr << "-------------------------------------------------------" << std::endl;
    std::cerr << "Error detected when launching the service manager" << std::endl;
    std::cerr << "ServiceManager: socket.listen() failed, shutting down service manager"<< std::endl; 
    std::cerr << "-------------------------------------------------------" << std::endl;

    return;
  }
  
  for(;;)
  {
    // LOOP FOREVER...

    IComSocket clientsocket;
    clientsocket.close();
    log_->putmsg("ServiceManager: connection attempt made by remote host");
    
    if (serversocket.accept(clientsocket)) 
    {
      status = "ServiceManager: handeling new request, launching a ServiceFrame\n";
      log_->putmsg(status);

      // Since this is a runnable it will destroy itself on exit
      ServiceFrame *sf;
      sf = new ServiceFrame(clientsocket,db_,log_);

      // The new thread to be launched
      Thread* thread = new Thread(sf,"service server");
      log_->putmsg("ServiceManager: new thread to handle service");
      thread->detach();	// It will do all it's stuff by itself
    }
    else 
    {
      log_->putmsg("ServiceManager: socket.accept() failed, trying once more");
      log_->putmsg(std::string("ServiceManager: socket error = ") + serversocket.geterror());
    }
  }
}

}

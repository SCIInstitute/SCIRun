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
 * LaunchService.h
 *
*/

#ifndef CORE_SERVICES_SERVICEFRAME_H
#define CORE_SERVICES_SERVICEFRAME_H 1

#include <Core/ICom/IComSocket.h>
#include <Core/Services/ServiceBase.h>
#include <Core/Services/Service.h>
#include <Core/Services/ServiceDB.h>
#include <Core/Services/ServiceManager.h>
#include <Core/Services/ServiceLog.h>
#include <Core/Thread/Mutex.h>


#include <string>
#include <vector>
#include <map>
#include <sstream>




namespace SCIRun {

class SCISHARE ServiceFrame : /*public Runnable,*/ public ServiceBase {

public:
	// After the manager accepts the connection
	// It creates a thread that will initiate the
	// thread that will communicate with the client
	// It needs a pointer to the manager to check the
	// services database
	
  ServiceFrame(IComSocketHandle socket, ServiceDBHandle db, ServiceLogHandle log);
	
	// Entrypoint for the thread. 
	void	run();
  void operator()() { run(); }
	
	bool	initiate();
	bool	runservice();
	bool	close();
	
private:

  std::string       startcommand_;
  int               timeout_;
	std::string       servicename_;
	int               session_;
	IComSocketHandle        socket_;
	ServiceDBHandle		db_;
	ServiceLogHandle	log_;
};

} // End namespace SCIRun

#endif

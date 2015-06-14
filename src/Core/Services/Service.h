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


// Service.h

#ifndef CORE_SERVICES_SERVICE_H
#define CORE_SERVICES_SERVICE_H 1

#include <Core/ICom/IComFwd.h>
#include <Core/Services/ServiceBase.h>
#include <Core/Thread/Mutex.h>
//#include <Core/Services/ServiceLog.h>
#include <boost/shared_ptr.hpp>
#include <map>

#include <Core/Services/share.h>

#ifdef _WIN32
#define DECLARE_SERVICE_MAKER(name) \
extern "C" __declspec(dllexport) Service* make_service_##name(ServiceContext& ctx) \
{ \
  return new name(ctx); \
}
#else
#define DECLARE_SERVICE_MAKER(name) \
extern "C" Service* make_service_##name(ServiceContext &ctx) \
{ \
  return new name(ctx); \
}
#endif

namespace SCIRun {

class SCISHARE ServiceContext
{
public:
  std::string                                  servicename;
  int                                          session;
  std::string                                  packagename;
  std::string                                  startcommand;
  int                                          timeout;
  std::map<std::string,std::string>            parameters;
  IComSocketHandle                             socket;
  ServiceLogHandle                             log;
};


class Service;
typedef boost::shared_ptr<Service> ServiceHandle;


class SCISHARE Service : public ServiceBase
{
public:
  explicit Service(const ServiceContext &ctx);
  virtual ~Service();

  // Run will be called by the thread environment as the entry point of
  // a new thread.
  void			operator()();

  // Entrypoint of the actual function of the service
  virtual void	execute();

  // Get information about this services
	bool					updateparameters();
  int						getsession();
  void          setsession(int session);

  std::string   getstartcommand();
  int           gettimeout();

  std::string		getservicename();
  std::string		getpackagename();
  std::string		getparameter(std::string);
  IComSocketHandle		getsocket();
  ServiceLogHandle getlog();
  void					putmsg(std::string line);
  // Communication functions

   bool			send(IComPacketHandle &packet);
   bool			recv(IComPacketHandle &packet);
   bool			poll(IComPacketHandle &packet);

   bool			getlocaladdress(IComAddress &address);
   bool			getremoteaddress(IComAddress &address);
   bool			isconnected();

  // Error retrieval mechanisms for communication errors

   std::string	geterror();
   int					geterrno();
   bool					haserror();

  // Error reporting, services log file

  void			errormsg(std::string error);
  void			warningmsg(std::string warning);

public:
  // Locking Handle needs Mutex to be called lock so we cannot reuse that name
   void			dolock();
   void			unlock();

private:
  Core::Thread::Mutex lock_;
  ServiceContext	ctx_;
};

} // namespace SCIRun

#endif

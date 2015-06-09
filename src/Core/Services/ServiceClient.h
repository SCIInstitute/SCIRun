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


// ServiceClient.h

#ifndef CORE_SERVICES_SERVICECLIENT_H
#define CORE_SERVICES_SERVICECLIENT_H 1

#include <Core/ICom/IComFwd.h>
#include <Core/Thread/Mutex.h>
#include <Core/Services/ServiceBase.h>
#include <Core/Services/share.h>

namespace SCIRun {

class SCISHARE ServiceClient : public ServiceBase
{
  public:
    ServiceClient();
    ~ServiceClient();

    bool  open(IComAddressHandle address, std::string servicename,
               int session, std::string passwd);

    bool  open(IComAddressHandle address, std::string servicename,
               int session, std::string passwd,
               int timeout, std::string startcommand);

    bool  close();

    ServiceClient* clone() const;

    ////////////////////////////////////////

    IComSocketHandle  getsocket();

    std::string geterror();
    std::string getremoteaddress();
    std::string getversion();
    std::string getsession();
    void        setsession(int session);

    bool    send(IComPacketHandle &packet);
    bool    recv(IComPacketHandle &packet);
    bool    poll(IComPacketHandle &packet);
    void    seterror(std::string);
    void    clearerror();

  private:
    Core::Thread::Mutex lock_;
    int          session_;
    std::string  version_;
    std::string  error_;
    int          errno_;
    IComSocketHandle   socket_;
    bool         need_send_end_stream_;
};

typedef boost::shared_ptr<ServiceClient> ServiceClientHandle;

}

#endif

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

// ServiceClient.cc

#include <Core/Services/ServiceClient.h>
#include <Core/ICom/IComPacket.h>
#include <Core/ICom/IComSocket.h>
#include <Core/ICom/IComAddress.h>

namespace SCIRun {

ServiceClient::ServiceClient() :
  lock_("service client lock"),
  session_(0),
  errno_(0),
  need_send_end_stream_(false)
{
}

ServiceClient::~ServiceClient()
{
  // Make sure we close the socket
  if (need_send_end_stream_)
  {
    IComPacketHandle packet(new IComPacket());
    packet->settag(TAG_END_STREAM);
    packet->setid(0);
    socket_->send(packet);
    need_send_end_stream_ = false;
  }

  socket_->close();
}


ServiceClient*  ServiceClient::clone() const
{
  ServiceClient* ptr = new ServiceClient();
  ptr->socket_= socket_;
  ptr->session_ = session_;
  ptr->version_ = version_;
  ptr->error_   = error_;
  ptr->errno_   = errno_;
  return(ptr);
}

bool ServiceClient::open(IComAddressHandle address, std::string servicename, int session, std::string passwd)
{
  // record for debugging purposes
  session_ = session;

  // reroute to internal address if possible
  if (!address->isvalid())
  {
    seterror("The address specified is not valid");
    return(false);
  }

  if (!(address->isinternal()))
  {
    // If the address is local, the quickest is to make use of the
    // local internal server. Hence if the IP address matches the
    // local machine it is rerouted to the internal server. Same is
    // true if someone specifies 'localhost'
    IComAddress localaddress;
    localaddress.setlocaladdress();
    IComAddress localhost(address->getprotocol(),"localhost","any");

    if (*address == localaddress)
    {
      address->setaddress("internal","servicemanager");
    }
    if (*address == localhost)
    {
      address->setaddress("internal","servicemanager");
    }
  }


  if (!( socket_->create(address->getprotocol())))
  {
    std::string err = "ServiceClient: Could not create a socket to connect to service (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!(socket_->settimeout(180,0)))
  {
    std::string err = "ServiceClient: Could not set timeout for socket (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!( socket_->connect(*address)))
  {
    std::string err = "ServiceClient: Could not connect to service (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  IComPacketHandle packet(new IComPacket());

  if (!packet)
  {
    std::string err = "ServiceClient: Could not create packet";
    seterror(err);
    socket_->close();
    return(false);
  }

  packet->settag(TAG_RQSV);
  packet->setid(session);
  packet->setstring(servicename);

  // Wait no more than 20 seconds for a connection.  If this one
  // fails, that is no disaster, so we continue anyway.
  socket_->settimeout(20,0);

  if (!(socket_->send(packet)))
  {
    std::string err = "ServiceClient: Could not send package to service (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }


  if (!(socket_->recv(packet)))
  {
    std::string err = "ServiceClient: Service did not respond to our request (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if ( packet->gettag() != TAG_RQSS)
  {
    if (packet->gettag() == TAG_RQFL)
    {
      std::string err = "Service request failed (" + packet->getstring() + ")";
      seterror(err);
      socket_->close();
      return(false);
    }

    std::string err = "Service request failed (No error returned from server)";
    seterror(err);
    socket_->close();
    return(false);
  }

  packet->clear();

  packet->settag(TAG_AUTH);
  packet->setid(0);
  packet->setstring(passwd);

  if (!(socket_->send(packet)))
  {
    std::string err = "Could not send authentication data (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!(socket_->recv(packet)))
  {
    std::string err = "Service did not respond to our request (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (packet->gettag() != TAG_AUSS)
  {
    if (packet->gettag() == TAG_AUFL)
    {
      std::string err = "Service authentication failed (" + packet->getstring() + ")";
      seterror(err);
      socket_->close();
      return(false);
    }
    std::string err = "Service authentication failed (No error returned from server)";
    seterror(err);
    socket_->close();
    return(false);
  }

  packet->settag(TAG_STRT);
  packet->setid(0);
  packet->setstring("");

  if (!(socket_->send(packet)))
  {
    std::string err = "Could not send startcommand data (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!(socket_->recv(packet)))
  {
    std::string err = "Service did not respond to our request (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (packet->gettag() != TAG_STSS)
  {
    if (packet->gettag() == TAG_STFL)
    {
      std::string err = "Service startcommand failed (" + packet->getstring() + ")";
      seterror(err);
      socket_->close();
      return(false);
    }
    std::string err = "Service startcommand failed (No error returned from server)";
    seterror(err);
    socket_->close();
    return(false);
  }

  version_ = packet->getstring();
  need_send_end_stream_ = true;

  clearerror();
  return(true);
}





bool ServiceClient::open(IComAddressHandle address, std::string servicename,
                         int session, std::string passwd,
                         int timeout, std::string startcommand)
{
  // record for debugging purposes
  session_ = session;

  // reroute to internal address if possible
  if (!address->isvalid())
  {
    seterror("The address specified is not valid");
    return(false);
  }

  if (!(address->isinternal()))
  {
    // If the address is local, the quickest is to make use of the
    // local internal server. Hence if the IP address matches the
    // local machine it is rerouted to the internal server. Same is
    // true if someone specifies 'localhost'
    IComAddress localaddress;
    localaddress.setlocaladdress();
    IComAddress localhost(address->getprotocol(),"localhost","any");

    if (*address == localaddress)
    {
      address->setaddress("internal","servicemanager");
    }
    if (*address == localhost)
    {
      address->setaddress("internal","servicemanager");
    }
  }


  if (!( socket_->create(address->getprotocol())))
  {
    std::string err = "ServiceClient: Could not create a socket to connect to service (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!(socket_->settimeout(timeout,0)))
  {
    std::string err = "ServiceClient: Could not set timeout for socket (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!( socket_->connect(*address)))
  {
    std::string err = "ServiceClient: Could not connect to service (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  IComPacketHandle packet(new IComPacket());

  if (!packet)
  {
    std::string err = "ServiceClient: Could not create packet";
    seterror(err);
    socket_->close();
    return(false);
  }

  packet->settag(TAG_RQSV);
  packet->setid(session);
  packet->setstring(servicename);

  // Wait no more than 20 seconds for a connection.  If this one
  // fails, that is no disaster, so we continue anyway.
  socket_->settimeout(20,0);

  if (!(socket_->send(packet)))
  {
    std::string err = "ServiceClient: Could not send package to service (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }


  if (!(socket_->recv(packet)))
  {
    std::string err = "ServiceClient: Service did not respond to our request (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if ( packet->gettag() != TAG_RQSS)
  {
    if (packet->gettag() == TAG_RQFL)
    {
      std::string err = "Service request failed (" + packet->getstring() + ")";
      seterror(err);
      socket_->close();
      return(false);
    }

    std::string err = "Service request failed (No error returned from server)";
    seterror(err);
    socket_->close();
    return(false);
  }

  packet->clear();

  packet->settag(TAG_AUTH);
  packet->setid(0);
  packet->setstring(passwd);

  if (!(socket_->send(packet)))
  {
    std::string err = "Could not send authentication data (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!(socket_->recv(packet)))
  {
    std::string err = "Service did not respond to our request (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (packet->gettag() != TAG_AUSS)
  {
    if (packet->gettag() == TAG_AUFL)
    {
      std::string err = "Service authentication failed (" + packet->getstring() + ")";
      seterror(err);
      socket_->close();
      return(false);
    }
    std::string err = "Service authentication failed (No error returned from server)";
    seterror(err);
    socket_->close();
    return(false);
  }

  packet->settag(TAG_STRT);
  packet->setid(timeout);
  packet->setstring(startcommand);

  if (!(socket_->send(packet)))
  {
    std::string err = "Could not send startcommand data (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (!(socket_->recv(packet)))
  {
    std::string err = "Service did not respond to our request (" + socket_->geterror() + ")";
    seterror(err);
    socket_->close();
    return(false);
  }

  if (packet->gettag() != TAG_STSS)
  {
    if (packet->gettag() == TAG_STFL)
    {
      std::string err = "Service startcommand failed (" + packet->getstring() + ")";
      seterror(err);
      socket_->close();
      return(false);
    }
    std::string err = "Service startcommand failed (No error returned from server)";
    seterror(err);
    socket_->close();
    return(false);
  }

  version_ = packet->getstring();
  need_send_end_stream_ = true;

  clearerror();
  return(true);
}




bool ServiceClient::close()
{

  if (need_send_end_stream_)
  {
    IComPacketHandle packet(new IComPacket());
    packet->settag(TAG_END_STREAM);
    packet->setid(0);
    socket_->send(packet);
    need_send_end_stream_ = false;
  }
  socket_->close();
  clearerror();
  return(true);
}

bool ServiceClient::send(IComPacketHandle &packet)
{
  if (!socket_->send(packet))
  {
    seterror(socket_->geterror());
    return(false);
  }
  clearerror();
  return(true);
}

bool ServiceClient::recv(IComPacketHandle &packet)
{
  if (!socket_->recv(packet))
  {
    seterror(socket_->geterror());
    return(false);
  }
  clearerror();
  return(true);
}

bool ServiceClient::poll(IComPacketHandle &packet)
{
  if (!socket_->poll(packet))
  {
    seterror(socket_->geterror());
    return(false);
  }
  clearerror();
  return(true);
}

std::string ServiceClient::geterror()
{
  return(error_);
}

std::string ServiceClient::getversion()
{
  return(version_);
}

void ServiceClient::seterror(std::string error)
{
  error_ = error;
}

void ServiceClient::clearerror()
{
  error_ = "";
}

std::string ServiceClient::getremoteaddress()
{
  IComAddress address;
  socket_->getremoteaddress(address);
  return(address.geturl());
}

std::string ServiceClient::getsession()
{
  std::ostringstream oss;
  oss << session_;
  return(oss.str());
}

IComSocketHandle ServiceClient::getsocket()
{
  return(socket_);
}

void ServiceClient::setsession(int session)
{
  session_ = session;
}

}

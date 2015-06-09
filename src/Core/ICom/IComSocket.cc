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
 *  IComSocket.cc
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */

#include <Core/ICom/IComSocket.h>
#include <Core/ICom/IComVirtualSocket.h>
#include <Core/ICom/IComINetSocket.h>
#include <Core/ICom/IComSslSocket.h>
#include <Core/ICom/IComInternalSocket.h>

namespace SCIRun {


// Create a new socket
IComSocket::IComSocket(std::string protocol) :
  socket_(0),
  protocol_(protocol)
{
  error_.error = "";		// no error has been reported yet
  error_.errnr = 0;		// no error
}

// Remove the socket
IComSocket::~IComSocket()
{
  clear();				// clear up the socket
}

// create a socket and overrule the type setting
bool IComSocket::create(std::string protocol)
{
  if (socket_) clear();
  protocol_ = protocol;
  return(create());
}

bool IComSocket::create()
{
  if (socket_) clear();

  if (protocol_ == "scirun")
  {
    try
    {
      socket_.reset(new IComINetSocket());
      return(true);
    }
    catch(...)
    {
      socket_.reset();
      error_.error = "Could not allocate socket";
      error_.errnr = EBADF;
      return(false);
    }
  }

#if 0
  if (protocol_ == "sciruns")
  {
    try
    {
      socket_.reset(new IComSslSocket());
      return(true);
    }
    catch(...)
    {
      socket_.reset();
      error_.error = "Could not allocate socket";
      error_.errnr = EBADF;
      return(false);
    }
  }
#endif

  if (protocol_ == "internal")
  {
    try
    {
      socket_.reset(new IComInternalSocket());
      return(true);
    }
    catch(...)
    {
      socket_.reset();
      error_.error = "Could not allocate socket";
      error_.errnr = EBADF;
      return(false);
    }
  }

  // The algorithm should not get here, unless the protocol
  // is not an existing one
  error_.error = "Unknown socket protocol has been specified";
  error_.errnr = EBADF;

  std::cerr << "Socket creation failed as the socket type ("
	    << protocol_ << ") is an unknown type" << std::endl;
  return(false);
}


void IComSocket::clear()
{
  socket_.reset();

  // reset error, socket is gone so we do not want to know about it anymore
  error_.error = "";
  error_.errnr = 0;
}

bool IComSocket::nosocketerror()
{
  error_.error = "No socket has yet been created";
  error_.errnr = EBADF;
  return(false);
}

// Get a string with the last error description
inline	std::string IComSocket::geterror()
{
  return(error_.error);
}

// Get the last reported errno
inline int IComSocket::geterrno()
{
  return(error_.errnr);
}

inline	bool IComSocket::haserror()
{
  return(error_.errnr != 0);
}

// Get the protocol used in this socket
inline	std::string IComSocket::getsocketprotocol()
{
  return(protocol_);
}

// Get the ptr to the internal socket structure
IComVirtualSocketHandle IComSocket::getsocketptr()
{
  return(socket_);
}

inline bool IComSocket::close()
{
  // Currently closing the socket will completely close the socket and
  // it will even remove any error messages. Hence it will always return
  // true. However for symmetry this function returns a bool as well
  // indicating success.
  clear();
  return(true);
}

inline bool IComSocket::getremoteaddress(IComAddress &address)
{
  if (socket_) {
    return(socket_->getremoteaddress(address,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::getlocaladdress(IComAddress &address)
{
  if (socket_) {
    return(socket_->getlocaladdress(address,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::settimeout(int secs, int microsecs)
{
  if (socket_) {
    return(socket_->settimeout(secs,microsecs,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::listen()
{
  if (socket_) {
    return(socket_->listen(error_));
  } else {
    return(nosocketerror());
  }
}

bool IComSocket::accept(IComSocketHandle& newsock)
{
  if (socket_) {
    return(socket_->accept(newsock,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::poll(IComPacketHandle& packet)
{
  if (socket_) {
    return(socket_->poll(packet,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::send(IComPacketHandle& packet)
{
  if (socket_) {
    return(socket_->send(packet,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::recv(IComPacketHandle& packet)
{
  if (socket_) {
    return(socket_->recv(packet,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::connect(IComAddress& address, conntype conn)
{
  if (socket_) {
    return(socket_->connect(address,conn,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::bind(IComAddress& address)
{
  if (socket_) {
    return(socket_->bind(address,error_));
  } else {
    return(nosocketerror());
  }
}

inline bool IComSocket::isconnected()
{
  if (socket_) {
    return(socket_->isconnected(error_));
  } else {
    return(nosocketerror());
  }
}

} // namespace SCIRun

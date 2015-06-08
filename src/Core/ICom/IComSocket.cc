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

// copy constructor
IComSocket::IComSocket(const IComSocket &s) :
  socket_(0)
{
  protocol_ = s.protocol_;	// copy the protocol 
  error_ = s.error_;			// copy the last error
  if (s.socket_)
  {
    s.socket_->dolock();
    s.socket_->ref_cnt++;
    socket_ = s.socket_;
    s.socket_->unlock();
  }
} 


IComSocket& IComSocket::operator=(const IComSocket &s)
{
  if (this != &s)
  {
    if (socket_) clear();
    socket_ = 0;
    protocol_ = s.protocol_;
    error_ = s.error_;
    if (s.socket_)
    {
      s.socket_->dolock();
      s.socket_->ref_cnt++;
      socket_ = s.socket_;
      s.socket_->unlock();
    }
  }
  return(*this);
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
      socket_ = static_cast<IComVirtualSocket *>(new IComINetSocket());
      socket_->ref_cnt = 1;
      return(true);
    }
    catch(...)
    {
      socket_ = 0;
      error_.error = "Could not allocate socket";
      error_.errnr = EBADF;
      return(false);
    }
  }

  if (protocol_ == "sciruns")
  {
    try
    {
      socket_ = static_cast<IComVirtualSocket *>(new IComSslSocket());
      socket_->ref_cnt = 1;
      return(true);
    }
    catch(...)
    {
      socket_ = 0;
      error_.error = "Could not allocate socket";
      error_.errnr = EBADF;
      return(false);
    }
  }
	
  if (protocol_ == "internal")
  {
    try
    {
      socket_ = static_cast<IComVirtualSocket *>(new IComInternalSocket());
      socket_->ref_cnt = 1;
      return(true);
    }
    catch(...)
    {
      socket_ = 0;
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
  if (socket_)
  {
    IComVirtualSocket* oldsocket = 0;
    socket_->dolock();
    socket_->ref_cnt--;
    oldsocket = socket_;
    socket_->unlock();
    socket_ = 0;
    if (oldsocket->ref_cnt == 0) 
    {
      IComSocketError err;
      if (oldsocket) oldsocket->close(err);
      if (oldsocket) delete oldsocket;
    }
		
  }
	
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

} // namespace SCIRun

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

#include <Core/ICom/IComVirtualSocket.h>

namespace SCIRun {
 
IComVirtualSocket::IComVirtualSocket() :
    UsedWithLockingHandleAndMutex("virtual_socket_lock")
{
}

IComVirtualSocket::~IComVirtualSocket()
{
}

bool	IComVirtualSocket::socket_not_implemented(IComSocketError &err)
{
	err.error = "Virtual Socket: this socket implementation should not be called";
	err.errnr = EBADF;
	return(false);
}

bool	IComVirtualSocket::bind(IComAddress& /*address*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}

bool	IComVirtualSocket::connect(IComAddress& /*address*/, conntype /*conn*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}
	
bool	IComVirtualSocket::close(IComSocketError &err)
{
	return(socket_not_implemented(err));
}

bool	IComVirtualSocket::getlocaladdress(IComAddress& /*address*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}
	
bool	IComVirtualSocket::getremoteaddress(IComAddress& /*address*/, IComSocketError &err)
{
 	return(socket_not_implemented(err));
}
	
bool	IComVirtualSocket::settimeout(int /*secs*/, int /*microsecs*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}
 

bool	IComVirtualSocket::listen(IComSocketError &err)
{
	return(socket_not_implemented(err));
}

bool    IComVirtualSocket::accept(IComSocket& /*newsock*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}

bool	IComVirtualSocket::poll(IComPacketHandle& /*packet*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}

bool	IComVirtualSocket::send(IComPacketHandle& /*packet*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}

bool	IComVirtualSocket::recv(IComPacketHandle& /*packet*/, IComSocketError &err)
{
	return(socket_not_implemented(err));
}
 
bool	IComVirtualSocket::isconnected(IComSocketError &err)
{
	return(socket_not_implemented(err));
} 

 
} // namespace

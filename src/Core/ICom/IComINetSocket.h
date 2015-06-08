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
 *  IComVirtualSocket.h 
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */
 
#ifndef JGS_SCI_CORE_ICOM_INETSOCKET_H
#define JGS_SCI_CORE_ICOM_INETSOCKET_H 1

#include <Core/ICom/IComBase.h>
#include <Core/ICom/IComPacket.h>
#include <Core/ICom/IComAddress.h>
#include <Core/ICom/IComVirtualSocket.h>
#include <Core/ICom/IComSocket.h>
#include <Core/Thread/Mutex.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/signal.h>
#else
#include <winsock2.h>
#endif


#include <iostream>



namespace SCIRun {

class IComINetSocket : public IComVirtualSocket {

  public:
	IComINetSocket();
	virtual ~IComINetSocket();
	
  public:
  
	bool	bind(IComAddress& address, IComSocketError &err);
	bool	connect(IComAddress& address, conntype conn, IComSocketError &err);	
	
	bool	close(IComSocketError &err);	

	bool	getlocaladdress(IComAddress &address, IComSocketError &err);	// Get local address
	bool	getremoteaddress(IComAddress &address, IComSocketError &err);	// Get remote address
	
	bool	settimeout(int secs, int microsecs, IComSocketError &err);	// set the time out of the socket 

	bool	listen(IComSocketError &err);					// Listen for a connection
	bool    accept(IComSocket& newsock, IComSocketError &err);	// Accept the connection and get a new socket object
	
	bool	poll(IComPacketHandle &packet, IComSocketError &err);	// Poll whether there is a packet waiting (non blocking)
	bool	send(IComPacketHandle &packet, IComSocketError &err);	// Send a packet (blocking)
	bool	recv(IComPacketHandle &packet, IComSocketError &err);	// Recv a packet (blocking)	

	bool	isconnected(IComSocketError &err);

  private:
  
	int connect_timeout(int sockfd, const sockaddr* sa, socklen_t salen,int nsecs);
	void* connect_alarm(int signo);

	// error translation functions
	// The same error numbers can mean a slightly different
	// error for the different io functions, hence we create
	// an error message for each function we rely on. At least
	// when there are multiple errors we can expect in our implementation
	std::string getsocketerror(int errnr);
	std::string getbinderror(int errnr);
	std::string	getconnecterror(int errnr);
	std::string	getlistenerror(int errnr);
	std::string getsockopterror(int errnr);
	std::string getaccepterror(int errnr);	
	std::string getsenderror(int errnr);	
	std::string getrecverror(int errnr);	

  
	int		bytesinbuffer_;
	char	buffer_[32];
	
	int		socketfd_;
	bool	hassocket_;
	bool	isconnected_;
	int		secs_;
	int		microsecs_;
	
	IComAddress localaddress_;
	IComAddress remoteaddress_;
	
	// THIS FUNCTION NEEDS TO BE COMPLETED
};

}

#endif

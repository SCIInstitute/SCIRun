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

#include <Core/ICom/IComINetSocket.h>
#include <string.h>
#include <stdio.h>

namespace SCIRun {


IComINetSocket::IComINetSocket() :
  socketfd_(0), hassocket_(false), isconnected_(false), secs_(0), microsecs_(0)
{

}

IComINetSocket::~IComINetSocket()
{
	IComSocketError err;	// dummy error, we are not interested in errors anymore at this point 
	if (hassocket_) close(err);
}

std::string IComINetSocket::getsocketerror(int errnr)
{
	switch(errnr)
	{
		case EPROTONOSUPPORT: return(std::string("The requesteed protocol is not supported in this domain"));
		case EMFILE: return(std::string("The process file descriptor table is full"));
		case ENFILE: return(std::string("The system file descriptor table is full"));
		case EACCES: return(std::string("No permission to create socket of requested type/protocol"));
		case ENOBUFS: return(std::string("Not enough memory to allocate buffer space"));
	}
	return(std::string("Unknown error"));
}

std::string IComINetSocket::getbinderror(int errnr)
{
	switch(errnr)
	{
		case EBADF: return(std::string("Socket descriptor is not valid"));
		case ENOTSOCK: return(std::string("Socket descriptor is not a socket"));
		case EADDRNOTAVAIL: return(std::string("Requested address is not available"));
		case EADDRINUSE: return(std::string("Requested address is in use"));
		case EINVAL: return(std::string("This socket has already been connected"));
		case EACCES: return(std::string("The current address is protocted and the current user does not have permission to use it"));
		case EFAULT: return(std::string("Invalid socket address"));
	}
	return(std::string("Unknown error"));
}

std::string IComINetSocket::getconnecterror(int errnr)
{
	switch(errnr)
	{
		case EBADF: return(std::string("Sock descriptor is not valid"));
		case ENOTSOCK: return(std::string("Socket descriptor is not a socket"));
		case EADDRNOTAVAIL: return(std::string("Requested address is not available"));
		case EAFNOSUPPORT: return(std::string("Socket is imcompatible with address"));
		case EISCONN: return(std::string("Socket is already connected"));
		case ETIMEDOUT: return(std::string("Connection process timed out"));
		case ECONNREFUSED: return(std::string("Connection was refused by remote host"));
		case EHOSTUNREACH:
		case ENETUNREACH: return(std::string("Network could not be reached from this host"));
		case EADDRINUSE: return(std::string("Address is already in use"));
		case EFAULT: return(std::string("Invalid socket address"));
	}
	return(std::string("Unknown error"));
}		


std::string IComINetSocket::getlistenerror(int errnr)
{
	switch(errnr)
	{
		case EBADF: return(std::string("Sock descriptor is not valid"));
		case ENOTSOCK: return(std::string("Socket descriptor is not a socket"));
		case EOPNOTSUPP: return(std::string("Listen not supported by socket"));
	}
	return(std::string("Unknown error"));
}	


std::string IComINetSocket::getsockopterror(int errnr)
{
	switch(errnr)
	{
		case EBADF: return(std::string("Sock descriptor is not valid"));
		case ENOTSOCK: return(std::string("Socket descriptor is not a socket"));
		case ENOPROTOOPT: return(std::string("Timeout socket option not supported"));
		case EFAULT: return(std::string("Timeout option is invalid for the type of socket"));
		case EDOM: return(std::string("Value for timeout is out of range"));
	}
	return(std::string("Unknown error"));
}	

std::string IComINetSocket::getaccepterror(int errnr)
{
	switch(errnr)
	{
		case EBADF: return(std::string("Sock descriptor is not valid"));
		case ENOTSOCK: return(std::string("Socket descriptor is not a socket"));
		case EOPNOTSUPP: return(std::string("Socket is not a TCP socket"));
		case EFAULT: return(std::string("Invalid address"));
		case EWOULDBLOCK: return(std::string("Who made this socket non-blocking??"));
		case EMFILE: return(std::string("The process file descriptor table is full"));
		case ENFILE: return(std::string("The system file descriptor table is full"));		
	}
	return(std::string("Unknown error"));
}	

std::string IComINetSocket::getsenderror(int errnr)
{
	switch(errnr)
	{
		case EBADF: return(std::string("Sock descriptor is not valid"));
		case ENOTSOCK: return(std::string("Socket descriptor is not a socket"));
		case EFAULT: return(std::string("The send buffer is invalid"));
		case EMSGSIZE: return(std::string("Size of the packet was too large for this socket"));
		case ENOBUFS: return(std::string("Could not allocate enough buffers for socket"));
		case EHOSTUNREACH: return(std::string("Could not reach destination address"));		
        case EPIPE: return(std::string("Remote end of socket got closed"));
	}
	return(std::string("Unknown error"));
}

std::string IComINetSocket::getrecverror(int errnr)
{
	switch(errnr)
	{
		case EBADF: return(std::string("Sock descriptor is not valid"));
		case ENOTSOCK: return(std::string("Socket descriptor is not a socket"));
		case ENOTCONN: return(std::string("The socket is not connected"));
		case EFAULT: return(std::string("Receive buffer is invalid"));
	}
	return(std::string("Unknown error"));
}

		
bool	IComINetSocket::bind(IComAddress& address, IComSocketError &err)
{
	
	if (!address.isvalid()) 
	{	// This piece is thread safe -> no locking required
		err.errnr = EADDRNOTAVAIL;
		err.error = "Invalid internet address";
		return(false);
	}
	
	if (address.isinternal()) 
	{	// This piece is thread safe -> no locking required
		err.errnr = EADDRNOTAVAIL;
		err.error = "Address is an internal address and not an internet address";
		return(false);
	}
	
	bool success = false;	// Condition variable indicating that creating the socket was a success
	int	 addressnum = 0;	// The next loop goes through all addresses that qualify the user input
	
	// When an address does not work, we skip to the next one until we find an address that works.
	// In case no valid address is found we return an error
	while ((success==false)&&(addressnum < address.getnumaddresses()))
	{
		if (address.isipv4(addressnum))
		{
			// This involves changing the socket structure which is shared with other threads
			dolock();
			if((socketfd_ = ::socket(AF_INET,SOCK_STREAM,0)) < 0)
			{
				err.errnr = errno;
				err.error = getsocketerror(err.errnr);
				socketfd_ = 0;
				hassocket_ = false;
				unlock();
				addressnum++;
				continue;
			}
			hassocket_ = true;
			
			if(::bind(socketfd_,address.getsockaddr(addressnum),address.getsockaddrlen(addressnum)) < 0)
			{
				err.errnr = errno;
				err.error = getbinderror(err.errnr);				
#ifdef _WIN32
				::closesocket(socketfd_);
#else
				::close(socketfd_);
#endif
				hassocket_ = false;
				socketfd_ = 0;
				unlock();
				addressnum++;
				continue;
			}
			unlock();
			success = true;
		}
	
		if (address.isipv6(addressnum))
		{
			dolock();
			if((socketfd_ = ::socket(AF_INET6,SOCK_STREAM,0)) < 0)
			{
				err.errnr = errno;
				err.error = getsocketerror(err.errnr);
				socketfd_ = 0;
				hassocket_ = false;
				unlock();
				addressnum++;
				continue;
			}
			hassocket_ = true;
			if(::bind(socketfd_,address.getsockaddr(),address.getsockaddrlen()) < 0)
			{
				err.errnr = errno;
				err.error = getbinderror(err.errnr);
#ifdef _WIN32
				::closesocket(socketfd_);
#else
				::close(socketfd_);
#endif
				hassocket_ = false;
				socketfd_ = 0;
				unlock();
				addressnum++;
				continue;
			}
			unlock();
			success = true;
		}
	}
	
	if (!success) return(false);
	
	address.selectaddress(addressnum);
	localaddress_ = address;
	isconnected_ = false;

	if ((secs_ > 0)||(microsecs_ > 0)) return(settimeout(secs_,microsecs_,err));

	// clear errors
	err.errnr = 0;
	err.error = "";
	return(true);
}

int		IComINetSocket::connect_timeout(int sockfd, const sockaddr* sa, socklen_t salen,int /*nsecs*/)
{
// We can switch on this option by setting JGS_USE_CONNECT_TIMEOUT
// By default Unix will timeout after 75 secs anyway.
// So there is no major problem if this function is not included

// This still needs to be finished
#ifdef _JGS_USE_CONNECT_TIMEOUT 
	sig_t	sigfunc;
	int		ret;
	
	sigfunc = ::signal(SIGALRM,connect_alarm);
	::alarm(nsecs);
	
	if ( (ret = ::connect(sockfd,sa,salen)) < 0)
	{
		if (errno == EINTR)
		{
			errno = ETIMEDOUT;
		}
	}
	
	::alarm(0);
	::signal(SIGALRM,sigfunc);
	
	return(ret);
#else
	return(::connect(sockfd,sa,salen));
#endif
}

void*	IComINetSocket::connect_alarm(int /*signo*/)
{
	return(0);
}

bool	IComINetSocket::connect(IComAddress& address, conntype /*conn*/, IComSocketError &err)
{
	
	if (!address.isvalid()) 
	{	// This piece is thread safe -> no locking required
		err.errnr = EADDRNOTAVAIL;
		err.error = "Invalid internet address";
		return(false);
	}
	
	if (address.isinternal()) 
	{	// This piece is thread safe -> no locking required
		err.errnr = EADDRNOTAVAIL;
		err.error = "Address is an internal address and not an internet address";
		return(false);
	}
	
	bool success = false;	// Condition variable indicating that creating the socket was a success
	int	 addressnum = 0;	// The next loop goes through all addresses that qualify the user input
	int  tries = 0;
	
	// When an address does not work, we skip to the next one until we find an address that works.
	// In case no valid address is found we return an error. 
	// This routine checks various addresses:
	// Hence the returned error is a failure to connect to the last one in the local address structure

	while ((success==false)&&(addressnum < address.getnumaddresses()))
	{

		if (address.isipv4(addressnum))
		{
			// from this point on we are trying to change the internal socket structure
			// Hence we need to lock it to make it thread safe
			dolock();
			if((socketfd_ = ::socket(AF_INET,SOCK_STREAM,0)) < 0)
			{
				// Could not allocate socket
				// Find out why
				err.errnr = errno;
				err.error = getsocketerror(errno);
				socketfd_ = 0;
				hassocket_ = false;
				unlock();
				addressnum++;
				tries = 0;
				continue;
			}
			hassocket_ = true;

			// Two ways of connecting
			// One with a time out and one without a time out
			// Anyway Unix will always use a time out of 75sec.
			// But for the impatient we can set a shorter time out
			
			int retvalue;
			if (secs_ > 0)
			{
				retvalue = connect_timeout(socketfd_,address.getsockaddr(addressnum),address.getsockaddrlen(addressnum),secs_);
			}
			else
			{
				retvalue = ::connect(socketfd_,address.getsockaddr(addressnum),address.getsockaddrlen(addressnum));
			}
			
			if( retvalue < 0)
			{
				// Find out what went wrong in connecting to the server
				err.errnr = errno;
                
				err.error = getconnecterror(errno);
#ifdef _WIN32
				::closesocket(socketfd_);
#else
				::close(socketfd_);
#endif
				hassocket_ = false;
				socketfd_ = 0;
				unlock();
				// If we were interupted or the connection was already
				// being made, try once more
				// Literature suggests using select to test when the process is done
				// However here we just close the socket and thus kill the interrupted
				// connection process. We just try once more. It is not the most elegant
				// solution, but it should work.
				if ((err.errnr != EINTR)&&(err.errnr != EINPROGRESS)) 
				{ 
					addressnum++; tries = 0;
				} 
				else 
				{
					tries++;
					if (tries == 10) addressnum++;
				}
				continue;
			}
			unlock();
			success = true;
		}
	
		if (address.isipv6(addressnum))
		{
			tries++;
			dolock();
			if((socketfd_ = ::socket(AF_INET6,SOCK_STREAM,0)) < 0)
			{
				// Could not allocate socket
				// Find out why
				err.errnr = errno;
				err.error = getsocketerror(errno);
				socketfd_ = 0;
				hassocket_ = false;
				unlock();
				addressnum++;
				tries = 0;
				continue;
			}
			hassocket_ = true;
			
			int retvalue;
			if (secs_ > 0)
			{
				retvalue = connect_timeout(socketfd_,address.getsockaddr(addressnum),address.getsockaddrlen(addressnum),secs_);
			}
			else
			{
				retvalue = ::connect(socketfd_,address.getsockaddr(addressnum),address.getsockaddrlen(addressnum));
			}
			
			if( retvalue < 0)
			{
				// Find out what went wrong in connecting to the server
				err.errnr = errno;
				err.error = getconnecterror(errno);
#ifdef _WIN32
				::closesocket(socketfd_);
#else
				::close(socketfd_);
#endif
				hassocket_ = false;
				socketfd_ = 0;
				unlock();
				// If we were interupted or the connection was already
				// being made, try once more
				// Literature suggests using select to test when the process is done
				// However here we just close the socket and thus kill the interrupted
				// connection process. We just try once more. It is not the most elegant
				// solution, but it should work.
				if ((err.errnr != EINTR)&&(err.errnr != EINPROGRESS)) 
				{ 
					addressnum++; tries = 0;
				} 
				else 
				{
					tries++;
					if (tries == 10) addressnum++;
				}
				continue;
			}
			unlock();
			success = true;
		}
	}
	if (!success) 
	{
		return(false);
	}
	
	address.selectaddress(addressnum);
	remoteaddress_ = address;
	isconnected_ = true;
	
	// Reset the error, as we did not encouter one
	err.error = "";
	err.errnr = 0;
	
	return(true);	
}


	
bool	IComINetSocket::close(IComSocketError &err)
{

	dolock();

	if (hassocket_)
	{
		::shutdown(socketfd_,2);

		// Whatever error occured th socket should be closed
		// and that is what we wanted.
		
		socketfd_ = 0;
		isconnected_ = false;
	}
	unlock();

	// Whatever we did it should be closed now
	err.error = "";
	err.errnr = 0;
	return(true);
}


bool IComINetSocket::getlocaladdress(IComAddress &address,IComSocketError &err)
{
	address = localaddress_;
	
	// There is no way this function can fail
	// If there is no address an invalid address is generated
	// which can be tested for in the IComAddress class
	err.errnr = 0;
	err.error = "";
	return(true);
}
	
bool IComINetSocket::getremoteaddress(IComAddress &address,IComSocketError &err)
{
	address = remoteaddress_;
	
	// There is no way this function can fail
	// If there is no address an invalid address is generated
	// which can be tested for in the IComAddress class
	err.errnr = 0;
	err.error = "";
	return(true);
}
	
bool IComINetSocket::settimeout(int secs, int microsecs, IComSocketError &err)
{
	dolock();

	secs_ = secs;
	microsecs_ = microsecs; 
	if (hassocket_)
	{
#ifdef _WIN32
                int t = secs * 1000 + microsecs/1000; // takes ms
                int size = sizeof(int);
                char* tvp = (char*) t;
#else
		struct timeval tv;
		tv.tv_sec = secs;
		tv.tv_usec = microsecs;
                int size = sizeof(tv);
                struct timeval* tvp = &tv;
#endif
		if ( ::setsockopt(socketfd_,SOL_SOCKET,SO_RCVTIMEO,tvp,size) < 0)
		{
			err.errnr = errno;
			err.error = getsockopterror(errno);
			unlock();
			return(false);
		}
		if ( ::setsockopt(socketfd_,SOL_SOCKET,SO_SNDTIMEO,tvp,size) < 0);
		{
			err.errnr = errno;
			err.error = getsockopterror(errno);
			unlock();
			return(false);
		}
	}
	
	unlock();
	err.errnr = 0;
	err.error = "";
	return(true);
}
 

bool	IComINetSocket::listen(IComSocketError &err)
{
	if (hassocket_)
	{
		dolock();
		int ret = 0;
		
		while (1)
		{
			// In case some implementation decides it can interrupt
			// a listen state of a thread. I haven't found any suggestions
			// that there is such a system, but to be safe a protection is
			// implemented against it.
			
			ret = ::listen(socketfd_,50);
			if (ret < 0)
			{
				if (errno == EINTR) continue;
			}
			break;
		}
		
		if(ret < 0) 
		{
			err.errnr = errno;
			err.error = getlistenerror(errno);
			unlock();
			return(false);
		}
		unlock();
		err.errnr = 0;
		err.error = "";
		return(true);
	}

	err.errnr = EBADF;
	err.error = "No socket has been created to listen to";
	return(false);
}

bool    IComINetSocket::accept(IComSocket& newsock, IComSocketError &err)
{
	if (!hassocket_) 
	{
		err.errnr = EBADF;
		err.error = "No socket has been created for accepting connections";
		return(false);
	}
	
	// Here we start changing the content of the structure
	// Or at least we do not want to have two processes accepting connections at the
	// same time. Hence protect the accept procedure with a lock
	
	dolock();
	int newfd = -1;			// Socket created by the system on which the accepted connection can be found
	IComAddress newaddress; // The address of this new connection
	
    
	if (localaddress_.isipv4())
	{ 
		sockaddr_in sa;
		socklen_t salen = sizeof(sockaddr_in);
		// Try to get the file descriptor of the socket that th system connected for use
		while (1)
		{
			newfd = ::accept(socketfd_,reinterpret_cast<sockaddr *>(&sa),&salen);
			if (newfd < 0)
			{	
				// If some process interupted our call to accept, we have to call the
				// accept function again. 
				// On some systems io calls are interupted when a software interrupt occurs
				// Hence we have to restart such a call
				if (errno == EINTR) continue;
			}
			break;
		}
        
		if (newfd < 0)
		{
			// An error occured, so we need to deal with that
			err.errnr = errno;
			err.error = getaccepterror(errno);
			unlock();
			return(false);
		}
        
		if (salen == sizeof(sockaddr_in)) newaddress.setaddress("scirun",reinterpret_cast<sockaddr *>(&sa));
	}

	if (localaddress_.isipv6())
	{ 
		sockaddr_in6 sa;
		socklen_t salen = sizeof(sockaddr_in6);
		while (1)
		{
			newfd = ::accept(socketfd_,reinterpret_cast<sockaddr *>(&sa),&salen);
			if (newfd < 0)
			{	
				// If some process interupted our call to accept, we have to call the
				// accept function again. 
				// On some systems io calls are interupted when a software interrupt occurs
				// Hence we have to restart such a call manually
				if (errno == EINTR) continue;
			}
			break;
		}
		if (newfd < 0)
		{
			// An error occured, so we need to deal with that
			err.errnr = errno;
			err.error = getaccepterror(errno);
			unlock();
			return(false);
		}
		if (salen == sizeof(sockaddr_in6)) newaddress.setaddress("scirun",reinterpret_cast<sockaddr *>(&sa));
	}

	// Make sure there are no connections from this socket
	// More precise, this is a IComSocket, which is no more than
	// a handle to a socket structure. Hence normally it is shared with
	// another handle, hence closing it will only detach this socket
	// from the real socket structure.

	newsock.close();

	if(!(newsock.create("scirun")))
	{
		err.errnr = errno;
		err.error = getsocketerror(errno);
		unlock();
		return(false);
	}
	
	// As mentioned above, newsock is only a handle to a IComVitrualSocket
	// If everything is OK, this vitrual socket has been overloaded with the
	// real socket. In the next line we try to get to this real socket
	IComINetSocket* newsocket = dynamic_cast<IComINetSocket*>(newsock.getsocketptr());

	// In case that was not a success...
	if (newsocket == 0)
	{
		newsock.close();
		unlock();
		err.errnr = EBADF;
		err.error = "Internal pointer to socket is not valid";
		return(false);
	}
	
	// Now we have the internal structure of the newly created
	// INet socket. Which we now have to fill with the new information
	// This is a kind of cheating, as we go to the lowest level directly to
	// create the new socket. The problem is that the system already create a
	// socket for us, hence we need to artificially set the information in this
	// new socket.
	
	newsocket->socketfd_ = newfd;
	newsocket->hassocket_ = true;
	newsocket->isconnected_ = true;
	newsocket->remoteaddress_ = newaddress;

	// We still need to set the local address
	// We do it the lazy way and do not look up the real address of this new one
	// We just copy the address from the listen socket. This is what people should
	// be interested anyway
	newsocket->localaddress_ = localaddress_;

	unlock();
	err.errnr = 0;
	err.error = "";
	return(true);
}


bool	IComINetSocket::poll(IComPacketHandle &packet, IComSocketError &err)
{
	char buffer[1];
	int len = 0;

	if (packet == 0) packet = new IComPacket;
	
#ifdef _WIN32
        int flags = MSG_PEEK;
#else
        int flags = MSG_PEEK|MSG_DONTWAIT;
#endif
	// We just peek, non-blocking whether we can read a byte
	// If so a package is coming and it is worthwile to wait for it
	len = ::recv(socketfd_,&(buffer[0]),1,flags);
	if ((len < 0)&&(errno != EAGAIN))
	{

	}
	if (len)
	{
		return(recv(packet,err));
	}
	err.errnr = 0;
	err.error = "";
	return(false);
	// No package is waiting so the polling was not success, though there is no error
}

bool	IComINetSocket::send(IComPacketHandle &packet, IComSocketError &err)
{
	int header[8];
	int	len;
	int bytessend = 0;
	int bytestosend = 0;
	char *buf = 0;
	
    // If we are writing to a socket that was close, we want
    // an error msg, we do not want the program to quit as the
    // default handler does.
	//    ::signal(SIGPIPE,SIG_IGN);
    
	::strncpy(reinterpret_cast<char *>(&(header[0])),"icpacket",8);
	header[2] = packet->getelsize();
	header[3] = packet->getdatasize();
	header[4] = packet->gettag();
	header[5] = packet->getid();
	header[6] = packet->getparam1();
	header[7] = packet->getparam2();
	
	dolock();
	bytessend = 0;
	bytestosend = 32;
	buf = reinterpret_cast<char *>(&header[0]);
	while (bytessend < bytestosend)
	{
		len = ::send(socketfd_,&(buf[bytessend]),bytestosend-bytessend,0);
		if (len < 0)
		{
			if ((errno == EINTR)||(errno == EAGAIN)) continue;
            if ((errno == EPIPE)) isconnected_ = false;
            break;
		}
		bytessend += len;
	}
	
	if (len < 0)
	{
		err.errnr = errno;
		err.error = getsenderror(errno);
		unlock();
		return(false);
	}
	
	if (bytessend != bytestosend) 
	{
		err.errnr = EMSGSIZE;
		err.error = "Could not send the complete packet";
		unlock();
		return(false);
	}

  	bytessend = 0;
	bytestosend = header[2]*header[3];
	buf = static_cast<char *>(packet->getbuffer());
	while (bytessend < bytestosend)
	{
		len = ::send(socketfd_,&(buf[bytessend]),bytestosend-bytessend,0);
		if (len < 0)
		{
			if ((errno == EINTR)||(errno == EAGAIN)) continue;
            if ((errno == EPIPE)) isconnected_ = false;
            break;            
		}
		bytessend += len;
	}

	
	if (len < 0)
	{
		err.errnr = errno;
		err.error = getsenderror(errno);
		unlock();
		return(false);
	}
	
	
	if (bytessend != bytestosend) 
	{
		err.errnr = EMSGSIZE;
		err.error = "Could not send the complete packet";
		unlock();
		return(false);
	}
	
	unlock();
	
	err.errnr = 0;
	err.error = "";
	return(true);
}


bool	IComINetSocket::recv(IComPacketHandle &packet, IComSocketError &err)
{
	char buffer[24];
	int  len = 0;
	int  bytesread =0;
	int  bytestoread = 0;
	bool byteswap = false;

	if (packet == 0) packet = new IComPacket();

	buffer[8] = 0;
	
    len = 0;
	bytesread   = 0;  
	bytestoread = 8;
	while (bytesread < bytestoread)
	{
		len = ::recv(socketfd_,&(buffer[bytesread]),(bytestoread-bytesread),0);	
		if (len < 0)
		{
			if ((errno == EINTR)||(errno == EAGAIN)) continue;
			break;
		}
		bytesread += len;
	}
	
	if (len < 0) 
	{
		if (errno == ENOTCONN) isconnected_ = false;
		err.errnr = errno;
		err.error = getrecverror(errno);
		return(false);
	}
	if (bytesread != bytestoread)
	{
		err.errnr = EMSGSIZE;
		err.error = std::string("Could not read enough bytes from stream");
		return(false);
	}
	if (::strncmp("icpacket",buffer,8) != 0) 
	{
		err.errnr = EMSGSIZE;
		err.error = std::string("Protocol fault, received improper data");
		return(false);
	}
	
    len = 0;
	bytesread = 0;
	bytestoread = 24;
	
	while (bytesread < bytestoread)
	{
		len = ::recv(socketfd_,&(buffer[bytesread]),(bytestoread-bytesread),0);	
		if (len < 0)
		{
			if ((errno == EINTR)||(errno == EAGAIN)) continue;
			break;
		}
		bytesread += len;
	}
	
	if (len < 0)
	{
		if (errno == ENOTCONN) isconnected_ = false;
		err.errnr = errno;
		err.error = getrecverror(errno);
		return(false);
	}
	if (bytesread != bytestoread)
	{
		err.errnr = EMSGSIZE;
		err.error = std::string("Could not read enough bytes from stream");
		return(false);		
	}	
	long* header = reinterpret_cast<long *>(buffer);

	
	if (header[0] > 255)
	{	// The whole thing must be byte swapped if the first number is over 255
		packet->swap_bytes(header,6,4);
		byteswap = true;
	}
	
	packet->setelsize(header[0]);
	packet->setdatasize(header[1]);
	packet->settag(header[2]);
	packet->setid(header[3]);
	packet->setparam1(header[4]);
	packet->setparam2(header[5]);
	
	int bytesize = header[0]*header[1];
    if (bytesize > 0)
    {
        packet->newbuffer(bytesize);
        char *buf = static_cast<char *>(packet->getbuffer());
        if (buf == 0)
        {
            // Can report a proper message, but that will be to no avail
            // we did run out of memory
            std::cerr << "Did run out of memory\n";
            return(false);
        }
        
        len = 0;
        bytesread = 0;
        bytestoread = bytesize;
        while(bytesread <bytestoread)
        {
            len = ::recv(socketfd_,&(buf[bytesread]),(bytesize-bytesread),0);
            if (len < 0)
            {
                if ((errno == EINTR)||(errno == EAGAIN)) continue;
                break;
            }
            bytesread += len; 
        }
        
        
        if (len < 0)
        {
            if (errno == ENOTCONN) isconnected_ = false;
            err.errnr = errno;
            err.error = getrecverror(errno);
            packet->clear();
            return(false);
        }
        if (bytesread != bytestoread)
        {
            err.errnr = EMSGSIZE;
            err.error = std::string("Could not read enough bytes from stream");
            return(false);		
        }	
        
        if (byteswap) packet->swap_bytes(buf,header[1],header[0]);
	}
    
	err.errnr = 0;
	err.error = "";				
	return(true);
}
 
bool	IComINetSocket::isconnected(IComSocketError& /*err*/)
{
	// I need to update this function to have a better mechanism for detecting 
	// broken connections. This currently only checks of the socket was originally
	// connected
	return(isconnected_);
} 


} // end namespace


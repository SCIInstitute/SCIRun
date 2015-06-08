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
 *  IComSocket.h 
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */
 
#if !defined(IComSocket_h)
#define IComSocket_h

#include <errno.h>
#include <Core/ICom/IComBase.h>
#include <Core/ICom/IComPacket.h>
#include <Core/ICom/IComAddress.h>
#include <Core/ICom/IComVirtualSocket.h>


#include <iostream>


#include <Core/ICom/share.h>

// This class functions like a handle, but is more complicated
// Since errors are stored in unix in errno and this one is cleared
// after invoking the next io instruction, the IComSocket class is a
// general interface handler to one the underlying socket implementations
// The socket implementations do both the protocol algorithms as well as the
// communication. The latter is done to optimize transmission effiecency

// THREADS: Creating a socket and binding/connecting it should be done in the
// same thread. Though receiving and sending operations can be spread over 
// multiple threads, as long as each thread has its own copy of this IComSocket
// descriptor. Do not share the same socket descriptor between threads, 
// this will causes problems with the error reporting functions and the 
// close function.

// Using a IComSocket object per thread is hardly any overhead on memory
// Only recent errors are stored per object. This allows the users to retrieve
// the last error that occured in the code this thread is running.

// There is no need to use LockingHandles with this class, 
// it has a more extensive locking model build in.

namespace SCIRun {

class IComSocket;
class IComPacket;
class IComVirtualSocket;

class SCISHARE IComSocket : public IComBase {

public:
  // the public functions 
	
  // FUNCTIONS FOR CREATING/COPYING AND DELETING OBJECT
  IComSocket(std::string protocol = "internal");	 // constructor 
  virtual ~IComSocket();			         // destructor 
  IComSocket(const IComSocket &s);			 // copy constructor 
  IComSocket& operator=(const IComSocket &s);	    // assignment constructor 

  // THIS FUNCTION MUST BE CALLED TO CREATE SOCKET
  bool	create();
  bool	create(std::string protocol);  	// overrule setting made in constructor

  // TO DESIGNATE A SOCKET AS A CLIENT OR A SERVER CALL bind OR connect
  bool	bind(IComAddress& address);
  // Connect socket to a certain address
  bool	connect(IComAddress& address, conntype conn = DIRECT);	
	
  // CLOSE THE SOCKET CONNECTION, THIS WILL FREE THE SOCKET
  bool	close();		     // close the socket 

  // GET INFORMATION ON WHERE THE LOCAL AND OTHER END OF THE SOCKET ARE
  // In case of an internal socket the client socket will not have any name
  bool	getlocaladdress(IComAddress &address);	// Get local address
  bool	getremoteaddress(IComAddress &address);	// Get remote address
	
  // SET THE AMOUNT OF TIME WE SHOULD WAIT FOR AN OPERATION TO CONNECT
  // set the time out of the socket 
  bool	settimeout(int secs, int microsecs);	

  // LISTEN AND WAIT FOR SOMEONE TO CONNECT
  // There is no time out on this operation
  // Once listen returns, use accept to create the other end of a 
  // connected socket pair sock needs to be an empty object. 
  // All other information stored will be cleared
  // The return value will indicate whether a connection could be made
  bool	listen();		 // Listen for a connection

  // Accept the connection and get a new socket object
  // The communication is over this socket
  // Verification is done here as well
  bool    accept(IComSocket& sock); 

	
  // The three main functions for reading and writing data. 
  // The last two observe the time out that had been set. 
  // The first one is non-blocking and only blocks if a message is 
  // being transmitted. It will return immediately if there is no message, 
  // otherwise it will wait for a timeout or until the package has been 
  // completely received.

  // All functions perform a proper byteswapping
  // Poll whether there is a packet waiting (non blocking)	
  bool	poll(IComPacketHandle &packet);
  // Send a packet (blocking)
  bool	send(IComPacketHandle &packet);
  // Recv a packet (blocking)	
  bool	recv(IComPacketHandle &packet);
	
  // This function is not garanteed to work for each socket type and 
  // implementation and may require so time to execute 
  // (to check whether conenction still exists)
  bool	isconnected();
	
  // FUNCTIONS TO GET TO THE INTERNALS OF THE SOCKET IMPLEMENTATION
	
  // Get the pointer to the actual socket implementation
  IComVirtualSocket*	getsocketptr();
  // Get the type of the socket
  std::string			getsocketprotocol();
	
  // FUNCTIONS TO RETRIEVE AN ERROR
	
  // These functions need to be called directly after the function that failed
  // These functions only retain information on the last function called
  // Get the last error
  std::string			geterror();  
  // Get the error number as defined by unix
  int				geterrno();
  // Did the last failure indicate an error?  
  bool				haserror();
	
private:
  void clear();
  bool nosocketerror();

  IComVirtualSocket*	socket_;
  std::string			protocol_;

  // These are kept here separately, so each thread can use a different 
  // IComSocket structure with its own error report functionality and 
  // still share the descriptor and other statistics with other threads
  IComSocketError		error_;
};


// INLINE FUNCTIONS

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
inline IComVirtualSocket* IComSocket::getsocketptr()
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

inline bool IComSocket::accept(IComSocket& newsock)
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

} // end namespace

#endif

 


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

#ifndef JGS_SCI_CORE_ICOM_VIRTUALSOCKET_H
#define JGS_SCI_CORE_ICOM_VIRTUALSOCKET_H 1

#include <errno.h>
#include <Core/ICom/IComBase.h>
#include <Core/ICom/IComSocket.h>
#include <Core/Thread/Mutex.h>

namespace SCIRun {

class IComPacket;

class IComVirtualSocket : public IComBase
{
public:
	IComVirtualSocket();

	// We overload this class with useful implementations of different
	// socket classes. Hence the destructor NEEDS to be virtual
	virtual ~IComVirtualSocket();

	// Functions for locking and unlocking the structure
	// ALL OVERLOADED FUNCTIONS SHOULD USE THESE IF THEY NEED
	// TO CHANGE ANYTHING IN THE SOCKET STRUCTURE!!
	// THE ICOMSOCKET ONLY LOCKS AND UNLOCKS FOR CHANGES TO THE REFCOUNTER

	void	dolock();
	void	unlock();

  public:

	// CREATE SERVER SOCKET
	virtual bool	bind(IComAddress& address,IComSocketError &err);
													// Bind socket to a certain address
	// CREATE CLIENT SOCKET
	virtual bool	connect(IComAddress& address, conntype conn,IComSocketError &err);
													// Connect socket to a certain address

	// CLOSE THE SOCKET CONNECTION, THIS WILL FREE THE SOCKET
	virtual bool	close(IComSocketError &err);	// close the socket

	// GET INFORMATION ON WHERE THE LOCAL AND OTHER END OF THE SOCKET ARE
	// In case of an internal socket the client socket will not have any name
	virtual bool getlocaladdress(IComAddress &address,IComSocketError &err);	// Get local address
	virtual bool getremoteaddress(IComAddress &address,IComSocketError &err);	// Get remote address

	// SET THE AMOUNT OF TIME WE SHOULD WAIT FOR AN OPERATION TO CONNECT
	virtual bool settimeout(int secs, int microsecs,IComSocketError &err);	// set the time out of the socket

	// LISTEN AND WAIT FOR SOMEONE TO CONNECT
	// There is no time out on this operation
	virtual bool	listen(IComSocketError &err);	// Listen for a connection
	// Once listen returns, use accept to create the other end of a connected socket pair
	// sock needs to be an empty object. All other information stored will be cleared
	// The return value will indicate whether a connection could be made
	virtual bool    accept(IComSocketHandle& newsock, IComSocketError &err);	// Accept the connection and get a new socket object
													// The communication is over this socket
													// Verification is done here as well

	// The three main functions for reading and writing data. The last two observe the time out
	// that had been set. The first one is non-blocking and only blocks if a message is being transmitted
	// It will return immediately if there is no message, otherwise it will wait for a timeout or until the
	// package has been completely received.
	// All functions perform a proper byteswapping

	virtual bool	poll(IComPacketHandle &packet,IComSocketError &err);	// Poll whether there is a packet waiting (non blocking)
	virtual bool	send(IComPacketHandle &packet,IComSocketError &err);	// Send a packet (blocking)
	virtual bool	recv(IComPacketHandle &packet,IComSocketError &err);	// Recv a packet (blocking)

	virtual bool	isconnected(IComSocketError &err);				// Test for connectivity

protected:
  // Signal that a feature is not implemented yet
  bool		socket_not_implemented(IComSocketError &err);
  Core::Thread::Mutex lock_;
};

}

#endif

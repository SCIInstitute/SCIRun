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
 *  IComInternalSocket.cc
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */

#include <Core/ICom/IComInternalSocket.h>
#include <Core/ICom/IComAddress.h>
#include <Core/Thread/Mutex.h>

using namespace SCIRun::Core::Thread;

namespace SCIRun {

Mutex internalsocketlock_("internal_socket_list");
std::map<std::string,IComInternalSocket*> internalsocketlist_;

IComInternalSocket::IComInternalSocket() :
	listen_(false),
	connected_(false),
	registered_(false),
	localaddress_("internal","no_name"),
	remotesocket_(0),
	waitconnection_("connection_list_condition_variable"),
	waitpacket_("packet_list_condition_variable"),
	secs_(0),
	microsecs_(0)
{
}

IComInternalSocket::~IComInternalSocket()
{
	IComSocketError err;
	if (registered_ || listen_ || connected_)
	{
		close(err);
	}
}

bool IComInternalSocket::bind(IComAddress& address, IComSocketError &err)
{
	internalsocketlock_.lock();		// Full access to the global list maintaining all internal sockets
	dolock();							// Full access to this socket

	try
	{
		if (!address.isinternal()) 	throw invalid_address();

		std::string name = address.getinternalname();
		if (internalsocketlist_[name] != 0)  throw invalid_address();

		internalsocketlist_[name] = this;
		registered_ = true;

		// Reinitialize the socket
		localaddress_ = address;
		remoteaddress_.clear();
		remotesocket_ = 0;

		connectionlist_.clear();
		packetlist_.clear();

		unlock();
		internalsocketlock_.unlock();
	}

	catch(...)
	{
		err.errnr = EADDRNOTAVAIL;
		err.error = "Could not resolve address";
		unlock();
		internalsocketlock_.unlock();
		return(false);
	}

	err.errnr = 0;
	err.error = "";
	return(true);
}


bool	IComInternalSocket::connect(IComAddress& address, conntype /*conn*/, IComSocketError &err)
{
	IComInternalSocket* isock = 0;

	internalsocketlock_.lock();
	dolock();

	try
	{
		if (!address.isinternal()) throw invalid_address();

		isock = internalsocketlist_[address.getinternalname()];

		if (isock == 0) throw invalid_address();
		if (isock == this)
		{
			std::cerr << "Socket tried to connect to itself\n";
			throw invalid_address();
		}

		isock->dolock();
		internalsocketlock_.unlock();

		if (isock->listen_ == false) throw invalid_address();	// Socket is not listening

		// Create a new socket;
		IComSocketHandle newsocket(new IComSocket);
		newsocket->create("internal");

		auto rsock = boost::dynamic_pointer_cast<IComInternalSocket>(newsocket->getsocketptr());

		if (!rsock) throw could_not_open_socket();

		// Exchange the address information
		rsock->remoteaddress_ = localaddress_;
		rsock->remotesocket_ = this;
		rsock->listen_ = false;
		rsock->connected_ = true;

		remoteaddress_ = address;
		remotesocket_ = rsock.get();
		listen_ = false;
		connected_ = true;

		isock->connectionlist_.push_back(newsocket);

		// Wake up the internal server
		isock->unlock();
		isock->waitconnection_.conditionBroadcast();

    UniqueLock lock(lock_.get());
		waitconnection_.wait(lock);	// This should unlock the lock on the socket
		unlock();
		err.errnr = 0;
		err.error = "";
		return(true);
	}

	catch(...)
	{
		if (isock) isock->unlock();
		internalsocketlock_.unlock();
		err.errnr = EADDRNOTAVAIL;
		err.error = "Could not connect to specified address";
		unlock();
		return(false);
	}

}


bool IComInternalSocket::listen(IComSocketError &err)
{
	dolock();
	if (connected_)
	{	// We cannot listen on a connected socket
		unlock();
		err.errnr = EISCONN;
		err.error = "Socket is connected to another socket";
		return(false);
	}
	listen_ = true;
  UniqueLock lock(lock_.get());
	waitconnection_.wait(lock);
	unlock();
	err.errnr = 0;
	err.error = "";

	return(true);
}


bool IComInternalSocket::accept(IComSocketHandle& newsock, IComSocketError &err)
{
	dolock();
	if (!listen_)
	{
		unlock();
		err.errnr = EOPNOTSUPP;
		err.error = "This socket is not in listening mode";
		return(false);
	}

    if (connectionlist_.size() == 0)
    {
        // wait for an incoming packet
        UniqueLock lock(lock_.get());
        waitconnection_.wait(lock);
    }

	if (connectionlist_.size() > 0)
	{
		newsock = connectionlist_.front();
		connectionlist_.pop_front();
		err.errnr = 0;
		err.error = "";
		unlock();
		auto sptr = boost::dynamic_pointer_cast<IComInternalSocket>(newsock->getsocketptr());

		if(!(sptr->remotesocket_))
		{
			err.errnr = ENOTCONN;
			err.error = "This socket is not connected";
			return(false);
		}

		sptr->remotesocket_->dolock();
		sptr->remotesocket_->connected_ = true;
		sptr->remotesocket_->waitconnection_.conditionBroadcast();
		sptr->remotesocket_->unlock();

		return(true);
	}
	else
	{
		unlock();
		err.errnr = EWOULDBLOCK;
		err.error = "No connection is available, whereas a signal was given that one should be available";
		return(false);
	}
}


bool	IComInternalSocket::close(IComSocketError &err)
{
	internalsocketlock_.lock();
	dolock();

	if (connected_ && remotesocket_)
	{
		remotesocket_->dolock();
		remotesocket_->connected_ = false; // disconnect the other halve
		remotesocket_->remotesocket_ = 0;
		remotesocket_->waitpacket_.conditionBroadcast();
		remotesocket_->waitconnection_.conditionBroadcast();
		remotesocket_->unlock();
	}


	if (registered_)
	{
		if (localaddress_.isinternal())
		{
			std::string name = localaddress_.getinternalname();
			internalsocketlist_.erase(name);
		}
	}

	unlock();
	internalsocketlock_.unlock();

	// It is fully unconnected now, so we can release the locks
	// We need to do this as the connectionlist_.clear(), could invoke
	// this instruction again.

	// There should be no threads waiting for this socket
	// in case there are just release them.

	connected_ = false;
	listen_ = false;
	registered_ = false;

	waitpacket_.conditionBroadcast();
	waitconnection_.conditionBroadcast();

	remotesocket_ = 0;
	packetlist_.clear();
	connectionlist_.clear();


	err.errnr = 0;
	err.error = "";
	return(true);
}

bool IComInternalSocket::getlocaladdress(IComAddress &address, IComSocketError &err)
{
	dolock();
	address = localaddress_;
	unlock();
	err.errnr = 0;
	err.error = "";
	return(true);
}

bool IComInternalSocket::getremoteaddress(IComAddress &address, IComSocketError &err)
{
	dolock();
	address = remoteaddress_;
	unlock();

	err.errnr = 0;
	err.error = "";
	return(true);
}

bool IComInternalSocket::settimeout(int secs, int microsecs, IComSocketError &err)
{
	dolock();
	secs_ = secs;
	microsecs_ = microsecs;
	unlock();
	err.errnr = 0;
	err.error = "";
	return(true);
}


bool	IComInternalSocket::poll(IComPacketHandle &packet, IComSocketError &err)
{
	dolock();
	if (!connected_)
	{
		unlock();
		err.errnr = ENOTCONN;
		err.error = "Socket is not connected";
		return(false);
	}

	if (packetlist_.size() > 0)
	{
		packet = packetlist_.front();
		packetlist_.pop_front();
		unlock();

		err.errnr = 0;
		err.error = "";
		return(true);
	}
	else
	{
		unlock();
		err.errnr = 0;
		err.error = "";
		return(false);
	}
}


bool	IComInternalSocket::send(IComPacketHandle &packet, IComSocketError &err)
{
	dolock();
	if (!connected_)
	{
		unlock();
		err.errnr = ENOTCONN;
		err.error = "Socket is not connected";
		return(false);
	}

	if (!packet)
	{
		unlock();
		err.errnr = ENOTCONN;		// Need to change this one
		err.error = "empty packet handle was supplied";
		return(false);
	}

	IComInternalSocket* rsock = remotesocket_;
	if (!rsock)
	{
		unlock();
		err.errnr = ENOTCONN;
		err.error = "Socket is not connected";
		return(false);
	}
	unlock();

	rsock->dolock();
	rsock->packetlist_.push_back(packet);
	rsock->waitpacket_.conditionBroadcast();
  rsock->unlock();


	// This one is here to make the code more robust
	// If some decides to reuse a packet, there is one
	// that is not coupled to the one just send
	// otherwise efforts may interfere
	packet.reset(new IComPacket());

	err.errnr = 0;
	err.error = "";
	return(true);
}

bool IComInternalSocket::recv(IComPacketHandle &packet, IComSocketError &err)
{
	dolock();
	if (!connected_)
	{
		unlock();
		err.errnr = ENOTCONN;
		err.error = "Socket is not connected";
		return(false);
	}
	if (packetlist_.size() > 0)
	{
		packet = packetlist_.front();
		packetlist_.pop_front();
		unlock();
		err.errnr = 0;
		err.error = "";
		return(true);
	}
	else
	{
    UniqueLock lock(lock_.get());
		waitpacket_.wait(lock);
		if (packetlist_.size() > 0)
		{
			packet = packetlist_.front();
			packetlist_.pop_front();
			unlock();
			err.errnr = 0;
			err.error = "";
			return(true);
		}
		else
		{
			unlock();
			err.errnr = EINTR;
			err.error = "Receiving process was somehow interrupted";
			return(false);
		}
	}
}

bool	IComInternalSocket::isconnected(IComSocketError &err)
{
	err.errnr = 0;
	err.error = "";
	return(connected_);
}

}

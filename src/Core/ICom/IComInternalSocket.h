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

#ifndef JGS_SCI_CORE_ICOM_INTERNALSOCKET_H
#define JGS_SCI_CORE_ICOM_INTERNALSOCKET_H 1

#include <Core/ICom/IComAddress.h>
#include <Core/ICom/IComVirtualSocket.h>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/ConditionVariable.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif


#include <string>
#include <map>
#include <deque>



namespace SCIRun {

  class IComInternalSocket;
  class IComSocket;

  class IComInternalSocket : public IComVirtualSocket {

  public:
    IComInternalSocket();
    virtual ~IComInternalSocket();

  public:

    virtual bool    bind(IComAddress& address, IComSocketError &err) override;
    virtual bool    connect(IComAddress& address, conntype conn, IComSocketError &err) override;

    virtual bool    close(IComSocketError &err) override;                    // close the socket

    virtual bool    getlocaladdress(IComAddress& address, IComSocketError &err) override;    // Get local address
    virtual bool    getremoteaddress(IComAddress& address, IComSocketError &err) override;   // Get remote address

    virtual bool    settimeout(int secs, int microsecs, IComSocketError &err) override;      // set the time out of the socket

    virtual bool    listen(IComSocketError &err) override;   // Listen for a connection
    virtual bool    accept(IComSocketHandle& newsock, IComSocketError &err) override;      // Accept the connection and get a new socket object

    virtual bool    poll(IComPacketHandle &packet, IComSocketError &err) override;                                           // Poll whether there is a packet waiting (non blocking)
    virtual bool    send(IComPacketHandle &packet, IComSocketError &err) override;                                           // Send a packet (blocking)
    virtual bool    recv(IComPacketHandle &packet, IComSocketError &err) override;                                           // Recv a packet (blocking)

    virtual bool    isconnected(IComSocketError &err) override;

  private:

    bool                            listen_;
    bool                            connected_;
    bool                            registered_;

    IComAddress                     localaddress_;
    IComAddress                     remoteaddress_;
    IComInternalSocket*            remotesocket_;

    std::deque<IComPacketHandle>     packetlist_;
    std::deque<IComSocketHandle>           connectionlist_;

    Core::Thread::ConditionVariable               waitconnection_;
    Core::Thread::ConditionVariable               waitpacket_;

    int                             secs_;
    int                             microsecs_;

  };

}

#endif

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

#include <Core/ICom/IComBase.h>
#include <Core/ICom/IComPacket.h>
#include <Core/ICom/IComAddress.h>
#include <Core/ICom/IComSocket.h>
#include <Core/Thread/Mutex.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif


#include <string>
#include <map>
#include <list>



namespace SCIRun {

  class IComInternalSocket;
  class IComSocket;

  class IComInternalSocket : public IComVirtualSocket {

  public:
    IComInternalSocket();
    virtual ~IComInternalSocket();
        
  public:
  
    bool    bind(IComAddress& address, IComSocketError &err);
    bool    connect(IComAddress& address, conntype conn, IComSocketError &err);     
        
    bool    close(IComSocketError &err);                    // close the socket 

    bool    getlocaladdress(IComAddress& address, IComSocketError &err);    // Get local address
    bool    getremoteaddress(IComAddress& address, IComSocketError &err);   // Get remote address
        
    bool    settimeout(int secs, int microsecs, IComSocketError &err);      // set the time out of the socket 

    bool    listen(IComSocketError &err);   // Listen for a connection
    bool    accept(IComSocket& newsock, IComSocketError &err);      // Accept the connection and get a new socket object
        
    bool    poll(IComPacketHandle &packet, IComSocketError &err);                                           // Poll whether there is a packet waiting (non blocking)
    bool    send(IComPacketHandle &packet, IComSocketError &err);                                           // Send a packet (blocking)
    bool    recv(IComPacketHandle &packet, IComSocketError &err);                                           // Recv a packet (blocking)     

    bool    isconnected(IComSocketError &err);

  private:
  
    bool                            listen_;
    bool                            connected_;
    bool                            registered_;
  
    IComAddress                     localaddress_;
    IComAddress                     remoteaddress_;
    IComInternalSocket*             remotesocket_;

    std::list<IComPacketHandle>     packetlist_;
    std::list<IComSocket>           connectionlist_;
        
    ConditionVariable               waitconnection_;
    ConditionVariable               waitpacket_;
        
    int                             secs_;
    int                             microsecs_;
        
  };

}

#endif

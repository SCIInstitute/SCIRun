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
 * ServiceFrame.cc
 *
 */

#include <exception>

#include <Core/Services/ServiceFrame.h>
#include <Core/SystemCall/SystemCallError.h>
#include <Core/Thread/ThreadError.h>

namespace SCIRun { 

  ServiceFrame::ServiceFrame(IComSocket socket, ServiceDBHandle db, ServiceLogHandle log) :
    Runnable(true), 
    socket_(socket), 
    db_(db), 
    log_(log)
  {
  }

  void ServiceFrame::run()
  {
    // Initiateservice returns a true if the
    // service exists and the authentication went OK
    if (initiate()) runservice();
    close();
  }


  bool ServiceFrame::initiate()
  {
	
    // DEBUG INFORMATION
    log_->putmsg("ServiceFrame: initiated a new service framework");
		
    // Test some basic things on the connection
    if (socket_.isconnected() == false) 
      {
        log_->putmsg("ServiceFrame: socket does not appear to be connected, hence exiting request");
        log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());		
        return(false);  // We are not connected
      }
	
    IComPacketHandle  packet;
	
    // Currently I set a timeout so if no decent data is comming we can abort
    // The first packet is always the tricky one, as the process sending us data is
    // not SCIRun it does not know the protocol, so we need to be able to detect bad
    // data. Currently a timeout will abort if the data is not proper
	
    socket_.settimeout(5,0);
    if (!socket_.recv(packet))
    {
      // For improper data it does not make any sense to respond:
      // The application on the other side is probably not SCIRun anyway
      log_->putmsg("ServiceFrame: Did receive an improper service request (did not recognise communication protocol)");
      log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());		
      return(false);
    }
	
    if (packet->gettag() != TAG_RQSV)
    {
      // The first package should be a request service
      // If not something weird is going on, and thus abort
      log_->putmsg("ServiceFrame: Did receive an improper service request (communication protocol recognised, but message does not make sense)");
      std::ostringstream oss;
      oss << "Packet: tag=" << packet->gettag() << ",  id=" << packet->getid() << ", datasize=" << packet->getdatasize() << ", elsize=" << packet->getelsize() <<"\n";
      log_->putmsg(oss.str());
      return(false);
    }
	
    if (packet->getelsize() != 1)
    {
      // Somebody send us data, but it is not a string
      // So no clue on what is going on. So replying that
      // the service cannot be found
  
      packet->settag(TAG_RQFL);
      packet->setid(0);
  
      packet->setstring(std::string("Service does not exist"));
      socket_.send(packet);

      log_->putmsg("ServiceFrame: Did receive an improper service request (communication protocol recognised,but message does not contain a service name)");
      return(false);
    }
	
    session_ = packet->getid();
    servicename_ = packet->getstring();

    log_->putmsg("ServiceFrame: searching for service (" + servicename_ + ")" );

    if (!db_->isservice(servicename_))
    {
      // A legitimate request was made, unfortunately we
      // do not have the service requested.
      // probably the package containing the service was not
      // compiled. 
      // Hence we reply that the service is not available
  
      packet->settag(TAG_RQFL);
      packet->setid(0);
      packet->setstring(std::string("Service does not exist"));
      if (!(socket_.send(packet)))
        {
          log_->putmsg("ServiceFrame: could not send reply to client");
          log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());
        }

      std::string str = "ServiceFrame: Did receive a service request for a non-existing service (" + servicename_ + ")"; 
      log_->putmsg(str);

      return(false);
    }
	
    // We tested before whether the service existed
    // So it should have a service info
    ServiceInfo *si = db_->getserviceinfo(servicename_);

    log_->putmsg("ServiceFrame: got service information record");

    // Check whether client should have access
    // (1) It needs a valid internet address
    // (2) It needs to be on the approved internet addresses list
    // If not we simply refuse the connection
	
    // Obtain client's IP address
    IComAddress address;
    if (!(socket_.getremoteaddress(address)))
    {
      packet->settag(TAG_RQFL);
      packet->setid(0);
      packet->setstring(std::string("Could not resolve client's internet address"));
      socket_.send(packet);
  
      std::string str = "ServiceFrame: Could not resolve client's internet address (DNS=" + address.getinetname() + " IP=" + address.getipname() + ")";
      log_->putmsg(str);
      return(false);

    }

    log_->putmsg("ServiceFrame: found client's address");
	
    // Compare it against the list of approved hosts
    if(!(si->rhosts.compare(address)))
      {
        packet->settag(TAG_RQFL);
        packet->setid(0);
        packet->setstring(std::string("Access refused"));
        socket_.send(packet);
	
        std::string str = "ServiceFrame: Access refused (DNS=" + address.getinetname() + " IP=" + address.getipname() + ")";	
        log_->putmsg(str);
        return(false);		
      }
	
    log_->putmsg("ServiceFrame: host is on the list of accepted clients");
	
    // So now we can send out a message that the client is approved
	
    packet->settag(TAG_RQSS);
    packet->setid(session_);
    packet->setstring(servicename_);
    if(!(socket_.send(packet)))
    {
      log_->putmsg("ServiceFrame: could not send reply to client");
      log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());
      return(false);	
    }
	
    log_->putmsg("ServiceFrame: send message of acceptance to client");
	
  
    // We are not done yet, the client should now send us a valid password
    // This is not a perfect solution, but it will work for now
	
    if (!socket_.recv(packet))
    {
      // For improper data it does not make any sense to respond:
      // The application on the other side is probably not SCIRun anyway
      log_->putmsg("ServiceFrame: Did receive an improper authentication request (did not recognise communication protocol)");
      log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());		
      return(false);
    }
	
    if (packet->gettag() != TAG_AUTH)
    {
      // The first package should be a request service
      // If not something weird is going on, and thus abort
      log_->putmsg("ServiceFrame: Did receive an improper authentication request (communication protocol recognised, but message does not make sense)");
      return(false);
    }

    log_->putmsg("ServiceFrame: waiting for authentication");

	
    if ((si->passwd == "")||(address.isinternal()))
      {
        // The internal server does not need a password
        packet->settag(TAG_AUSS);
        packet->setid(0);
        packet->setstring(si->version);
        if(!(socket_.send(packet)))
          {
            log_->putmsg("ServiceFrame: could not send reply to client");
            log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());
            return(false);
          }

      }
    else
      {
        if (packet->getelsize() != 1)
          {
            // Somebody send us data, but it is not a string
            // So no clue on what is going on. So replying that
            // the service cannot be found
			
            packet->settag(TAG_AUFL);
            packet->setid(0);
            packet->setstring(std::string("Improper password"));
            socket_.send(packet);
		
            log_->putmsg("ServiceFrame: Did receive an improper authentication request (communication protocol recognised,but message does not contain a password)");
            return(false);
          }
		
        std::string passwd = packet->getstring();
        if (si->passwd == passwd)
          {
  			
            packet->settag(TAG_AUSS);
            packet->setid(0);
            packet->setstring(si->version);
            if(!(socket_.send(packet)))
              {
                log_->putmsg("ServiceFrame: could not send reply to client");
                log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());
                return(false);
              }
          }
        else
          {
            packet->settag(TAG_AUFL);
            packet->setid(0);
            packet->setstring(std::string("Improper password"));
            if(!(socket_.send(packet)))
              {
                log_->putmsg("ServiceFrame: could not send reply to client");
                log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());
              }
		
            log_->putmsg("ServiceFrame: Invalid password)");
            return(false);
          }
      }



    if (!socket_.recv(packet))
    {
      // For improper data it does not make any sense to respond:
      // The application on the other side is probably not SCIRun anyway
      log_->putmsg("ServiceFrame: Did receive an improper startcommand request (did not recognise communication protocol)");
      log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());		
      return(false);
    }
	
    if (packet->gettag() != TAG_STRT)
    {
      // The first package should be a request service
      // If not something weird is going on, and thus abort
      log_->putmsg("ServiceFrame: Did receive an improper startcommand request (communication protocol recognised, but message does not make sense)");
      return(false);
    }

    log_->putmsg("ServiceFrame: waiting for startcommand");


    if (packet->getelsize() != 1)
    {
      // Somebody send us data, but it is not a string
      // So no clue on what is going on. So replying that
      // the service cannot be found

      packet->settag(TAG_STFL);
      packet->setid(0);
      packet->setstring(std::string("Improper length"));
      socket_.send(packet);

      log_->putmsg("ServiceFrame: Did receive an improper startcommand request (communication protocol recognised,but message does not contain a password)");
      return(false);
    }

    startcommand_ = packet->getstring();
    timeout_ = packet->getid();
    
    packet->settag(TAG_STSS);
    packet->setid(0);
    packet->setstring(si->version);
    if(!(socket_.send(packet)))
    {
      log_->putmsg("ServiceFrame: could not send reply to client");
      log_->putmsg(std::string("ServiceFrame: socket error = ") + socket_.geterror());
      return(false);
    }

    log_->putmsg("ServiceFrame: service ready to go");
    return(true);
  }


  bool ServiceFrame::runservice()
  {
    ServiceInfo* info;
    info = db_->getserviceinfo(servicename_);
	
    ServiceContext ctx;
    ctx.servicename = servicename_;
    ctx.session = session_;
    ctx.packagename = info->packagename;
    ctx.parameters = info->parameters;
    ctx.socket = socket_;
    ctx.startcommand = startcommand_;
    ctx.timeout = timeout_;
    ctx.log = log_;
	
    // Run the external code that makes up the service
    Service* serv = info->maker(ctx);
    if (!serv) return(false);
	
    try
      {
        std::string status = "ServiceFrame: launching service " + servicename_;
        log_->putmsg(status);
        serv->run();
      }
    catch (std::exception &e)
      {
        std::string str = "ServiceFrame: Service '" + servicename_ + "' generated an exception";
        std::cerr << str << std::endl;
        log_->putmsg(str);
        str = "ServiceFrame: std-exception = " + std::string(e.what());
        std::cerr << str << std::endl;
        log_->putmsg(str);
        delete(serv);
        return(false);
      }
    catch(ThreadError &e)
      {
        std::string str = "ServiceFrame: Service '" + servicename_ + "' generated an exception while deleting the service";
        std::cerr << str << std::endl; 
        log_->putmsg(str);
        str = "ServiceFrame: std-exception = " + std::string(e.message());
        std::cerr << str << std::endl;
        log_->putmsg(str);
        delete(serv);
        return(false);
      }
    catch(...)
      {
        std::string str = "ServiceFrame: Service '" + servicename_ + "' generated an exception";
        std::cerr << str << std::endl;
        log_->putmsg(str);
        delete serv;
        throw;
      }

    
    try
      {
        delete serv;
      }
    catch (std::exception &e)
      {
        std::string str = "ServiceFrame: Service '" + servicename_ + "' generated an exception while deleting the service";
        std::cerr << str << std::endl; 
        log_->putmsg(str);
        str = "ServiceFrame: std-exception = " + std::string(e.what());
        std::cerr << str << std::endl;
        log_->putmsg(str);
        return(false);
      }
    catch(ThreadError &e)
      {
        std::string str = "ServiceFrame: Service '" + servicename_ + "' generated an exception while deleting the service";
        std::cerr << str << std::endl; 
        log_->putmsg(str);
        str = "ServiceFrame: std-exception = " + std::string(e.message());
        std::cerr << str << std::endl;
        log_->putmsg(str);
        return(false);
      }
    catch(SystemCallError &e)
      {
        std::string str = "ServiceFrame: Service '" + servicename_ + "' generated an exception while deleting the service";
        std::cerr << str << std::endl; 
        log_->putmsg(str);
        str = "ServiceFrame: std-exception = " + std::string(e.geterror());
        std::cerr << str << std::endl;
        log_->putmsg(str);
        return(false);
      }    
    catch(...)
      {
        std::string str = "ServiceFrame: Service '" + servicename_ + "' generated an exception while deleting the service";
        std::cerr << str << std::endl;
        log_->putmsg(str);
        throw;
        return false;
      }
    return false;
  }


  bool ServiceFrame::close()
  {
    // Close the connection
    socket_.close();
    return(true);
  }

} // namespace

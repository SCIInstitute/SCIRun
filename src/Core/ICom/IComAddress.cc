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
 * FILE: IComAddress.cc
 * AUTH: Jeroen Stinstra
 */

#include <Core/ICom/IComAddress.h>
#include <Core/Thread/Mutex.h>
#include <boost/lexical_cast.hpp>

#define JGS_SCIRUNS_DEFAULT_PORT "9554"
#define JGS_SCIRUN_DEFAULT_PORT "9553"

// Functions for addressing

namespace SCIRun {

// Create a global mutex for locking access to DNS, not thread-safe
// Each call into the kernel will be locked until the data has been
// processed.

SCIRun::Core::Thread::Mutex DNSLock("dnslock");

IComAddress::IComAddress() :
isvalid_(false), isinternal_(false)
{
}

IComAddress::IComAddress(std::string address) :
isvalid_(false), isinternal_(false)
{
    setaddress(address);
}

IComAddress::IComAddress(std::string protocol, std::string name, std::string servname, std::string iptype) :
isvalid_(false), isinternal_(false)
{
    setaddress(protocol,name,servname,iptype);
}

IComAddress::IComAddress(std::string protocol, sockaddr* sa) :
isvalid_(false), isinternal_(false)
{
    setaddress(protocol,sa);
}

IComAddress::IComAddress(std::string protocol, std::string name) :
isvalid_(false), isinternal_(false)
{
    setaddress(protocol,name);
}

IComAddress::~IComAddress()
{
}

bool IComAddress::isvalid()
{
    return(isvalid_);
}

unsigned short IComAddress::getport(int addressnum)
{
    if ((isvalid_)&&(!isinternal_)) return(portnum_[addressnum]);
    return(0);
}

IPaddress IComAddress::getipaddress(int addressnum)
{
    if ((isvalid_)&&(!isinternal_))
    {
        return(ipaddress_[addressnum]);
    }
    else
    {
        return(IPaddress(0));
    }
}

std::string IComAddress::getinternalname()
{
    if (isinternal_) return(inetname_[0]);
    return(std::string(""));
}

void IComAddress::clear()
{
    isinternal_ = false;
    isvalid_ = false;
    portnum_.clear();    // vector for storing portnumbers
    inetname_.clear();    // vector for storing the inetnet "DNS" names
    ipname_.clear();    // vector for storing the ipaddress in text
    ipaddress_.clear(); // vector for storing the ipaddress bytes
    sin_.clear();        // vector for storing the ipv4 sockaddresses
    sin6_.clear();        // vector for storing the ipv6 sockaddresses
}


bool IComAddress::operator==(const IComAddress& a) const
{
    if (!a.isvalid_) return(false);
    if (!isvalid_) return(false);
    if ((a.isinternal_)&&(isinternal_))
    {
        if (a.inetname_[0] == inetname_[0]) return(true);
        return(false);
    }
    if ((!a.isinternal_)&&(!isinternal_))
    {
        for (size_t r=0; r < portnum_.size(); r++)
        {
            for (size_t q=0; q< a.portnum_.size(); q++)
            {
                {
                    if (ipaddress_[r].size() == a.ipaddress_[q].size())
                    if ((portnum_[r] == a.portnum_[q])||(servname_[r] == "any")||(a.servname_[q] == "any"))
                    {
                        bool test = true;
                        for (size_t p=0; p < ipaddress_[r].size(); p++) if(a.ipaddress_[q][p] != ipaddress_[r][p]) test = false;
                        if (test) return(true);
                    }
                }
            }
        }
    }
    return(false);
}

int IComAddress::getnumaddresses()
{
    return(inetname_.size());
}

bool IComAddress::isipv4(int addressnum)
{
    if ((isvalid_)&&(!isinternal_)) if (addressnum < static_cast<int>(ipaddress_.size())) if(ipaddress_[addressnum].size() == 4) return(true);
    return(false);
}

bool IComAddress::isipv6(int addressnum)
{
    if ((isvalid_)&&(!isinternal_)) if (addressnum < static_cast<int>(ipaddress_.size())) if(ipaddress_[addressnum].size() == 16) return(true);
    return(false);
}

bool IComAddress::isinternal()
{
    if ((isinternal_)&&(isvalid_)) return(true);
    return(false);
}

bool IComAddress::setaddress(std::string protocol,std::string name,std::string servname,std::string iptype)
{
    clear();

    // The most complicated function in this class:

    // in case the user wants an internal address, use
    // the function that was made for internal addresses
    if (protocol == "internal") return(setaddress(protocol,name));

    // if the user did not give us any portnumbers to work with
    // get the default ones.

    // Here we should check the environment variables as well
    // Code for that should go here VVVVVVVVVVVVVV

    // Otherwisse default to build in values
    if ((servname == "")&&(protocol=="scirun")) servname = JGS_SCIRUN_DEFAULT_PORT;
    if ((servname == "")&&(protocol=="sciruns")) servname = JGS_SCIRUNS_DEFAULT_PORT;

    // If we still do not have a portnumber, just fail
    if (servname == "") return(false);

    // We need one of both to do a proper search
    if ((servname == "any")&&(name=="")) return(false);

    // We know the protocol ...
    protocol_ = protocol;
    isinternal_ = false;
    isvalid_ = false;

    // Assume we do not specify what kind of address we want
#ifdef _WIN32
    int            socktype = AF_INET;
#else
    sa_family_t    socktype = AF_INET;
#endif
    // in case of a server you want to be able to specify whether you have a
    // ipv6 or ipv4 server. The following settings overrule the default setting
    if (iptype == "ipv4") socktype = AF_INET;
    if (iptype == "ipv6") socktype = AF_INET6;

    addrinfo *results, *res;
    addrinfo hints;

    // Lock all DNS functions
    DNSLock.lock();


    memset(&hints,0,sizeof(addrinfo));
    // Give some hints on what we are looking for
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = socktype;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_canonname = 0;
    hints.ai_addr = 0;
    hints.ai_next = 0;

    try
    {
        results = 0;    // Make sure we do not free empty space

        if (name == "")
        {    // no name supplied, so we only want an any address template
            hints.ai_flags |= AI_PASSIVE;
            #ifndef HAVE_BAD_GETADDRINFO
                if ( ::getaddrinfo(0,servname.c_str(),&hints,&(results))) throw could_not_resolve_address();
            #else
                // On linux this function fails in some distributions....
                // Hopefully newer  releases will have better implementations
                if ( ga_getaddrinfo(0,servname.c_str(),&hints,&(results))) throw could_not_resolve_address();
            #endif
        }
        else
        {
            if (servname == "any")
            {
            #ifndef HAVE_BAD_GETADDRINFO
                if ( ::getaddrinfo(name.c_str(),0,&hints,&(results))) throw could_not_resolve_address();
            #else
                if ( ga_getaddrinfo(name.c_str(),0,&hints,&(results))) throw could_not_resolve_address();
            #endif
            }
            else
            {
            #ifndef HAVE_BAD_GETADDRINFO
                if ( ::getaddrinfo(name.c_str(),servname.c_str(),&hints,&(results))) throw could_not_resolve_address();
            #else
                if ( ga_getaddrinfo(name.c_str(),servname.c_str(),&hints,&(results))) throw could_not_resolve_address();
            #endif
            }
        }

        if (results)
        {
            // Sort address in the following order: first ipv4 then ipv6
            res = results;

            // count the number of addresses that meet our specifications
            int numaddresses = 1;
            while (res->ai_next != 0) { numaddresses++; res = res->ai_next; }


            // we split the number of addresses into one ipv4 and one ipv6 cluster
            int numipv4 = 0;
            int numipv6 = 0;
            res = results;
            for (int p=0;p<numaddresses;p++)
            {
                if (res->ai_family == AF_INET) numipv4++;
                if (res->ai_family == AF_INET6) numipv6++;
                res = res->ai_next;
            }

            // total number of addresses spans both the ipv4 and ipv6 space
            numaddresses = numipv4+numipv6;

            // Set the canonical name of an address
            inetname_.resize(numaddresses);
            for (int p=0;p<numaddresses; p++) inetname_[p] = name;    // set the default name
            // If our search resulted in an unique DNS name, put this name in the inetname field for each address
            if (results->ai_canonname) for (int p=0;p<numaddresses; p++) inetname_[p] = std::string(results->ai_canonname);

            // Set the portname, this one is unique as well as we did supply it ourselves
            servname_.resize(numaddresses);
            for (int p=0;p<numaddresses; p++) servname_[p] = servname;

            // Set all other information retrieved using getaddrinfo()

            // Socketaddresses are split over sin_ and sin6_,
            // the first ones correspond to ipv4 addresses and the later ones to the ipv6 addresses
            sin_.resize(numipv4);
            sin6_.resize(numipv6);
            ipaddress_.resize(numaddresses);
            ipname_.resize(numaddresses);
            portnum_.resize(numaddresses);

            int sincnt = 0;    // count howmany ipv4 addresses we encountered sofar
            int sin6cnt = 0; // idem for ipv6
            res = results;
            for (size_t p=0;p<static_cast<size_t>(numaddresses);p++)
            {
                if (res->ai_family == AF_INET)
                {    // IPv4
                    sockaddr_in *saddr = reinterpret_cast<sockaddr_in *>(res->ai_addr);
                    // get sockaddr
                    sin_[sincnt] = *saddr;
                    char    *addr = reinterpret_cast<char *>(&(saddr->sin_addr));
                    ipaddress_[sincnt].resize(4);
                    // Copy the actual ipaddress
                    for (int q=0;q<4;q++) ipaddress_[sincnt][q] = addr[q];
                    portnum_[sincnt] = ntohs(saddr->sin_port);
                    // Copy the address as a text string
                    char    str[46];
#ifdef _WIN32
                    // inet_ntop doesn't exist on win32, is this what we want to do instead?
                    char* strp = str;
                    strp = ::inet_ntoa(saddr->sin_addr);
                    if (!strp) throw could_not_resolve_address();
#else
                    if (!(::inet_ntop(AF_INET, &(ipaddress_[sincnt][0]), str, sizeof(str)))) throw could_not_resolve_address();
#endif
                    ipname_[sincnt] = std::string(str);
                    sincnt++;

                }
                if (res->ai_family == AF_INET6)
                {    // IPv6
                    sockaddr_in6 *saddr = reinterpret_cast<sockaddr_in6 *>(res->ai_addr);
                    sin6_[sin6cnt] = *saddr;
                    char    *addr = reinterpret_cast<char *>(&(saddr->sin6_addr));
                    ipaddress_[sin6cnt+numipv4].resize(16);
                    for (int q=0;q<16;q++) ipaddress_[sin6cnt+numipv4][q] = addr[q];
                    portnum_[sin6cnt+numipv4] = ntohs(saddr->sin6_port);
                    char    str[46];
#ifdef _WIN32
                    // inet_ntop doesn't exist on win32, is this what we want to do instead?
                    // win32 inet_ntoa does not support ipv6
                    //str = ::inet_ntoa(saddr->sin_addr);
                    //if (str[0] == 0)
                      throw could_not_resolve_address();
#else
                    if (!(::inet_ntop(AF_INET6, &(ipaddress_[sin6cnt+numipv4][0]), str, sizeof(str)))) throw could_not_resolve_address();
#endif
                    ipname_[sin6cnt+numipv4] = std::string(str);
                    sin6cnt++;

                }

                res = res->ai_next;
            }


            // In case we did not find any suitable addresses
            if (numaddresses == 0) throw could_not_resolve_address();

            // free the results as it was memory that was allocated for us
            #ifndef HAVE_BAD_GETADDRINFO
                if (results) ::freeaddrinfo(results); results = 0;
            #else
                if (results) ga_freeaddrinfo(results); results = 0;
            #endif
        }
        DNSLock.unlock();    // unlock the DNS
        isvalid_ = true;
        return(true);

    }
    catch (...)
    {    // Make sure we do unlock the DNS and deallocate the memory used
        DNSLock.unlock();
        #ifndef HAVE_BAD_GETADDRINFO
            if (results) ::freeaddrinfo(results);
        #else
            if (results) ga_freeaddrinfo(results);
        #endif
        clear();
        return(false);
    }
}


bool IComAddress::setaddress(std::string protocol, std::string name)
{
    clear();

    // in case the user does not want to have an internal address
    if (protocol != "internal")
    {
        return(setaddress(protocol,name,""));
    }

    // internal addresses are easy:
    // a string is used as an unique identifier

    protocol_ = protocol;
    inetname_.resize(1);
    inetname_[0] = name;
    isinternal_ = true;
    isvalid_ = true;
    return(true);
}


bool IComAddress::setprotocol(std::string protocol)
{
    if (!isvalid_) return(false);
    if (isinternal_) return(false);    // you cannot change the protocol of an internal address
    protocol_ = protocol;
    return(true);
}

bool IComAddress::setaddress(std::string str)
{
    std::string protocol;
    std::string name;
    std::string portnum;

    clear();

    protocol = "scirun";
    size_t protocolsep = str.find("://");
    if (protocolsep < str.size())
    {
        protocol = str.substr(0,protocolsep);
        str = str.substr(protocolsep+3);
    }

    size_t firstcolon = str.find(':');
    size_t firstdot = str.find('.');

    if (firstcolon >= str.size())
    {    // IPV4 with no port assignment or IPV6 using DNS
        name = str;
        portnum = "";
    }

    if (firstdot >= str.size())
    {    // IPV6 without port or internal name
        // internal name has no colons
        if (firstcolon < str.size())
        {    // a ':' but no '.' => must be an internalname with a port or
            // an ipv6 without port
            // an ipv6 needs to have second ':'
            std::string str2 = str.substr(firstcolon+1);
            size_t secondcolon = str2.find(':');
            if (secondcolon < str2.size())
            {
                size_t numcolon = 0;
                std::string str3 = str2;
                size_t poscolon = 0;
                while((poscolon = str3.find(':')) < str3.size()) {numcolon++; str3 = str3.substr(poscolon+1);}
                if (numcolon > 5)
                {
                    size_t lastcolon= str.rfind(':');
                    name = str.substr(0,lastcolon);
                    portnum = str.substr(lastcolon+1);
                }
                else
                {
                    name = str;
                    portnum = "";
                }
            }
            else
            {
                name = str.substr(0,firstcolon);
                portnum = str.substr(firstcolon+1);
                if (protocol == "") protocol = "internal";
                if (protocol != "internal") return(false);
            }
        }
        else
        { // no colon, no dot => internal name
            name = str;
            portnum = "";
            if (protocol == "") protocol = "internal";
            if (protocol != "internal") return(false);
        }
    }
    if (firstcolon > firstdot)
    {    // must be an IPv4 address
        name = str.substr(0,firstcolon);
        portnum = str.substr(firstcolon+1);
    }

    if (firstdot < firstcolon)
    {    // must be an IPv4 address
        name = str.substr(0,firstdot);
        portnum = str.substr(firstdot+1);
    }

    // if there are any slashes before the namestring, remove them
    if (name[0] == '/') name = name.substr(1);
    if (name[0] == '/') name = name.substr(1);

    if (protocol == "internal")
    {    // for an internal address we do not need the portnumber
        return(setaddress(protocol,name));
    }
    else
    {    // this is a full internet address
        return(setaddress(protocol,name,portnum));
    }
}

bool IComAddress::setaddress(std::string protocol, sockaddr *sa)
{
    clear();

    // supply ineternet address as a socket address, this function is mainly intended for
    // the socket implementation to be able to translate a specific socket address back into
    // a string that is readable to the user

    if (protocol == "internal") return(false);

    if (sa->sa_family == AF_INET)
    {    // for ipv4 addresses

         // We only expect to have one unique address for this one
        inetname_.resize(1);
        servname_.resize(1);
        ipname_.resize(1);
        portnum_.resize(1);
        ipaddress_.resize(1);
        sin_.resize(1);

        // Unix design requires us to use a reinterpret_cast here
        sockaddr_in *saddr = reinterpret_cast<sockaddr_in *>(sa);

        sin_[0] = *saddr;
        char    *addr = reinterpret_cast<char *>(&(sin_[0].sin_addr));    // this is in network order
        ipaddress_[0].resize(4);
        for (int p=0;p<4;p++) ipaddress_[0][p] = addr[p]; // copy the ipaddress byte by byte

        portnum_[0] = ntohs(saddr->sin_port);    // translate the portnumber from network order into local byte order

        DNSLock.lock();    // getnameinfo is not thread safe hence use a mutex to gain exclusive access to the function
        // This function crashes on FreeBSD, hence do not use it
        // ::getnameinfo(reinterpret_cast<sockaddr*>(&(sin_[0])),sizeof(sin_[0]),host,NI_MAXHOST,0,0,0);
        hostent *hst = ::gethostbyaddr(addr,4,AF_INET);
        if (hst) inetname_[0] = hst->h_name;
#ifndef _WIN32
        // win32 doesn't have this - fix later
        ::endhostent();
#endif
        DNSLock.unlock();

        // Retrieve the ipname as text
        char    str[46];    // maximum for both ipv4 and ipv6
        // The next function needs better error management
#ifdef _WIN32
        // inet_ntop doesn't exist on win32, is this what we want to do instead?
        char* strp = str;
        strp = ::inet_ntoa(saddr->sin_addr);
        if (!strp) throw could_not_resolve_address();
#else
        if (!(::inet_ntop(AF_INET, &(ipaddress_[0][0]), str, 46))) str[0] = '\0';
#endif
        ipname_[0] = std::string(str);

        // Set the DNS names
        std::ostringstream oss;
        oss << portnum_[0];
        servname_[0] = oss.str();

        // we are done, so declare the address as valid
        isvalid_ = true;
        isinternal_ = false;
        protocol_ = protocol;
        return(true);
    }

    if (sa->sa_family == AF_INET6)
    {    // The same as before but now for ipv6
        inetname_.resize(1);
        servname_.resize(1);
        ipname_.resize(1);
        portnum_.resize(1);
        ipaddress_.resize(1);
        sin6_.resize(1);

        sockaddr_in6 *saddr = reinterpret_cast<sockaddr_in6 *>(sa);
        sin6_[0] = *saddr;

        char    *addr = reinterpret_cast<char *>(&(saddr->sin6_addr));
        ipaddress_[0].resize(16);
        for (int p=0;p<16;p++) ipaddress_[0][p] = addr[p];

        portnum_[0] = ntohs(saddr->sin6_port);

        DNSLock.lock();
        //::getnameinfo(reinterpret_cast<sockaddr *>(&(sin6_[0])),sizeof(sockaddr_in6),host,NI_MAXHOST,0,0,0);
        hostent *hst = ::gethostbyaddr(addr,16,AF_INET);
        if (hst) inetname_[0] = hst->h_name;
        DNSLock.unlock();

        char    str[46];
#ifdef _WIN32
        // inet_ntop doesn't exist on win32, is this what we want to do instead?
        // win32 inet_ntoa does not support ipv6
        //str = ::inet_ntoa(saddr->sin_addr);
        //if (str[0] == 0)
        throw could_not_resolve_address();
#else
        if (!(::inet_ntop(AF_INET6, &(ipaddress_[0][0]), str, 46))) str[0] = '\0';
#endif
        ipname_[0] = std::string(str);

        std::ostringstream oss;
        oss << portnum_[0];
        servname_[0] = oss.str();

        isvalid_ = true;
        isinternal_ = false;
        protocol_ = protocol;
        return(true);
    }

    return(false);
}


sockaddr* IComAddress::getsockaddr(int numaddress)
{
    if (!isvalid_) return(0);
    // ordering of socket addresses: first the ipv4 and then the ipv6
    // addresses
    // use getsockaddrlen() to get the appropriate length of the socket
    if (numaddress+1 > static_cast<int>(sin_.size()))
    {    // ipv6
        numaddress -= sin_.size();
        if (numaddress < static_cast<int>(sin6_.size()))
        {
            return(reinterpret_cast<sockaddr *>(&(sin6_[numaddress])));
        }
        else
        { // hence it must be ipv4
            return(0);
        }
    }
    else
    {
        return(reinterpret_cast<sockaddr *>(&(sin_[numaddress])));
    }
}

socklen_t    IComAddress::getsockaddrlen(int numaddress)
{
    if (!isvalid_) return(0);
    // ordering of socket addresses: first the ipv4 and then the ipv6
    // addresses
    if (numaddress+1 > static_cast<int>(sin_.size()))
    {    // it has to be an ipv6 address
        if (numaddress < static_cast<int>(sin6_.size())) return(sizeof(sockaddr_in6));
        return(0);
    }
    else
    {    // otherwise it has to be ipv4
        return(sizeof(sockaddr_in));
    }
}

std::string IComAddress::getinetname(int numaddress)
{
    if ((!isvalid_)||(isinternal_)) return(std::string(""));
    if (static_cast<int>(inetname_.size()) > numaddress)     return(inetname_[numaddress]);
    return(std::string(""));
}

std::string IComAddress::getipname(int numaddress)
{
    if ((!isvalid_)||(isinternal_)) return(std::string(""));
    if (static_cast<int>(ipname_.size()) > numaddress)     return(ipname_[numaddress]);
    return(std::string(""));
}

std::string IComAddress::getservname(int numaddress)
{
    if ((!isvalid_)||(isinternal_)) return(std::string(""));
    if (static_cast<int>(servname_.size()) > numaddress)     return(servname_[numaddress]);
    return(std::string(""));
}


std::string    IComAddress::geturl(int addressnum)
{
    std::string str("");    // initiate return string
    if (!isvalid_) return(str);    // invalid .....
    if (addressnum > static_cast<int>(inetname_.size())) return(str); // out of range

    if (isinternal_)
    {    // an internal address is denoted as following
        // internal://my_internal_server_name:0
        // A portname is generated by set to zero as we do not use it
        str = "internal://" + inetname_[addressnum] + ":0";
    }
    if (!isinternal_)
    {
        // an ip address is structured according to the normal rules of IPaddresses
        // for the scirun protocol:
        // scirun://something.sci.utah.edu:9553
        //
        // As protocol does not automatically imply portnum, it is recommended to supply
        // a portnumber, hence it is generated in the output
        std::ostringstream oss;
        oss << portnum_[addressnum];
        str = protocol_ + "://" + inetname_[addressnum] + ":" + oss.str();
    }
    return(str);
}


std::string IComAddress::getprotocol()
{
    if (isvalid_) return(protocol_);    // invalid addresses do not have a protocol
    return(std::string(""));
}

bool IComAddress::selectaddress(int addressnum)
{
    if (!isvalid_) return(false);    // In case it is not valid do nothing
    if (addressnum >= static_cast<int>(inetname_.size())) return(false);    // See if number is too big
    if (isinternal_) return(true);    // internal address is always unique and hence will only have one entry

    // Get the data out of the structure, so we can resize the fields without losing
    // information.
    std::string inetname = inetname_[addressnum];
    std::string ipname = ipname_[addressnum];
    std::string servname = servname_[addressnum];
    IPaddress    ipaddress = ipaddress_[addressnum];
    unsigned short portnum    = portnum_[addressnum];

    // resize the fields and put the selected field at the first place
    inetname_.resize(1);
    inetname_[0] = inetname;
    ipname_.resize(1);
    ipname_[0] = ipname;
    servname_[0].resize(1);
    servname_[0] = servname;

    ipaddress_.resize(1);
    ipaddress_[0] = ipaddress;
    portnum_.resize(1);
    portnum_[0] = portnum;

    // The socket address needs to be treated seperately as we sorted the data with first
    // the ipv4 addresses and the ipv6 addresses
    if (addressnum < static_cast<int>(sin_.size()))
    {    // in case the selected one is an ipv4 address
        sockaddr_in sin = sin_[addressnum];
        sin_.resize(1);
        sin6_.resize(0);
        sin_[0] = sin;
    }
    else
    {    // in case of an ipv6 address
        sockaddr_in6 sin6 = sin6_[addressnum-sin_.size()];
        sin_.resize(0);
        sin6_.resize(1);
        sin6_[0] = sin6;
    }

    return(true);
}

bool IComAddress::setlocaladdress()
{

    clear();
#ifdef _WIN32
    char hostname[256];
    gethostname(hostname, 256);
    std::string name(hostname);
#else
    struct utsname localname;

    if (::uname(&localname) < 0)
    {
        return(false);
    }

    std::string name(localname.nodename);
#endif
    return(setaddress("scirun",name,"any"));
}

// For debugging purposes
std::string IComAddress::printaddress()
{
    if (!isvalid()) return(std::string(""));

    std::ostringstream oss;
    oss << "Number of addresses:" << getnumaddresses() << "\n\n";

    if (!isinternal())
    {
        for (size_t p=0; p < portnum_.size(); p++)
        {
            oss << "IPaddress[" << p << "] =" ;
            for (size_t q=0; q < ipaddress_[p].size(); q++) oss << static_cast<unsigned short>(ipaddress_[p][q]) << ".";
            oss << "\n";
            oss << "Portnumber[" << p << "] = " << portnum_[p] << ".\n";
            oss << "DNSName[" << p << "] = " << inetname_[p] << ".\n";
            oss << "ServName[" << p << "] = " << servname_[p]  << ".\n";
            oss << "IPName[" << p << "] = " << ipname_[p] << ".\n\n";
        }
    }
    else
    {
        oss << "InternalAddress = " << inetname_[0] << ".\n";
    }

    return(oss.str());
}

IComAddress    *IComAddress::clone() const
{
    IComAddress *address = new IComAddress;

    address->protocol_ = protocol_;
    address->isinternal_ = isinternal_;
    address->isvalid_ = isvalid_;
    address->ipaddress_ = ipaddress_;
    address->portnum_ = portnum_;
    address->inetname_ = inetname_;
    address->ipname_ = ipname_;
    address->servname_ = servname_;
    address->sin_ = sin_;
    address->sin6_ = sin6_;

    return(address);
}



#ifdef HAVE_BAD_GETADDRINFO
    // simulation of the getaddrinfo

int IComAddress::ga_getaddrinfo(const char *hostname, const char *servname,
			const struct addrinfo *hintsp, struct addrinfo **result)
{
	int					rc, error, nsearch;
	char				**ap, *canon;
	struct hostent		*hptr;
	struct ga_search    search[3], *sptr;
	struct addrinfo		hints, *aihead, **aipnext;

	aihead = 0;		/* initialize automatic variables */
	aipnext = &aihead;
	canon = 0;
    error = 0;

	if (hintsp == 0)
    {
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
	}
    else
    {
		hints = *hintsp;		/* struct copy */
    }

	error = ga_echeck(hostname, servname, hints.ai_flags, hints.ai_family,
						 hints.ai_socktype, hints.ai_protocol);
    if (error) goto bad;

	nsearch = ga_nsearch(hostname, &hints, &search[0]);

	for (sptr = &search[0]; sptr < &search[nsearch]; sptr++)
    {

        if (isdigit(sptr->host[0]))
        {
			struct in_addr	inaddr;

#ifndef _WIN32
                        // win32 doesn't have this - fix later
			if (::inet_pton(AF_INET, sptr->host, &inaddr) == 1)
            {
				if (hints.ai_family != AF_UNSPEC && hints.ai_family != AF_INET)
#ifdef _WIN32
					{ error = EAI_FAMILY; goto bad; }
#else
					{ error = EAI_ADDRFAMILY; goto bad; }
#endif
				if (sptr->family != AF_INET)
					continue;		/* ignore */
				error = ga_aistruct(&aipnext, &hints, &inaddr, AF_INET);
				if (error) goto bad;
			}
#endif
		}

		if ((isxdigit(sptr->host[0]) || sptr->host[0] == ':') && (strchr(sptr->host, ':') != 0))
        {

			struct in6_addr	in6addr;
#ifndef _WIN32
			if (::inet_pton(AF_INET6, sptr->host, &in6addr) == 1) {
				if (hints.ai_family != AF_UNSPEC && hints.ai_family != AF_INET6)
#ifdef _WIN32
					{ error = EAI_FAMILY; goto bad; }
#else
					{ error = EAI_ADDRFAMILY; goto bad; }
#endif
				if (sptr->family != AF_INET6)
					continue;		/* ignore */
				error = ga_aistruct(&aipnext, &hints, &in6addr, AF_INET6);
				if (error) goto bad;
				continue;
			}
#endif
		}

        hptr = ::gethostbyname(sptr->host);

        // hptr = ::gethostbyname2(sptr->host, sptr->family);
		if (hptr == NULL) {
			if (nsearch == 2)
				continue;	/* failure OK if multiple searches */
            error = EAI_NONAME;
            goto bad;
		}

			/* 4check for address family mismatch if one specified */
		if (hints.ai_family != AF_UNSPEC && hints.ai_family != hptr->h_addrtype)
		{
#ifdef _WIN32
	    error = EAI_FAMILY; goto bad;
#else
            error = EAI_ADDRFAMILY; goto bad;
#endif
        }

			/* 4save canonical name first time */
		if (hostname != NULL && hostname[0] != '\0' &&
			(hints.ai_flags & AI_CANONNAME) && canon == NULL)
        {
			if ( (canon = strdup(hptr->h_name)) == NULL)
			{
            	error = EAI_MEMORY; goto bad;
            }
		}

		for (ap = hptr->h_addr_list; *ap != NULL; ap++)
        {
			error = ga_aistruct(&aipnext, &hints, *ap, hptr->h_addrtype);
			if (error) goto bad;
		}
	}
	if (aihead == NULL)
    {
		error = EAI_NONAME; goto bad;
    }

	if (hostname != NULL && hostname[0] != '\0' &&
		hints.ai_flags & AI_CANONNAME)
    {
		if (canon != NULL)
        {
			aihead->ai_canonname = canon;	/* strdup'ed earlier */
        }
		else
        {
			if ( (aihead->ai_canonname = ::strdup(search[0].host)) == NULL)
            {
				error = EAI_MEMORY; goto bad;
            }
		}
	}

	if (servname != NULL && servname[0] != '\0') {
		if ( (rc = ga_serv(aihead, &hints, servname)) != 0)
        {
			error = rc; goto bad;
        }
	}

	*result = aihead;	/* pointer to first structure in linked list */
	return(0);

bad:
	ga_freeaddrinfo(aihead);	/* free any alloc'ed memory */
	return(error);
}


int IComAddress::ga_echeck(const char *hostname, const char *servname,int flags, int family, int socktype, int protocol)
{
	if (flags & ~(AI_PASSIVE | AI_CANONNAME))
		return(EAI_BADFLAGS);	/* unknown flag bits */

	if (hostname == NULL || hostname[0] == '\0')
    {
		if (servname == NULL || servname[0] == '\0')
			return(EAI_NONAME);	/* host or service must be specified */
	}

	switch(family)
    {
		case AF_UNSPEC:
			break;
		case AF_INET:
			if (socktype != 0 &&
				(socktype != SOCK_STREAM &&
				 socktype != SOCK_DGRAM &&
				 socktype != SOCK_RAW))
				return(EAI_SOCKTYPE);	/* invalid socket type */
			break;
		case AF_INET6:
			if (socktype != 0 &&
				(socktype != SOCK_STREAM &&
				 socktype != SOCK_DGRAM &&
				 socktype != SOCK_RAW))
				return(EAI_SOCKTYPE);	/* invalid socket type */
			break;
		default:
			return(EAI_FAMILY);		/* unknown protocol family */
	}
	return(0);
}


int IComAddress::ga_nsearch(const char *hostname, const struct addrinfo *hintsp,struct ga_search *search)
{
	int		nsearch = 0;

	if (hostname == NULL || hostname[0] == '\0')
    {
		if (hintsp->ai_flags & AI_PASSIVE)
        {
				/* 4no hostname and AI_PASSIVE: implies wildcard bind */
			switch (hintsp->ai_family)
            {
			case AF_INET:
				search[nsearch].host = "0.0.0.0";
				search[nsearch].family = AF_INET;
				nsearch++;
				break;
			case AF_INET6:
				search[nsearch].host = "0::0";
				search[nsearch].family = AF_INET6;
				nsearch++;
				break;
			case AF_UNSPEC:
				search[nsearch].host = "0::0";	/* IPv6 first, then IPv4 */
				search[nsearch].family = AF_INET6;
				nsearch++;
				search[nsearch].host = "0.0.0.0";
				search[nsearch].family = AF_INET;
				nsearch++;
				break;
			}
		}
        else
        {
				/* 4no host and not AI_PASSIVE: connect to local host */
			switch (hintsp->ai_family)
            {
			case AF_INET:
				search[nsearch].host = "localhost";	/* 127.0.0.1 */
				search[nsearch].family = AF_INET;
				nsearch++;
				break;
			case AF_INET6:
				search[nsearch].host = "0::1";
				search[nsearch].family = AF_INET6;
				nsearch++;
				break;
			case AF_UNSPEC:
				search[nsearch].host = "0::1";	/* IPv6 first, then IPv4 */
				search[nsearch].family = AF_INET6;
				nsearch++;
				search[nsearch].host = "localhost";
				search[nsearch].family = AF_INET;
				nsearch++;
				break;
			}
		}
	}
    else
    {	/* host is specified */
		switch (hintsp->ai_family)
        {
		case AF_INET:
			search[nsearch].host = hostname;
			search[nsearch].family = AF_INET;
			nsearch++;
			break;
		case AF_INET6:
			search[nsearch].host = hostname;
			search[nsearch].family = AF_INET6;
			nsearch++;
			break;
		case AF_UNSPEC:
			search[nsearch].host = hostname;
			search[nsearch].family = AF_INET6;	/* IPv6 first */
			nsearch++;
			search[nsearch].host = hostname;
			search[nsearch].family = AF_INET;	/* then IPv4 */
			nsearch++;
			break;
		}
	}
	return(nsearch);
}

void IComAddress::ga_freeaddrinfo(struct addrinfo *aihead)
{
	struct addrinfo	*ai, *ainext;

	for (ai = aihead; ai != NULL; ai = ainext) {
		if (ai->ai_addr != NULL)
			free(ai->ai_addr);		/* socket address structure */

		if (ai->ai_canonname != NULL)
			free(ai->ai_canonname);

		ainext = ai->ai_next;	/* can't fetch ai_next after free() */
		free(ai);				/* the addrinfo{} itself */
	}
}

int IComAddress::ga_aistruct(struct addrinfo ***paipnext, const struct addrinfo *hintsp,const void *addr, int family)
{
	struct addrinfo	*ai;

	if ( (ai = reinterpret_cast<struct addrinfo *>(calloc(1, sizeof(struct addrinfo)))) == 0)
		return(EAI_MEMORY);

	ai->ai_next = 0;
	ai->ai_canonname = 0;
	**paipnext = ai;
	*paipnext = &ai->ai_next;

	if ( (ai->ai_socktype = hintsp->ai_socktype) == 0) ai->ai_flags |= 4;

	ai->ai_protocol = hintsp->ai_protocol;

	switch ((ai->ai_family = family))
    {
		case AF_INET:
        {
			struct sockaddr_in	*sinptr;
			if ( (sinptr = reinterpret_cast<struct sockaddr_in *>(calloc(1, sizeof(struct sockaddr_in)))) == 0) return(EAI_MEMORY);
			sinptr->sin_family = AF_INET;
            //    sinptr->sin_len = sizeof(struct sockaddr_in);
			memcpy(&sinptr->sin_addr, addr, sizeof(struct in_addr));
			ai->ai_addr = (struct sockaddr *) sinptr;
			ai->ai_addrlen = sizeof(struct sockaddr_in);
			break;
		}
		case AF_INET6:
    {
			struct sockaddr_in6	*sin6ptr;
			if ( (sin6ptr = reinterpret_cast<struct sockaddr_in6 *>(calloc(1, sizeof(struct sockaddr_in6)))) == 0) return(EAI_MEMORY);

			sin6ptr->sin6_family = AF_INET6;
            // sin6ptr->sin6_len = sizeof(struct sockaddr_in6);
			memcpy(&sin6ptr->sin6_addr, addr, sizeof(struct in6_addr));
			ai->ai_addr = (struct sockaddr *) sin6ptr;
			ai->ai_addrlen = sizeof(struct sockaddr_in6);
			break;
		}
	}
	return(0);
}

int IComAddress::ga_serv(struct addrinfo *aihead, const struct addrinfo *hintsp, const char *serv)
{
	int				port, rc, nfound;
	struct servent	*sptr;

	nfound = 0;
	if (isdigit(serv[0]))
  {
    /* check for port number string first */

    std::string serv_str(serv);
    int tport = boost::lexical_cast<int>(serv_str);
    port = htons(tport);
		if (hintsp->ai_socktype)
    {
				/* 4caller specifies socket type */
			if ( (rc = ga_port(aihead, port, hintsp->ai_socktype)) < 0)
				return(EAI_MEMORY);
			nfound += rc;
		}
    else
    {
				/* 4caller does not specify socket type */
			if ( (rc = ga_port(aihead, port, SOCK_STREAM)) < 0)
				return(EAI_MEMORY);
			nfound += rc;
			if ( (rc = ga_port(aihead, port, SOCK_DGRAM)) < 0)
				return(EAI_MEMORY);
			nfound += rc;
		}
	}
    else
    {
			/* 4try service name, TCP then UDP */
		if (hintsp->ai_socktype == 0 || hintsp->ai_socktype == SOCK_STREAM)
        {
			if ( (sptr = ::getservbyname(serv, "tcp")) != NULL)
            {
				if ( (rc = ga_port(aihead, sptr->s_port, SOCK_STREAM)) < 0)
					return(EAI_MEMORY);
				nfound += rc;
			}
		}
		if (hintsp->ai_socktype == 0 || hintsp->ai_socktype == SOCK_DGRAM)
        {
			if ( (sptr = ::getservbyname(serv, "udp")) != NULL)
            {
				if ( (rc = ga_port(aihead, sptr->s_port, SOCK_DGRAM)) < 0)
					return(EAI_MEMORY);
				nfound += rc;
			}
		}
	}

	if (nfound == 0)
    {
		if (hintsp->ai_socktype == 0)
        {
			return(EAI_NONAME);	/* all calls to getservbyname() failed */
        }
		else
        {
			return(EAI_SERVICE);/* service not supported for socket type */
        }
	}
	return(0);
}

int IComAddress::ga_port(struct addrinfo *aihead, int port, int socktype)
{
	int				nfound = 0;
	struct addrinfo	*ai;

	for (ai = aihead; ai != NULL; ai = ai->ai_next)
    {
		if (ai->ai_flags & 4)
        {
			if (ai->ai_socktype != 0)
            {
				if ( (ai = ga_clone(ai)) == NULL)
					return(-1);		/* memory allocation error */
			}
		}
        else if (ai->ai_socktype != socktype)
		{
            	continue;		/* ignore if mismatch on socket type */
        }

		ai->ai_socktype = socktype;

		switch (ai->ai_family)
        {
			case AF_INET:
				((struct sockaddr_in *) ai->ai_addr)->sin_port = port;
				nfound++;
				break;
			case AF_INET6:
				((struct sockaddr_in6 *) ai->ai_addr)->sin6_port = port;
				nfound++;
				break;
		}
	}
	return(nfound);
}

struct addrinfo* IComAddress::ga_clone(struct addrinfo *ai)
{
	struct addrinfo	*nai;

	if ( (nai = reinterpret_cast<struct addrinfo *>(calloc(1, sizeof(struct addrinfo)))) == NULL) return(NULL);

	nai->ai_next = ai->ai_next;
	ai->ai_next = nai;

	nai->ai_flags = 0;
	nai->ai_family = ai->ai_family;
	nai->ai_socktype = ai->ai_socktype;
	nai->ai_protocol = ai->ai_protocol;
	nai->ai_canonname = NULL;
	nai->ai_addrlen = ai->ai_addrlen;
	if ( (nai->ai_addr = reinterpret_cast<struct sockaddr *>(calloc(1,ai->ai_addrlen))) == NULL)
		return(NULL);
	::memcpy(nai->ai_addr, ai->ai_addr, ai->ai_addrlen);

	return(nai);
}

#endif

}

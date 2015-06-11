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
 *  IComBase.h
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */

#ifndef JGS_SCI_CORE_ICOM_ICOMBASE_H
#define JGS_SCI_CORE_ICOM_ICOMBASE_H 1

#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif

#include <Core/ICom/IComFwd.h>
#include <Core/ICom/share.h>

namespace SCIRun {

	// Define all the enums here

	enum conntype			{ DIRECT = 1, SSH_TUNNEL = 2};

class SCISHARE IComBase {

	// Define all the communication errors here
  public:
	class   icomerror {}; // general socket/packet communication error

	class   could_not_open_socket		: public icomerror {};
	class   invalid_port_number			: public icomerror {};
	class   could_not_resolve_address   : public icomerror {};
	class   not_a_string_packet			: public icomerror {};
	class   invalid_data_format			: public icomerror {};
	class   invalid_address				: public icomerror {};

};


} // end namespace

#endif

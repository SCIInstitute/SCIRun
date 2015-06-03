#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
 *  SystemCallError.h
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */



#ifndef CORE_SYSTEMCALL_SYSTEMCALLERROR_H
#define CORE_SYSTEMCALL_SYSTEMCALLERROR_H 1


#include <string>
#include <iostream>



#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdio.h>

#include <Core/SystemCall/share.h>

namespace SCIRun {

class SCISHARE SystemCallBase {

  public:
    SystemCallBase();
    virtual ~SystemCallBase();
    
  public:
  
    enum sce_error {	
						SCE_NOERROR = 0,	
						SCE_IOERROR,		// IOError, look in errno for more details
						SCE_NOSYSMANAGER,	// SystemCallmanager was never started
						SCE_NOFIFO,			// Could not create fifo
						SCE_FIFOOPEN,		// Could not open fifo
						SCE_FIFOFLAG,		// Could not change fifo flags
						SCE_SYSMANAGER,		// The SystemCalManager process generated ane error
						SCE_INUSE,			// The SystemCall is already in use
						SCE_NOCMD,			// No command was specified
						SCE_THREADERROR,	// Could not create thread
						SCE_BADALLOC
					};
	
};


class SCISHARE SystemCallError : public SystemCallBase {

  public:
	SystemCallError(std::string error, int errnr, sce_error = SCE_IOERROR);
	
	std::string geterror();
	int			geterrno();
	int			geterrclass();
	
  private:
	std::string error_;
	int			errnr_;
	int			errclass_;

};

inline	std::string SystemCallError::geterror()
{
	return(error_);
}

inline	int SystemCallError::geterrno()
{
	return(errnr_);
}

inline int SystemCallError::geterrclass()
{
	return(errclass_);	
}


} // end namespace

#endif
#endif
/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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


///
///@file  Exception.h
///@brief Base class for all SCI Exceptions
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  July 1999
///

#ifndef Core_Exceptions_Exception_h
#define Core_Exceptions_Exception_h

#include <string>
#include <Core/Utils/Exception.h>
#include <sci_defs/error_defs.h>
#include <Core/Exceptions/share.h>

namespace SCIRun {
  class SCISHARE Exception : public std::exception
  {
  public:
    Exception();
    virtual ~Exception() NOEXCEPT;
    virtual const char* message() const=0;
    virtual const char* type() const=0;
    const char* stackTrace() const {
      return stacktrace_;
    }

    const char* what() const NOEXCEPT
    {
      return message();
    }

    static void sci_throw(const Exception& exc);
    static bool alwaysFalse();
  protected:
    const char* stacktrace_;
  private:
    Exception& operator=(const Exception&);
  };

  SCISHARE std::string getStackTrace(void* context = 0);
} // End namespace SCIRun


#if USE_SCI_THROW
#define SCI_THROW(exc) do {SCIRun::Exception::sci_throw(exc);throw exc;} while(SCIRun::Exception::alwaysFalse())
#else
#define SCI_THROW(exc) BOOST_THROW_EXCEPTION(exc)
#endif

#endif

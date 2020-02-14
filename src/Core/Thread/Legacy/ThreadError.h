/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
///@file   ThreadError.h
///@brief  Exception class for unusual errors in the thread library
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   August 1999
///

#ifndef Core_Thread_ThreadError_h
#define Core_Thread_ThreadError_h

#include <Core/Exceptions/Exception.h>

#include <string>


namespace SCIRun {
/**************************************

@class
   ThreadError

KEYWORDS
   Exception, Thread

@details
   An exception class for serious thread library errors.  They are
   often not recoverable.

****************************************/
	class ThreadError : public Exception {
	public:
	    //////////
	    /// Constructor for the ThreadError class.  Message is
	    /// a human readable string that explains the reason for
	    /// the error
	    ThreadError(const std::string& message);

	    //////////
	    /// Copy ctor
	    ThreadError(const ThreadError&);

	    //////////
	    /// Destructor
	    virtual ~ThreadError();

	    //////////
	    /// returns the message associated with this error
	    virtual const char* message() const;

	    //////////
	    /// returns the name of this exception (the name of this class)
	    virtual const char* type() const;

	protected:
	private:
	    std::string message_;

	    ThreadError& operator=(const ThreadError&);
	};
} // End namespace SCIRun

#endif

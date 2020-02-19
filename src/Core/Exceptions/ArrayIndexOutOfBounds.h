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
///@file  ArrayIndexOutOfBounds.h
///@brief Exception to indicate a failed bounds check
///
///@author
///    Steven G. Parker
///    Department of Computer Science
///    University of Utah
///@date  July 1999
///

#ifndef Core_Exceptions_ArrayIndexOutOfBounds_h
#define Core_Exceptions_ArrayIndexOutOfBounds_h

#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Exceptions/Exception.h>
#include <Core/Exceptions/share.h>

namespace SCIRun {
	class SCISHARE ArrayIndexOutOfBounds : public Exception {
	public:
	    ArrayIndexOutOfBounds(index_type value, index_type lower, index_type upper,
                                  const char* file, int line);
	    ArrayIndexOutOfBounds(const ArrayIndexOutOfBounds&);
	    virtual ~ArrayIndexOutOfBounds() NOEXCEPT;
	    virtual const char* message() const;
	    virtual const char* type() const;
	protected:
	private:
	    long value, lower, upper;
	    char* msg;

	    ArrayIndexOutOfBounds& operator=(const ArrayIndexOutOfBounds);
	};
} // End namespace SCIRun

#endif

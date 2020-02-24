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
///@file  DimensionMismatch.h
///@brief Exception to indicate a failed bounds check
///
///@author
///       Michael Callahan
///       Department of Computer Science
///       University of Utah
///@date  August 2000
///

#ifndef Core_Exceptions_DimensionMismatch_h
#define Core_Exceptions_DimensionMismatch_h

#include <Core/Exceptions/Exception.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Exceptions/share.h>

namespace SCIRun {
	class SCISHARE DimensionMismatch : public Exception {
	public:
    DimensionMismatch(size_type value, size_type expected, const char* file = 0, int line = -1);
    DimensionMismatch(const DimensionMismatch&);
    virtual ~DimensionMismatch() NOEXCEPT;
    virtual const char* message() const;
    virtual const char* type() const;

	private:
    long value, expected;
    std::string message_;

	    DimensionMismatch& operator=(const DimensionMismatch);
	};
} // End namespace SCIRun

#endif  // Core_Exceptions_DimensionMismatch_h

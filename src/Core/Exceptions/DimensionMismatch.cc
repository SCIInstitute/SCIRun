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

#include <Core/Exceptions/DimensionMismatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>

namespace SCIRun {

DimensionMismatch::DimensionMismatch(size_type value, size_type expected, const char* file, int line)
    :  Exception(), value(value), expected(expected)
{
  std::ostringstream s;
  s << "A DimensionMismatch exception was thrown: got " << value << ", expected " << expected << " in "
    << (file ? file : "<unknown>") << ":" << line << "\n";
  message_ = s.str();

#ifdef EXCEPTIONS_CRASH
    std::cout << message_ << "\n";
#endif
}

DimensionMismatch::DimensionMismatch(const DimensionMismatch& copy)
    : Exception(copy), message_(copy.message_)
{
}

DimensionMismatch::~DimensionMismatch() NOEXCEPT
{
}

const char* DimensionMismatch::message() const
{
    return message_.c_str();
}

const char* DimensionMismatch::type() const
{
    return "DimensionMismatch";
}
} // End namespace SCIRun

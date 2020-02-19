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
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  March 2000
///

#include <Core/Exceptions/ArrayIndexOutOfBounds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>

namespace SCIRun {

ArrayIndexOutOfBounds::
ArrayIndexOutOfBounds(index_type value, index_type lower, index_type upper,
                                             const char* file, int line)
    : value(value), lower(lower), upper(upper)
{
  std::ostringstream s;
  s << "An ArrayIndexOutOfBounds exception was thrown.\n"
    << file << ":" << line << "\n"
    << "Array index " << value << " out of range ["
    << lower << ", " << upper << ")";
  msg = (char*)(s.str().c_str());
#ifdef EXCEPTIONS_CRASH
    std::cout << msg << "\n";
#endif
}

ArrayIndexOutOfBounds::ArrayIndexOutOfBounds(const ArrayIndexOutOfBounds& copy)
  : Exception(copy), value(copy.value), lower(copy.lower), upper(copy.upper),
    msg(strdup(copy.msg))
{
}

ArrayIndexOutOfBounds::~ArrayIndexOutOfBounds() NOEXCEPT
{
}

const char* ArrayIndexOutOfBounds::message() const
{
    return msg;
}

const char* ArrayIndexOutOfBounds::type() const
{
    return "ArrayIndexOutOfBounds";
}

} // End namespace SCIRun

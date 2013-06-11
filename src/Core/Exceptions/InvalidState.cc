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
 *  InvalidState.h: Generic exception for internal errors
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   July 1999
 *
 */

#include <Core/Exceptions/InvalidState.h>
#include <iostream>
#include <sstream>

namespace SCIRun {


InvalidState::InvalidState(const std::string& message, const char* file, int line)
    : Exception(), message_(message)
{
  if (file)
  {
    std::ostringstream s;
    s << "An InvalidState exception was thrown.\n"
      << message << "\n"
      << (file ? file : "<unknown>") << ":" << line << "\n";
    message_ = s.str();
  }

#ifdef EXCEPTIONS_CRASH
  std::cout << message_ << "\n";
#endif
}

InvalidState::InvalidState(const InvalidState& copy)
    : Exception(copy), message_(copy.message_)
{
}

InvalidState::~InvalidState() noexcept(true)
{
}

const char* InvalidState::message() const
{
    return message_.c_str();
}

const char* InvalidState::type() const
{
    return "InvalidState";
}

} // End namespace SCIRun

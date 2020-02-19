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
///       McKay Davis
///       Department of Computer Science
///       University of Utah
///@date  Feb 2005
///

#ifndef Core_Exceptions_GuiException_h
#define Core_Exceptions_GuiException_h

#include <Core/Exceptions/Exception.h>
#include <string>

namespace SCIRun {
class GuiException : public Exception {
public:
  GuiException(const std::string& msg) : msg_(msg) { stacktrace_ = 0; }
  virtual ~GuiException() NOEXCEPT {}
  virtual const char* message() const { return msg_.c_str(); }
  virtual const char* type() const { return "GuiException"; }
protected:
private:
  std::string msg_;
  GuiException& operator=(const GuiException);
};
} // End namespace SCIRun

#endif  // Core_Exceptions_GuiException_h

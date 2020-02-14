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
///@file  IllegalValue.h
///@brief Generic exception for invalid values
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  July 1999
///

#ifndef Core_Exceptions_IllegalValue_h
#define Core_Exceptions_IllegalValue_h

#include <Core/Exceptions/Exception.h>

#include <string>
#include <sstream>


namespace SCIRun {
  template <class T>
  class IllegalValue : public Exception {
  public:
    IllegalValue(const std::string&, const T& value);
    IllegalValue(const IllegalValue&);
    virtual ~IllegalValue() NOEXCEPT;
    virtual const char* message() const;
    virtual const char* type() const;
  protected:
  private:
    std::string message_;
    IllegalValue& operator=(const IllegalValue&);
  };

  template <class T>
  IllegalValue<T>::IllegalValue(const std::string& message, const T& value)
  {
    std::ostringstream msgbuf;
    msgbuf << message << ", value=" << value;
    message_=msgbuf.str();
  }

  template <class T>
  IllegalValue<T>::IllegalValue(const IllegalValue& copy)
    : message_(copy.message_)
  {
  }

  template <class T>
  IllegalValue<T>::~IllegalValue()
  {
  }

  template <class T>
  const char* IllegalValue<T>::message() const
  {
    return message_.c_str();
  }

  template <class T>
  const char* IllegalValue<T>::type() const
  {
    return "IllegalValue";
  }

} // End namespace SCIRun

#endif

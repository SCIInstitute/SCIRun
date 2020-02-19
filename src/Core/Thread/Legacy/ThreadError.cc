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
///@file  ThreadError.cc
///@brief Exception class for unusual errors in the thread library
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///@date  August 1999
///

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Thread/Legacy/ThreadError.h>
namespace SCIRun {


ThreadError::ThreadError(const std::string& message)
  : message_(message)
{
}

ThreadError::ThreadError(const ThreadError& copy)
  : Exception(copy), message_(copy.message_)
{
}

ThreadError::~ThreadError()
{
}

const char*
ThreadError::message() const
{
  return message_.c_str();
}

const char*
ThreadError::type() const
{
  return "ThreadError";
}


} // End namespace SCIRun
#endif

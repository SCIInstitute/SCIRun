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
///@file  ProgressReporter.cc
///
///@author
///       Yarden Livnat
///       Department of Computer Science
///       University of Utah
///@date  Jul 2003
///

#include <Core/Utils/ProgressReporter.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core;

//ProgressReporter::ProgressReporter() :
//  progressCurrent_(/*"ProgressReporter::progress_amount_", */0),
//  progressMax_(100)
//{
//}

ProgressReporter::~ProgressReporter()
{
}


int AtomicCounter::operator()() const
{
  counter_.fetch_add(1);
  return counter_;
}

boost::atomic<int> AtomicCounter::counter_(0);

bool SCIRun::Core::replaceSubstring(std::string& str, const std::string& from, const std::string& to)
{
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

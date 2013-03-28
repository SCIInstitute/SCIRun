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
 *  ProgressReporter.cc
 *
 *  Written by:
 *   Yarden Livnat
 *   Department of Computer Science
 *   University of Utah
 *   Jul 2003
 *
 */

#include <Core/Util/ProgressReporter.h>

namespace SCIRun {

ProgressReporter::ProgressReporter() :
  progress_current_("ProgressReporter::progress_amount_", 0),
  progress_max_(100)
{
}


ProgressReporter::~ProgressReporter()
{
}


void
ProgressReporter::report_start(const std::string&)
{
}

void
ProgressReporter::report_end()
{
}


void
ProgressReporter::error(const std::string& msg)
{
  std::cerr << "Error: " << msg << std::endl;
}


void
ProgressReporter::warning(const std::string& msg)
{
  std::cerr << "Warning: " << msg << std::endl;
}


void
ProgressReporter::remark(const std::string& msg)
{
  std::cerr << "Remark: " << msg << std::endl;
}

void
ProgressReporter::status(const std::string& msg)
{
  std::cerr << msg << std::endl;
}

void
ProgressReporter::add_raw_message(const std::string& msg)
{
  std::cerr << msg;
}


std::ostream &
ProgressReporter::msg_stream()
{
  return std::cerr;
}


void
ProgressReporter::msg_stream_flush()
{
  std::cerr.flush();
}

void
ProgressReporter::update_progress(double)
{
}

void
ProgressReporter::update_progress(int, int)
{
}

} // End namespace SCIRun

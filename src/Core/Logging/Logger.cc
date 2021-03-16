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


/// @todo Documentation Core/Logging/Logger.cc

#include <sstream>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Logging/ScopedTimeRemarker.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Core::Logging;

LegacyLoggerInterface::~LegacyLoggerInterface() {}

SimpleScopedTimer::SimpleScopedTimer() : start_(std::chrono::steady_clock::now())
{
}

double SimpleScopedTimer::elapsedSeconds() const
{
  const auto end = std::chrono::steady_clock::now();
  const std::chrono::duration<double> elapsedSeconds = end - start_;
  return elapsedSeconds.count();
}

ScopedTimeRemarker::ScopedTimeRemarker(LegacyLoggerInterface* log, const std::string& label) : log_(log), label_(label)
{}

ScopedTimeRemarker::~ScopedTimeRemarker()
{
  std::ostringstream perf;
  perf << label_ <<  " took " << timer_.elapsedSeconds() << " seconds." << std::endl;
  log_->status(perf.str());
}

ScopedTimeLogger::ScopedTimeLogger(const std::string& label, bool shouldLog): label_(label), shouldLog_(shouldLog)
{
  if (shouldLog_)
    LOG_DEBUG("{} starting.", label_);
}

ScopedTimeLogger::~ScopedTimeLogger()
{
  auto time = timer_.elapsedSeconds();
  if (shouldLog_)
    LOG_DEBUG("{} took {} seconds.", label_, time);
}

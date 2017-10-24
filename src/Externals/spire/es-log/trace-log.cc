/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <es-log/trace-log.h>

using namespace spire;
using namespace SCIRun::Core::Logging;

Logger2 RendererLog::get()
{
  static bool first = true;
  static Logger2 logger(spdlog::basic_logger_mt("renderer", "renderer.log"));
  logger->set_level(spdlog::level::trace);
  logger->flush_on(spdlog::level::info);

  if (first)
  {
    first = false;
    logger->info("Start of Renderer log.");
  }
  return logger;
}

ScopedFunctionLogger::ScopedFunctionLogger(const char* functionName) : functionName_(functionName)
{
  RendererLog::get()->trace("Entering function: {}", functionName_);
}

ScopedFunctionLogger::~ScopedFunctionLogger()
{
  RendererLog::get()->trace("Leaving function: {}", functionName_);
}

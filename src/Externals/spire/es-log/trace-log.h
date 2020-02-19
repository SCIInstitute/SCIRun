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


#ifndef ES_TRACE_LOG_H
#define ES_TRACE_LOG_H

#include <Core/Logging/LoggerFwd.h>
#include <spire/scishare.h>

namespace spire
{
  //class SCISHARE RendererLog
  //{
  //public:
  //  static SCIRun::Core::Logging::Logger2 get();
  //  static const char* name();
  //private:
  //  static SCIRun::Core::Logging::Logger2 logger_;
  //};
}

#define logRendererError(...) //spire::RendererLog::get()->error(__VA_ARGS__)
#define logRendererWarning(...)// spire::RendererLog::get()->warn(__VA_ARGS__)
#define logRendererInfo(...) //spire::RendererLog::get()->info(__VA_ARGS__)

#ifdef RENDERER_TRACE_ON
  #define SPDLOG_TRACE_ON
  #include <spdlog/spdlog.h>
  #define RENDERER_LOG(...) SPDLOG_TRACE(spire::RendererLog::get(), __VA_ARGS__)
  #define RENDERER_LOG_FUNCTION_SCOPE LOG_FUNCTION_SCOPE(spire::RendererLog);
#else
  #define RENDERER_LOG(...)
  #define RENDERER_LOG_FUNCTION_SCOPE
#endif

#endif

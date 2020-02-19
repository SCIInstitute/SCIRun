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


#ifndef CORE_LOGGING_SCOPEDFUNCTIONLOGGER_H
#define CORE_LOGGING_SCOPEDFUNCTIONLOGGER_H

#include <Core/Logging/Log.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Logging
    {
      template <class LogType>
      struct ScopedFunctionLogger
      {
        explicit ScopedFunctionLogger(const char* functionName) :
          logger_(spdlog::get(LogType::name())),
          functionName_(functionName)
        {
          if (logger_)
            logger_->trace("Entering function: {}", functionName_);
        }
        ~ScopedFunctionLogger()
        {
          if (logger_)
            logger_->trace("Leaving function: {}", functionName_);
        }
      private:
        Logger2 logger_;
        const char* functionName_;
      };

      struct ScopedFunctionLoggerGeneral
      {
        explicit ScopedFunctionLoggerGeneral(const char* functionName) :
          logger_(SCIRun::Core::Logging::GeneralLog::Instance().get()),
          functionName_(functionName)
        {
          if (logger_)
            logger_->debug("Entering function: {}", functionName_);
        }
        ~ScopedFunctionLoggerGeneral()
        {
          if (logger_)
            logger_->debug("Leaving function: {}", functionName_);
        }
      private:
        Logger2 logger_;
        const char* functionName_;
      };
    }
  }
}

#ifndef WIN32
#define LOG_FUNCTION_SCOPE_OLD(LogType) SCIRun::Core::Logging::ScopedFunctionLogger<LogType> sfl ## __LINE__ (LOG_FUNC);
#define LOG_FUNCTION_SCOPE SCIRun::Core::Logging::ScopedFunctionLoggerGeneral sfl ## __LINE__ (LOG_FUNC);
#else
#define LOG_FUNCTION_SCOPE
#endif

#endif

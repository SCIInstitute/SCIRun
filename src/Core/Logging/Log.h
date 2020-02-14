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


/// @todo Documentation Core/Logging/Log.h

#ifndef CORE_LOGGING_LOG_H
#define CORE_LOGGING_LOG_H

#include <string>
#include <Core/Logging/LoggerFwd.h>
#ifndef Q_MOC_RUN
#include <Core/Utils/Singleton.h>
#include <boost/filesystem/path.hpp>

// older Mac compiler does not support thread-local storage
#if defined(__APPLE__) && defined(DISABLE_SPDLOG_TLS)
#define SPDLOG_NO_TLS
#endif
#include <spdlog/spdlog.h>

#endif
#include <Core/Logging/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Logging
    {
      class SCISHARE LogAppenderStrategy
      {
      public:
        virtual ~LogAppenderStrategy() {}
        virtual void log4(const std::string& message) const = 0;
      };

      typedef boost::shared_ptr<LogAppenderStrategy> LogAppenderStrategyPtr;

      SCISHARE bool useLogCheckForWindows7();

      class SCISHARE LogSettings final
      {
        CORE_SINGLETON(LogSettings)
      public:
        LogSettings();
        void setLogDirectory(const boost::filesystem::path& dir);
        boost::filesystem::path logDirectory();
        void setVerbose(bool v);
        bool verbose() const;
      private:
        bool verbose_{false};
        boost::filesystem::path directory_;
      };

      class SCISHARE Log2
      {
      public:
        Log2(const std::string& name, bool useLog);
        Logger2 get();
        void addSink(spdlog::sink_ptr sink);
        void addCustomSink(LogAppenderStrategyPtr appender)
        {
          customSinks_.push_back(appender);
        }
        void setVerbose(bool v);
        bool verbose() const;
      protected:
        void addColorConsoleSink();
        bool useLog_;
      private:
        Logger2 logger_;
        std::string name_;
        bool verbose_{false};
        std::vector<spdlog::sink_ptr> sinks_;
        std::vector<LogAppenderStrategyPtr> customSinks_;
      };

      class SCISHARE ModuleLog final : public Log2
      {
        CORE_SINGLETON(ModuleLog)
      public:
        ModuleLog();
      };

      class SCISHARE GeneralLog final : public Log2
      {
        CORE_SINGLETON(GeneralLog)
      public:
        GeneralLog();
      };
    }
  }

  template <class... T>
  void LOG_DEBUG(const char* fmt, T&&... args)
  {
    auto log = SCIRun::Core::Logging::GeneralLog::Instance().get();
    if (log)
      log->debug(fmt, args...);
  }

  inline void LOG_DEBUG(const std::string& str)
  {
    auto log = SCIRun::Core::Logging::GeneralLog::Instance().get();
    if (log)
      log->debug(str);
  }

  template <class... T>
  void LOG_TRACE(const char* fmt, T&&... args)
  {
    auto log = SCIRun::Core::Logging::GeneralLog::Instance().get();
    if (log)
      log->trace(fmt, args...);
  }

  template <class... T>
  void logInfo(const char* fmt, T&&... args)
  {
    auto log = SCIRun::Core::Logging::GeneralLog::Instance().get();
    if (log)
      log->info(fmt, args...);
  }

  template <class... T>
  void logWarning(const char* fmt, T&&... args)
  {
    auto log = SCIRun::Core::Logging::GeneralLog::Instance().get();
    if (log)
      log->warn(fmt, args...);
  }

  template <class... T>
  void logError(const char* fmt, T&&... args)
  {
    auto log = SCIRun::Core::Logging::GeneralLog::Instance().get();
    if (log)
      log->error(fmt, args...);
  }

  template <class... T>
  void logCritical(const char* fmt, T&&... args)
  {
    auto log = SCIRun::Core::Logging::GeneralLog::Instance().get();
    if (log)
      log->critical(fmt, args...);
  }

  #define DEBUG_LOG_LINE_INFO LOG_DEBUG("Debugging info: file {} line {} function {}", __FILE__, __LINE__, LOG_FUNC);
}

#endif

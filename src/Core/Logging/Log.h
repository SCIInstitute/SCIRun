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
/// @todo Documentation Core/Logging/Log.h

#ifndef CORE_LOGGING_LOG_H
#define CORE_LOGGING_LOG_H

#include <string>
#include <Core/Logging/LoggerFwd.h>
#ifndef Q_MOC_RUN
#include <Core/Utils/Singleton.h>
#include <boost/filesystem/path.hpp>
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
        explicit Log2(const std::string& name);
        Logger2 get();
        void addSink(spdlog::sink_ptr sink)
        {
          sinks_.push_back(sink);
        }
        void addCustomSink(LogAppenderStrategyPtr appender)
        {
          customSinks_.push_back(appender);
        }
        void setVerbose(bool v);
        bool verbose() const;
      protected:
        void addColorConsoleSink();
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

      template <class... T>
      void LOG_DEBUG(const char* fmt, T&&... args)
      {
        SCIRun::Core::Logging::GeneralLog::Instance().get()->debug(fmt, args...);
      }

      inline void LOG_DEBUG(const std::string& str)
      {
        SCIRun::Core::Logging::GeneralLog::Instance().get()->debug(str);
      }
    }
  }
}

#endif

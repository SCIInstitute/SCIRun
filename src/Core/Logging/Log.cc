/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Logging/Log.h>

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

#include <boost/lexical_cast.hpp>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Core::Logging;

namespace SCIRun
{
  namespace Core
  {
    namespace Logging
    {
      class LogImpl
      {
      public:
        LogImpl() : cppLogger_(log4cpp::Category::getRoot())
        {
          // Creates a simple log4cpp logger.
          log4cpp::Appender* p_appender = new log4cpp::OstreamAppender("console", &std::cout);
          log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
          layout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S} [%p] %c: %m%n");
          p_appender->setLayout(layout);

          cppLogger_.setPriority(log4cpp::Priority::DEBUG);
          cppLogger_.addAppender(p_appender);
        }

        void log(LogLevel level, const std::string& msg)
        {
          cppLogger_ << translate(level) << msg;
        }

        void stream(LogLevel level)
        {
          latestCategoryStream_ = cppLogger_ << translate(level); 
        }

        void stream(const std::string& msg)
        {
          latestCategoryStream_ << msg;
        }

        log4cpp::Priority::PriorityLevel translate(LogLevel level)
        {
          // Translate pix logging level to cpp logging level
          log4cpp::Priority::PriorityLevel cpp_level = log4cpp::Priority::NOTSET;
          switch (level)
          {
          case NOTSET: // allow fall through
          case EMERG:  cpp_level = log4cpp::Priority::EMERG;  break;
          case ALERT:  cpp_level = log4cpp::Priority::ALERT;  break;
          case CRIT:   cpp_level = log4cpp::Priority::CRIT;   break;
          case ERROR:  cpp_level = log4cpp::Priority::ERROR;  break;
          case WARN:   cpp_level = log4cpp::Priority::WARN;   break;
          case NOTICE: cpp_level = log4cpp::Priority::NOTICE; break;
          case INFO:   cpp_level = log4cpp::Priority::INFO;   break;
          case DEBUG:  cpp_level = log4cpp::Priority::DEBUG;  break;
          default:         
            THROW_INVALID_ARGUMENT("Unknown log level: " + boost::lexical_cast<std::string>((int)level));
          };
          if (cpp_level == log4cpp::Priority::NOTSET)
          {
            THROW_INVALID_ARGUMENT("Could not set log level.");
          }
          return cpp_level;
        }

      private:
        log4cpp::Category& cppLogger_;
        log4cpp::CategoryStream latestCategoryStream_;
      };
    }
  }
}

Log::Log() : impl_(new LogImpl)
{
} 

Log& Log::get()
{
  static Log logger;
  return logger;
}

void Log::log(LogLevel level, const std::string& msg)
{
  impl_->log(level, msg);
}

Log& SCIRun::Core::Logging::operator<<(Log& log, LogLevel level)
{
  log.impl_->stream(level);
  return log;
}

Log& SCIRun::Core::Logging::operator<<(Log& log, const std::string& msg)
{
  log.impl_->stream(msg);
  return log;
}
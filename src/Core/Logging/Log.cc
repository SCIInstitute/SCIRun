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
#include <log4cpp/FileAppender.hh>
#include <log4cpp/BasicLayout.hh>

#include <boost/lexical_cast.hpp>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Core::Logging;

namespace SCIRun
{
  namespace Core
  {
    namespace Logging
    {
      class LogStreamImpl
      {
      public:
        LogStreamImpl(const log4cpp::CategoryStream& s) : stream_(s) {}
        log4cpp::CategoryStream stream_;
      };

      class LogImpl
      {
      public:
        LogImpl() : cppLogger_(log4cpp::Category::getRoot()), latestStream_(new LogStreamImpl(cppLogger_.infoStream()))
        {
<<<<<<< HEAD
          std::string pattern("%d{%Y-%m-%d %H:%M:%S.%l} [%p] %m%n");

          log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
          auto layout1 = new log4cpp::PatternLayout();
          std::string backupPattern1 = layout1->getConversionPattern();
          try
          {
            layout1->setConversionPattern(pattern);
          }
          catch (log4cpp::ConfigureFailure& exception)
          {
            /// @todo: log?
            std::cerr << "Caught ConfigureFailure exception: " << exception.what() << std::endl
              << "Restoring original pattern: (" << backupPattern1 << ")" << std::endl;
            layout1->setConversionPattern(backupPattern1);
          }
          appender1->setLayout(layout1);

          log4cpp::Appender *appender2 = new log4cpp::FileAppender("default", "scirun5.log");
          auto layout2 = new log4cpp::PatternLayout();
          std::string backupPattern2 = layout1->getConversionPattern();
          try
          {
            layout2->setConversionPattern(pattern);
          }
          catch (log4cpp::ConfigureFailure& exception)
          {
            /// @todo: log?
            std::cerr << "Caught ConfigureFailure exception: " << exception.what() << std::endl
              << "Restoring original pattern: (" << backupPattern2 << ")" << std::endl;
            layout2->setConversionPattern(backupPattern2);
          }
          appender2->setLayout(layout2);
=======
          setAppenders();
        }
>>>>>>> ead7d0073d5f955de2d7e3e7d1e80122cf8aad18

        LogImpl(const std::string& name) : cppLogger_(log4cpp::Category::getInstance(name)), latestStream_(new LogStreamImpl(cppLogger_.infoStream()))
        {
          //TODO
          setAppenders();
          cppLogger_.setAdditivity(false);
          cppLogger_.setPriority(log4cpp::Priority::INFO);  //?
        }

        void log(LogLevel level, const std::string& msg)
        {
          cppLogger_ << translate(level) << msg;
        }

        Log::Stream& stream(LogLevel level)
        {
          latestStream_ = Log::Stream(new LogStreamImpl(cppLogger_ << translate(level)));
          return latestStream_;
        }

        bool verbose() const 
        {
          return cppLogger_.getPriority() == log4cpp::Priority::DEBUG;
        }

        void setVerbose(bool v)
        {
          cppLogger_.setPriority(v ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
        }

        void flush()
        {
          latestStream_.flush();
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
          case ERROR_LOG:  cpp_level = log4cpp::Priority::ERROR;  break;
          case WARN:   cpp_level = log4cpp::Priority::WARN;   break;
          case NOTICE: cpp_level = log4cpp::Priority::NOTICE; break;
          case INFO:   cpp_level = log4cpp::Priority::INFO;   break;
          case DEBUG_LOG:  cpp_level = log4cpp::Priority::DEBUG;  break;
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
        Log::Stream latestStream_;

        void setAppenders()
        {
          std::string pattern("%d{%Y-%m-%d %H:%M:%S.%l} %c [%p] %m%n");

          log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
          auto layout1 = new log4cpp::PatternLayout();
          std::string backupPattern1 = layout1->getConversionPattern();
          try
          {
            layout1->setConversionPattern(pattern);
          }
          catch (log4cpp::ConfigureFailure& exception)
          {
            // TODO: log?
            std::cerr << "Caught ConfigureFailure exception: " << exception.what() << std::endl
              << "Restoring original pattern: (" << backupPattern1 << ")" << std::endl;
            layout1->setConversionPattern(backupPattern1);
          }
          appender1->setLayout(layout1);

          log4cpp::Appender *appender2 = new log4cpp::FileAppender("default", "scirun5.log");
          auto layout2 = new log4cpp::PatternLayout();
          std::string backupPattern2 = layout1->getConversionPattern();
          try
          {
            layout2->setConversionPattern(pattern);
          }
          catch (log4cpp::ConfigureFailure& exception)
          {
            // TODO: log?
            std::cerr << "Caught ConfigureFailure exception: " << exception.what() << std::endl
              << "Restoring original pattern: (" << backupPattern2 << ")" << std::endl;
            layout2->setConversionPattern(backupPattern2);
          }
          appender2->setLayout(layout2);

          cppLogger_.addAppender(appender1);
          cppLogger_.addAppender(appender2);
        }
      };
    }
  }
}

Log::Log() : impl_(new LogImpl)
{
} 

Log::Log(const std::string& name) : impl_(new LogImpl(name))
{
}

Log& Log::get()
{
  static Log logger;
  return logger;
}

Log& Log::get(const std::string& name)
{
  //TODO: make thread safe
  static std::map<std::string, boost::shared_ptr<Log>> logs;
  auto i = logs.find(name);
  if (i == logs.end())
  {
    logs[name] = boost::shared_ptr<Log>(new Log(name));
    return *logs[name];
  }
  return *i->second;
}

void Log::flush()
{
  impl_->flush();
}

void Log::log(LogLevel level, const std::string& msg)
{
  impl_->log(level, msg);
}

Log::Stream& SCIRun::Core::Logging::operator<<(Log& log, LogLevel level)
{
  return log.impl_->stream(level);
}

void Log::Stream::stream(const std::string& msg)
{
  impl_->stream_ << msg;
}

void Log::Stream::stream(double x)
{
  impl_->stream_ << x;
}

void Log::Stream::flush()
{
  impl_->stream_.flush();
}

Log::Stream::Stream(LogStreamImpl* impl) : impl_(impl) {}

Log::Stream& SCIRun::Core::Logging::operator<<(Log::Stream& log, const std::string& msg)
{
  log.stream(msg);
  return log;
}

Log::Stream& SCIRun::Core::Logging::operator<<(Log::Stream& log, double x)
{
  log.stream(x);
  return log;
}

//super hacky and dumb. need to figure out proper way
Log::Stream& SCIRun::Core::Logging::operator<<(Log::Stream& log, std::ostream&(*func)(std::ostream&))
{
  log.flush();
  return log;
}

void Log::setVerbose(bool v)
{
  impl_->setVerbose(v);
}

bool Log::verbose() const
{
  return impl_->verbose();
}

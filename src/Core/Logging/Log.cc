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
/// @todo Documentation Core/Logging/Log.cc

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
#include <boost/filesystem.hpp>
#include <Core/Utils/Exception.h>

// Includes for platform specific functions to get directory to store temp files and user data
#ifdef _WIN32
#include <shlobj.h>
#include <tlhelp32.h>
#include <windows.h>
#include <LMCons.h>
#include <psapi.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#ifndef __APPLE__
#include <unistd.h>
#include <sys/sysinfo.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/mount.h>
#endif
#endif

using namespace SCIRun::Core::Logging;

namespace SCIRun
{
  namespace Core
  {
    namespace Logging
    {
      class Log4cppAppenderAdaptor : public log4cpp::Appender
      {
      public:
        explicit Log4cppAppenderAdaptor(LogAppenderStrategyPtr appender, const std::string& name)
          : log4cpp::Appender(name), appender_(appender)
        {}

        virtual void doAppend(const log4cpp::LoggingEvent& event) override
        {
          if (layout_)
            appender_->log4(layout_->format(event));
          else
            appender_->log4(event.message);
        }

        virtual bool reopen() override
        {
          return true;
        }

        virtual void close() override
        {
        }

        virtual bool requiresLayout() const override
        {
          return false;
        }

        virtual void setLayout(log4cpp::Layout* layout) override
        {
          layout_.reset(layout);
        }

        virtual void setThreshold(log4cpp::Priority::Value priority) override
        {
        }

        virtual log4cpp::Priority::Value getThreshold() override
        {
          return log4cpp::Priority::INFO;
        }

        virtual void setFilter(log4cpp::Filter* filter) override
        {
        }

        virtual log4cpp::Filter* getFilter() override
        {
          return nullptr;
        }
      private:
        LogAppenderStrategyPtr appender_;
        std::unique_ptr<log4cpp::Layout> layout_;
      };

      class LogStreamImpl
      {
      public:
        LogStreamImpl(const log4cpp::CategoryStream& s) : stream_(s) {}
        log4cpp::CategoryStream stream_;
      };

      class LogImpl
      {
      public:
        LogImpl() : name_("root"), cppLogger_(log4cpp::Category::getRoot()), latestStream_(new LogStreamImpl(cppLogger_.infoStream()))
        {
          setBasicAppenders();
        }

        explicit LogImpl(const std::string& name) : name_(name), cppLogger_(log4cpp::Category::getInstance(name)), latestStream_(new LogStreamImpl(cppLogger_.infoStream()))
        {
          /// @todo
          //setBasicAppenders();
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

        void addCustomAppender(Log4cppAppenderAdaptor* appender)
        {
          trySetPattern(appender);
          cppLogger_.addAppender(appender);
        }

        void clearAppenders()
        {
          cppLogger_.removeAllAppenders();
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

        boost::filesystem::path file_;
      private:
        std::string name_;
        log4cpp::Category& cppLogger_;
        Log::Stream latestStream_;

        void setBasicAppenders()
        {
          log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
          trySetPattern(appender1);

          file_ = Log::logDirectory() / ("scirun5_" + name_ + ".log");
          log4cpp::Appender *appender2 = new log4cpp::FileAppender("default", file_.string());
          trySetPattern(appender2);

          cppLogger_.addAppender(appender1);
          cppLogger_.addAppender(appender2);
        }

        void trySetPattern(log4cpp::Appender* appender)
        {
          static const std::string pattern("%d{%Y-%m-%d %H:%M:%S.%l} %c [%p] %m%n");
          std::unique_ptr<log4cpp::PatternLayout> layout(new log4cpp::PatternLayout());
          std::string backupPattern = layout->getConversionPattern();
          try
          {
            layout->setConversionPattern(pattern);
          }
          catch (log4cpp::ConfigureFailure& exception)
          {
            /// @todo: log?
            std::cerr << "Caught ConfigureFailure exception: " << exception.what() << std::endl
              << "Restoring original pattern: (" << backupPattern << ")" << std::endl;
            layout->setConversionPattern(backupPattern);
          }
          appender->setLayout(layout.release());
        }
      };
    }
  }
}

Log::Log() : impl_(new LogImpl)
{
  init();
}

Log::Log(const std::string& name) : impl_(new LogImpl(name))
{
  init();
}

void Log::init()
{
  *this << DEBUG_LOG << "Logging to file: " << impl_->file_.string() << std::endl;
}

boost::filesystem::path Log::directory_(ApplicationHelper().configDirectory());

boost::filesystem::path Log::logDirectory() { return directory_; }
void Log::setLogDirectory(const boost::filesystem::path& dir)
{
  directory_ = dir;
}

Log& Log::get()
{
  static Log logger;
  return logger;
}

Log& Log::get(const std::string& name)
{
  /// @todo: make thread safe
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

void Log::addCustomAppender(boost::shared_ptr<LogAppenderStrategy> appender)
{
  impl_->addCustomAppender(new Log4cppAppenderAdaptor(appender, "custom"));
}

void Log::clearAppenders()
{
  impl_->clearAppenders();
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

//TODO: move

// following ugly code copied from Seg3D.

bool ApplicationHelper::get_user_directory( boost::filesystem::path& user_dir, bool config_path)
{
#ifdef _WIN32
  TCHAR dir[MAX_PATH];

  // Try to create the local application directory
  // If it already exists return the name of the directory.

  if( config_path )
  {
    if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_LOCAL_APPDATA, 0, 0, dir ) ) )
    {
      user_dir = boost::filesystem::path( dir );
      return true;
    }
    else
    {
      std::cerr << "Could not get user directory.";
      return false;
    }
  }
  else
  {
    if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_MYDOCUMENTS, 0, 0, dir ) ) )
    {
      user_dir = boost::filesystem::path( dir );
      return true;
    }
    else
    {
      std::cerr << "Could not get user directory.";
      return false;
    }
  }
#else

  if ( getenv( "HOME" ) )
  {
    user_dir = boost::filesystem::path( getenv( "HOME" ) );

    if (! boost::filesystem::exists( user_dir ) )
    {
      std::cerr << "Could not get user directory.";
      return false;
    }

    return true;
  }
  else
  {
    std::cerr << "Could not get user directory.";
    return false;
  }
#endif
}

bool ApplicationHelper::get_config_directory( boost::filesystem::path& config_dir )
{
  boost::filesystem::path user_dir;
  if ( !( get_user_directory( user_dir, true ) ) ) return false;

#ifdef _WIN32
  config_dir = user_dir / applicationName();
#else
  std::string dot_app_name = std::string( "." ) + applicationName();
  config_dir = user_dir / dot_app_name;
#endif

  if ( !( boost::filesystem::exists( config_dir ) ) )
  {
    if ( !( boost::filesystem::create_directory( config_dir ) ) )
    {
      std::cerr << "Could not create directory: " << config_dir.string() << std::endl;
      return false;
    }

    std::cerr << "Created directory: " << config_dir.string() << std::endl;
  }

  return true;
}

bool ApplicationHelper::get_user_name( std::string& user_name )
{
#ifdef _WIN32
  TCHAR name[UNLEN+1];
  DWORD length = UNLEN;

  if ( GetUserName( name, &length ) )
  {
    user_name = std::string( name );
    return true;
  }
  else
  {
    std::cerr << "Could not resolve user name.";
    return false;
  }
#else
  if ( getenv( "USER" ) )
  {
    user_name = std::string( getenv( "USER" ) );
    return true;
  }
  else
  {
    std::cerr << "Could not resolve user name.";
    return false;
  }
#endif

}

boost::filesystem::path ApplicationHelper::configDirectory()
{
  boost::filesystem::path config;
  if (!get_config_directory(config))
    return boost::filesystem::current_path();
  return config;
}

std::string ApplicationHelper::applicationName()
{
  return "SCIRun";
}

ApplicationHelper::ApplicationHelper()
{
#if WIN32
  boost::filesystem::path::imbue( std::locale( "" ) );
#endif
  boost::filesystem::path dummy("boost bug workaround");
  if (dummy.string().empty())
    std::cout << dummy.string() << std::endl;
}

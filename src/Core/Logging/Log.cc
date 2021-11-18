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


#include <Core/Logging/Log.h>
#include <Core/Logging/ApplicationHelper.h>
#include <boost/filesystem.hpp>
#include <Core/Utils/Exception.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/base_sink.h>
#include <iostream>

using namespace SCIRun::Core::Logging;

#ifdef WIN32
#include <windows.h>
#include <VersionHelpers.h>
#endif

CORE_SINGLETON_IMPLEMENTATION(LogSettings)

bool LogSettings::verbose() const
{
  return verbose_;
}

void LogSettings::setVerbose(bool v)
{
  verbose_ = v;
  spdlog::set_level(v ? spdlog::level::debug : spdlog::level::warn);

  GeneralLog::Instance().setVerbose(v);
}

bool SCIRun::Core::Logging::useLogCheckForWindows7()
{
#ifdef WIN32
  return !(IsWindows7OrGreater() && !IsWindows8OrGreater());
#else
  return true;
#endif
}

  //          static const std::string pattern("%d{%Y-%m-%d %H:%M:%S.%l} %c [%p] %m%n");

LogSettings::LogSettings()
{
  directory_ = ApplicationHelper().configDirectory();
}

boost::filesystem::path LogSettings::logDirectory()
{
  return directory_;
}

void LogSettings::setLogDirectory(const boost::filesystem::path& dir)
{
  directory_ = dir;
}

namespace
{
  class ThreadedSink : public spdlog::sinks::base_sink<std::mutex>
  {
  public:
    explicit ThreadedSink(LogAppenderStrategyPtr appender) : appender_(appender)
    {
    }
  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
		  fmt::memory_buffer formatted;
		  sink::formatter_->format(msg, formatted);
      appender_->log4(fmt::to_string(formatted));
    }
    void flush_() override
    {
    }
  private:
    LogAppenderStrategyPtr appender_;
  };
}

template <class T>
Logger2 makeLoggerLogged(const std::string& name, T&& sinkBegin, T&& sinkEnd)
{
  //printf("Parameters: %s %Ii\n", name.c_str(), std::distance(sinkBegin, sinkEnd));
  return std::make_shared<spdlog::logger>(name, sinkBegin, sinkEnd);
}

Logger2 Log2::get()
{
  if (useLog_)
  {
    static std::mutex mutex;
    if (!logger_)
    {
      std::lock_guard<std::mutex> g(mutex);
      if (!logger_)
      {
        std::transform(customSinks_.begin(), customSinks_.end(), std::back_inserter(sinks_),
          [](LogAppenderStrategyPtr app) { return std::make_shared<ThreadedSink>(app); });
        logger_ = makeLoggerLogged(name_, sinks_.begin(), sinks_.end());
        logger_->trace("{} log initialized.", name_);
		    logger_->flush_on(spdlog::level::err);
        setVerbose(verbose());
      }
    }
  }

  return logger_;
}

Log2::Log2(const std::string& name, bool useLog) : useLog_(useLog), name_(name)
{
  //printf("Log2(): %s\n", name_.c_str());
}

void Log2::addColorConsoleSink()
{
  if (useLog_)
  {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    addSink(consoleSink);
  }
}

bool Log2::verbose() const
{
  return verbose_;
}

void Log2::setVerbose(bool v)
{
  verbose_ = v;
  if (logger_)
  {
    logger_->set_level(v ? spdlog::level::debug : spdlog::level::warn);
  }
}

void Log2::addSink(spdlog::sink_ptr sink)
{
  sinks_.push_back(sink);
}

GeneralLog::GeneralLog() : Log2("root", useLogCheckForWindows7())
{
  if (useLog_)
  {
    addColorConsoleSink();
    auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      (LogSettings::Instance().logDirectory() / "scirun5_root_v3.log").string(), 1024 * 1024, 3);
    addSink(rotating);
  }
}

ModuleLog::ModuleLog() : Log2("module", useLogCheckForWindows7())
{
  if (useLog_)
  {
    addColorConsoleSink();
  }
}

CORE_SINGLETON_IMPLEMENTATION(ModuleLog)
CORE_SINGLETON_IMPLEMENTATION(GeneralLog)

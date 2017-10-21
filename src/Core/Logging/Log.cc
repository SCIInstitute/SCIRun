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

#include <Core/Logging/Log.h>
#include <Core/Logging/ApplicationHelper.h>
#include <boost/filesystem.hpp>
#include <Core/Utils/Exception.h>
#include <Core/Thread/Mutex.h>

using namespace SCIRun::Core::Logging;

CORE_SINGLETON_IMPLEMENTATION(LogSettings)

bool LogSettings::verbose() const
{
  return verbose_;
}

void LogSettings::setVerbose(bool v)
{
  verbose_ = v;
  spdlog::set_level(v ? spdlog::level::debug : spdlog::level::info);
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
    void _sink_it(const spdlog::details::log_msg& msg) override
    {
      appender_->log4(msg.formatted.str());
    }
    void _flush() override
    {
    }
  private:
    LogAppenderStrategyPtr appender_;
  };
}

Logger2 Log2::get()
{
  static Thread::Mutex mutex(name_);
  if (!logger_)
  {
    Thread::Guard g(mutex.get());
    if (!logger_)
    {
      spdlog::set_async_mode(1 << 10);
      std::transform(customSinks_.begin(), customSinks_.end(), std::back_inserter(sinks_),
        [](LogAppenderStrategyPtr app) { return std::make_shared<ThreadedSink>(app); });
      logger_ = std::make_shared<spdlog::logger>(name_, sinks_.begin(), sinks_.end());
      logger_->info("{} log initialized.", name_);
      setVerbose(verbose());
    }
  }
  return logger_;
}

Log2::Log2(const std::string& name) : name_(name)
{
}

void Log2::addColorConsoleSink()
{
  auto consoleSink = spdlog::stdout_color_mt("dummy" + name_)->sinks()[0];
  addSink(consoleSink);
}

bool Log2::verbose() const
{
  return verbose_;
}

void Log2::setVerbose(bool v)
{
  verbose_ = v;
  if (logger_)
    logger_->set_level(v ? spdlog::level::debug : spdlog::level::info);
}

GeneralLog::GeneralLog() : Log2("root")
{
  addColorConsoleSink();
  auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
    (LogSettings::Instance().logDirectory() / "scirun5_root_v2.log").string(), 1024*1024, 3);
  addSink(rotating);
}

CORE_SINGLETON_IMPLEMENTATION(ModuleLog)
CORE_SINGLETON_IMPLEMENTATION(GeneralLog)

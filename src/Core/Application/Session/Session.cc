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


#include <Core/Application/Session/Session.h>
#include <Core/Thread/Mutex.h>
#include <Core/Logging/Log.h>
#include <Core/DatabaseManager/DatabaseManager.h>
#include <Core/Utils/Exception.h>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <locale>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Database;

CORE_SINGLETON_IMPLEMENTATION( SessionManager );

SessionManager::SessionManager() {}

void SessionManager::initialize(const boost::filesystem::path& path)
{
  // TODO configure at application level
  boost::filesystem::path file = path / "scirun_session";
  SessionBuilder builder;
  session_ = builder.build(file);
  SessionUser::session_ = session_;
}

SessionHandle SessionManager::session()
{
  return session_;
}

namespace detail
{

class BasicSession : public SessionInterface
{
public:
  explicit BasicSession(SessionBackEndHandle backEnd) : backEnd_(backEnd) {}

  virtual void beginSession()
  {
    backEnd_->consume("", "Session begins.");
  }

  virtual void endSession()
  {
    backEnd_->consume("", "Session ends.");
  }

  virtual void recordSystemDetails()
  {
    // TODO
  }

  virtual void networkFileLoaded()
  {
    // TODO
  }

  virtual void networkFileSaved()
  {
    // TODO
  }

  virtual void networkEdited(/*EditDescription*/)
  {
    // TODO
  }

  virtual void executionStarted()
  {
    // TODO
  }

  virtual void executionFinished()
  {
    // TODO
  }

  virtual void moduleExecutionStarted(/*ModuleId*/)
  {
    // TODO
  }

  virtual void moduleExecutionFinished(/*ModuleId*/)
  {
    // TODO
  }

  virtual void recordStateVariable(/*ModuleId, Variable*/)
  {
    // TODO
  }

  virtual void fileRead(/*ModuleId, filename*/)
  {
    // TODO
  }

  virtual void fileWritten(/*ModuleId, filename*/)
  {
    // TODO
  }

  virtual void externalProgramAccessed()
  {
    // TODO
  }
private:
  SessionBackEndHandle backEnd_;
};

class NullSession : public SessionInterface
{
public:
  virtual void beginSession() {}
  virtual void endSession() {}
  virtual void recordSystemDetails() {}
  virtual void networkFileLoaded() {}
  virtual void networkFileSaved() {}
  virtual void networkEdited(/*EditDescription*/) {}
  virtual void executionStarted() {}
  virtual void executionFinished() {}
  virtual void moduleExecutionStarted(/*ModuleId*/) {}
  virtual void moduleExecutionFinished(/*ModuleId*/) {}
  virtual void recordStateVariable(/*ModuleId, Variable*/) {}
  virtual void fileRead(/*ModuleId, filename*/) {}
  virtual void fileWritten(/*ModuleId, filename*/) {}
  virtual void externalProgramAccessed() {}
};

class SessionFile : public SessionBackEnd
{
public:
  explicit SessionFile(const boost::filesystem::path& file) : file_(file), stream_(file.string().c_str(), std::ios_base::app),
    locale_(stream_.getloc(), new boost::posix_time::time_facet("%x %X")), mutex_("sessionFile")
  {
  }
  virtual void consume(const std::string& statement, const std::string& message)
  {
    Guard g(mutex_.get());
    namespace pt = boost::posix_time;
    const pt::ptime now = pt::second_clock::local_time();
    static bool firstTime = true;
    if (firstTime)
    {
      stream_.imbue(locale_);
      firstTime = false;
    }

    stream_ << "[" << now << "] : " << message << " (" << statement << ")"<< std::endl;
  }
private:
  boost::filesystem::path file_;
  std::ofstream stream_;
  std::locale locale_;
  Mutex mutex_;
};

class SessionDB : public SessionBackEnd
{
public:
  virtual void consume(const std::string& statement, const std::string& message)
  {
    // TODO
  }
};

class SessionTraceDB : public SessionBackEnd
{
public:
  explicit SessionTraceDB(const boost::filesystem::path& file) : file_(file)
  {
    std::string error;
    if (!db_.save_database(file, error))
      THROW_INVALID_ARGUMENT("Error creating/saving database: " + error);
    if (!db_.load_database(file, error))
      THROW_INVALID_ARGUMENT("Error loading database: " + error);
    if (!createTable())
      THROW_INVALID_ARGUMENT("Could not create table in SessionTraceDB");
  }
  ~SessionTraceDB()
  {
    std::string error;
    db_.save_database(file_, error);
  }
  virtual void consume(const std::string& statement, const std::string& message)
  {
    if (!insertRow(statement + " / " + message))
      std::cout << "problem inserting row" << std::endl;
  }
private:
  boost::filesystem::path file_;
  DatabaseManager db_;

  bool createTable()
  {
    std::string stmt = "CREATE TABLE IF NOT EXISTS Trace (SessionStatement TEXT)";
    std::string error;
    return db_.run_sql_statement(stmt, error);
  }

  bool insertRow(const std::string& statement)
  {
    //std::cout << "DB consume: " << statement << std::endl;
    //TODO: use Parameters
    std::string dequoted(statement);
    dequoted.erase(std::remove(dequoted.begin(), dequoted.end(), '\"'), dequoted.end());
    dequoted.erase(std::remove(dequoted.begin(), dequoted.end(), '\''), dequoted.end());
    std::string stmt = "INSERT INTO Trace VALUES ('" + statement + "')";
    std::string error;
    return db_.run_sql_statement(stmt, error);
  }
};

class CompositeSessionBackEnd : public SessionBackEnd
{
public:
  virtual void consume(const std::string& statement, const std::string& message)
  {
    for (const auto& backEnd : backEnds_)
    {
      backEnd->consume(statement, message);
    }
  }
  void add(SessionBackEndHandle backEnd)
  {
    if (backEnd)
      backEnds_.push_back(backEnd);
  }
private:
  std::vector<SessionBackEndHandle> backEnds_;
};

}

SessionHandle SessionBuilder::build(const boost::filesystem::path& file)
{
  boost::shared_ptr<detail::CompositeSessionBackEnd> compositeSession(new detail::CompositeSessionBackEnd);
  // try create db backend
  try
  {
    boost::filesystem::path dbFile(file);
    dbFile.replace_extension(".sqlite");
    SessionBackEndHandle db(new detail::SessionTraceDB(dbFile));
    compositeSession->add(db);
  }
  catch (InvalidArgumentException& e)
  {
  	std::cout << "Error creating DB session back end: " << e.what() << std::endl;
  }

  try
  {
    boost::filesystem::path textFile(file);
    textFile.replace_extension(".txt");
    SessionBackEndHandle text(new detail::SessionFile(textFile));
    compositeSession->add(text);
  }
  catch (InvalidArgumentException& e)
  {
    std::cout << "Error creating text session back end: " << e.what() << std::endl;
  }

  return boost::make_shared<detail::BasicSession>(compositeSession);
}

SessionHandle SessionUser::session()
{
  return session_;
}

SessionHandle SessionUser::session_(new detail::NullSession);

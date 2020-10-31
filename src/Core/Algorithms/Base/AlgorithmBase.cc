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


#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>

#include <Core/Utils/StringUtil.h>
#include <Core/Algorithms/Base/Name.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Base/AlgorithmParameterHelper.h>
#include <Core/Algorithms/Base/AlgorithmInputBuilder.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmFactory.h>
#include <Core/Logging/ConsoleLogger.h>
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;

Name::Name(const std::string& name) : name_(name)
{
  if (!std::all_of(name.begin(), name.end(), [](char c) { return isalnum(c) || c == '_'; }))
  {
    //Disabling this message since it messes up log creation order.
    //LOG_DEBUG("AlgorithmParameterName not accessible from Python: {}", name);
  }
}

namespace
{
  // Note: boost::serialization has trouble with NaN values, in addition to the platform differences.
  // Workaround will be to store a string in place of the actual double nan value.
  // TODO: investigate if this is a problem with infinities too. No modules store them at the moment.
  const std::string nanString = "NaN";
}

Variable::Variable(const Name& name, const Value& value) : name_(name)
{
  Variable::setValue(value);
}

void Variable::setValue(const Value& val)
{
  value_ = val;

  {
    if ("Filename" == name_.name() && boost::get<std::string>(&val))
    {
      auto stringPath = toString();
      {
        // TODO #787
        // loop through all paths in path list, checking if file in each dir; if one found replace and return
        if (replaceSubstring(stringPath, AlgorithmParameterHelper::dataDir().string(), AlgorithmParameterHelper::dataDirPlaceholder()))
        {
          value_ = stringPath;
          return;
        }

        for (const auto& path : AlgorithmParameterHelper::dataPath())
        {
          if (replaceSubstring(stringPath, path.string(), AlgorithmParameterHelper::dataDirPlaceholder()))
          {
            value_ = stringPath;
            return;
          }
        }

        return;
      }
    }
  }

  {
    if (boost::get<double>(&val))
    {
      auto doubleVal = toDouble();
      if (IsNan(doubleVal))
        value_ = nanString;
      return;
    }
  }
}

int AlgorithmParameter::toInt() const
{
  auto v = boost::get<int>(&value_);
  return v ? *v : 0;
}

double AlgorithmParameter::toDouble() const
{
  auto stringValue = toString();
  if (nanString == stringValue)
    return std::numeric_limits<double>::quiet_NaN();

  auto v = boost::get<double>(&value_);
  return v ? *v : toInt();
}

std::string AlgorithmParameter::toString() const
{
  auto v = boost::get<std::string>(&value_);
  return v ? *v : "";
}

boost::filesystem::path AlgorithmParameter::toFilename() const
{
  {
#ifdef _MSC_VER
    // fix for https://svn.boost.org/trac/boost/ticket/6320
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    Guard g(AlgorithmParameterHelper::lock_.get());
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    boost::filesystem::path::imbue( std::locale( "" ) );
    boost::filesystem::path dummy("boost bug workaround");
    LOG_DEBUG(dummy.string());
#endif
  }

  auto stringPath = toString();
  if (SCIRun::Core::replaceSubstring(stringPath, AlgorithmParameterHelper::dataDirPlaceholder(), ""))
  {
    // TODO #787
    // loop through all paths in path list, checking if file exists each time. return first one that exists.
    auto initialPath = AlgorithmParameterHelper::dataDir() / stringPath;
    if (boost::filesystem::exists(initialPath))
      return initialPath;

    for (const auto& path : AlgorithmParameterHelper::dataPath())
    {
      auto nextPath = path / stringPath;
      if (boost::filesystem::exists(nextPath))
        return nextPath;
    }
    //nothing found, let module deal with it.
    return initialPath;
  }

  boost::filesystem::path p(stringPath);
  return p;
}

bool AlgorithmParameter::toBool() const
{
  auto v = boost::get<bool>(&value_);
  return v ? *v : (toInt() != 0);
}

AlgoOption AlgorithmParameter::toOption() const
{
  auto opt = boost::get<AlgoOption>(&value_);
  return opt ? *opt : AlgoOption();
}

std::vector<Variable> AlgorithmParameter::toVector() const
{
  auto v = boost::get<std::vector<Variable>>(&value_);
  return v ? *v : std::vector<Variable>();
}

DatatypeHandle AlgorithmParameter::getDatatype() const
{
  return data_.get_value_or(nullptr);
}

Variable SCIRun::Core::Algorithms::makeVariable(const std::string& name, const Variable::Value& value)
{
  return Variable(Name(name), value);
}

void AlgorithmParameterHelper::setDataDir(const boost::filesystem::path& path)
{
  dataDir_ = path;
}

boost::filesystem::path AlgorithmParameterHelper::dataDir()
{
  return dataDir_;
}

void AlgorithmParameterHelper::setDataDirPlaceholder(const std::string& str)
{
  dataDirPlaceholder_ = str;
}

std::string AlgorithmParameterHelper::dataDirPlaceholder()
{
  return dataDirPlaceholder_;
}

void AlgorithmParameterHelper::setDataPath(const std::vector<boost::filesystem::path>& paths)
{
  paths_ = paths;
}

std::vector<boost::filesystem::path> AlgorithmParameterHelper::dataPath()
{
  return paths_;
}

boost::filesystem::path AlgorithmParameterHelper::dataDir_;
std::vector<boost::filesystem::path> AlgorithmParameterHelper::paths_;
std::string AlgorithmParameterHelper::dataDirPlaceholder_;
Mutex AlgorithmParameterHelper::lock_("fsbug");

AlgorithmLogger::AlgorithmLogger() : defaultLogger_(new ConsoleLogger)
{
  logger_ = defaultLogger_;
}

AlgorithmLogger::~AlgorithmLogger() {}

void AlgorithmLogger::setLogger(LoggerHandle logger)
{
  logger_ = logger;
}

void AlgorithmLogger::error(const std::string& error) const
{
  logger_->error(error);
}

void AlgorithmLogger::warning(const std::string& warning) const
{
  logger_->warning(warning);
}

void AlgorithmLogger::remark(const std::string& remark) const
{
  logger_->remark(remark);
}

void AlgorithmLogger::status(const std::string& status) const
{
  logger_->status(status);
}

bool AlgorithmLogger::errorReported() const
{
  return logger_->errorReported();
}

void AlgorithmLogger::setErrorFlag(bool flag)
{
  logger_->setErrorFlag(flag);
}

AlgorithmParameterList::AlgorithmParameterList() {}

bool AlgorithmParameterList::set(const AlgorithmParameterName& key, const AlgorithmParameter::Value& value)
{
  auto iter = parameters_.find(key);
  if (iter == parameters_.end())
    return keyNotFoundPolicy(key);
  iter->second.setValue(value);
  return true;
}

const AlgorithmParameter& AlgorithmParameterList::get(const AlgorithmParameterName& key) const
{
  auto iter = parameters_.find(key);
  if (iter == parameters_.end())
    BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound() << Core::ErrorMessage("Algorithm has no parameter with name " + key.name_));
  return iter->second;
}

void AlgorithmParameterList::addParameter(const AlgorithmParameterName& key, const AlgorithmParameter::Value& defaultValue)
{
  parameters_[key] = AlgorithmParameter(key, defaultValue);
}

AlgorithmStatusReporter::AlgorithmStatusReporter()
{
#if DEBUG
  setUpdaterFunc(defaultUpdaterFunc_);
#else
  setUpdaterFunc([](double x) {});
#endif
}

AlgorithmStatusReporter::UpdaterFunc AlgorithmStatusReporter::defaultUpdaterFunc_([](double r) { std::cout << "Algorithm at " << std::setiosflags(std::ios::fixed) << std::setprecision(2) << r*100 << "% complete" << std::endl;});

ScopedAlgorithmStatusReporter::ScopedAlgorithmStatusReporter(const AlgorithmStatusReporter* asr, const std::string& tag) : asr_(asr)
{
  if (asr_)
    asr_->report_start(tag);
}

ScopedAlgorithmStatusReporter::~ScopedAlgorithmStatusReporter()
{
  if (asr_)
    asr_->report_end();
}

DatatypeHandle& AlgorithmData::operator[](const Name& name)
{
  auto& vec = data_[name];
  if (vec.empty())
    vec.resize(1);
  return vec[0];
}

void AlgorithmParameterList::addOption(const AlgorithmParameterName& key, const std::string& defval, const std::string& options)
{
  std::set<std::string> opts;
  boost::split(opts, options, boost::is_any_of("|"));
  parameters_[key] = AlgorithmParameter(key, AlgoOption(defval, opts));
}

bool AlgorithmParameterList::setOption(const AlgorithmParameterName& key, const std::string& value)
{
  auto paramIt = parameters_.find(key);

  if (paramIt == parameters_.end())
    return keyNotFoundPolicy(key);

  auto param = paramIt->second.toOption();

  if (param.options_.find(value) == param.options_.end())
    BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound() << Core::ErrorMessage("parameter \"" + key.name_ + "\" has no option \"" + value + "\""));

  param.option_ = value;
  parameters_[key].setValue(param);
  return true;
}

bool AlgorithmParameterList::keyNotFoundPolicy(const AlgorithmParameterName& key) const
{
  BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound() << Core::ErrorMessage("Algorithm has no parameter/option with name " + key.name_));
  return false;
}

bool AlgorithmParameterList::getOption(const AlgorithmParameterName& key, std::string& value) const
{
  auto paramIt = parameters_.find(key);

  if (paramIt == parameters_.end())
    return keyNotFoundPolicy(key);

  value = paramIt->second.toOption().option_;
  return true;
}

std::string AlgorithmParameterList::getOption(const AlgorithmParameterName& key) const
{
  std::string value;
  getOption(key, value);
  return value;
}

bool AlgorithmParameterList::checkOption(const AlgorithmParameterName& key, const std::string& value) const
{
  std::string currentValue;
  getOption(key, currentValue);
  return boost::iequals(value, currentValue);
}

void AlgorithmParameterList::dumpAlgoState() const
{
  std::ostringstream ostr;
  ostr << "Algorithm state for " << typeid(*this).name() << std::endl;

  auto range = std::make_pair(paramsBegin(), paramsEnd());
  BOOST_FOREACH(const ParameterMap::value_type& pair, range)
  {
    ostr << "\t" << pair.first.name() << ": " << pair.second.value() << std::endl;
  }
  LOG_DEBUG(ostr.str());
}

bool SCIRun::Core::Algorithms::operator==(const AlgoOption& lhs, const AlgoOption& rhs)
{
  return lhs.option_ == rhs.option_ && lhs.options_ == rhs.options_;
}

std::ostream& SCIRun::Core::Algorithms::operator<<(std::ostream& out, const AlgoOption& op)
{
  return out << op.option_;
}

AlgoInputBuilder::AlgoInputBuilder() {}

AlgoInputBuilder& AlgoInputBuilder::operator()(const std::string& name, DatatypeHandle d)
{
  auto& vec = map_[Name(name)];
  if (vec.empty())
    vec.push_back(d);
  else
    vec[0] = d;
  return *this;
}

AlgorithmInput AlgoInputBuilder::build() const
{
  return AlgorithmInput(map_);
}

bool SCIRun::Core::Algorithms::operator==(const Name& lhs, const Name& rhs)
{
  return lhs.name_ == rhs.name_;
}

std::ostream& SCIRun::Core::Algorithms::operator<<(std::ostream& out, const Name& name)
{
  return out << name.name_;
}

AlgorithmInput SCIRun::Core::Algorithms::makeNullInput()
{
  return AlgorithmInput();
}

bool SCIRun::Core::Algorithms::operator==(const Variable& lhs, const Variable& rhs)
{
  return lhs.name() == rhs.name() && lhs.value() == rhs.value() && lhs.getDatatype() == rhs.getDatatype();
}

bool SCIRun::Core::Algorithms::operator!=(const Variable& lhs, const Variable& rhs)
{
  return !(lhs == rhs);
}

std::ostream& SCIRun::Core::Algorithms::operator<<(std::ostream& out, const Variable& var)
{
  return out << "[" << var.name() << ", " << var.value() << "]";
}

AlgorithmCollaborator::~AlgorithmCollaborator() {}

std::vector<std::string> Core::Algorithms::toStringVector(const Variable::List& list)
{
  return toTypedVector<std::string>(list, [](const Variable& v) { return v.toString(); });
}

std::vector<std::string> Core::Algorithms::toNameVector(const Variable::List& list)
{
  return toTypedVector<std::string>(list, [](const Variable& v) { return v.name().name(); });
}

std::vector<double> Core::Algorithms::toDoubleVector(const Variable::List& list)
{
  return toTypedVector<double>(list, [](const Variable& v) { return v.toDouble(); });
}

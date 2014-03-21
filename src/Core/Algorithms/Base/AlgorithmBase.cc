/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Logging/ConsoleLogger.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;

Name::Name(const std::string& name) : name_(name)
{
  if (!std::all_of(name.begin(), name.end(), isalnum))
  {
    //std::cout << "APN not accessible from Python: " << name << std::endl;
    //TODO: log this, exception is overkill.
    //THROW_INVALID_ARGUMENT("Algorithm parameter name must be alphanumeric");
  }
}

AlgorithmBase::~AlgorithmBase() {}

int AlgorithmParameter::getInt() const
{
  const int* v = boost::get<int>(&value_);
  return v ? *v : 0;
}

double AlgorithmParameter::getDouble() const
{
  const double* v = boost::get<double>(&value_);
  return v ? *v : getInt();
}

std::string AlgorithmParameter::getString() const
{
  const std::string* v = boost::get<std::string>(&value_);
  return v ? *v : "";
}

bool AlgorithmParameter::getBool() const
{
  const bool* v = boost::get<bool>(&value_);
  return v ? *v : (getInt() != 0);
}

AlgoOption AlgorithmParameter::getOption() const
{
  const AlgoOption* opt = boost::get<AlgoOption>(&value_);
  return opt ? *opt : AlgoOption();
}

DatatypeHandle AlgorithmParameter::getDatatype() const
{
  return data_;
}

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

AlgorithmParameterList::AlgorithmParameterList() {}

void AlgorithmParameterList::set(const AlgorithmParameterName& key, const AlgorithmParameter::Value& value)
{
  auto iter = parameters_.find(key);
  if (iter == parameters_.end())
    keyNotFoundPolicy(key);
  iter->second.value_ = value;
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
  return boost::get<DatatypeHandle&>(data_[name]);
}

void AlgorithmParameterList::add_option(const AlgorithmParameterName& key, const std::string& defval, const std::string& options)
{
  std::set<std::string> opts;
  auto lower = boost::to_lower_copy(options);
  boost::split(opts, lower, boost::is_any_of("|"));
  parameters_[key] = AlgorithmParameter(key, AlgoOption(defval, opts));
}

bool AlgorithmParameterList::set_option(const AlgorithmParameterName& key, const std::string& value)
{
  auto paramIt = parameters_.find(key);

  if (paramIt == parameters_.end())
    keyNotFoundPolicy(key);
  
  AlgoOption param = paramIt->second.getOption();
  std::string valueLower = boost::to_lower_copy(value);

  if (param.options_.find(valueLower) == param.options_.end())
    BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound() << Core::ErrorMessage("parameter \"" + key.name_ + "\" has no option \"" + valueLower + "\""));

  param.option_ = valueLower;
  parameters_[key].value_ = param;
  return true;
}

void AlgorithmParameterList::keyNotFoundPolicy(const AlgorithmParameterName& key)
{
  BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound() << Core::ErrorMessage("Algorithm has no parameter/option with name " + key.name_));
}

bool AlgorithmParameterList::get_option(const AlgorithmParameterName& key, std::string& value) const
{
  auto paramIt = parameters_.find(key);

  if (paramIt == parameters_.end())
    BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound() << Core::ErrorMessage("option key \"" + key.name_ + "\" was not defined in algorithm"));

  value = paramIt->second.getOption().option_;
  return true;
}

std::string AlgorithmParameterList::get_option(const AlgorithmParameterName& key) const
{
  std::string value;
  get_option(key, value);
  return value;
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
  map_[Name(name)] = d;
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
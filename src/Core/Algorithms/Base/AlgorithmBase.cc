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
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Logging/ConsoleLogger.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

AlgorithmBase::~AlgorithmBase() {}

int AlgorithmParameter::getInt() const
{
  const int* v = boost::get<int>(&value_);
  return v ? *v : 0;
}

double AlgorithmParameter::getDouble() const
{
  const double* v = boost::get<double>(&value_);
  return v ? *v : 0;
}

std::string AlgorithmParameter::getString() const
{
  const std::string* v = boost::get<std::string>(&value_);
  return v ? *v : "";
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
    BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound());
  iter->second.value_ = value;
}

const AlgorithmParameter& AlgorithmParameterList::get(const AlgorithmParameterName& key) const
{
  auto iter = parameters_.find(key);
  if (iter == parameters_.end())
    BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound());
  return iter->second;
}

void AlgorithmParameterList::addParameter(const AlgorithmParameterName& key, const AlgorithmParameter::Value& defaultValue)
{
  parameters_[key] = AlgorithmParameter(key, defaultValue);
}

ScopedAlgorithmReporter::ScopedAlgorithmReporter(AlgorithmStatusReporter* algo, const std::string& tag) : algo_(algo)
{
  algo_->algo_start(tag);
}

ScopedAlgorithmReporter::~ScopedAlgorithmReporter()
{
  algo_->algo_end();
}

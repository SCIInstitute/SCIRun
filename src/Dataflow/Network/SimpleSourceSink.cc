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


/// @todo Documentation Dataflow/Network/SimpleSourceSink.cc

#include <iostream>
#include <Dataflow/Network/SimpleSourceSink.h>
#include <Core/Logging/Log.h>
// don't really like this dependency
#include <Core/Algorithms/Describe/DescribeDatatype.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::General;

SimpleSink::SimpleSink() :
  hasChanged_(false),
  checkForNewDataOnSetting_(false)
{
  instances_.insert(this);
}

SimpleSink::~SimpleSink()
{
  instances_.erase(this);
}

void SimpleSink::waitForData()
{
  //do nothing
}

std::set<SimpleSink*> SimpleSink::instances_;

bool SimpleSink::globalPortCaching_(true); /// @todo: configurable on a port-by-port basis

bool SimpleSink::globalPortCachingFlag() { return globalPortCaching_; }

void SimpleSink::setGlobalPortCachingFlag(bool value)
{
  globalPortCaching_ = value;
  if (!globalPortCaching_)
  {
    SimpleSource::clearAllSources();
    invalidateAll();
  }
}

void SimpleSink::invalidateAll()
{
  for (auto sink : instances_)
    sink->invalidateProvider();
}

DatatypeHandleOption SimpleSink::receive()
{
  if (auto strong = weakData_.lock())
  {
    return strong;
  }
  return DatatypeHandleOption();
}

void SimpleSink::setData(DatatypeHandle data)
{
  if (auto strong = weakData_.lock())
  {
    if (data)
    {
      hasChanged_ = strong->id() != data->id();
    }
  }
  else if (data)
  {
    hasChanged_ = true;
  }

  weakData_ = data;
  if (data && hasChanged_ && checkForNewDataOnSetting_)
    dataHasChanged_(data);
}

void SimpleSink::forceFireDataHasChanged()
{
  auto data = weakData_.lock();
  dataHasChanged_(data);
}

DatatypeSinkInterface* SimpleSink::clone() const
{
  return new SimpleSink;
}

bool SimpleSink::hasChanged() const
{
  auto val = hasChanged_;
  hasChanged_ = false;
  return val;
}

boost::signals2::connection SimpleSink::connectDataHasChanged(const DataHasChangedSignalType::slot_type& subscriber)
{
  checkForNewDataOnSetting_ = true;
  return dataHasChanged_.connect(subscriber);
}

void SimpleSource::cacheData(DatatypeHandle data)
{
  data_ = data;
}

void SimpleSource::send(DatatypeSinkInterfaceHandle receiver) const
{
  auto sink = dynamic_cast<SimpleSink*>(receiver.get());
  if (!sink)
    THROW_INVALID_ARGUMENT("SimpleSource can only send to SimpleSinks");

  sink->setData(data_);
}

bool SimpleSource::hasData() const
{
  return data_ != nullptr;
}

SimpleSource::SimpleSource()
{
  instances_.insert(this);
}

SimpleSource::~SimpleSource()
{
  instances_.erase(this);
}

std::set<SimpleSource*> SimpleSource::instances_;

void SimpleSource::clearAllSources()
{
  for (auto source : instances_)
    source->data_.reset();
}

std::string SimpleSource::describeData() const
{
  DescribeDatatype dd;
  return dd.describe(data_);
}

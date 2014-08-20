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

/// @todo Documentation Dataflow/Network/SimpleSourceSink.cc

#include <Dataflow/Network/SimpleSourceSink.h>
#include <boost/foreach.hpp>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

SimpleSink::SimpleSink() : checkForNewDataOnSetting_(false)
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
  BOOST_FOREACH(SimpleSink* sink, instances_)
    sink->invalidateProvider();
}

DatatypeHandleOption SimpleSink::receive()
{
  if (dataProvider_)
  {
    auto data = dataProvider_();
    
    if (!globalPortCachingFlag())
      invalidateProvider();

    currentId_ = data->id();
    return data;
  }
  return DatatypeHandleOption();
}

void SimpleSink::setData(DataProvider dataProvider)
{
  if (dataProvider_)
  {
    if (currentId_)
      previousId_ = *currentId_;
  }

  dataProvider_ = dataProvider;

  if (dataProvider_)
    currentId_ = dataProvider_()->id();

  if (checkForNewDataOnSetting_ && dataProvider_)
  {
    if (hasChanged())
    {
      /*emit*/ dataHasChanged_(dataProvider_());
    }
  }
}

DatatypeSinkInterface* SimpleSink::clone() const
{
  return new SimpleSink;
}

bool SimpleSink::hasChanged() const
{
  if (!dataProvider_)
  {
    return false;
  }

  if (!previousId_)
  {
    return true;
  }
  return *previousId_ != *currentId_;
}

void SimpleSink::invalidateProvider()
{
  dataProvider_ = 0;
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

  sink->setData([this]() { return data_; });
  //addDeleteListener(sink);
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
  BOOST_FOREACH(SimpleSource* source, instances_)
    source->data_.reset();
}
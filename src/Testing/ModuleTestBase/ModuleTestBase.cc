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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/Tests/MockNetwork.h>

#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/DataflowInterfaces.h>
#include <boost/functional/factory.hpp>
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Dataflow/State/SimpleMapModuleState.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Network/SimpleSourceSink.h>
#include <Dataflow/Network/ModuleBuilder.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Testing;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class StubbedDatatypeSink : public DatatypeSinkInterface
{
public:
  virtual bool hasData() const { return true; }
  virtual void setHasData(bool) {}
  virtual void waitForData() override {}

  virtual DatatypeHandleOption receive() override { return data_; }
  virtual DatatypeSinkInterface* clone() const override { return new StubbedDatatypeSink; }
  virtual bool hasChanged() const override { return true; }

  void setData(DatatypeHandleOption data) { data_ = data; }
  virtual void invalidateProvider() override {}
  virtual void forceFireDataHasChanged() override {}

  virtual boost::signals2::connection connectDataHasChanged(const DataHasChangedSignalType::slot_type&) override { return {}; }
private:
  DatatypeHandleOption data_;
};

class TestSimpleSource : public SimpleSource
{
public:
  DatatypeHandle getDataForTesting() const { return data_; }
};

class MockAlgorithmFactory : public AlgorithmFactory
{
public:
  explicit MockAlgorithmFactory(bool verbose) : verbose_(verbose) {}
  virtual AlgorithmHandle create(const std::string& name, const AlgorithmCollaborator*) const override
  {
    if (verbose_)
      std::cout << "Creating mock algorithm named: " << name << std::endl;
    return boost::make_shared<NiceMock<MockAlgorithm>>();
  }
private:
  bool verbose_;
};

ModuleTestBase::ModuleTestBase() : factory_(new HardCodedModuleFactory)
{
  initModuleParameters();
}

void ModuleTestBase::initModuleParameters(bool verbose)
{
  ModuleBuilder::use_sink_type(boost::factory<StubbedDatatypeSink*>());
  ModuleBuilder::use_source_type(boost::factory<TestSimpleSource*>());
  DefaultModuleFactories::defaultAlgoFactory_.reset(new MockAlgorithmFactory(verbose));
  DefaultValue<AlgorithmParameterName>::Set(AlgorithmParameterName());
  DefaultValue<AlgorithmParameter>::Set(AlgorithmParameter());
  DefaultValue<const AlgorithmParameter&>::Set(algoParam_);
  DefaultValue<AlgorithmOutput>::Set(AlgorithmOutput());
  DefaultValue<AlgorithmInput>::Set(AlgorithmInput());
  LogSettings::Instance().setVerbose(verbose);
}

ModuleHandle ModuleTestBase::makeModule(const std::string& name)
{
  return CreateModuleFromUniqueName(*factory_, name);
}

void ModuleTestBase::stubPortNWithThisData(ModuleHandle module, size_t portNum, DatatypeHandle data)
{
  if (portNum < module->numInputPorts())
  {
    auto iport = module->inputPorts()[portNum];
    if (iport->nconnections() > 0)
      iport->detach(nullptr);
    iport->attach(nullptr);
    if (iport->isDynamic())
    {
      ModuleBuilder builder;
      auto newPortId = builder.cloneInputPort(module, iport->id());
    }
    DatatypeHandleOption o = data;
    dynamic_cast<StubbedDatatypeSink*>(iport->sink().get())->setData(o);
  }
}

DatatypeHandle ModuleTestBase::getDataOnThisOutputPort(ModuleHandle module, size_t portNum)
{
  if (portNum < module->numOutputPorts())
  {
    auto oport = module->outputPorts()[portNum];
    return boost::dynamic_pointer_cast<TestSimpleSource>(oport->source())->getDataForTesting();
  }
  return nullptr;
}

void ModuleTestBase::connectDummyOutputConnection(Dataflow::Networks::ModuleHandle module, size_t portNum)
{
  if (portNum < module->numOutputPorts())
  {
    auto oport = module->outputPorts()[portNum];
    oport->attach(nullptr);
  }
}

UseRealAlgorithmFactory::UseRealAlgorithmFactory()
{
  DefaultModuleFactories::defaultAlgoFactory_.reset(new HardCodedAlgorithmFactory);
}

UseRealAlgorithmFactory::~UseRealAlgorithmFactory()
{
  DefaultModuleFactories::defaultAlgoFactory_.reset(new MockAlgorithmFactory(true));
}

UseRealModuleStateFactory::UseRealModuleStateFactory()
{
  DefaultModuleFactories::defaultStateFactory_.reset(new SimpleMapModuleStateFactory);
}

UseRealModuleStateFactory::~UseRealModuleStateFactory()
{
  DefaultModuleFactories::defaultStateFactory_.reset();
}

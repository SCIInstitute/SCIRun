/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ConnectionId.h>
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

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
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
  virtual void setHasData(bool dataPresent) {}
  virtual void waitForData() {}

  virtual DatatypeHandleOption receive() { return data_; }
  virtual DatatypeSinkInterface* clone() const { return new StubbedDatatypeSink; }
  virtual bool hasChanged() const { return true; }

  void setData(DatatypeHandleOption data) { data_ = data; }
  virtual void invalidateProvider() {}

  virtual boost::signals2::connection connectDataHasChanged(const DataHasChangedSignalType::slot_type& subscriber) { return boost::signals2::connection(); }
private:
  DatatypeHandleOption data_;
};

class MockAlgorithmFactory : public AlgorithmFactory
{
public:
  explicit MockAlgorithmFactory(bool verbose) : verbose_(verbose) {}
  virtual AlgorithmHandle create(const std::string& name, const AlgorithmCollaborator* algoCollaborator) const
  {
    if (verbose_)
      std::cout << "Creating mock algorithm named: " << name << std::endl;
    return AlgorithmHandle(new NiceMock<MockAlgorithm>);
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
  Module::Builder::use_sink_type(boost::factory<StubbedDatatypeSink*>());
  Module::defaultAlgoFactory_.reset(new MockAlgorithmFactory(verbose));
  DefaultValue<AlgorithmParameterName>::Set(AlgorithmParameterName());
  DefaultValue<AlgorithmParameter>::Set(AlgorithmParameter());
  DefaultValue<const AlgorithmParameter&>::Set(algoParam_);
  DefaultValue<AlgorithmOutput>::Set(AlgorithmOutput());
  DefaultValue<AlgorithmInput>::Set(AlgorithmInput());
  Core::Logging::Log::get().setVerbose(verbose);
}

ModuleHandle ModuleTestBase::makeModule(const std::string& name)
{
  return CreateModuleFromUniqueName(*factory_, name);
}

void ModuleTestBase::stubPortNWithThisData(ModuleHandle module, size_t portNum, DatatypeHandle data)
{
  if (portNum < module->num_input_ports())
  {
    auto iport = module->inputPorts()[portNum];
    if (iport->nconnections() > 0)
      iport->detach(0);
    iport->attach(0);
    if (iport->isDynamic())
    {
      Module::Builder builder;
      auto newPortId = builder.cloneInputPort(module, iport->id());
    }
    DatatypeHandleOption o = data;
    dynamic_cast<StubbedDatatypeSink*>(iport->sink().get())->setData(o);
  }
}

DatatypeHandle ModuleTestBase::getDataOnThisOutputPort(ModuleHandle module, size_t portNum)
{
  if (portNum < module->num_output_ports())
  {
    auto oport = module->outputPorts()[portNum];
    //TODO: need a way to grab output values
   // return dynamic_cast<StubbedDatatypeSink*>(oport->source().get())->getData();
  }
  return DatatypeHandle();
}

void ModuleTestBase::connectDummyOutputConnection(Dataflow::Networks::ModuleHandle module, size_t portNum)
{
  if (portNum < module->num_output_ports())
  {
    auto oport = module->outputPorts()[portNum];
    oport->attach(0);
  }
}

FieldHandle SCIRun::Testing::CreateEmptyLatVol()
{
  size_type sizex = 3, sizey = 4, sizez = 5;
  return CreateEmptyLatVol(sizex, sizey, sizez);
}

FieldHandle SCIRun::Testing::CreateEmptyLatVol(size_type sizex, size_type sizey, size_type sizez)
{
  FieldInformation lfi("LatVolMesh", 1, "double");

  Point minb(-1.0, -1.0, -1.0);
  Point maxb(1.0, 1.0, 1.0);
  MeshHandle mesh = CreateMesh(lfi, sizex, sizey, sizez, minb, maxb);
  FieldHandle ofh = CreateField(lfi, mesh);
  ofh->vfield()->clear_all_values();
  return ofh;
}

UseRealAlgorithmFactory::UseRealAlgorithmFactory()
{
  Module::defaultAlgoFactory_.reset(new HardCodedAlgorithmFactory);
}

UseRealAlgorithmFactory::~UseRealAlgorithmFactory()
{
  Module::defaultAlgoFactory_.reset(new MockAlgorithmFactory(true));
}

UseRealModuleStateFactory::UseRealModuleStateFactory()
{
  Module::defaultStateFactory_.reset(new SimpleMapModuleStateFactory);
}

UseRealModuleStateFactory::~UseRealModuleStateFactory()
{
  Module::defaultStateFactory_.reset();
}

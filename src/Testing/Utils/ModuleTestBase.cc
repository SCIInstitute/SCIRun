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

#include <Testing/Utils/ModuleTestBase.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/Tests/MockNetwork.h>

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/DataflowInterfaces.h>
#include <boost/functional/factory.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
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

  void setData(DatatypeHandleOption data) { data_ = data; }
private:
  DatatypeHandleOption data_;
};

class MockAlgorithm : public AlgorithmBase
{
public:
  MOCK_CONST_METHOD1(run_generic, AlgorithmOutput(const AlgorithmInput&));
  MOCK_METHOD1(keyNotFoundPolicy, void(const AlgorithmParameterName&));
  //MOCK_METHOD2(set, void(const AlgorithmParameterName&, const AlgorithmParameter::Value&));
  //MOCK_CONST_METHOD1(get, const AlgorithmParameter&(const AlgorithmParameterName&));
  //MOCK_METHOD2(set_option, void(const AlgorithmParameterName&, const std::string& value));
  //MOCK_CONST_METHOD1(get_option, std::string(const AlgorithmParameterName&));
};

class MockAlgorithmFactory : public AlgorithmFactory
{
public:
  virtual AlgorithmHandle create(const std::string& name, const AlgorithmCollaborator* algoCollaborator) const
  {
    return AlgorithmHandle(new NiceMock<MockAlgorithm>);
  }
};

ModuleTest::ModuleTest() : factory_(new HardCodedModuleFactory)
{
  Module::Builder::use_sink_type(boost::factory<StubbedDatatypeSink*>());
  Module::defaultAlgoFactory_.reset(new MockAlgorithmFactory);
  DefaultValue<AlgorithmParameter>::Set(AlgorithmParameter());
  DefaultValue<AlgorithmOutput>::Set(AlgorithmOutput());
}

ModuleHandle ModuleTest::makeModule(const std::string& name)
{
  return CreateModuleFromUniqueName(*factory_, name);
}

void ModuleTest::stubPortNWithThisData(ModuleHandle module, size_t portNum, DatatypeHandle data)
{
  module->get_input_port(portNum)->attach(0);
  DatatypeHandleOption o = data;
  dynamic_cast<StubbedDatatypeSink*>(module->get_input_port(portNum)->sink().get())->setData(o);
}

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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/Tests/MockNetwork.h>

#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Network/DataflowInterfaces.h>
#include <boost/functional/factory.hpp>

#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Modules::Factory;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

//TODO: break out into separate library, we'll want to subclass this for every module we convert over.
class ModuleTest : public ::testing::Test
{
private:
  HardCodedModuleFactory factory;

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

protected:
  ModuleTest()
  {
    Module::Builder::use_sink_type(boost::factory<StubbedDatatypeSink*>());
  }

  ModuleHandle makeModule(const std::string& name)
  {
    return CreateModuleFromUniqueName(factory, name);
  }

  void stubPortNWithThisData(ModuleHandle module, size_t portNum, DatatypeHandle data)
  {
    module->get_input_port(portNum)->attach(0);
    DatatypeHandleOption o = data;
    dynamic_cast<StubbedDatatypeSink*>(module->get_input_port(portNum)->sink().get())->setData(o);
  }
};

class EvaluateLinearAlgebraUnaryModuleTests : public ModuleTest
{

};

TEST_F(EvaluateLinearAlgebraUnaryModuleTests, CanCreateWithMockAlgorithm)
{
  const std::string name = "EvaluateLinearAlgebraUnary";
  auto module = makeModule(name);

  EXPECT_EQ(name, module->get_module_name());

  DenseMatrixHandle m(new DenseMatrix(2,2));

  stubPortNWithThisData(module, 0, m);

  //TODO: mock module state for passing to algorithm
  //TODO: algorithm factory to provide mock algorithm
  module->execute();
}
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
#include <Modules/Math/ReportMatrixInfo.h>
#include <Core/Datatypes/Matrix.h>

#include <Dataflow/Network/Module.h>

#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Engine/Controller/DynamicPortManager.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Modules/Render/ViewScene.h>
#include <Modules/Visualization/ShowField.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Render;
using namespace SCIRun::Modules::Visualization;

class ReportMatrixInfoModuleTest : public ModuleTest
{
};

TEST_F(ReportMatrixInfoModuleTest, ThrowsForNullMatrices)
{
  auto sls = makeModule("ReportMatrixInfo");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(sls, 0, nullMatrix);

  EXPECT_THROW(sls->execute(), NullHandleOnPortException);
}

namespace
{
  void printInputPorts(ModuleHandle m)
  {
    std::cout << "INPUTS: " << m->get_module_name() << std::endl;
    for (InputPortHandle input : m->inputPorts())
    {
      std::cout << input->id() << " " << input->getIndex() << std::endl;
    }
  }
}


TEST(DynamicPortTests, DynamicPortsCloneThemselves)
{
  ModuleStateFactoryHandle sf_;
  AlgorithmFactoryHandle af_;
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, sf_, nullptr, af_, nullptr, nullptr);

  auto network = controller.getNetwork();
  ModuleHandle hasDynamic = network->add_module(ViewScene::staticInfo_);

  const std::string viewScenePortName = "GeneralGeom";

  EXPECT_EQ(1, hasDynamic->num_input_ports());
  EXPECT_EQ(1, hasDynamic->findInputPortsWithName(viewScenePortName).size());

  ModuleHandle input1 = network->add_module(ShowFieldModule::staticInfo_);
  ModuleHandle input2 = network->add_module(ShowFieldModule::staticInfo_);

  ConnectionAddedSignalType addedSignal;
  ConnectionRemovedSignalType removeSignal;
  DynamicPortManager dpm(addedSignal, removeSignal, &controller);
  printInputPorts(hasDynamic);
  auto oport = input1->outputPorts()[0];
  {
    auto iport = hasDynamic->inputPorts()[0];
    EXPECT_EQ(0, oport->nconnections());
    EXPECT_EQ(0, iport->nconnections());

    Connection c(oport, iport, "test");
    std::cout << 1 << std::endl;
    printInputPorts(hasDynamic);
    ConnectionDescription desc(OutgoingConnectionDescription(oport->getUnderlyingModuleId(), oport->id()), 
      IncomingConnectionDescription(iport->getUnderlyingModuleId(), iport->id()));
    addedSignal(desc);

    EXPECT_EQ(1, iport->nconnections());
    EXPECT_EQ(1, oport->nconnections());

    EXPECT_EQ(2, hasDynamic->num_input_ports());
    EXPECT_EQ(2, hasDynamic->findInputPortsWithName(viewScenePortName).size());

    removeSignal(ConnectionId::create(desc));
  }
  printInputPorts(hasDynamic);
  ASSERT_EQ(1, hasDynamic->num_input_ports());
  EXPECT_EQ(0, hasDynamic->inputPorts()[0]->nconnections());
  EXPECT_EQ(0, oport->nconnections());
  {
    auto iport = hasDynamic->inputPorts()[0];
    EXPECT_EQ(0, oport->nconnections());
    EXPECT_EQ(0, iport->nconnections());
    Connection c1(oport, iport, "test");
    printInputPorts(hasDynamic);
    ConnectionDescription desc1(OutgoingConnectionDescription(oport->getUnderlyingModuleId(), oport->id()), 
      IncomingConnectionDescription(iport->getUnderlyingModuleId(), iport->id()));
    addedSignal(desc1);
    EXPECT_EQ(1, iport->nconnections());
    EXPECT_EQ(1, oport->nconnections());

    EXPECT_EQ(2, hasDynamic->num_input_ports());
    EXPECT_EQ(2, hasDynamic->findInputPortsWithName(viewScenePortName).size());

    Connection c2(oport, hasDynamic->inputPorts()[1], "test");
    printInputPorts(hasDynamic);
    ConnectionDescription desc2(OutgoingConnectionDescription(oport->getUnderlyingModuleId(), oport->id()), 
      IncomingConnectionDescription(iport->getUnderlyingModuleId(), hasDynamic->inputPorts()[1]->id()));
    addedSignal(desc2);
    EXPECT_EQ(2, oport->nconnections());
    EXPECT_EQ(3, hasDynamic->num_input_ports());

    removeSignal(ConnectionId::create(desc1));
    removeSignal(ConnectionId::create(desc2));
  }
  printInputPorts(hasDynamic);
  ASSERT_EQ(1, hasDynamic->num_input_ports());
  EXPECT_EQ(1, hasDynamic->findInputPortsWithName(viewScenePortName).size());
  EXPECT_EQ(0, hasDynamic->inputPorts()[0]->nconnections());
  EXPECT_EQ(0, oport->nconnections());

  {
    auto iport = hasDynamic->inputPorts()[0];
    EXPECT_EQ(0, oport->nconnections());
    EXPECT_EQ(0, iport->nconnections());
    boost::shared_ptr<Connection> c1 = boost::make_shared<Connection>(oport, iport, "test");
    printInputPorts(hasDynamic);
    ConnectionDescription desc1(OutgoingConnectionDescription(oport->getUnderlyingModuleId(), oport->id()), 
      IncomingConnectionDescription(iport->getUnderlyingModuleId(), iport->id()));
    addedSignal(desc1);
    EXPECT_EQ(1, iport->nconnections());
    EXPECT_EQ(1, oport->nconnections());
    EXPECT_EQ(2, hasDynamic->num_input_ports());
    EXPECT_EQ(2, hasDynamic->findInputPortsWithName(viewScenePortName).size());
    printInputPorts(hasDynamic);
    Connection c2(oport, hasDynamic->inputPorts()[1], "test");
    ConnectionDescription desc2(OutgoingConnectionDescription(oport->getUnderlyingModuleId(), oport->id()), 
      IncomingConnectionDescription(iport->getUnderlyingModuleId(), hasDynamic->inputPorts()[1]->id()));
    addedSignal(desc2);
    EXPECT_EQ(2, oport->nconnections());
    EXPECT_EQ(3, hasDynamic->num_input_ports());
    EXPECT_EQ(3, hasDynamic->findInputPortsWithName(viewScenePortName).size());
    c1.reset();
    removeSignal(ConnectionId::create(desc1));

    EXPECT_EQ(1, oport->nconnections());
    EXPECT_EQ(2, hasDynamic->num_input_ports());
    removeSignal(ConnectionId::create(desc2));
  }
  ASSERT_EQ(1, hasDynamic->num_input_ports());
  EXPECT_EQ(0, hasDynamic->inputPorts()[0]->nconnections());
  EXPECT_EQ(0, oport->nconnections());
}
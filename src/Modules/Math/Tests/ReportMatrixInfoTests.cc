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



TEST(DynamicPortTests, DynamicPortsCloneThemselves)
{
  ModuleStateFactoryHandle sf_;
  AlgorithmFactoryHandle af_;
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  Network network(mf, sf_, af_);

  ModuleHandle hasDynamic = network.add_module(ViewScene::staticInfo_);

  EXPECT_EQ(1, hasDynamic->num_input_ports());

  ModuleHandle input1 = network.add_module(ShowFieldModule::staticInfo_);
  ModuleHandle input2 = network.add_module(ShowFieldModule::staticInfo_);

  ConnectionAddedSignalType addedSignal;
  ConnectionRemovedSignalType removeSignal;
  DynamicPortManager dpm(addedSignal, removeSignal, &network);

  auto oport = input1->get_output_port(0);
  auto iport = hasDynamic->get_input_port(0);
  {
    EXPECT_EQ(0, oport->nconnections());
    EXPECT_EQ(0, iport->nconnections());

    Connection c(input1, 0, hasDynamic, 0, "test");

    ConnectionDescription desc(OutgoingConnectionDescription(oport->getUnderlyingModuleId(), oport->getIndex()), 
      IncomingConnectionDescription(iport->getUnderlyingModuleId(), iport->getIndex()));
    addedSignal(desc);

    EXPECT_EQ(1, iport->nconnections());
    EXPECT_EQ(1, oport->nconnections());

    EXPECT_EQ(2, hasDynamic->num_input_ports());

    removeSignal(ConnectionId::create(desc));
  }

  EXPECT_EQ(0, iport->nconnections());
  EXPECT_EQ(0, oport->nconnections());

  EXPECT_EQ(1, hasDynamic->num_input_ports());
}
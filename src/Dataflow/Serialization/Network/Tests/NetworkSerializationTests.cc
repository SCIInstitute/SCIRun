/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Dataflow/Serialization/Network/ModuleDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/Tests/MockNetwork.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;

#include <stdexcept>
#include <fstream>
#include <boost/assign.hpp>

using namespace SCIRun::Dataflow::Networks;
using namespace boost::assign;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (int i = 0; i < m->rows(); ++i)
      for (int j = 0; j < m->cols(); ++j)
        (*m)(i, j) = 3.0 * i + j;
    return m;
  }
  DenseMatrixHandle matrix2()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (int i = 0; i < m->rows(); ++i)
      for (int j = 0; j < m->cols(); ++j)
        (*m)(i, j) = -2.0 * i + j;
    return m;
  }

  NetworkXML exampleNet()
  {
    ModuleLookupInfoXML info1;
    info1.module_name_ = "EvaluateLinearAlgebraUnary";  
    info1.category_name_ = "Math";
    info1.package_name_ = "SCIRun";

    ModuleLookupInfoXML info2;
    info2.module_name_ = "ReadMatrix";  
    info2.category_name_ = "DataIO";
    info2.package_name_ = "SCIRun";

    ModuleLookupInfoXML info3;
    info3.module_name_ = "WriteMatrix";  
    info3.category_name_ = "DataIO";
    info3.package_name_ = "SCIRun";

    ConnectionDescriptionXML conn;
    conn.out_.moduleId_ = ModuleId("ReadMatrix", 2);
    conn.in_.moduleId_ = ModuleId("EvaluateLinearAlgebraUnary", 1);
    conn.out_.portId_ = PortId(0, "Matrix");
    conn.in_.portId_ = PortId(0, "InputMatrix");

    ConnectionDescriptionXML conn2;
    conn2.out_.moduleId_ = ModuleId("EvaluateLinearAlgebraUnary", 1);
    conn2.in_.moduleId_ = ModuleId("WriteMatrix", 3);
    conn2.out_.portId_ = PortId(0, "Result");
    conn2.in_.portId_ = PortId(0, "MatrixToWrite");

    ConnectionsXML connections;
    connections += conn2, conn;

    ModuleMapXML mods;
    mods["EvaluateLinearAlgebraUnary:1"] = info1;
    mods["ReadMatrix:2"] = info2;
    mods["WriteMatrix:3"] = info3;

    NetworkXML network;
    network.connections = connections;
    network.modules = mods;

    return network;
  }
}

TEST(SerializeNetworkTest, RoundTripData)
{
  NetworkXML networkXML = exampleNet();
  NetworkXMLSerializer serializer;
  std::ostringstream ostr1;
  serializer.save_xml(networkXML, ostr1);
  const std::string xml1 = ostr1.str();

  std::istringstream istr(xml1);
  NetworkXMLHandle readIn = serializer.load_xml(istr);
  ASSERT_TRUE(readIn.get() != nullptr);
  std::ostringstream ostr2;
  serializer.save_xml(*readIn, ostr2);
  const std::string xml2 = ostr2.str();

  EXPECT_EQ(xml1, xml2);
}

TEST(SerializeNetworkTest, RoundTripObject)
{
  NetworkXML networkXML = exampleNet();

  NetworkXMLSerializer serializer;
  std::ostringstream ostr1;

  serializer.save_xml(networkXML, ostr1);

  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
  NetworkXMLConverter converter(mf, nullptr, nullptr, nullptr, &controller);
  NetworkHandle network = converter.from_xml_data(networkXML);
  ASSERT_TRUE(network.get() != nullptr);
  auto xml2 = converter.to_xml_data(network);
  ASSERT_TRUE(xml2.get() != nullptr);

  std::ostringstream ostr2;
  serializer.save_xml(xml2->network, ostr2);

  EXPECT_EQ(ostr1.str(), ostr2.str());
}


TEST(SerializeNetworkTest, FullTestWithModuleState)
{
  Module::resetIdGenerator();
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  ExecutionStrategyFactoryHandle exe(new DesktopExecutionStrategyFactory(boost::optional<std::string>()));
  NetworkEditorController controller(mf, sf, exe, nullptr, nullptr, nullptr);
  
  ModuleHandle matrix1Send = controller.addModule("SendTestMatrix");
  ModuleHandle matrix2Send = controller.addModule("SendTestMatrix");

  ModuleHandle transpose = controller.addModule("EvaluateLinearAlgebraUnary");
  ModuleHandle negate = controller.addModule("EvaluateLinearAlgebraUnary");
  ModuleHandle scalar = controller.addModule("EvaluateLinearAlgebraUnary");

  ModuleHandle multiply = controller.addModule("EvaluateLinearAlgebraBinary");
  ModuleHandle add = controller.addModule("EvaluateLinearAlgebraBinary");

  ModuleHandle report = controller.addModule("ReportMatrixInfo");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  NetworkHandle matrixMathNetwork = controller.getNetwork();
  EXPECT_EQ(9, matrixMathNetwork->nmodules());

  EXPECT_EQ(0, matrixMathNetwork->nconnections());
  matrixMathNetwork->connect(ConnectionOutputPort(matrix1Send, 0), ConnectionInputPort(transpose, 0));
  matrixMathNetwork->connect(ConnectionOutputPort(matrix1Send, 0), ConnectionInputPort(negate, 0));
  matrixMathNetwork->connect(ConnectionOutputPort(matrix2Send, 0), ConnectionInputPort(scalar, 0));
  matrixMathNetwork->connect(ConnectionOutputPort(negate, 0), ConnectionInputPort(multiply, 0));
  matrixMathNetwork->connect(ConnectionOutputPort(scalar, 0), ConnectionInputPort(multiply, 1));
  matrixMathNetwork->connect(ConnectionOutputPort(transpose, 0), ConnectionInputPort(add, 0));
  matrixMathNetwork->connect(ConnectionOutputPort(multiply, 0), ConnectionInputPort(add, 1));
  matrixMathNetwork->connect(ConnectionOutputPort(add, 0), ConnectionInputPort(report, 0));
  matrixMathNetwork->connect(ConnectionOutputPort(add, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(9, matrixMathNetwork->nconnections());

  //Set module parameters.
  matrix1Send->get_state()->setTransientValue("MatrixToSend", matrix1());
  matrix2Send->get_state()->setTransientValue("MatrixToSend", matrix2());
  transpose->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  negate->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  scalar->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  scalar->get_state()->setValue(Variables::ScalarValue, 4.0);
  multiply->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY);
  add->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraBinaryAlgorithm::ADD);

  auto xml = controller.saveNetwork();

  std::ostringstream ostr;
  XMLSerializer::save_xml(*xml, ostr, "network");
  std::cout << ostr.str() << std::endl;

  NetworkEditorController controller2(mf, sf, exe, nullptr, nullptr, nullptr);
  controller2.loadNetwork(xml);

  NetworkHandle deserialized = controller2.getNetwork();
  ASSERT_TRUE(deserialized.get() != nullptr);

  EXPECT_EQ(9, deserialized->nconnections());
  EXPECT_EQ(9, deserialized->nmodules());
  EXPECT_NE(matrixMathNetwork.get(), deserialized.get());

  ModuleHandle trans2 = deserialized->lookupModule(ModuleId("EvaluateLinearAlgebraUnary", 0));
  ASSERT_TRUE(trans2.get() != nullptr);
  EXPECT_EQ("EvaluateLinearAlgebraUnary", trans2->get_module_name());
  EXPECT_EQ(EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE, trans2->get_state()->getValue(Variables::Operator).toInt());
}

TEST(SerializeNetworkTest, FullTestWithDynamicPorts)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr);

  std::vector<ModuleHandle> showFields;
  showFields.push_back(controller.addModule("ShowField"));
  showFields.push_back(controller.addModule("ShowField"));
  showFields.push_back(controller.addModule("ShowField"));
  showFields.push_back(controller.addModule("ShowField"));
  showFields.push_back(controller.addModule("ShowField"));

  ModuleHandle view = controller.addModule("ViewScene");

  NetworkHandle net = controller.getNetwork();
  EXPECT_EQ(showFields.size() + 1, net->nmodules());

  EXPECT_EQ(0, net->nconnections());
  size_t port = 0;
  for (ModuleHandle show : showFields)
  {
    std::cout << "Attempting to connect to view scene on " << port << std::endl;
    controller.requestConnection(show->outputPorts()[0].get(), view->inputPorts()[port++].get());
  }
  EXPECT_EQ(showFields.size(), net->nconnections());

  auto xml = controller.saveNetwork();

  std::cout << "NOW TESTING SERIALIZED COPY" << std::endl;

  std::ostringstream ostr;
  XMLSerializer::save_xml(*xml, ostr, "network");

  NetworkEditorController controller2(mf, sf, nullptr, nullptr, nullptr, nullptr);
  controller2.loadNetwork(xml);

  NetworkHandle deserialized = controller2.getNetwork();
  ASSERT_TRUE(deserialized != nullptr);

  EXPECT_EQ(showFields.size(), deserialized->nconnections());
  EXPECT_EQ(showFields.size() + 1, deserialized->nmodules());
  EXPECT_NE(net.get(), deserialized.get());
}
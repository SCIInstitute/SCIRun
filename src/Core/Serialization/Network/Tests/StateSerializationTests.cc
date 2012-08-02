/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Serialization/Network/StateSerialization.h>
#include <Core/Serialization/Network/XMLSerializer.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <fstream>
#include <boost/assign.hpp>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::State;

using namespace boost::assign;

//NetworkXML exampleNet2()
//{
//  ModuleLookupInfoXML info1;
//  info1.module_name_ = "EvaluateLinearAlgebraUnary";  
//  info1.category_name_ = "Math";
//  info1.package_name_ = "SCIRun";
//
//  ModuleLookupInfoXML info2;
//  info2.module_name_ = "ReadMatrix";  
//  info2.category_name_ = "DataIO";
//  info2.package_name_ = "SCIRun";
//
//  ModuleLookupInfoXML info3;
//  info3.module_name_ = "WriteMatrix";  
//  info3.category_name_ = "DataIO";
//  info3.package_name_ = "SCIRun";
//
//  ConnectionDescriptionXML conn;
//  conn.moduleId1_ = "ReadMatrix2";
//  conn.moduleId2_ = "EvaluateLinearAlgebraUnary1";
//  conn.port1_ = 0;
//  conn.port2_ = 0;
//
//  ConnectionDescriptionXML conn2;
//  conn2.moduleId1_ = "EvaluateLinearAlgebraUnary1";
//  conn2.moduleId2_ = "WriteMatrix3";
//  conn2.port1_ = 0;
//  conn2.port2_ = 0;
//
//  ConnectionsXML connections;
//  connections += conn2, conn;
//
//  ModuleMapXML mods;
//  mods["EvaluateLinearAlgebraUnary1"] = info1;
//  mods["ReadMatrix2"] = info2;
//  mods["WriteMatrix3"] = info3;
//
//  NetworkXML network;
//  network.connections = connections;
//  network.modules = mods;
//
//  return network;
//}

namespace 
{
  using namespace SCIRun::Algorithms;
  AlgorithmParameterName intName("int");
  AlgorithmParameterName doubleName("double");
  AlgorithmParameterName stringName("string");
}

TEST(SerializeStateTest, RoundTrip)
{
  SimpleMapModuleStateXML state;

  state.setValue(intName, 4);
  state.setValue(doubleName, 3.14);
  const std::string s = "hello";
  state.setValue(stringName, s);

  std::ostringstream ostr1;
  const std::string root = "state";
  XMLSerializer::save_xml(state, ostr1, root);
  const std::string xml1 = ostr1.str();

  std::cout << xml1 << std::endl;

  std::istringstream istr(xml1);
  boost::shared_ptr<SimpleMapModuleStateXML> readIn = XMLSerializer::load_xml<SimpleMapModuleStateXML>(istr);
  ASSERT_TRUE(readIn);
  std::ostringstream ostr2;
  XMLSerializer::save_xml(*readIn, ostr2, root);
  const std::string xml2 = ostr2.str();

  std::cout << xml2 << std::endl;

  EXPECT_EQ(xml1, xml2);

  EXPECT_EQ(4, readIn->getValue(intName).getInt());
  EXPECT_EQ(3.14, readIn->getValue(doubleName).getDouble());
  EXPECT_EQ(s, readIn->getValue(stringName).getString());
}

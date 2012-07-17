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

#include <Core/Serialization/Network/ModuleDescriptionSerialization.h>
#include <Core/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Serialization/Network/NetworkXMLSerializer.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <fstream>
#include <boost/assign.hpp>

using namespace SCIRun::Domain::Networks;
using namespace boost::assign;

NetworkXML exampleNet()
{
  ConnectionDescriptionXML conn;
  conn.moduleId1_ = "module1";
  conn.moduleId2_ = "module2";
  conn.port1_ = 0;
  conn.port2_ = 1;

  ConnectionDescriptionXML conn2;
  conn2.moduleId1_ = "module1a";
  conn2.moduleId2_ = "module2a";
  conn2.port1_ = 1;
  conn2.port2_ = 2;

  ConnectionsXML connections;
  connections += conn, conn2;

  ModuleLookupInfoXML info1;
  info1.module_name_ = "ComputeSVD";  
  info1.category_name_ = "Math";
  info1.package_name_ = "SCIRun";

  ModuleLookupInfoXML info2;
  info2.module_name_ = "CreateMatrix";  
  info2.category_name_ = "Math";
  info2.package_name_ = "SCIRun";

  ModuleLookupInfo copy(info1);
  ModuleLookupInfoXML x(copy);

  ModuleMapXML mods;
  mods["ComputeSVD1"] = info1;
  mods["CreateMatrix2"] = info2;

  NetworkXML network;
  network.connections = connections;
  network.modules = mods;

  return network;
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
  ASSERT_TRUE(readIn);
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

  std::cout << ostr1.str() << std::endl;
  
  ModuleFactoryHandle mf(new SCIRun::Modules::Factory::HardCodedModuleFactory);
  NetworkXMLConverter converter(mf, ModuleStateFactoryHandle());
  NetworkHandle network = converter.from_xml_data(networkXML);
  ASSERT_TRUE(network);
  NetworkXMLHandle xml2 = converter.to_xml_data(network);
  ASSERT_TRUE(xml2);

  std::ostringstream ostr2;
  serializer.save_xml(*xml2, ostr2);

  EXPECT_EQ(ostr1.str(), ostr2.str());

  EXPECT_TRUE(false);
}
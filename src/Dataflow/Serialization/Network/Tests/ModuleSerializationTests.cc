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

#include <Dataflow/Serialization/Network/ModuleDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <fstream>
#include <boost/assign.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::State;
using namespace boost::assign;

TEST(ModuleDescriptionXMLTest, CanSerializeModuleInfo)
{
  ModuleLookupInfoXML info;
  info.module_name_ = "ComputeSVD";  
  info.category_name_ = "Math";
  info.package_name_ = "SCIRun";

  std::string filename("E:\\git\\SCIRunGUIPrototype\\src\\Samples\\");
  filename += "info.xml";

  XMLSerializer::save_xml(info, filename, "moduleInfo");

  ModuleLookupInfoXML newInfo;

  newInfo = *XMLSerializer::load_xml<ModuleLookupInfoXML>(filename);

  EXPECT_EQ(info.module_name_, newInfo.module_name_);
  EXPECT_EQ(info.category_name_, newInfo.category_name_);
  EXPECT_EQ(info.package_name_, newInfo.package_name_);
}

TEST(SerializeNetworkTest, WhatDoWeNeed)
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
  XMLSerializer::save_xml(network, "E:\\git\\SCIRunGUIPrototype\\src\\Samples\\network.srn", "network");
}
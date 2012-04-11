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

#include <gtest/gtest.h>
#include <Core/Dataflow/Network/Network.h>
#include <Core/Dataflow/Network/Connection.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <Core/Dataflow/Network/Tests/MockModule.h>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Networks::Mocks;

TEST(NetworkTests, CanAddAndRemoveModules)
{
  ModuleFactoryHandle moduleFactory(new MockModuleFactory);
  Network network(moduleFactory);
 
  EXPECT_EQ(0, network.nmodules());

  ModuleDescription md;
  md.module_name_ = "Module1";
  ModuleHandle m = network.add_module(md);
  EXPECT_EQ(md.module_name_, m->get_module_name());

  EXPECT_EQ(1, network.nmodules());

  EXPECT_TRUE(network.remove_module(m->get_id()));
  EXPECT_EQ(0, network.nmodules());

  EXPECT_FALSE(network.remove_module("not in the network"));
}

TEST(NetworkTests, CanAddAndRemoveConnections)
{
  ModuleFactoryHandle moduleFactory(new MockModuleFactory);
  Network network(moduleFactory);

  ModuleDescription md1;
  md1.module_name_ = "Module1";
  ModuleHandle m1 = network.add_module(md1);
  ModuleDescription md2;
  md2.module_name_ = "Module2";
  ModuleHandle m2 = network.add_module(md2);

  EXPECT_EQ(2, network.nmodules());

  ConnectionId connId = network.connect(m1, 1, m2, 2);
  EXPECT_EQ(1, network.nconnections());
  std::cout << connId.id_ << std::endl;

  EXPECT_TRUE(network.disconnect(connId));
  EXPECT_EQ(0, network.nconnections());

  ASSERT_TRUE(false);
}
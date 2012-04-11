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

#include <Core/Dataflow/Network/Module.h>
#include <gtest/gtest.h>

using namespace SCIRun::Domain::Networks;

TEST(ModuleTests, CanConstructEmptyModuleWithName)
{
  std::string name = "CreateMatrix";
  Module m(name);
  ASSERT_EQ(m.get_module_name(), name);
}

TEST(ModuleTests, CanBuildWithPorts)
{
  ModuleHandle module = Module::Builder().with_name("SolveLinearSystem")
    .add_input_port(Port::ConstructionParams("Matrix", "ForwardMatrix", "blue"))
    .add_input_port(Port::ConstructionParams("Matrix", "RHS", "blue"))
    .add_output_port(Port::ConstructionParams("Matrix", "Solution", "blue"))
    .build();
  ASSERT_EQ(2, module->num_input_ports());
  ASSERT_EQ(1, module->num_output_ports());
  ASSERT_EQ("SolveLinearSystem", module->get_module_name());
}
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Modules/Python/PythonInterfaceParser.h>
#include <Modules/Python/InterfaceWithPython.h>
#include <Dataflow/State/SimpleMapModuleState.h>

using namespace SCIRun::Core::Algorithms::Python;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Modules::Python;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;

TEST(PythonInterfaceParserTests, Basic)
{
  std::string moduleId = "InterfaceWithPython:0";
  std::vector<std::string> portIds = {"InputString:0"};
  auto state = boost::make_shared<SimpleMapModuleState>();
  for (const auto& outputVarToCheck : InterfaceWithPython::outputNameParameters())
  {
    state->setValue(Name(outputVarToCheck), std::string(""));
  }
  state->setValue(Name(portIds[0]), std::string("str1"));
  state->setValue(Name("PythonOutputString1Name"), std::string("out1"));

  PythonInterfaceParser parser(moduleId, state, portIds);

  std::string code =
    "s = str1\n"
    "out1 = s + \"!12!\"\n";

  auto convertedCode = parser.convertStandardCodeBlock(code);

  std::cout << convertedCode << std::endl;

 std::string expectedCode =
  "s = scirun_get_module_input_value(\"InterfaceWithPython:0\", \"InputString:0\")\n"
  "scirun_set_module_transient_state(\"InterfaceWithPython:0\",\"out1\",s + \"!12!\")\n\n";

  ASSERT_EQ(convertedCode, expectedCode);
}

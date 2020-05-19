/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


std::unique_ptr<InterfaceWithPythonCodeTranslatorImpl> makeParser()
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

  std::unique_ptr<InterfaceWithPythonCodeTranslatorImpl> parser(new InterfaceWithPythonCodeTranslatorImpl(
    [=]() { return moduleId; }, state, InterfaceWithPython::outputNameParameters()));
  parser->updatePorts(portIds);
  return parser;
}

TEST(PythonInterfaceParserTests, Basic)
{
  auto parser = makeParser();

  std::string code =
    "s = str1\n"
    "out1 = s + \"!12!\"\n";

  auto convertedCode = parser->translateIOSyntax({code, false});

  std::cout << convertedCode.code << std::endl;
  EXPECT_FALSE(convertedCode.isMatlab);

 std::string expectedCode =
  "s = scirun_get_module_input_value(\"InterfaceWithPython:0\", \"InputString:0\")\n"
  "scirun_set_module_transient_state(\"InterfaceWithPython:0\",\"out1\",s + \"!12!\")\n\n";

  ASSERT_EQ(convertedCode.code, expectedCode);
}

TEST(PythonInterfaceParserTests, BasicActual)
{
  auto parser = makeParser();

  std::string code =
    "s = str1\n"
    "out1 = s + \"!12!\"\n";

  auto convertedCode = parser->translateIOSyntax({code, false});

  std::cout << convertedCode.code << std::endl;
  EXPECT_FALSE(convertedCode.isMatlab);

 std::string expectedCode =
  "s = scirun_get_module_input_value(\"InterfaceWithPython:0\", \"InputString:0\")\n"
  "scirun_set_module_transient_state(\"InterfaceWithPython:0\",\"out1\",s + \"!12!\")\n\n";

  ASSERT_EQ(convertedCode.code, expectedCode);

  auto intermediate = parser->extractSpecialBlocks(code);

  std::cout << intermediate.begin()->code << std::endl;
  auto readyToConvert = parser->concatenateAndTranslateMatlabBlocks(intermediate);
  std::cout << readyToConvert.code << std::endl;
  auto convertedCode2 = parser->translateIOSyntax(readyToConvert);
  std::cout << convertedCode2.code << std::endl;

  ASSERT_EQ(convertedCode2.code, expectedCode + "\n");
}

TEST(PythonInterfaceParserTests, CanExtractSingleMatlabBlock)
{
  auto parser = makeParser();

  std::string code = "%%\na = 1\n%%";
  auto blocks = parser->extractSpecialBlocks(code);

  ASSERT_EQ(1, blocks.size());
  EXPECT_EQ("a = 1", blocks.begin()->code);
  EXPECT_TRUE(blocks.begin()->isMatlab);
}

TEST(PythonInterfaceParserTests, CanPreserveNormalPython)
{
  auto parser = makeParser();

  std::string code =
    "s = str1\n"
    "out1 = s + \"!12!\"\n";
  auto blocks = parser->extractSpecialBlocks(code);

  ASSERT_EQ(1, blocks.size());
  EXPECT_EQ(code, blocks.begin()->code);
  EXPECT_FALSE(blocks.begin()->isMatlab);
}

TEST(PythonInterfaceParserTests, CanExtractMultipleMatlabBlocks)
{
  auto parser = makeParser();

  std::string code =
    "%%\n"
    "a = 1\n"
    "%%\n"
    "%%\n"
    "b = 2\n"
    "%%\n";
  auto blocks = parser->extractSpecialBlocks(code);

  std::cout << "printing out" << std::endl;
  for (const auto& block : blocks)
  {
    std::cout << block.isMatlab << " : " << block.code << std::endl;
  }

  ASSERT_EQ(2, blocks.size());
  auto blockIterator = blocks.begin();
  EXPECT_EQ("a = 1", blockIterator->code);
  EXPECT_TRUE(blockIterator->isMatlab);
  blockIterator++;
  EXPECT_EQ("b = 2", blockIterator->code);
  EXPECT_TRUE(blockIterator->isMatlab);
}

TEST(PythonInterfaceParserTests, CanExtractMultipleMatlabBlocksBetweenNormalBlocks)
{
  auto parser = makeParser();

  std::string code =
    "s = str1\n"
    "print(s)\n"
    "%%\n"
    "a = 1\n"
    "%%\n"
    "s = s + '.'\n"
    "%%\n"
    "b = 2\n"
    "%%\n"
    "print(b)\n";

  auto blocks = parser->extractSpecialBlocks(code);

  std::cout << "printing out" << std::endl;
  for (const auto& block : blocks)
  {
    std::cout << block.isMatlab << " : " << block.code << std::endl;
  }

  ASSERT_EQ(5, blocks.size());
  auto blockIterator = blocks.begin();
  EXPECT_EQ("s = str1\nprint(s)", blockIterator->code);
  EXPECT_FALSE(blockIterator->isMatlab);
  blockIterator++;
  EXPECT_EQ("a = 1", blockIterator->code);
  EXPECT_TRUE(blockIterator->isMatlab);
  blockIterator++;
  EXPECT_EQ("s = s + '.'", blockIterator->code);
  EXPECT_FALSE(blockIterator->isMatlab);
  blockIterator++;
  EXPECT_EQ("b = 2", blockIterator->code);
  EXPECT_TRUE(blockIterator->isMatlab);
  blockIterator++;
  EXPECT_EQ("print(b)", blockIterator->code);
  EXPECT_FALSE(blockIterator->isMatlab);
}

#if 0
TEST(PythonInterfaceParserTests, CanConcatenateNormalBlocksUntilMatlabConversionWorks)
{
  auto parser = makeParser();

  std::string code =
    "s = str1\n"
    "print(s)\n"
    "%%\n"
    "a = 1\n"
    "%%\n"
    "s = s + '.'\n"
    "%%\n"
    "b = 2\n"
    "%%\n"
    "print(b)\n";

  auto blocks = parser->extractSpecialBlocks(code);

  auto regularPython = parser->concatenateNormalBlocks(blocks);
  EXPECT_FALSE(regularPython.isMatlab);
  std::cout << regularPython.code << std::endl;

  std::string expectedRegularPython =
    "s = str1\n"
    "print(s)\n"
    "s = s + '.'\n"
    "print(b)\n";

  EXPECT_EQ(expectedRegularPython, regularPython.code);
}
#endif

TEST(PythonInterfaceParserTests, CanTranslateFirstSpecificMatlabBlock)
{
  auto parser = makeParser();

  std::string code =
    "%%\n"
    "ofield = scirun_test_field(field1)\n"
    "%%\n";

  auto intermediate = parser->extractSpecialBlocks(code);
  std::cout << intermediate.begin()->code << std::endl;
  auto readyToConvert = parser->concatenateAndTranslateMatlabBlocks(intermediate);
  std::cout << readyToConvert.code << std::endl;
  auto convertedCode2 = parser->translateIOSyntax(readyToConvert);
  std::cout << convertedCode2.code << std::endl;

  EXPECT_TRUE(convertedCode2.isMatlab);

  static std::string expectedCode =
  "__field1 = convertfieldtomatlab(field1)\n"
  "__ofield = __eng.scirun_test_field(__field1, nargout=1)\n"
  "ofield = convertfieldtopython(__ofield)\n";

  ASSERT_EQ(convertedCode2.code, expectedCode + "\n");
}

/*

[PYTHON] x, y = f(a, b, c)
[PYTHON]
__field1=convertfieldtomatlab(a)
__field2=convertfieldtomatlab( b)
__field3=convertfieldtomatlab( c)
__x, __y = eng.f(__field1, __field2, __field3, nargout=2)
x = convertfieldtopython(__x)
y = convertfieldtopython(__y)

*/


TEST(PythonInterfaceParserTests, CanTranslateBasicMatlabBlock)
{
  auto parser = makeParser();

  std::string code =
    "%%\n"
    "o1 = scirun_test_field_1(i1)\n"
    "%%\n";

  auto convertedCode = parser->translate(code);
  EXPECT_TRUE(convertedCode.isMatlab);

  static std::string expectedCode =
  "__i1 = convertfieldtomatlab(i1)\n"
  "__o1 = __eng.scirun_test_field_1(__i1, nargout=1)\n"
  "o1 = convertfieldtopython(__o1)\n";

  ASSERT_EQ(convertedCode.code, expectedCode + "\n");
}

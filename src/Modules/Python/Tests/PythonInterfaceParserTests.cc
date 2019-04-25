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

std::unique_ptr<PythonInterfaceParser> makeParser()
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

  std::unique_ptr<PythonInterfaceParser> parser(new PythonInterfaceParser(moduleId, state, portIds));
  return parser;
}

TEST(PythonInterfaceParserTests, Basic)
{
  auto parser = makeParser();

  std::string code =
    "s = str1\n"
    "out1 = s + \"!12!\"\n";

  auto convertedCode = parser->convertStandardCodeBlock({code, false});

  std::cout << convertedCode << std::endl;

 std::string expectedCode =
  "s = scirun_get_module_input_value(\"InterfaceWithPython:0\", \"InputString:0\")\n"
  "scirun_set_module_transient_state(\"InterfaceWithPython:0\",\"out1\",s + \"!12!\")\n\n";

  ASSERT_EQ(convertedCode, expectedCode);
}

TEST(PythonInterfaceParserTests, BasicActual)
{
  auto parser = makeParser();

  std::string code =
    "s = str1\n"
    "out1 = s + \"!12!\"\n";

  auto convertedCode = parser->convertStandardCodeBlock({code, false});

  std::cout << convertedCode << std::endl;

 std::string expectedCode =
  "s = scirun_get_module_input_value(\"InterfaceWithPython:0\", \"InputString:0\")\n"
  "scirun_set_module_transient_state(\"InterfaceWithPython:0\",\"out1\",s + \"!12!\")\n\n";

  ASSERT_EQ(convertedCode, expectedCode);

  auto intermediate = parser->extractSpecialBlocks(code);

  std::cout << intermediate.begin()->code << std::endl;
  auto readyToConvert = parser->concatenateNormalBlocks(intermediate);
  std::cout << readyToConvert.code << std::endl;
  auto convertedCode2 = parser->convertStandardCodeBlock(readyToConvert);
  std::cout << convertedCode2 << std::endl;

  ASSERT_EQ(convertedCode2, expectedCode + "\n");
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

  //FAIL() << "todo";
}

/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Math/CreateMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::Networks;

class CreateMatrixModuleTest : public ModuleTest
{
protected:
  DenseMatrixHandle parse(const std::string& text, const std::string& delimiter)
  {
    UseRealModuleStateFactory f;
    auto module = makeModule("CreateMatrix");
    module->get_state()->setValue(Parameters::TextEntry, text);
    module->get_state()->setValue(Parameters::MatrixTextDelimiter, delimiter);
    module->execute();
    return std::dynamic_pointer_cast<DenseMatrix>(getDataOnThisOutputPort(module, 0));
  }
};

TEST_F(CreateMatrixModuleTest, ParsesSpaceDelimitedEntry)
{
  auto matrix = parse("1 2\n3 4", "Space");
  ASSERT_TRUE(matrix != nullptr);
  ASSERT_EQ(2, matrix->nrows());
  ASSERT_EQ(2, matrix->ncols());
  EXPECT_EQ(1.0, (*matrix)(0, 0));
  EXPECT_EQ(4.0, (*matrix)(1, 1));
}

TEST_F(CreateMatrixModuleTest, ParsesCommaDelimitedEntry)
{
  auto matrix = parse("1,2\n3,4", "Comma");
  ASSERT_TRUE(matrix != nullptr);
  ASSERT_EQ(2, matrix->nrows());
  ASSERT_EQ(2, matrix->ncols());
  EXPECT_EQ(2.0, (*matrix)(0, 1));
  EXPECT_EQ(4.0, (*matrix)(1, 1));
}

TEST_F(CreateMatrixModuleTest, ParsesCommaDelimitedEntryWithSpaces)
{
  auto matrix = parse("1, 2\n3, 4", "Comma");
  ASSERT_TRUE(matrix != nullptr);
  ASSERT_EQ(2, matrix->nrows());
  ASSERT_EQ(2, matrix->ncols());
  EXPECT_EQ(3.0, (*matrix)(1, 0));
}

TEST_F(CreateMatrixModuleTest, ParsesSemicolonDelimitedEntry)
{
  auto matrix = parse("1;2\n3;4", "Semicolon");
  ASSERT_TRUE(matrix != nullptr);
  ASSERT_EQ(2, matrix->nrows());
  ASSERT_EQ(2, matrix->ncols());
  EXPECT_EQ(2.0, (*matrix)(0, 1));
}

TEST_F(CreateMatrixModuleTest, ParsesTabDelimitedEntry)
{
  auto matrix = parse("1\t2\n3\t4", "Tab");
  ASSERT_TRUE(matrix != nullptr);
  ASSERT_EQ(2, matrix->nrows());
  ASSERT_EQ(2, matrix->ncols());
  EXPECT_EQ(4.0, (*matrix)(1, 1));
}

TEST_F(CreateMatrixModuleTest, UnknownDelimiterFallsBackToSpaceBehavior)
{
  // Networks saved before the delimiter option existed have no delimiter
  // state; parsing must behave exactly as it always has.
  auto matrix = parse("5 6\n7 8", "");
  ASSERT_TRUE(matrix != nullptr);
  ASSERT_EQ(2, matrix->nrows());
  ASSERT_EQ(2, matrix->ncols());
  EXPECT_EQ(8.0, (*matrix)(1, 1));
}

TEST_F(CreateMatrixModuleTest, EmptyEntryProducesEmptyMatrix)
{
  auto matrix = parse("", "Comma");
  ASSERT_TRUE(matrix != nullptr);
  EXPECT_EQ(0, matrix->nrows());
}

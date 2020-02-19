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


#include <Core/Algorithms/Describe/DescribeDatatype.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/MatrixTypeConversions.h>


using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Testing;
using namespace SCIRun::Core::Algorithms::General;

TEST(DescribeDatatypeAlgorithmTests, CanDescribeString)
{
  DescribeDatatype algo;
  StringHandle s(new String("foo"));

  auto desc = algo.describe(s);

  EXPECT_EQ("[String Data]  Contents:\nfoo", desc);
}

TEST(DescribeDatatypeAlgorithmTests, CanDescribeMatrix)
{
  DescribeDatatype algo;

  auto m = MAKE_DENSE_MATRIX_HANDLE((1.0,2.0,3.0)(0.0,0.0,0.0)(-1.0,-2.0,-3.0));

  auto desc = algo.describe(m);

  EXPECT_EQ("[Matrix Data] Info:\nType:\tDenseMatrix\n# Rows:\t3\n# Columns:\t3\n# Elements:\t9\nMinimum:\t-3\nMaximum:\t3\n", desc);
}

TEST(DescribeDatatypeAlgorithmTests, CanDescribeField)
{
  DescribeDatatype algo;

  auto field = CreateEmptyLatVol();
  auto desc = algo.describe(field);

  EXPECT_EQ("[Field Data] Info:\nType: GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,HexTrilinearLgn<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > \nCenter: [0 0 0]\nSize: [2 2 2]\nData min,max: 0 , 0\n# nodes: 60\n# elements: 24\n# data: 60\nData location: Nodes (linear basis)\nDims (x,y,z): [3 4 5]\nGeometric size: 8\n", desc);
}

TEST(DescribeDatatypeAlgorithmTests, CanDescribeComplexDenseMatrix)
{
  DescribeDatatype algo;

  auto m = boost::make_shared<ComplexDenseMatrix>(2, 2);
  *m << complex{ 1, 2 }, complex{ 3, -1 }, complex{ 0, 1 }, complex{ -2, -1 };

  auto desc = algo.describe(m);

  EXPECT_EQ("[Complex Matrix Data] Info:\nType:\t\tComplexDenseMatrix\n# Rows:\t\t2\n# Columns:\t\t2\n# Elements:\t\t4\nMinimum (by norm):\t(0,1)\nMaximum (by norm):\t(3,-1)\n", desc);
}

TEST(DescribeDatatypeAlgorithmTests, CanDescribeComplexSparseMatrix)
{
  DescribeDatatype algo;

  ComplexDenseMatrix m(2, 2);
  m << complex{ 1, 2 }, complex{ 3, -1 }, complex{ 0, 1 }, complex{ -2, -1 };
  auto s = convertMatrix::fromDenseToSparse(m);

  auto desc = algo.describe(s);

  EXPECT_EQ("[Complex Matrix Data] Info:\nType:\t\tComplexSparseRowMatrix\n# Rows:\t\t2\n# Columns:\t\t2\n# Elements:\t\t4\nMinimum (by norm):\t(0,1)\nMaximum (by norm):\t(3,-1)\n", desc);
}

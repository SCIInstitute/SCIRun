/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
 
#include <Core/Algorithms/Describe/DescribeDatatype.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>


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

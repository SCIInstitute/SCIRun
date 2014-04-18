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
 
#include <gtest/gtest.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateGradientsAlgo.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

void runTest(.../*int basis, const std::string& expectedBasisTypeTemplate, const std::string& expectedBasisString, int expectedNumData*/)
{
  //FieldInformation lfi("LatVolMesh", basis, "double");

  //size_type sizex = 3, sizey = 4, sizez = 5;
  //Point minb(-1.0, -1.0, -1.0);
  //Point maxb(1.0, 1.0, 1.0);
  //MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
  //FieldHandle ofh = CreateField(lfi,mesh);
  //ofh->vfield()->clear_all_values();

  //ReportFieldInfoAlgorithm algo;

  //auto info = algo.run(ofh);

  //EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point> > ," + expectedBasisTypeTemplate + "<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > ", info.type);
  //EXPECT_EQ(0, info.dataMin);
  //EXPECT_EQ(0, info.dataMax);
  //EXPECT_EQ(expectedNumData, info.numdata_);
  //EXPECT_EQ(sizex * sizey * sizez, info.numnodes_);
  //EXPECT_EQ((sizex-1) * (sizey-1) * (sizez-1), info.numelements_);
  //EXPECT_EQ(expectedBasisString, info.dataLocation);
}

TEST(CalculateGradientsAlgoTests, DISABLED_Foo)
{
  FAIL() << "TODO";
  runTest(-1, "NoDataBasis", "None (nodata basis)", 0);
  runTest(0, "ConstantBasis", "Cells (constant basis)", 24);
  runTest(1, "HexTrilinearLgn", "Nodes (linear basis)", 60);
}

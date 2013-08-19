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
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundary.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

TEST(GetFieldBoundaryTest, LatVolBoundary)
{
  FieldInformation lfi("LatVolMesh", -1, "double");

  size_type sizex = 3, sizey = 4, sizez = 5;
  Point minb(-1.0, -1.0, -1.0);
  Point maxb(1.0, 1.0, 1.0);
  MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
  FieldHandle ofh = CreateField(lfi,mesh);
  ofh->vfield()->clear_all_values();

  GetFieldBoundaryAlgo algo;

  FieldHandle boundary;
  MatrixHandle mapping;
  algo.run(ofh, boundary, mapping);

  ASSERT_TRUE(boundary);
  ASSERT_TRUE(mapping);
/*
  EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,NoDataBasis<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > ", info.type);
  EXPECT_EQ(0, info.dataMin);
  EXPECT_EQ(0, info.dataMax);
  EXPECT_EQ(0, info.numdata_);
  EXPECT_EQ(sizex * sizey * sizez, info.numnodes_);
  EXPECT_EQ((sizex-1) * (sizey-1) * (sizez-1), info.numelements_);
  EXPECT_EQ("None (nodata basis)", info.dataLocation);*/
}
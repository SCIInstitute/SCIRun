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

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

// TODO: test with more field types...

namespace
{
void runTest(int basis, int expectedMatrixRows, int expectedMatrixColumns, const std::string& expectedMatrixString = "")
{
  std::cout << "Basis # " << basis << std::endl;
  FieldInformation lfi("LatVolMesh", basis, "double");

  size_type sizex = 2, sizey = 3, sizez = 4;
  Point minb(-1.0, -1.0, -1.0);
  Point maxb(1.0, 1.0, 1.0);
  MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
  FieldHandle ofh = CreateField(lfi,mesh);
  ofh->vfield()->clear_all_values();

  GetFieldBoundaryAlgo algo;

  FieldHandle boundary;
  MatrixHandle mapping;
  algo.run(ofh, boundary, mapping);

  ASSERT_TRUE(boundary.get() != nullptr);

  /// @todo: need assertions on boundary field

  if (basis != -1)
  {
    ASSERT_TRUE(mapping != nullptr);
    EXPECT_EQ(expectedMatrixRows, mapping->nrows());
    EXPECT_EQ(expectedMatrixColumns, mapping->ncols());
    EXPECT_EQ(expectedMatrixString, matrix_to_string(*convertMatrix::toDense(mapping)));
  }
}

const std::string matrixCells =
  "1 0 0 0 0 0 \n"
  "1 0 0 0 0 0 \n"
  "1 0 0 0 0 0 \n"
  "1 0 0 0 0 0 \n"
  "0 1 0 0 0 0 \n"
  "0 1 0 0 0 0 \n"
  "0 1 0 0 0 0 \n"
  "0 1 0 0 0 0 \n"
  "0 0 1 0 0 0 \n"
  "0 0 1 0 0 0 \n"
  "0 0 1 0 0 0 \n"
  "0 0 0 1 0 0 \n"
  "0 0 0 1 0 0 \n"
  "0 0 0 1 0 0 \n"
  "0 0 0 0 1 0 \n"
  "0 0 0 0 1 0 \n"
  "0 0 0 0 1 0 \n"
  "0 0 0 0 1 0 \n"
  "0 0 0 0 0 1 \n"
  "0 0 0 0 0 1 \n"
  "0 0 0 0 0 1 \n"
  "0 0 0 0 0 1 \n";

const std::string matrixNodes =
  "1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 \n"
  "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 \n";
}

TEST(GetFieldBoundaryTest, LatVolBoundary)
{
  runTest(0, 22, 6, matrixCells);
  runTest(-1, 0, 0);
  runTest(1, 24, 24, matrixNodes);

  /*
  EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,NoDataBasis<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > ", info.type);
  EXPECT_EQ(0, info.dataMin);
  EXPECT_EQ(0, info.dataMax);
  EXPECT_EQ(0, info.numdata_);
  EXPECT_EQ(sizex * sizey * sizez, info.numnodes_);
  EXPECT_EQ((sizex-1) * (sizey-1) * (sizez-1), info.numelements_);
  EXPECT_EQ("None (nodata basis)", info.dataLocation);*/
}

TEST(GetFieldBoundaryTest, CanLogErrorMessage)
{
  GetFieldBoundaryAlgo algo;

  FieldHandle input, output;
  MatrixHandle mapping;

  EXPECT_FALSE(algo.run(input, output, mapping));

  EXPECT_FALSE(algo.run(input, output));
}

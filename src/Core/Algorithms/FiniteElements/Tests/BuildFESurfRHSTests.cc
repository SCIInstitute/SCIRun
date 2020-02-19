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

   Author:              Moritz Dannhauer
   Last Modification:   November 2 2017
*/

#include <gtest/gtest.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildRHS/BuildFESurfRHS.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/ConvertMatrixType.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

class  BuildFESurfRHSTests : public ::testing::Test
{
protected:

  FieldHandle CreateEmptyLatVol(size_type sizex = 3, size_type sizey = 4, size_type sizez = 5, data_info_type type=DOUBLE_E)
  {
    FieldInformation lfi(LATVOLMESH_E, LINEARDATA_E, type);
    Point minb(-1.0, -1.0, -1.0);
    Point maxb(1.0, 1.0, 1.0);
    MeshHandle mesh = CreateMesh(lfi, sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    return ofh;
  }
};

TEST_F(BuildFESurfRHSTests, CreateLatVolTest)
{
  BuildFESurfRHSAlgo algo;
  auto size=3,len=size*size*size;
  FieldHandle latVol = CreateEmptyLatVol(size, size, size);

  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(8,3));
  double t=0.22984884706593014525;
  (*m)(0,0) =  t; (*m)(0,1) =  t; (*m)(0,2) =  t;
  (*m)(1,0) = -t; (*m)(1,1) =  t; (*m)(1,2) = -t;
  (*m)(2,0) = -t; (*m)(2,1) = -t; (*m)(2,2) =  t;
  (*m)(3,0) =  t; (*m)(3,1) = -t; (*m)(3,2) = -t;
  (*m)(4,0) =  t; (*m)(4,1) = -t; (*m)(4,2) = -t;
  (*m)(5,0) = -t; (*m)(5,1) = -t; (*m)(5,2) =  t;
  (*m)(6,0) = -t; (*m)(6,1) =  t; (*m)(6,2) = -t;
  (*m)(7,0) =  t; (*m)(7,1) =  t; (*m)(7,2) =  t;
  SetFieldDataAlgo algo2;
  FieldHandle latVol_with_data = algo2.runImpl(latVol, m);

  FieldHandle output_field;
  MatrixHandle RHSMatrix;
  algo.run(latVol_with_data,output_field, RHSMatrix);
  double expected_res[] = {0.17238663529944761588,0.1149244235329650865,0.057462211766482536313,0.1149244235329650865,6.9388939039072283776e-18,-0.1149244235329650865,
0.057462211766482536313,0.1149244235329650865,-0.057462211766482543251,0.1149244235329650865,6.9388939039072283776e-18,0.11492442353296507263,
6.9388939039072283776e-18,0.0,6.9388939039072283776e-18,-0.1149244235329650865,6.9388939039072283776e-18,-0.1149244235329650865,0.057462211766482543251,
-0.1149244235329650865,-0.057462211766482543251,0.11492442353296507263,6.9388939039072283776e-18,-0.1149244235329650865,-0.057462211766482543251,
-0.1149244235329650865,-0.17238663529944764363 };

  auto rhs = convertMatrix::toColumn(RHSMatrix);

  for(int i=0;i<len; i++)
  {
   EXPECT_NEAR((double)(*rhs)(i),(double)expected_res[i],0.000001);
  }
}

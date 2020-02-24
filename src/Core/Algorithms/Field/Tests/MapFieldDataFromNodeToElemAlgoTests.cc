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
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromNodeToElem.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

  FieldHandle CreateTetMesh1()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/test_mapfielddatafromelemtonode.fld");
  }

  FieldHandle CreateTetMesh2()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/test_mapfielddatafromnodetoelem.fld");
  }

  DenseMatrixHandle test_MapFieldDataFromNodeToElemFLD_IntAvr()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (3.325000000000000177635684,0.0)
      (3.29166666666666651863693,0.0)
      (3.533333333333333214909544,0.0)
      (3.325000000000000177635684,0.0)
      (3.741666666666666252183404,0.0)
      (3.783333333333333214909544,0.0));
  }

  DenseMatrixHandle test_mapfielddatafromnodetoelemFLD_Min()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (2.5,0.0)
      (2.0,0.0)
      (3.33333333333333333333333333,0.0)
      (2.5,0.0)
      (3.33333333333333333333333333,0.0)
      (3.5,0.0));
  }

  DenseMatrixHandle test_mapfielddatafromnodetoelemFLD_Max()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (3.8,0.0)
      (4.33333333333333333333333333,0.0)
      (3.8,0.0)
      (3.8,0.0)
      (4.33333333333333333333333333,0.0)
      (4.33333333333333333333333333,0.0));
  }

  DenseMatrixHandle test_mapfielddatafromnodetoelemFLD_Sum()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (13.3,0.0)
      (13.1666666666666666666666666,0.0)
      (14.1333333333333333333333333,0.0)
      (13.3,0.0)
      (14.9666666666666666666666666,0.0)
      (15.1333333333333333333333333,0.0));
  }

  DenseMatrixHandle test_mapfielddatafromnodetoelemFLD_Med()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (3.5,0.0)
      (3.5,0.0)
      (3.5,0.0)
      (3.5,0.0)
      (3.8,0.0)
      (3.8,0.0));
  }


TEST(MapFieldDataFromNodeToElemAlgoTests,TestInterpolationAverage)
{
 MapFieldDataFromNodeToElemAlgo algo;

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Average");

 FieldHandle input=CreateTetMesh2();

 FieldHandle result = algo.runImpl(input);

 DenseMatrixHandle expected=test_MapFieldDataFromNodeToElemFLD_IntAvr();

 ASSERT_TRUE(result->vfield()->num_values() == 6);

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp=0;
   result->vfield()->get_value(tmp,idx);
   EXPECT_NEAR( (*expected)(idx,0),tmp, 1e-5);
 }

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Interpolation");

 FieldHandle input2=CreateTetMesh2();

 FieldHandle result2 = algo.runImpl(input);

 ASSERT_TRUE(result2->vfield()->num_values() == 6);

 for (VMesh::Elem::index_type idx = 0; idx < result2->vfield()->num_values(); idx++)
 {
   double tmp=0;
   result2->vfield()->get_value(tmp,idx);
   EXPECT_NEAR( (*expected)(idx,0),tmp, 1e-5);
 }

}

TEST(MapFieldDataFromNodeToElemAlgoTests, TestDataAlreadyOnElems)
{

 MapFieldDataFromNodeToElemAlgo algo;

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Average");

 FieldHandle input=CreateTetMesh1();

 FieldHandle result;
 try
 {
  result = algo.runImpl(input);
 } catch (...) {}

}


TEST(MapFieldDataFromNodeToElemAlgoTests, TestMin)
{
 MapFieldDataFromNodeToElemAlgo algo;

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Min");

 FieldHandle input=CreateTetMesh2();

 FieldHandle result = algo.runImpl(input);

 DenseMatrixHandle expected=test_mapfielddatafromnodetoelemFLD_Min();

 ASSERT_TRUE(result->vfield()->num_values() == 6);

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp=0;
   result->vfield()->get_value(tmp,idx);
   EXPECT_NEAR( (*expected)(idx,0),tmp, 1e-5);
 }

}


TEST(MapFieldDataFromNodeToElemAlgoTests, TestMax)
{

 MapFieldDataFromNodeToElemAlgo algo;

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Max");

 FieldHandle input=CreateTetMesh2();

 FieldHandle result = algo.runImpl(input);

 DenseMatrixHandle expected=test_mapfielddatafromnodetoelemFLD_Max();

 ASSERT_TRUE(result->vfield()->num_values() == 6);

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp=0;
   result->vfield()->get_value(tmp,idx);
   EXPECT_NEAR( (*expected)(idx,0),tmp, 1e-5);
 }

}


TEST(MapFieldDataFromNodeToElemAlgoTests, TestSum)
{

 MapFieldDataFromNodeToElemAlgo algo;

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Sum");

 FieldHandle input=CreateTetMesh2();

 FieldHandle result = algo.runImpl(input);

 DenseMatrixHandle expected=test_mapfielddatafromnodetoelemFLD_Sum();

 ASSERT_TRUE(result->vfield()->num_values() == 6);

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp=0;
   result->vfield()->get_value(tmp,idx);
   EXPECT_NEAR( (*expected)(idx,0),tmp, 1e-5);
 }

}


TEST(MapFieldDataFromNodeToElemAlgoTests, TestMedian)
{

 MapFieldDataFromNodeToElemAlgo algo;

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Median");

 FieldHandle input=CreateTetMesh2();

 FieldHandle result = algo.runImpl(input);

 DenseMatrixHandle expected=test_mapfielddatafromnodetoelemFLD_Med();

 ASSERT_TRUE(result->vfield()->num_values() == 6);

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp=0;
   result->vfield()->get_value(tmp,idx);
   EXPECT_NEAR( (*expected)(idx,0),tmp, 1e-5);
 }

}

TEST(MapFieldDataFromNodeToElemAlgoTests, TestNone)
{

 MapFieldDataFromNodeToElemAlgo algo;

 algo.setOption(MapFieldDataFromNodeToElemAlgo::Method, "None");

 FieldHandle input=CreateTetMesh2();

 FieldHandle result;
 try
 {
  result = algo.runImpl(input);
 } catch (...) {}

 if(result)
 {
   FAIL() << " ERROR: THIS MESSAGE SHOULD NOT APPEAR! " << std::endl;
 }

}

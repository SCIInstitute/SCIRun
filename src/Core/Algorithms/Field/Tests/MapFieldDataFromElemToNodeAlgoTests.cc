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
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromElemToNode.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;


  FieldHandle TetMesh1()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/test_mapfielddatafromelemtonode.fld");
  }

   FieldHandle TetMesh2()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/test_mapfielddatafromnodetoelem.fld");
  }

  DenseMatrixHandle test_mapfielddatafromelemtonodeFLD_Min()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (1,0)
      (3,0)
      (2,0)
      (2,0)
      (1,0)
      (1,0)
      (1,0)
      (2,0));
  }

  DenseMatrixHandle test_mapfielddatafromelemtonodeFLD_Max()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (6,0)
      (4,0)
      (5,0)
      (2,0)
      (6,0)
      (4,0)
      (6,0)
      (6,0));
  }

  DenseMatrixHandle test_mapfielddatafromelemtonodeFLD_Sum()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (21,0)
      (7,0)
      (10,0)
      (2,0)
      (7,0)
      (5,0)
      (19,0)
      (13,0));
  }

  DenseMatrixHandle test_mapfielddatafromelemtonodeFLD_Med()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (4,0)
      (4,0)
      (3,0)
      (2,0)
      (6,0)
      (4,0)
      (4,0)
      (5,0));
  }


TEST(MapFieldDataFromElemToNode, TetMeshTestInterpolateWithFile)
{
 MapFieldDataFromElemToNodeAlgo algo;

 FieldHandle  result = algo.runImpl(TetMesh1());

 ASSERT_TRUE(result->vfield()->num_values() == 8);

 FieldHandle expected_result = TetMesh2();

 ASSERT_TRUE(expected_result->vfield()->num_values() == 8);

 DenseMatrixHandle output(new DenseMatrix(8, 1));

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   result->vfield()->get_value((*output)(idx, 0),idx);
 }

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp = 0;
   expected_result->vfield()->get_value(tmp,idx);
   EXPECT_NEAR( tmp,(*output)(idx, 0), 1e-16);
 }

}

TEST(MapFieldDataFromElemToNode, TetMeshTestMin)
{

 MapFieldDataFromElemToNodeAlgo algo;

 algo.setOption(MapFieldDataFromElemToNodeAlgo::Method, "Min");

 FieldHandle  result = algo.runImpl(TetMesh1());

 ASSERT_TRUE(result->vfield()->num_values() == 8);

 DenseMatrixHandle output(new DenseMatrix(8, 1));

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   result->vfield()->get_value((*output)(idx, 0),idx);
 }

 DenseMatrixHandle expected_result_min = test_mapfielddatafromelemtonodeFLD_Min();

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp = (*expected_result_min)(idx,0);
   EXPECT_NEAR( tmp,(*output)(idx, 0), 1e-16);
 }

}


TEST(MapFieldDataFromElemToNode, TetMeshTestMax)
{
 MapFieldDataFromElemToNodeAlgo algo;

 algo.setOption(MapFieldDataFromElemToNodeAlgo::Method, "Max");

 FieldHandle  result = algo.runImpl(TetMesh1());

 ASSERT_TRUE(result->vfield()->num_values() == 8);

 DenseMatrixHandle output(new DenseMatrix(8, 1));

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   result->vfield()->get_value((*output)(idx, 0),idx);
 }

 DenseMatrixHandle expected_result_max = test_mapfielddatafromelemtonodeFLD_Max();

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp = (*expected_result_max)(idx,0);
   EXPECT_NEAR( tmp,(*output)(idx, 0), 1e-16);
 }

}


TEST(MapFieldDataFromElemToNode, TetMeshTestSum)
{
 MapFieldDataFromElemToNodeAlgo algo;

 algo.setOption(MapFieldDataFromElemToNodeAlgo::Method, "Sum");

 FieldHandle  result = algo.runImpl(TetMesh1());

 ASSERT_TRUE(result->vfield()->num_values() == 8);

 DenseMatrixHandle output(new DenseMatrix(8, 1));

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   result->vfield()->get_value((*output)(idx, 0),idx);
 }

 DenseMatrixHandle expected_result_sum = test_mapfielddatafromelemtonodeFLD_Sum();

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp = (*expected_result_sum)(idx,0);
   EXPECT_NEAR( tmp,(*output)(idx, 0), 1e-16);
 }

}


TEST(MapFieldDataFromElemToNode, TetMeshTestMed)
{

 MapFieldDataFromElemToNodeAlgo algo;

 algo.setOption(MapFieldDataFromElemToNodeAlgo::Method, "Median");

 FieldHandle  result = algo.runImpl(TetMesh1());

 ASSERT_TRUE(result->vfield()->num_values() == 8);

 DenseMatrixHandle output(new DenseMatrix(8, 1));

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   result->vfield()->get_value((*output)(idx, 0),idx);
 }

 DenseMatrixHandle expected_result_med = test_mapfielddatafromelemtonodeFLD_Med();

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   double tmp = (*expected_result_med)(idx,0);
   EXPECT_NEAR( tmp,(*output)(idx, 0), 1e-16);
 }

}

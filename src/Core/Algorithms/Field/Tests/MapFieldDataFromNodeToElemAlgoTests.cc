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
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/MapFieldDataFromNodeToElem.h>
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

  FieldHandle CreateTetMesh1()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "mapfielddatafrom_/test_mapfielddatafromelemtonode.fld");
  }

  
  DenseMatrixHandle test_MapFieldDataFromNodeToElemFLD_IntAvr()
  {
    return MAKE_DENSE_MATRIX_HANDLE(
      (3.325000000000000177635684,0)
      (3.29166666666666651863693,0)
      (3.533333333333333214909544,0)
      (3.325000000000000177635684,0)
      (3.741666666666666252183404,0)
      (3.783333333333333214909544,0));
  }
  
  
  DenseMatrixHandle test_MapFieldDataFromNodeToElemFLD_Min()
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

  DenseMatrixHandle test_MapFieldDataFromNodeToElemFLD_Max()
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
  
  DenseMatrixHandle test_MapFieldDataFromNodeToElemFLD_Sum()
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
  
  DenseMatrixHandle test_MapFieldDataFromNodeToElemFLD_Med()
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
  
TEST(MapFieldDataFromNodeToElemTestIntAvr, TetMeshTest)
{

 MapFieldDataFromNodeToElemAlgo algo;
 
 algo.set_option(MapFieldDataFromNodeToElemAlgo::Method, "average");

 FieldHandle input=CreateTetMesh1();

 FieldHandle result = algo.run(input);
 
 std::cout << result->vfield()->num_values() << std::endl;

 ASSERT_TRUE(result->vfield()->num_values() == 8);
 
 DenseMatrixHandle output(new DenseMatrix(8, 1));

 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 { 
   result->vfield()->get_value((*output)(idx, 0),idx);
 }
 
 DenseMatrixHandle expected_result_min = test_MapFieldDataFromNodeToElemFLD_IntAvr();
 
 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   //double tmp = (*expected_result_min)(idx,0);
   //EXPECT_NEAR( tmp,(*output)(idx, 0), 1e-5);
   std::cout << (*output)(idx, 0) << std::endl;
 } 
 
}


TEST(MapFieldDataFromNodeToElemTestMin, TetMeshTest)
{

 
 
}


TEST(MapFieldDataFromNodeToElemTestMax, TetMeshTest)
{

 
 
}


TEST(MapFieldDataFromNodeToElemTestSum, TetMeshTest)
{

 
}


TEST(MapFieldDataFromNodeToElemTestMed, TetMeshTest)
{


 
}

TEST(MapFieldDataFromNodeToElemTestNone, TetMeshTest)
{


 
}

TEST(MapFieldDataFromNodeToElemTestWrongInterpolationFunction, TetMeshTest)
{
 MapFieldDataFromNodeToElemAlgo algo;
 
 algo.set_option(MapFieldDataFromNodeToElemAlgo::Method, "interpolation");

 
}




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
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Utils/StringUtil.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

//TODO (DAN) : No way to import (ReadField) TriSurf (on nodes or elements) with defined tensors
  
  FieldHandle TriSurfOnNodeVector()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }
  
  FieldHandle TriSurfOnNodeScalar()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }
  
  FieldHandle TriSurfOnElemVector()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_elem/vector/tri_vector_on_elem.fld");
  }
  
  FieldHandle TriSurfOnElemScalar()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_elem/scalar/tri_scalar_on_elem.fld");
  }  
  
  FieldHandle TetMeshOnNodeVector()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }
  
  FieldHandle TetMeshOnNodeScalar()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
   
  FieldHandle TetMeshOnNodeTensor()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_node/tensor/tet_tensor_on_node.fld");
  }
  
  FieldHandle TetMeshOnElemTensor()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_elem/tensor/tet_tensor_on_elem.fld");
  }
  
  FieldHandle TetMeshOnElemVector()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }
  
  FieldHandle TetMeshOnElemScalar()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
    
  FieldHandle PointCloudOnNodeScalar()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/point_cloud/scalar/pts_scalar.fld");
  }

  FieldHandle PointCloudOnNodeVector()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/point_cloud/vector/pts_vector.fld");
  }
  
  FieldHandle PointCloudOnNodeTensor()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "getfielddata/point_cloud/tensor/pts_tensor.fld");
  }
  

  
  DenseMatrixHandle TriSurfOnNodeVectorMat()
  {     
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_node/vector/tri_vector_on_node.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;
  }
  
  DenseMatrixHandle TriSurfOnNodeScalarMat()
  {   
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;   
  }
  
  DenseMatrixHandle TriSurfOnElemVectorMat()
  {     
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_elem/vector/tri_vector_on_elem.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;  
  }
  
  DenseMatrixHandle TriSurfOnElemScalarMat()
  {     
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tri_surf/data_defined_on_elem/scalar/tri_scalar_on_elem.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat; 
  }  
  
  DenseMatrixHandle TetMeshOnNodeVectorMat()
  {   
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;  
  }
  
  DenseMatrixHandle TetMeshOnNodeScalarMat()
  {     
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat; 
  }
   
  DenseMatrixHandle TetMeshOnNodeTensorMat()
  {  
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_node/tensor/tet_tensor_on_node.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat; 
  }
  
  DenseMatrixHandle TetMeshOnElemTensorMat()
  {     
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_elem/tensor/tet_tensor_on_elem.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat; 
  }
  
  DenseMatrixHandle TetMeshOnElemVectorMat()
  {    
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat; 
  }
  
  DenseMatrixHandle TetMeshOnElemScalarMat()
  {    
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;
  }
    
  DenseMatrixHandle PointCloudOnNodeScalarMat()
  {     
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/point_cloud/scalar/pts_scalar.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle PointCloudOnNodeVectorMat()
  {   
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/point_cloud/vector/pts_vector.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;  
  }
  
  DenseMatrixHandle PointCloudOnNodeTensorMat()
  {     
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "getfielddata/point_cloud/tensor/pts_tensor.mat";
   auto mat = matrix_cast::as_dense(algo.run(path.string()));
   return mat;
  }
  


TEST(GetFieldDataTest, TriSurfOnNodeScalar)
{
 
 GetFieldDataAlgo algo;
  
 DenseMatrixHandle result = algo.run(TriSurfOnNodeScalar());
 DenseMatrixHandle expected_result = TriSurfOnNodeScalarMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    }
}


TEST(GetFieldDataTest, TriSurfOnNodeVector)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(TriSurfOnNodeVector());
 DenseMatrixHandle expected_result = TriSurfOnNodeVectorMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    }
 
}

TEST(GetFieldDataTest, TriSurfOnElemScalar)
{
 
 GetFieldDataAlgo algo;
 
 DenseMatrixHandle result = algo.run(TriSurfOnElemScalar());
 DenseMatrixHandle expected_result = TriSurfOnElemScalarMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}


TEST(GetFieldDataTest, TriSurfOnElemVector)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(TriSurfOnElemVector());
 DenseMatrixHandle expected_result = TriSurfOnElemVectorMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}


TEST(GetFieldDataTest, TetMeshOnNodeScalar)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(TetMeshOnNodeScalar());
 DenseMatrixHandle expected_result = TetMeshOnNodeScalarMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    }  
 
}

TEST(GetFieldDataTest, TetMeshOnNodeVector)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(TetMeshOnNodeVector());
 DenseMatrixHandle expected_result = TetMeshOnNodeVectorMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}

TEST(GetFieldDataTest, TetMeshOnNodeTensor)
{
 /*  Does not work since SCIRun5 ReadField cannot load in that files
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(TetMeshOnNodeTensor());
 DenseMatrixHandle expected_result = TetMeshOnNodeTensorMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    }  
 */
}


TEST(GetFieldDataTest, TetMeshOnElemScalar)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(TetMeshOnElemScalar());
 DenseMatrixHandle expected_result = TetMeshOnElemScalarMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}

TEST(GetFieldDataTest, TetMeshOnElemVector)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(TetMeshOnElemVector());
 DenseMatrixHandle expected_result = TetMeshOnElemVectorMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}

TEST(GetFieldDataTest, TetMeshOnElemTensor)
{
 
 GetFieldDataAlgo algo;
 
 std::cout << "ok!" << std::endl;
 
 
 DenseMatrixHandle result = algo.run(TetMeshOnElemTensor());
 DenseMatrixHandle expected_result = TetMeshOnElemTensorMat();
 
 std::cout << "nr:" << result->nrows() << std::endl;
 std::cout << "nc:" << result->ncols() << std::endl;
 
 std::cout << "enr:" << expected_result->nrows() << std::endl;
 std::cout << "enc:" << expected_result->ncols() << std::endl;
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    }   
  
}


TEST(GetFieldDataTest, PointCloudOnNodeScalar)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(PointCloudOnNodeScalar());
 DenseMatrixHandle expected_result = PointCloudOnNodeScalarMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}

TEST(GetFieldDataTest, PointCloudOnNodeVector)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(PointCloudOnNodeVector());
 DenseMatrixHandle expected_result = PointCloudOnNodeVectorMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}

TEST(GetFieldDataTest, PointCloudOnNodeTensor)
{
 
 GetFieldDataAlgo algo;
   
 DenseMatrixHandle result = algo.run(PointCloudOnNodeTensor());
 DenseMatrixHandle expected_result = PointCloudOnNodeTensorMat();
 
 ASSERT_TRUE(result->nrows() == expected_result->nrows());
 ASSERT_TRUE(result->ncols() == expected_result->ncols());
 
 for(VMesh::Elem::index_type i = 0; i < result->ncols(); i++)
   for (VMesh::Elem::index_type j = 0; j < result->nrows(); j++)
    {
     EXPECT_NEAR( (*result)(i,j), (*expected_result)(i,j), 1e-16);
    } 
 
}



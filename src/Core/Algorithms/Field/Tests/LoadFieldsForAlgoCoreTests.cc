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
   Last Modification:   April 10 2014
*/


#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>

//TODO (DAN) : No way to import (ReadField) TriSurf (on nodes or elements) with defined tensors

  FieldHandle TriSurfOnNodeVector()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }

  FieldHandle TriSurfOnNodeScalar()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }

  FieldHandle TriSurfOnElemVector()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_elem/vector/tri_vector_on_elem.fld");
  }

  FieldHandle TriSurfOnElemScalar()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_elem/scalar/tri_scalar_on_elem.fld");
  }

  FieldHandle TetMeshOnNodeVector()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }

  FieldHandle TetMeshOnNodeScalar()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }

  FieldHandle TetMeshOnNodeTensor()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/tensor/tet_tensor_on_node.fld");
  }

  FieldHandle TetMeshOnElemTensor()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/tensor/tet_tensor_on_elem.fld");
  }

  FieldHandle TetMeshOnElemVector()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }

  FieldHandle TetMeshOnElemScalar()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }

  FieldHandle PointCloudOnNodeScalar()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/point_cloud/scalar/pts_scalar.fld");
  }

  FieldHandle PointCloudOnNodeVector()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/point_cloud/vector/pts_vector.fld");
  }

  FieldHandle PointCloudOnNodeTensor()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/point_cloud/tensor/pts_tensor.fld");
  }

  DenseMatrixHandle TriSurfOnNodeVectorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/vector/tri_vector_on_node.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TriSurfOnNodeScalarMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TriSurfOnNodeTensorMat()
  {
   /*ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.mat";
   auto mat = castMatrix::toDense(algo.run(path.string())); */
   return DenseMatrixHandle();
  }

  DenseMatrixHandle TriSurfOnElemVectorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tri_surf/data_defined_on_elem/vector/tri_vector_on_elem.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TriSurfOnElemScalarMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tri_surf/data_defined_on_elem/scalar/tri_scalar_on_elem.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TriSurfOnElemTensorMat()
  {
   /*ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.mat";
   auto mat = castMatrix::toDense(algo.run(path.string())); */
   return DenseMatrixHandle();
  }

  DenseMatrixHandle TetMeshOnNodeVectorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TetMeshOnNodeScalarMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TetMeshOnNodeTensorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/tensor/tet_tensor_on_node.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TetMeshOnElemTensorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/tensor/tet_tensor_on_elem.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TetMeshOnElemVectorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle TetMeshOnElemScalarMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle PointCloudOnNodeScalarMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/point_cloud/scalar/pts_scalar.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle PointCloudOnNodeVectorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/point_cloud/vector/pts_vector.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  DenseMatrixHandle PointCloudOnNodeTensorMat()
  {
   ReadMatrixAlgorithm algo;
   auto path = TestResources::rootDir() / "Fields/point_cloud/tensor/pts_tensor.mat";
   auto mat = castMatrix::toDense(algo.run(path.string()));
   return mat;
  }

  FieldHandle LoadTet()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tet.fld");
  }

  FieldHandle LoadTetWithData()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tetWithData.fld");
  }

  FieldHandle LoadTri()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/tri.fld");
  }

  FieldHandle LoadPts()
  {
   return loadFieldFromFile(TestResources::rootDir() / "Fields/pts.fld");
  }

  FieldHandle SplitFieldByConnectedRegionModuleTetTests()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/splitfieldbyconnectedregion_tet.fld");
  }

  FieldHandle SplitFieldByConnectedRegionModuleTriTests()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/splitfieldbyconnectedregion_tri.fld");
  }

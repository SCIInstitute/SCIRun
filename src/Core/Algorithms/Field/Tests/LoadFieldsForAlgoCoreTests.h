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


#ifndef LOAD_FIELDS_FOR_ALGO_CORE_TESTS_H__
#define LOAD_FIELDS_FOR_ALGO_CORE_TESTS_H__

#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Algorithms::DataIO;
//using namespace SCIRun::Core::Algorithms::Fields;

DenseMatrixHandle TriSurfOnNodeVectorMat();
DenseMatrixHandle TriSurfOnNodeScalarMat();
DenseMatrixHandle TriSurfOnNodeTensorMat();
DenseMatrixHandle TriSurfOnElemVectorMat();
DenseMatrixHandle TriSurfOnElemScalarMat();
DenseMatrixHandle TetMeshOnNodeVectorMat();
DenseMatrixHandle TetMeshOnNodeScalarMat();
DenseMatrixHandle TetMeshOnNodeTensorMat();
DenseMatrixHandle TetMeshOnElemTensorMat();
DenseMatrixHandle TetMeshOnElemVectorMat();
DenseMatrixHandle TetMeshOnElemScalarMat();
DenseMatrixHandle PointCloudOnNodeScalarMat();
DenseMatrixHandle PointCloudOnNodeVectorMat();
DenseMatrixHandle PointCloudOnNodeTensorMat();
FieldHandle LoadTet();
FieldHandle LoadTetWithData();
FieldHandle LoadTri();
FieldHandle LoadPts();
FieldHandle TriSurfOnNodeVector();
FieldHandle TriSurfOnNodeScalar();
FieldHandle TriSurfOnElemVector();
FieldHandle TriSurfOnElemScalar();
FieldHandle TetMeshOnNodeVector();
FieldHandle TetMeshOnNodeScalar();
FieldHandle TetMeshOnNodeTensor();
FieldHandle TetMeshOnElemTensor();
FieldHandle TetMeshOnElemVector();
FieldHandle TetMeshOnElemScalar();
FieldHandle PointCloudOnNodeScalar();
FieldHandle PointCloudOnNodeVector();
FieldHandle PointCloudOnNodeTensor();
FieldHandle SplitFieldByConnectedRegionModuleTetTests();
FieldHandle SplitFieldByConnectedRegionModuleTriTests();

#endif

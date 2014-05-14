
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

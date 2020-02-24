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


#include <Modules/Math/ConvertRealToComplexMatrix.h>
#include <Modules/Math/ConvertComplexToRealMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Eigen/Dense>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class ConvertComplexToRealMatrixModuleTests : public ModuleTest
{

};

TEST_F(ConvertComplexToRealMatrixModuleTests, ThrowsForNullFieldInput)
{
  auto csdf = makeModule("ConvertComplexToRealMatrix");
  FieldHandle nullField;
  stubPortNWithThisData(csdf, 0, nullField);
  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}


TEST_F(ConvertComplexToRealMatrixModuleTests, ThrowsForNullMatrixInput)
{
  auto csdf = makeModule("ConvertComplexToRealMatrix");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(csdf, 0, nullMatrix);
  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}

TEST_F(ConvertComplexToRealMatrixModuleTests, SparseMatrixOnInputPort)
{
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(1,1) = 2;
  m->insert(2,2) = 3;
  m->makeCompressed();
  auto csdf = makeModule("ConvertComplexToRealMatrix");
  stubPortNWithThisData(csdf, 0, m);
  EXPECT_THROW(csdf->execute(), WrongDatatypeOnPortException);
}

TEST_F(ConvertComplexToRealMatrixModuleTests, DenseMatrixOnInputPort)
{
  DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
  (*m)(0) = 1;
  (*m)(1) = 2;
  (*m)(2) = 3;
  auto csdf = makeModule("ConvertComplexToRealMatrix");
  stubPortNWithThisData(csdf, 0, m);
  EXPECT_THROW(csdf->execute(), WrongDatatypeOnPortException);
}

TEST_F(ConvertComplexToRealMatrixModuleTests, ColumnMatrixOnInputPort)
{
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
  (*m)(0,0) = 1;
  (*m)(1,0) = 2;
  (*m)(2,0) = 3;
  auto csdf = makeModule("ConvertComplexToRealMatrix");
  stubPortNWithThisData(csdf, 0, m);
  EXPECT_THROW(csdf->execute(), WrongDatatypeOnPortException);
}


TEST_F(ConvertComplexToRealMatrixModuleTests, ThisShouldWork_dense)
{
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2,2));
  (*m)(0,0) = 1;
  (*m)(0,1) = 2;
  (*m)(1,0) = 3;
  (*m)(1,1) = 4;
  DenseMatrixHandle n(boost::make_shared<DenseMatrix>(2,2));
  (*n)(0,0) = -1;
  (*n)(0,1) = -2;
  (*n)(1,0) = -3;
  (*n)(1,1) = -4;
  auto real2complex = makeModule("ConvertRealToComplexMatrix");
  stubPortNWithThisData(real2complex, 0, m);
  stubPortNWithThisData(real2complex, 1, n);
  EXPECT_NO_THROW(real2complex->execute());
  auto real2complex_output = getDataOnThisOutputPort(real2complex, 0);

  ASSERT_TRUE(real2complex_output != nullptr);
  auto  real2complex_result = boost::dynamic_pointer_cast<ComplexDenseMatrix>(real2complex_output);

  auto complex2real = makeModule("ConvertComplexToRealMatrix");
  stubPortNWithThisData(complex2real, 0, real2complex_result);
  connectDummyOutputConnection(complex2real, 0);
  connectDummyOutputConnection(complex2real, 1);
  EXPECT_NO_THROW(complex2real->execute());
  auto complex2real_output1 = getDataOnThisOutputPort(complex2real, 0);
  auto complex2real_output2 = getDataOnThisOutputPort(complex2real, 1);

  ASSERT_TRUE(complex2real_output1 != nullptr);
  ASSERT_TRUE(complex2real_output2 != nullptr);

  auto  complex2real_result1 = boost::dynamic_pointer_cast<DenseMatrix>(complex2real_output1);
  auto  complex2real_result2 = boost::dynamic_pointer_cast<DenseMatrix>(complex2real_output2);

  for(int x=0; x<complex2real_result1->nrows(); x++)
   for (int y=0; y<complex2real_result1->ncols(); y++)
    {
     ASSERT_TRUE((*complex2real_result1)(x,y) == (*m)(x,y));
     ASSERT_TRUE((*complex2real_result2)(x,y) == (*n)(x,y));
    }
}

TEST_F(ConvertComplexToRealMatrixModuleTests, ThisShouldWork_sparse)
{
  auto mat_dim=3;
  auto m(boost::make_shared<SparseRowMatrix>(mat_dim, mat_dim));
  m->insert(0, 0) = 1;
  m->insert(0, 2) = 1;
  m->insert(1, 0) = 3;
  m->insert(1, 1) = 5;
  m->insert(2, 1) = 1;
  m->insert(2, 2) = 3;
  m->makeCompressed();
  auto n(boost::make_shared<SparseRowMatrix>(mat_dim, mat_dim));
  n->insert(0, 1) = 1;
  n->insert(0, 2) = 2;
  n->insert(1, 0) = 8;
  n->insert(1, 2) = -3;
  n->insert(2, 0) = 9;
  n->insert(2, 2) = 8;
  n->makeCompressed();
  auto real2complex = makeModule("ConvertRealToComplexMatrix");
  stubPortNWithThisData(real2complex, 0, m);
  stubPortNWithThisData(real2complex, 1, n);
  EXPECT_NO_THROW(real2complex->execute());
  auto real2complex_output = getDataOnThisOutputPort(real2complex, 0);
  ASSERT_TRUE(real2complex_output != nullptr);
  auto  real2complex_result = boost::dynamic_pointer_cast<ComplexSparseRowMatrix>(real2complex_output);

  auto complex2real = makeModule("ConvertComplexToRealMatrix");
  stubPortNWithThisData(complex2real, 0, real2complex_result);
  connectDummyOutputConnection(complex2real, 0);
  connectDummyOutputConnection(complex2real, 1);
  EXPECT_NO_THROW(complex2real->execute());

  auto complex2real_output1 = getDataOnThisOutputPort(complex2real, 0);
  auto complex2real_output2 = getDataOnThisOutputPort(complex2real, 1);

  ASSERT_TRUE(complex2real_output1 != nullptr);
  ASSERT_TRUE(complex2real_output2 != nullptr);

  auto  complex2real_result1 = boost::dynamic_pointer_cast<SparseRowMatrix>(complex2real_output1);
  auto  complex2real_result2 = boost::dynamic_pointer_cast<SparseRowMatrix>(complex2real_output2);

  for (int k = 0; k <complex2real_result1 ->outerSize(); ++k)
  {
    for (SparseRowMatrix::InnerIterator it(*complex2real_result1, k); it; ++it)
    {
      auto tmp = it.value();
      ASSERT_TRUE(m->coeffRef(it.row(), it.col()) == tmp);
    }
  }

  for (int k = 0; k <complex2real_result2 ->outerSize(); ++k)
  {
    for (SparseRowMatrix::InnerIterator it(*complex2real_result2, k); it; ++it)
    {
      auto tmp = it.value();
      ASSERT_TRUE(n->coeffRef(it.row(), it.col()) == tmp);

    }
  }

}

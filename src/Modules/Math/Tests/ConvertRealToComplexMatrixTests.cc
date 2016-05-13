/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
//#include <Core/Algorithms/Math/ConvertRealToComplexMatrix.h>
#include <Modules/Math/ConvertRealToComplexMatrix.h>
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

class ConvertRealToComplexMatrixModuleTests : public ModuleTest
{

};

TEST_F(ConvertRealToComplexMatrixModuleTests, ThrowsForNullFieldInput)
{
  auto csdf = makeModule("ConvertRealToComplexMatrix");
  FieldHandle nullField;
  stubPortNWithThisData(csdf, 0, nullField);
  stubPortNWithThisData(csdf, 1, nullField);
  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}

TEST_F(ConvertRealToComplexMatrixModuleTests, ThrowsForNullMatrixInput)
{
  auto csdf = makeModule("ConvertRealToComplexMatrix");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(csdf, 0, nullMatrix);
  stubPortNWithThisData(csdf, 1, nullMatrix);
  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}

TEST_F(ConvertRealToComplexMatrixModuleTests, SparseMatrixOnInputPort)
{
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(1,1) = 2;
  m->insert(2,2) = 3;
  m->makeCompressed();
  auto csdf = makeModule("ConvertRealToComplexMatrix");
  stubPortNWithThisData(csdf, 0, m);
  stubPortNWithThisData(csdf, 1, m);
  EXPECT_NO_THROW(csdf->execute());
}

TEST_F(ConvertRealToComplexMatrixModuleTests, DenseMatrixOnInputPort)
{
  DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
  (*m)(0) = 1;
  (*m)(1) = 2;
  (*m)(2) = 3;
  auto csdf = makeModule("ConvertRealToComplexMatrix");
  stubPortNWithThisData(csdf, 0, m);
  stubPortNWithThisData(csdf, 1, m);
  EXPECT_NO_THROW(csdf->execute());
}

TEST_F(ConvertRealToComplexMatrixModuleTests, ColumnMatrixOnInputPort)
{
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
  (*m)(0,0) = 1;
  (*m)(1,0) = 2;
  (*m)(2,0) = 3;
  auto csdf = makeModule("ConvertRealToComplexMatrix");
  stubPortNWithThisData(csdf, 0, m);
  stubPortNWithThisData(csdf, 1, m);
  EXPECT_NO_THROW(csdf->execute());
}

TEST_F(ConvertRealToComplexMatrixModuleTests, ThisShouldWork)
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
  auto csdf = makeModule("ConvertRealToComplexMatrix");
  stubPortNWithThisData(csdf, 0, m);
  stubPortNWithThisData(csdf, 1, n);
  EXPECT_NO_THROW(csdf->execute());
  auto output = getDataOnThisOutputPort(csdf, 0);
  ASSERT_TRUE(output != nullptr);
  auto result = boost::dynamic_pointer_cast<ComplexDenseMatrix>(output);
  for(int x=0; x<2; x++)
   for (int y=0; y<2; y++)
    {
     auto  input_real = (*m)(x,y), input_imag=(*n)(x,y);
     auto output_real = (*result)(x,y).real(), output_imag = (*result)(x,y).imag();
     ASSERT_TRUE(input_real == output_real);
     ASSERT_TRUE(input_imag == output_imag);
    }
}

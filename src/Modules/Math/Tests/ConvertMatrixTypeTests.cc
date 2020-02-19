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


#include <Core/Algorithms/Math/ConvertMatrixType.h>
#include <Modules/Legacy/Math/ConvertMatrixType.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class ConvertMatrixTypeModuleTests : public ModuleTest
{

};

TEST_F(ConvertMatrixTypeModuleTests, ThrowsForNullFieldInput)
{
  auto csdf = makeModule("ConvertMatrixType");
  FieldHandle nullField;
  stubPortNWithThisData(csdf, 0, nullField);
  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}

TEST_F(ConvertMatrixTypeModuleTests, ThrowsForNullMatrixInput)
{
  auto csdf = makeModule("ConvertMatrixType");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(csdf, 0, nullMatrix);
  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}

TEST_F(ConvertMatrixTypeModuleTests, SparseMatrixOnInputPort)
{
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(1,1) = 2;
  m->insert(2,2) = 3;
  m->makeCompressed();
  auto csdf = makeModule("ConvertMatrixType");
  stubPortNWithThisData(csdf, 0, m);
  EXPECT_NO_THROW(csdf->execute());
}

TEST_F(ConvertMatrixTypeModuleTests, DenseMatrixOnInputPort)
{
  DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
  (*m)(0) = 1;
  (*m)(1) = 2;
  (*m)(2) = 3;
  auto csdf = makeModule("ConvertMatrixType");
  stubPortNWithThisData(csdf, 0, m);
  EXPECT_NO_THROW(csdf->execute());
}

TEST_F(ConvertMatrixTypeModuleTests, ColumnMatrixOnInputPort)
{
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
  (*m)(0,0) = 1;
  (*m)(1,0) = 2;
  (*m)(2,0) = 3;
  auto csdf = makeModule("ConvertMatrixType");
  stubPortNWithThisData(csdf, 0, m);
  EXPECT_NO_THROW(csdf->execute());
}

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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Legacy/Math/SelectSubMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

using namespace SCIRun::Testing;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class SelectSubMatrixModuleTest : public ModuleTest
{
};

DenseMatrixHandle CreateDenseMatrix()
{
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
  (*m)(0,0) = 1;
  (*m)(1,0) = 2;
  (*m)(2,0) = 3;
  return m;
}

DenseColumnMatrixHandle CreateColumnMatrix()
{
  DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
  (*m)(0) = 1;
  (*m)(1) = 2;
  (*m)(2) = 3;
  return m;
}

SparseRowMatrixHandle CreateSparseMatrix()
{
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(1,1) = 2;
  m->insert(2,2) = 3;
  m->makeCompressed();
  return m;
}

TEST_F(SelectSubMatrixModuleTest, NothingOnPortZeroOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, NullOnPortZeroNothingOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(sls, 0, nullMatrix);
  EXPECT_THROW(sls->execute(), NullHandleOnPortException);
}

TEST_F(SelectSubMatrixModuleTest, NothingOnPortZeroOneNullOnTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(sls, 2, nullMatrix);
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, NothingOnPortZeroNullOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(sls, 1, nullMatrix);
  stubPortNWithThisData(sls, 2, nullMatrix);
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, NullOnPortZeroOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(sls, 0, nullMatrix);
  stubPortNWithThisData(sls, 1, nullMatrix);
  stubPortNWithThisData(sls, 2, nullMatrix);
  EXPECT_THROW(sls->execute(), NullHandleOnPortException);
}

TEST_F(SelectSubMatrixModuleTest, SparseMatrixOnPortZeroNullOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(sls, 0, CreateSparseMatrix());
  stubPortNWithThisData(sls, 1, nullMatrix);
  stubPortNWithThisData(sls, 2, nullMatrix);
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, SparseMatrixOnPortZeroNothingOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  stubPortNWithThisData(sls, 0, CreateSparseMatrix());
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, ColumnMatrixOnPortZeroNothingOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  stubPortNWithThisData(sls, 0, CreateColumnMatrix());
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, DenseMatrixOnPortZeroNothingOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  stubPortNWithThisData(sls, 0, CreateDenseMatrix());
  EXPECT_NO_THROW(sls->execute());
}

TEST_F(SelectSubMatrixModuleTest, DenseMatrixOnPortZeroSparseMatrixOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  stubPortNWithThisData(sls, 0, CreateDenseMatrix());
  stubPortNWithThisData(sls, 1, CreateSparseMatrix());
  stubPortNWithThisData(sls, 2, CreateSparseMatrix());
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, DenseMatrixOnPortZeroColumnMatrixOnOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  stubPortNWithThisData(sls, 0, CreateDenseMatrix());
  stubPortNWithThisData(sls, 1, CreateColumnMatrix());
  stubPortNWithThisData(sls, 2, CreateColumnMatrix());
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SelectSubMatrixModuleTest, DenseMatrixOnPortZeroOneTwo)
{
  auto sls = makeModule("SelectSubMatrix");
  stubPortNWithThisData(sls, 0, CreateDenseMatrix());
  stubPortNWithThisData(sls, 1, CreateDenseMatrix());
  stubPortNWithThisData(sls, 2, CreateDenseMatrix());
  EXPECT_NO_THROW(sls->execute());
}

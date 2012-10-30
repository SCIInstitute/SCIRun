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
#include <gmock/gmock.h>

#include <fstream>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::TestUtils;
using namespace SCIRun;
using namespace ::testing;

TEST(SolveLinearSystemTests, CanSolveDarrell)
{
  ReadMatrixAlgorithm reader;
  SparseRowMatrixHandle A;
  {
    ScopedTimer t("reading sparse matrix");
    A = matrix_cast::as_sparse(reader.run("E:\\stuff\\CGDarrell\\A_txt.mat"));
  }
  ASSERT_TRUE(A);
  EXPECT_EQ(428931, A->nrows());
  EXPECT_EQ(428931, A->ncols());

  DenseMatrixHandle rhs;
  {
    ScopedTimer t("reading rhs");
    rhs = matrix_cast::as_dense(reader.run("E:\\stuff\\CGDarrell\\RHS_text.mat"));
  }
  ASSERT_TRUE(rhs);
  EXPECT_EQ(428931, rhs->nrows());
  EXPECT_EQ(1, rhs->ncols());

  DenseColumnMatrixHandle x0;
  ASSERT_FALSE(x0); // algo object will initialize x0 to the zero vector

  SolveLinearSystemAlgo algo;
  algo.set(SolveLinearSystemAlgo::MaxIterations(), 500);
  algo.set(SolveLinearSystemAlgo::TargetError(), 7e-4);

  DenseColumnMatrixHandle solution;
  {
    ScopedTimer t("Running solver");
    ASSERT_TRUE(algo.run(A, matrix_convert::to_column(rhs), x0, solution));
  }
  ASSERT_TRUE(solution);
  EXPECT_EQ(428931, solution->nrows());
  EXPECT_EQ(1, solution->ncols());


  WriteMatrixAlgorithm writer;
  writer.run(solution, "E:\\stuff\\CGDarrell\\portedSolution.txt");
}
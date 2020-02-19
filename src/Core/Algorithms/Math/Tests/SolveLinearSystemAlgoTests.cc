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


#include <Testing/Utils/SCIRunUnitTests.h>

#include <fstream>
#include <boost/filesystem.hpp>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::TestUtils;
using namespace SCIRun;
using namespace ::testing;

void CanSolveDarrellWithMethod(const std::string& method, double solutionError)
{
  auto Afile = TestResources::rootDir() / "CGDarrell" / "A.mat";
  auto rhsFile = TestResources::rootDir() / "CGDarrell" / "RHS.mat";
  if (!boost::filesystem::exists(Afile) || !boost::filesystem::exists(rhsFile))
  {
    FAIL() << "TODO: Issue #142 will standardize these file locations other than being on Dan's hard drive." << std::endl
      << "Once that issue is done however, this will be a user setup error." << std::endl;
    return;
  }

  ReadMatrixAlgorithm reader;
  SparseRowMatrixHandle A;
  {
    ScopedTimer t("reading sparse matrix");
    A = castMatrix::toSparse(reader.run(Afile.string()));
  }
  ASSERT_TRUE(A.get() != nullptr);
  EXPECT_EQ(428931, A->nrows());
  EXPECT_EQ(428931, A->ncols());

  DenseMatrixHandle rhs;
  {
    ScopedTimer t("reading rhs");
    rhs = castMatrix::toDense(reader.run(rhsFile.string()));
  }
  ASSERT_TRUE(rhs.get() != nullptr);
  EXPECT_EQ(428931, rhs->nrows());
  EXPECT_EQ(1, rhs->ncols());

  DenseColumnMatrixHandle x0;
  ASSERT_FALSE(x0); // algo object will initialize x0 to the zero vector

  SolveLinearSystemAlgo algo;
  algo.set(Variables::MaxIterations, 500);
  algo.set(Variables::TargetError, 7e-4);
  algo.setOption(Variables::Method, method);
  algo.setUpdaterFunc([](double x) {});

  DenseColumnMatrixHandle solution;
  {
    ScopedTimer t("Running solver");
    ASSERT_TRUE(algo.run(A, convertMatrix::toColumn(rhs), x0, solution));
  }
  ASSERT_TRUE(solution.get() != nullptr);
  EXPECT_EQ(428931, solution->nrows());
  EXPECT_EQ(1, solution->ncols());

  auto scirun4solutionFile = TestResources::rootDir() / "CGDarrell" / ("dan_sol_" + method + ".mat");
  auto scirun4solution = reader.run(scirun4solutionFile.string());
  ASSERT_TRUE(scirun4solution.get() != nullptr);
  DenseColumnMatrixHandle expected = convertMatrix::toColumn(scirun4solution);

  EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*expected, *solution, solutionError);

  WriteMatrixAlgorithm writer;
  auto portedSolutionFile = TestResources::rootDir() / "CGDarrell" / ("portedSolution_" + method + ".txt");
  writer.run(solution, portedSolutionFile.string());

  auto diff = *expected - *solution;
  auto maxDiff = diff.maxCoeff();
  std::cout << "max diff is: " << maxDiff << std::endl;
}

/// todo: switch these disabled tests to nightly mode. They are overly long for normal continuous builds.

TEST(SolveLinearSystemTests, DISABLED_CanSolveDarrell_CG)
{
  double solutionError;
  /// @todo: investigate this significant difference
#ifdef WIN32
  solutionError = 0.15;
#else
  solutionError = 0.23;
#endif
  CanSolveDarrellWithMethod("cg", solutionError);
}

TEST(SolveLinearSystemTests, DISABLED_CanSolveDarrell_BICG)
{
  double solutionError = 0.001;
  CanSolveDarrellWithMethod("bicg", solutionError);
}

TEST(SolveLinearSystemTests, DISABLED_CanSolveDarrell_Jacobi)
{
  /// @todo: doesn't converge for this system. Problem?
  double solutionError = 105;
  CanSolveDarrellWithMethod("jacobi", solutionError);
}

TEST(SolveLinearSystemTests, DISABLED_CanSolveDarrell_MINRES)
{
  /// @todo: converges but not as accurate.
  double solutionError = 2.4;
  CanSolveDarrellWithMethod("minres", solutionError);
}

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

#if GTEST_HAS_COMBINE

using ::testing::Values;
using ::testing::Combine;
using ::testing::Range;

class SolveLinearSystemTestsAlgoParameterized : public ::testing::TestWithParam < std::tuple<const char*, const char*, int> >
{
public:
	ReadMatrixAlgorithm reader;
	SparseRowMatrixHandle A;
	DenseMatrixHandle rhs;
	DenseColumnMatrixHandle x0;
	SolveLinearSystemAlgo algo;
	DenseColumnMatrixHandle solution;

	~SolveLinearSystemTestsAlgoParameterized() {}
protected:
	virtual void SetUp()
	{
		auto Afile = TestResources::rootDir() / "Matrices" /  "moritz_A.mat";

		auto rhsFile = TestResources::rootDir() / "Matrices" / "moritz_b.mat";
		if (!boost::filesystem::exists(Afile) || !boost::filesystem::exists(rhsFile))
		{
			FAIL() << "TODO: Issue #142 will standardize these file locations other than being on Dan's hard drive." << std::endl
				<< "Once that issue is done however, this will be a user setup error." << std::endl;
			return;
		}
		{
			ScopedTimer t("reading sparse matrix");
			A = castMatrix::toSparse(reader.run(Afile.string()));
		}
		{
			ScopedTimer t("reading rhs");
			rhs = castMatrix::toDense(reader.run(rhsFile.string()));
		}
		 // algo object will initialize x0 to the zero vector

		algo.set(Variables::Preconditioner, std::string(std::get<1>(GetParam())));
		algo.set(Variables::MaxIterations, 670);
		algo.set(Variables::TargetError, std::get<2>(GetParam()));

		algo.set(Variables::MaxIterations, 667);
		algo.set(Variables::TargetError, 1e-4);

		algo.setOption(Variables::Method, std::string(std::get<0>(GetParam())));
		algo.setUpdaterFunc([](double x) {});

		/*
		{
			ScopedTimer t("Running solver");
		}
        const std::string& c = ::std::tr1::get<0>(GetParam());

		auto solutionFile = TestResources::rootDir() / ("dan_sol_" + c + ".mat");

		auto scirun4solution = reader.run(solutionFile.string());
		ASSERT_TRUE(scirun4solution.get() != nullptr);
		DenseColumnMatrixHandle expected = convertMatrix::toColumn(scirun4solution);

		EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*expected, *solution, ::std::tr1::get<1>(GetParam()));

		WriteMatrixAlgorithm writer;
		auto portedSolutionFile = TestResources::rootDir() / ("portedSolution_" +  c + ".txt");
		writer.run(solution, portedSolutionFile.string());

		auto diff = *expected - *solution;
		auto maxDiff = diff.maxCoeff();
		std::cout << "max diff is: " << maxDiff << std::endl;
		*/
		}
	virtual void TearDown(){}
};

TEST_P(SolveLinearSystemTestsAlgoParameterized, CanSolveDarrellParameterized)
{
	//EXPECT_NO_FATAL_FAILURE(SolveLinearSystemTestsAlgoParameterized);
		ASSERT_TRUE(A.get() != nullptr);
		EXPECT_EQ(10149, A->nrows());
		EXPECT_EQ(10149, A->ncols());

		ASSERT_TRUE(rhs.get() != nullptr);
		EXPECT_EQ(10149, rhs->nrows());
		EXPECT_EQ(1, rhs->ncols());

		ASSERT_FALSE(x0);

		ASSERT_TRUE(algo.run(A, convertMatrix::toColumn(rhs), x0, solution));

		ASSERT_TRUE(solution.get() != nullptr);
		EXPECT_EQ(10149, solution->nrows());
		EXPECT_EQ(1, solution->ncols());
}

INSTANTIATE_TEST_CASE_P(
	CanSolveDarrellParameterized,
	SolveLinearSystemTestsAlgoParameterized,
	Combine(Values(
		"cg",
		"bicg",
		"jacobi",
		"minres"
		),
		Values("jacobi","none"),
	Values(1e-1,1e-3,1e-4,1e-5))
	);
#else
TEST(DummyTest, CombineIsNotSupportedOnThisPlatform(){}
#endif

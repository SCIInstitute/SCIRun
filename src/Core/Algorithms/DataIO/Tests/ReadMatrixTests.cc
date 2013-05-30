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

#include <Testing/Utils/SCIRunUnitTests.h>

#include <Core/Datatypes/Tests/MatrixTestCases.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Utils/StringUtil.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::TestUtils;

TEST(ReadMatrixTests, CanReadFromStream)
{
  const std::string matrixString = "0 1 2\n3 4 5\n6 7 8\n";

  std::istringstream istr(matrixString);

  DenseMatrix in;

  istr >> in;

  EXPECT_EQ(matrix1(), in);
}

TEST(ReadMatrixTests, RoundTripViaString)
{
  DenseMatrix m = matrix1();

  std::ostringstream ostr;
  ostr << m;
  std::string str = ostr.str();
  std::istringstream istr(str);
  DenseMatrix m2;
  istr >> m2;

  EXPECT_EQ(m, m2);
}

TEST(ReadMatrixAlgorithmTest, TestFromRealTextFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "matrix1.txt";
  if (boost::filesystem::exists(filename))
  {
    DenseMatrixConstHandle matrix = matrix_cast::as_dense(algo.run(filename.string()));
    ASSERT_TRUE(matrix);
    EXPECT_EQ(matrix1(), *matrix);
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, ThrowsForFileNotFound)
{
  ReadMatrixAlgorithm algo;
  const std::string filename = "ZZZ:\\no way this file exists............\\\\////\\\\//";
  EXPECT_THROW(algo.run(filename), Core::Algorithms::AlgorithmInputException);
}

TEST(ReadMatrixAlgorithmTest, TestSparseFromRealASCIIMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "sp2.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::sparse(matrix));

    auto sp = matrix_cast::as_sparse(matrix);

    DenseMatrix a(2, 3);
    a << 1, 0, 3.5,
      -1, 2, 0;

    //TODO: compare dense and sparse
    //EXPECT_EQ(a, *mat);
    EXPECT_EQ(to_string(a), to_string(sp->castForPrinting()));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestDenseFromRealASCIIMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "CGDarrell" / "xScirun.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::dense(matrix));

    auto dense = matrix_cast::as_dense(matrix);
    EXPECT_EQ(1, dense->cols());
    EXPECT_EQ(428931, dense->rows());
    EXPECT_NEAR(-7.86543, dense->minCoeff(), 1e-4);
    EXPECT_NEAR(8.90886, dense->maxCoeff(), 1e-4);
    EXPECT_DOUBLE_EQ(-0.346299309398506, (*dense)(0,0));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestColumnFromRealASCIIMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "CGDarrell" / "xScirunColumn.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::column(matrix));

    auto col = matrix_cast::as_column(matrix);
    EXPECT_EQ(1, col->cols());
    EXPECT_EQ(428931, col->rows());
    EXPECT_NEAR(-7.86543, col->minCoeff(), 1e-4);
    EXPECT_NEAR(8.90886, col->maxCoeff(), 1e-4);
    EXPECT_DOUBLE_EQ(-0.346299309398506, (*col)(0,0));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, DISABLED_TestLargeSparseFromRealASCIIMatFile)
{
  ReadMatrixAlgorithm algo;
  auto AFile = TestResources::rootDir() / "CGDarrell" / "A_txt.mat";
  if (boost::filesystem::exists(AFile))
  {
    auto matrix = algo.run(AFile.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::sparse(matrix));

    auto sp = matrix_cast::as_sparse(matrix);

    EXPECT_EQ(428931, sp->rows());
    EXPECT_EQ(428931, sp->cols());
    EXPECT_EQ(5540569, sp->nonZeros());
  }
  else
    std::cout << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, UnknownFileFormatThrows)
{
  ReadMatrixAlgorithm algo;
  auto notAMatrixFile = TestResources::rootDir() /  "scirun5demo.srn";
  if (boost::filesystem::exists(notAMatrixFile))
  {
    EXPECT_THROW(algo.run(notAMatrixFile.string()), Core::Algorithms::AlgorithmInputException);
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, DISABLED_ThrowsForMatlabFilesICantThemReadYet)
{
  //TODO: will hook up Matlab readers soon.
  ReadMatrixAlgorithm algo;
  auto matlabFile = TestResources::rootDir() / "CGDarrell" / "RHS.mat";
  if (boost::filesystem::exists(matlabFile))
  {
    EXPECT_THROW(algo.run(matlabFile.string()), Core::Algorithms::AlgorithmInputException);
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, DISABLED_NonMatrixTextFile)
{
  //TODO: will hook-up Importers soon.
  ReadMatrixAlgorithm algo;
  const boost::filesystem::path txt("E:/seg3d/trunk/src/Install.txt");
  if (boost::filesystem::exists(txt))
  {
    EXPECT_THROW(algo.run(txt.string()), Core::Algorithms::AlgorithmInputException);
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestSparseFromRealBinaryMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "CGDarrell" / "eye3x3sparse_bin.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::sparse(matrix));

    auto sp = matrix_cast::as_sparse(matrix);

    DenseMatrix a = DenseMatrix::Identity(3,3);

    //TODO: compare dense and sparse
    //EXPECT_EQ(a, *mat);
    EXPECT_EQ(to_string(a), to_string(sp->castForPrinting()));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestDenseFromRealBinaryMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "CGDarrell" / "eye3x3dense_bin.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::dense(matrix));

    auto dense = matrix_cast::as_dense(matrix);

    DenseMatrix a = DenseMatrix::Identity(3,3);

    EXPECT_EQ(to_string(a), to_string(*dense));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestColumnFromRealBinaryMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "CGDarrell" / "columnBinMat.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::dense(matrix)); // artifact of v4 IO

    auto col = matrix_convert::to_column(matrix);
    DenseColumnMatrix expected(3);
    expected << 1, 2, 3;
    EXPECT_EQ(to_string(*col), to_string(expected));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestColumnFromRealBinaryMatFile2)
{
  ReadMatrixAlgorithm algo;
  auto filename = boost::filesystem::path("E:\\columnBin2.mat");
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix);
    ASSERT_TRUE(matrix_is::column(matrix)); 

    auto col = matrix_cast::as_column(matrix);
    DenseColumnMatrix expected(5);
    expected << 0, 1, 2, 3, 4;
    EXPECT_EQ(to_string(*col), to_string(expected));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

void CallLegacyPio(const boost::filesystem::path& filename, const DenseMatrix& expected = DenseMatrix::Identity(3,3))
{
  if (boost::filesystem::exists(filename))
  {
    PiostreamPtr stream = auto_istream(filename.string());
    if (!stream)
    {
      FAIL() << "Error reading file '" << filename << "'.";
    }

    MatrixHandle matrix;
    Pio(*stream, matrix);

    ASSERT_TRUE(matrix);
    EXPECT_EQ(expected.nrows(), matrix->nrows());
    EXPECT_EQ(expected.ncols(), matrix->ncols());

    std::cout << *matrix << std::endl;
    EXPECT_EQ(to_string(*matrix), to_string(expected));
  }
  else
    FAIL() << "file does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, CallLegacyPioDense)
{
  CallLegacyPio(TestResources::rootDir() / "CGDarrell" / "eye3x3dense_bin.mat");
}

TEST(ReadMatrixAlgorithmTest, CallLegacyPioSparse)
{
  CallLegacyPio(TestResources::rootDir() / "CGDarrell" / "eye3x3sparse_bin.mat");
}
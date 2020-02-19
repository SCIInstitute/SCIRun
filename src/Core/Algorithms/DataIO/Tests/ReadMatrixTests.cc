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
#include <Core/Datatypes/Tests/MatrixTestCases.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Utils/StringUtil.h>
#include <boost/filesystem.hpp>

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
  auto filename = TestResources::rootDir() / "Matrices" / "matrix1.txt";
  if (boost::filesystem::exists(filename))
  {
    DenseMatrixConstHandle matrix = castMatrix::toDense(algo.run(filename.string()));
    ASSERT_TRUE(matrix.get() != nullptr);
    EXPECT_EQ(matrix1(), *matrix);
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
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
  auto filename = TestResources::rootDir() / "Matrices" / "sp2.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::sparse(matrix));

    auto sp = castMatrix::toSparse(matrix);

    DenseMatrix a(3, 4);
    a << 1, 0, 0, -1,
      0, 2, 0, 0,
      0, 0, 3, 0;

    EXPECT_EQ(a, *convertMatrix::toDense(matrix));
#if !DEBUG
    EXPECT_EQ(to_string(a), to_string(sp->castForPrinting()));
#endif
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestDenseFromRealASCIIMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "Matrices" / "xScirun.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::dense(matrix));

    auto dense = castMatrix::toDense(matrix);

    EXPECT_EQ(4, dense->cols());
    EXPECT_EQ(3, dense->rows());
    EXPECT_NEAR(-1, dense->minCoeff(), 1e-12);
    EXPECT_NEAR(3, dense->maxCoeff(), 1e-12);
    EXPECT_DOUBLE_EQ(1, (*dense)(0,0));
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestColumnFromRealASCIIMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "Matrices" / "xScirunColumn.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::column(matrix));

    auto col = castMatrix::toColumn(matrix);
    EXPECT_EQ(1, col->cols());
    EXPECT_EQ(5, col->rows());
    EXPECT_NEAR(-4, col->minCoeff(), 1e-12);
    EXPECT_NEAR(3, col->maxCoeff(), 1e-12);
    EXPECT_DOUBLE_EQ(1, (*col)[0]);
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, DISABLED_TestLargeSparseFromRealASCIIMatFile)
{
  ReadMatrixAlgorithm algo;
  auto AFile = TestResources::rootDir() / "Matrices" / "CGDarrell" / "A_txt.mat";
  if (boost::filesystem::exists(AFile))
  {
    auto matrix = algo.run(AFile.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::sparse(matrix));

    auto sp = castMatrix::toSparse(matrix);

    EXPECT_EQ(428931, sp->rows());
    EXPECT_EQ(428931, sp->cols());
    EXPECT_EQ(5540569, sp->nonZeros());
  }
  else
    FAIL() << "file " << AFile.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, UnknownFileFormatThrows)
{
  ReadMatrixAlgorithm algo;
  auto notAMatrixFile = TestResources::rootDir() / "Other" / "scirun5demo.srn";
  if (boost::filesystem::exists(notAMatrixFile))
  {
    EXPECT_THROW(algo.run(notAMatrixFile.string()), Core::Algorithms::AlgorithmInputException);
  }
  else
    FAIL() << "file " << notAMatrixFile.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, DISABLED_ThrowsForMatlabFilesICantThemReadYet)
{
  /// @todo: will hook up Matlab readers soon.
  ReadMatrixAlgorithm algo;
  auto matlabFile = TestResources::rootDir() / "Matrices" / "CGDarrell" / "RHS.mat";
  if (boost::filesystem::exists(matlabFile))
  {
    EXPECT_THROW(algo.run(matlabFile.string()), Core::Algorithms::AlgorithmInputException);
  }
  else
    FAIL() << "file " << matlabFile.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, DISABLED_NonMatrixTextFile)
{
  /// @todo: will hook-up Importers soon.
  ReadMatrixAlgorithm algo;
  const boost::filesystem::path txt("E:/seg3d/trunk/src/Install.txt");
  if (boost::filesystem::exists(txt))
  {
    EXPECT_THROW(algo.run(txt.string()), Core::Algorithms::AlgorithmInputException);
  }
  else
    FAIL() << "file " << txt.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestSparseFromRealBinaryMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "Matrices" / "eye3x3sparse_bin.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::sparse(matrix));

    auto sp = castMatrix::toSparse(matrix);

    DenseMatrix a = DenseMatrix::Identity(3,3);

    EXPECT_EQ(a, *convertMatrix::toDense(matrix));
#if !DEBUG
    EXPECT_EQ(to_string(a), to_string(sp->castForPrinting()));
#endif
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestDenseFromRealBinaryMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "Matrices" / "eye3x3dense_bin.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::dense(matrix));

    auto dense = castMatrix::toDense(matrix);

    DenseMatrix a = DenseMatrix::Identity(3,3);

    EXPECT_EQ(to_string(a), to_string(*dense));
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestColumnFromRealBinaryMatFile)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "Matrices" / "columnBinMat.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::dense(matrix)); // artifact of v4 IO

    auto col = convertMatrix::toColumn(matrix);
    DenseColumnMatrix expected(3);
    expected << 1, 2, 3;
    EXPECT_EQ(to_string(*col), to_string(expected));
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, TestColumnFromRealBinaryMatFile2)
{
  ReadMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "Matrices" / "columnBin2.mat";
  if (boost::filesystem::exists(filename))
  {
    auto matrix = algo.run(filename.string());
    ASSERT_TRUE(matrix.get() != nullptr);
    ASSERT_TRUE(matrixIs::column(matrix));

    auto col = castMatrix::toColumn(matrix);
    DenseColumnMatrix expected(5);
    expected << 0, 1, 2, 3, 4;
    EXPECT_EQ(to_string(*col), to_string(expected));
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
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

    ASSERT_TRUE(matrix.get() != nullptr);
    EXPECT_EQ(expected.nrows(), matrix->nrows());
    EXPECT_EQ(expected.ncols(), matrix->ncols());

    std::cout << *matrix << std::endl;

    EXPECT_EQ(expected, *convertMatrix::toDense(matrix));
#if !DEBUG
    EXPECT_EQ(to_string(expected), to_string(*matrix));
#endif
  }
  else
    FAIL() << "file " << filename.string() << " does not exist, skipping test." << std::endl;
}

TEST(ReadMatrixAlgorithmTest, CallLegacyPioDense)
{
  CallLegacyPio(TestResources::rootDir() / "Matrices" / "eye3x3dense_bin.mat");
}

TEST(ReadMatrixAlgorithmTest, CallLegacyPioSparse)
{
  CallLegacyPio(TestResources::rootDir() / "Matrices" / "eye3x3sparse_bin.mat");
}

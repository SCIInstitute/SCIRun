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
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms;

namespace
{
  DenseMatrix matrix1()
  {
    DenseMatrix m (3, 3);
    for (int i = 0; i < m.rows(); ++ i)
      for (int j = 0; j < m.cols(); ++ j)
        m(i, j) = 3.0 * i + j + 1;
    return m;
  }
}

TEST(WriteMatrixTests, CanWriteToStream)
{
  const std::string matrixString = "1 2 3 \n4 5 6 \n7 8 9 \n";

  std::ostringstream ostr;

  DenseMatrix out = matrix1();

  ostr << out;

  EXPECT_EQ(matrixString, ostr.str());
}

TEST(WriteMatrixAlgorithmTest, TestToRealTextFile)
{
  WriteMatrixAlgorithm algo;
  auto filename = TestResources::rootDir() / "TransientOutput" / "matrix1Out.txt";

  DenseMatrixHandle m1(matrix1().clone());
  algo.run(m1, filename.string());
}

TEST(WriteMatrixAlgorithmTest, RoundTripRealTextFile)
{
  WriteMatrixAlgorithm write;
  auto filename = TestResources::rootDir() / "TransientOutput" / "matrix1Out.txt";

  DenseMatrixHandle m1(matrix1().clone());
  write.run(m1, filename.string());

  ReadMatrixAlgorithm read;
  DenseMatrixConstHandle roundTrip =  castMatrix::toDense(read.run(filename.string()));
  ASSERT_TRUE(roundTrip.get() != nullptr);

  EXPECT_EQ(*m1, *roundTrip);
}

TEST(WriteMatrixAlgorithmTest, RoundTripRealBinaryFile)
{
  WriteMatrixAlgorithm write;
  auto filename = TestResources::rootDir() / "TransientOutput" / "matrix1Out.mat";

  DenseMatrixHandle m1(matrix1().clone());
  write.run(m1, filename.string());

  ReadMatrixAlgorithm read;
  DenseMatrixConstHandle roundTrip =  castMatrix::toDense(read.run(filename.string()));
  ASSERT_TRUE(roundTrip.get() != nullptr);

  EXPECT_EQ(*m1, *roundTrip);
}

TEST(WriteMatrixAlgorithmTest, ThrowsWithNullInput)
{
  WriteMatrixAlgorithm algo;
  EXPECT_THROW(algo.run(DenseMatrixHandle(), "a.txt"), AlgorithmInputException);
}

TEST(WriteMatrixAlgorithmTest, CheckBoostExceptionLoggingCapability)
{
  try
  {
    WriteMatrixAlgorithm algo;
    algo.run(DenseMatrixHandle(), "a.txt");
  }
  catch (boost::exception& e)
  {
    std::cout << boost::diagnostic_information(e) << std::endl;
  }
}

#define PRINT_MATRIX_BASE(x) std::cout << #x << " = \n" << static_cast<const MatrixBase<double>&>((x)) << std::endl

TEST(WriteMatrixTest, CanPrintSparseMatrix)
{
  SparseRowMatrix m(3,3);
  m.insert(0,0) = 1;
  m.insert(1,2) = -1.4;

  PRINT_MATRIX_BASE(m);

  std::ostringstream ostr;
  ostr << m.castForPrinting();
#if !DEBUG
  EXPECT_EQ("1 0 0 \n0 0 -1.4 \n0 0 0 \n", ostr.str());
#else
  const std::string debugSparse =
    "Nonzero entries:\n"
    "(1,0) (_,_) (-1.4,2) (_,_) (_,_) (_,_) \n\n"
    "Outer pointers:\n"
    "0 2 4  $\n"
    "Inner non zeros:\n"
    "1 1 0  $\n\n"
    "1 0 0 \n"
    "0 0 -1.4 \n"
    "0 0 0 \n";
  EXPECT_EQ(debugSparse, ostr.str());
#endif
}

TEST(WriteMatrixTest, CanPrintColumnMatrix)
{
  DenseColumnMatrix m(3);
  m << 1,2,0;

  std::ostringstream ostr;
  ostr << m;
  EXPECT_EQ("1\n2\n0", ostr.str());
}

namespace
{
  SparseRowMatrixHandle readSparseMatrixFile(const boost::filesystem::path& filename)
  {
    std::cout << "Reading file: " << filename << std::endl;
    ReadMatrixAlgorithm read;
    return castMatrix::toSparse(read.run(filename.string()));
  }

  DenseMatrixHandle readDenseMatrixFile(const boost::filesystem::path& filename)
  {
    std::cout << "Reading file: " << filename << std::endl;
    ReadMatrixAlgorithm read;
    return castMatrix::toDense(read.run(filename.string()));
  }

  void writeMatrixToFile(const MatrixHandle& matrix, const boost::filesystem::path& filename)
  {
    std::cout << "Writing file: " << filename << std::endl;
    WriteMatrixAlgorithm write;
    write.run(matrix, filename.string());
  }
}

TEST(WriteMatrixAlgorithmTest, RoundTripRealBinaryFileSparse)
{
  auto sparse4 = readSparseMatrixFile(TestResources::rootDir() / "Matrices" / "sparse_v4.mat");
  ASSERT_TRUE(sparse4.get() != nullptr);
  EXPECT_EQ(5, sparse4->nrows());
  EXPECT_EQ(6, sparse4->ncols());

  auto v5file = TestResources::rootDir() / "Matrices" / "sparse_v5.mat";
  writeMatrixToFile(sparse4, v5file);

  auto sparse5 = readSparseMatrixFile(v5file);

  ASSERT_TRUE(sparse5.get() != nullptr);
  EXPECT_EQ(*sparse4, *sparse5);
}

TEST(WriteMatrixAlgorithmTest, RoundTripRealBinaryFileDense)
{
  auto dense4 = readDenseMatrixFile(TestResources::rootDir()  / "Matrices" / "dense_v4.mat");
  ASSERT_TRUE(dense4.get() != nullptr);
  EXPECT_EQ(5, dense4->nrows());
  EXPECT_EQ(6, dense4->ncols());

  auto v5file = TestResources::rootDir()  / "Matrices" / "dense_v5.mat";
  writeMatrixToFile(dense4, v5file);

  auto dense5 = readDenseMatrixFile(v5file);

  ASSERT_TRUE(dense5.get() != nullptr);
  EXPECT_EQ(*dense4, *dense5);
}

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

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>

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
  DenseMatrix matrixNonSquare()
  {
    DenseMatrix m (3, 4);
    for (int i = 0; i < m.rows(); ++ i)
      for (int j = 0; j < m.cols(); ++ j)
        m(i, j) = 3.5 * i + j;
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
  const std::string filename = "E:\\git\\SCIRunGUIPrototype\\src\\Samples\\matrix1Out.txt";

  DenseMatrixHandle m1(matrix1().clone());
  algo.run(m1, filename);
}

TEST(WriteMatrixAlgorithmTest, RoundTripRealTextFile)
{
  WriteMatrixAlgorithm write;
  const std::string filename = "E:\\git\\SCIRunGUIPrototype\\src\\Samples\\matrix1Out.txt";

  DenseMatrixHandle m1(matrix1().clone());
  write.run(m1, filename);

  ReadMatrixAlgorithm read;
  DenseMatrixConstHandle roundTrip = read.run(filename);

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
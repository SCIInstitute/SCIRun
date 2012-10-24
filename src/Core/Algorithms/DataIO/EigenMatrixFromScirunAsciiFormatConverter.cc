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

#include <Core/Algorithms/DataIO/EigenMatrixFromScirunAsciiFormatConverter.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::DataIO::internal;

namespace
{
  template <typename T>
  std::vector<T> parseLineOfNumbers(const std::string& line)
  {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(" ");
    tokenizer tok(line, sep);

    std::vector<T> numbers;
    std::transform(tok.begin(), tok.end(), std::back_inserter(numbers), boost::lexical_cast<T,std::string>);

    return numbers;
  }

  bool fileContainsString(const std::string& filename, const std::string& str)
  {
    std::ifstream input(filename);
    std::string line;

    while (std::getline(input, line)) 
    {
      auto index = line.find(str);
      if (index != std::string::npos && line.find('\0') > index)
        return true;
    }

    return false;
  }
}

MatrixHandle EigenMatrixFromScirunAsciiFormatConverter::make(const std::string& matFile)
{
  if (fileContainsString(matFile, "DenseMatrix"))
    return makeDense(matFile);
  if (fileContainsString(matFile, "SparseRowMatrix"))
    return makeSparse(matFile);
  if (fileContainsString(matFile, "ColumnMatrix"))
    return makeColumn(matFile);

  //TODO: no access to error(), need alternative for logging this exception
  BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Unknown SCIRun matrix format"));
}

SparseRowMatrixHandle EigenMatrixFromScirunAsciiFormatConverter::makeSparse(const std::string& matFile)
{
  SparseData data = convertRaw(parseSparseMatrixString(getMatrixContentsLine(readFile(matFile)).get()).get());
  SparseRowMatrixHandle mat(new SparseRowMatrix(data.get<0>(), data.get<1>()));

  typedef Eigen::Triplet<double> T;
  std::vector<T> tripletList;
  {
    const Indices& rowAcc = data.get<3>();
    const Indices& cols = data.get<4>();
    const Data& values = data.get<5>();
    size_t estimation_of_entries = values.size();
    tripletList.reserve(estimation_of_entries);

    int count = 0;
    int nextRow;
    for (int r = 0; r < mat->rows(); ++r)
    {
      nextRow = rowAcc[r+1];
      while (count < nextRow)
      {
        //(*dm)[r][columns_[count]]=data_[count];
        tripletList.push_back(T(r, cols[count], values[count]));
        count++;
      }
    }
  }

  mat->setFromTriplets(tripletList.begin(), tripletList.end());
  mat->makeCompressed();
  return mat;
}

boost::optional<std::string> EigenMatrixFromScirunAsciiFormatConverter::getMatrixContentsLine(const std::string& matStr)
{
  std::istringstream reader(matStr);
  std::string line;
  while(std::getline(reader, line))
  {
    if (line.length() > 2 && isdigit(line[0]))
      return line;
  }
  return boost::optional<std::string>();
}

std::string EigenMatrixFromScirunAsciiFormatConverter::readFile(const std::string& filename)
{
  std::ifstream matFile(filename.c_str());
  std::string matStr((std::istreambuf_iterator<char>(matFile)),
    std::istreambuf_iterator<char>());
  return matStr;
}

DenseMatrixHandle EigenMatrixFromScirunAsciiFormatConverter::makeDense(const std::string& matFile)
{
  DenseData data = convertRaw(parseDenseMatrixString(getMatrixContentsLine(readFile(matFile)).get()).get());
  DenseMatrixHandle mat(new DenseMatrix(data.get<0>(), data.get<1>()));

  auto values = data.get<2>().begin();
  for (int i = 0; i < mat->rows(); ++i)
    for (int j = 0; j < mat->cols(); ++j)
      (*mat)(i,j) = *values++;

  return mat;
}

DenseColumnMatrixHandle EigenMatrixFromScirunAsciiFormatConverter::makeColumn(const std::string& matFile)
{
  DenseData data = convertRaw(parseColumnMatrixString(getMatrixContentsLine(readFile(matFile)).get()).get());
  DenseColumnMatrixHandle mat(new DenseColumnMatrix(data.get<0>()));

  auto values = data.get<2>().begin();
  for (int i = 0; i < mat->rows(); ++i)
      (*mat)(i) = *values++;

  return mat;
}

boost::optional<EigenMatrixFromScirunAsciiFormatConverter::RawDenseData> EigenMatrixFromScirunAsciiFormatConverter::parseDenseMatrixString(const std::string& matString)
{
  boost::regex r("(\\d+) (\\d+) \\{0 (.*)\\}\\}");
  boost::smatch what;
  regex_match(matString, what, r);
  if (what.size() == 4)
  {
    return boost::make_tuple(what[1], what[2], what[3]);
  }
  return boost::optional<RawDenseData>();
}

boost::optional<EigenMatrixFromScirunAsciiFormatConverter::RawDenseData> EigenMatrixFromScirunAsciiFormatConverter::parseColumnMatrixString(const std::string& matString)
{
  boost::regex r("(\\d+) (.*)\\}");
  boost::smatch what;
  regex_match(matString, what, r);
  if (what.size() == 3)
  {
    return boost::make_tuple(what[1], "1", what[2]);
  }
  return boost::optional<RawDenseData>();
}

EigenMatrixFromScirunAsciiFormatConverter::DenseData EigenMatrixFromScirunAsciiFormatConverter::convertRaw(const RawDenseData& data)
{
  return boost::make_tuple(
    boost::lexical_cast<int>(data.get<0>()),
    boost::lexical_cast<int>(data.get<1>()),
    parseLineOfNumbers<double>(data.get<2>()));
}

boost::optional<EigenMatrixFromScirunAsciiFormatConverter::RawSparseData> EigenMatrixFromScirunAsciiFormatConverter::parseSparseMatrixString(const std::string& matString)
{
  boost::regex r("(\\d+) (\\d+) (\\d+) \\{8 (.*)\\}\\{8 (.*)\\}\\{(.*)\\}\\}");
  boost::smatch what;
  regex_match(matString, what, r);
  if (what.size() == 7)
  {
    return boost::make_tuple(what[1], what[2], what[3], what[4], what[5], what[6]);
  }
  return boost::optional<RawSparseData>();
}

EigenMatrixFromScirunAsciiFormatConverter::SparseData EigenMatrixFromScirunAsciiFormatConverter::convertRaw(const RawSparseData& data)
{
  return boost::make_tuple(
    boost::lexical_cast<int>(data.get<0>()),
    boost::lexical_cast<int>(data.get<1>()),
    boost::lexical_cast<int>(data.get<2>()),
    parseLineOfNumbers<int>(data.get<3>()),
    parseLineOfNumbers<int>(data.get<4>()),
    parseLineOfNumbers<double>(data.get<5>()));
}

/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Testing/Utils/MatrixTestUtilities.h>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;

template <typename T>
std::vector<T> parseLineOfNumbers(const std::string& line)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(" ");
  tokenizer tok(line, sep);

  std::vector<T> numbers;
  std::transform(tok.begin(), tok.end(), std::back_inserter(numbers), [](const std::string& s){ return boost::lexical_cast<T>(s);});
  return numbers;
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

    size_t count = 0;
    size_t nextRow;
    for (size_t r = 0; r < mat->rows(); ++r)
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

  BOOST_FOREACH(double x, data.get<2>())
    *mat << x;

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

EigenMatrixFromScirunAsciiFormatConverter::DenseData EigenMatrixFromScirunAsciiFormatConverter::convertRaw(const RawDenseData& data)
{
  return boost::make_tuple(
    boost::lexical_cast<size_t>(data.get<0>()),
    boost::lexical_cast<size_t>(data.get<1>()),
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
    boost::lexical_cast<size_t>(data.get<0>()),
    boost::lexical_cast<size_t>(data.get<1>()),
    boost::lexical_cast<size_t>(data.get<2>()),
    parseLineOfNumbers<size_t>(data.get<3>()),
    parseLineOfNumbers<size_t>(data.get<4>()),
    parseLineOfNumbers<double>(data.get<5>()));
}

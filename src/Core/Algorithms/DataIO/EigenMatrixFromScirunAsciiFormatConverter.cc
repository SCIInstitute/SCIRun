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


#include <boost/timer.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/make_shared.hpp>

#include <iostream>
#include <fstream>
#include <streambuf>

#include <Core/Algorithms/DataIO/EigenMatrixFromScirunAsciiFormatConverter.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Utils/FileUtil.h>
#include <Core/Utils/StringUtil.h>


using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms::DataIO::internal;

EigenMatrixFromScirunAsciiFormatConverter::EigenMatrixFromScirunAsciiFormatConverter(const ProgressReporter* reporter) : reporter_(reporter)
{
}

MatrixHandle EigenMatrixFromScirunAsciiFormatConverter::make(const std::string& matFile)
{
  if (reporter_)
    reporter_->update_progress(0.01);
  if (fileContainsString(matFile, "DenseMatrix"))
    return makeDense(matFile);
  if (fileContainsString(matFile, "SparseRowMatrix"))
    return makeSparse(matFile);
  if (fileContainsString(matFile, "ColumnMatrix"))
    return makeColumn(matFile);

  /// @todo: no access to error(), need alternative for logging this exception
  BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Unknown SCIRun matrix format"));
}

SparseRowMatrixHandle EigenMatrixFromScirunAsciiFormatConverter::makeSparse(const std::string& matFile)
{
  SparseData data = convertRaw(parseSparseMatrixString(getMatrixContentsLine(readFile(matFile)).get()).get());
  if (reporter_)
    reporter_->update_progress(0.7);
  SparseRowMatrixHandle mat(boost::make_shared<SparseRowMatrix>(data.get<0>(), data.get<1>()));

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
        tripletList.push_back(T(r, cols[count], values[count]));
        count++;
      }
    }
  }

  mat->setFromTriplets(tripletList.begin(), tripletList.end());
  mat->makeCompressed();
  if (reporter_)
    reporter_->update_progress(1);
  return mat;
}

boost::optional<std::string> EigenMatrixFromScirunAsciiFormatConverter::getMatrixContentsLine(const std::string& matStr)
{
  if (reporter_)
    reporter_->update_progress(0.2);

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
  if (reporter_)
    reporter_->update_progress(0.1);
  return matStr;
}

DenseMatrixHandle EigenMatrixFromScirunAsciiFormatConverter::makeDense(const std::string& matFile)
{
  DenseData data = convertRaw(parseDenseMatrixString(getMatrixContentsLine(readFile(matFile)).get()).get());
  DenseMatrixHandle mat(boost::make_shared<DenseMatrix>(data.get<0>(), data.get<1>()));

  auto values = data.get<2>().begin();
  for (int i = 0; i < mat->rows(); ++i)
    for (int j = 0; j < mat->cols(); ++j)
      (*mat)(i,j) = *values++;

  return mat;
}

DenseColumnMatrixHandle EigenMatrixFromScirunAsciiFormatConverter::makeColumn(const std::string& matFile)
{
  DenseData data = convertRaw(parseColumnMatrixString(getMatrixContentsLine(readFile(matFile)).get()).get());
  DenseColumnMatrixHandle mat(boost::make_shared<DenseColumnMatrix>(data.get<0>()));

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
    return boost::make_tuple(what[1].str(), what[2].str(), what[3].str());
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
    return boost::make_tuple(what[1].str(), std::string("1"), what[2].str());
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
  boost::regex r("(\\d+) (\\d+) (\\d+) \\{8 (.*)\\}\\{8 (.*)\\}\\{(.*)\\}\\}\\h*");
  boost::smatch what;
  regex_match(matString, what, r);
  if (what.size() == 7)
  {
    if (reporter_)
      reporter_->update_progress(0.4);
    return boost::make_tuple(what[1].str(), what[2].str(), what[3].str(), what[4].str(), what[5].str(), what[6].str());
  }
  return boost::optional<RawSparseData>();
}

EigenMatrixFromScirunAsciiFormatConverter::SparseData EigenMatrixFromScirunAsciiFormatConverter::convertRaw(const RawSparseData& data)
{
  if (reporter_)
    reporter_->update_progress(0.5);
  return boost::make_tuple(
    boost::lexical_cast<int>(data.get<0>()),
    boost::lexical_cast<int>(data.get<1>()),
    boost::lexical_cast<int>(data.get<2>()),
    parseLineOfNumbers<int>(data.get<3>()),
    parseLineOfNumbers<int>(data.get<4>()),
    parseLineOfNumbers<double>(data.get<5>()));
}

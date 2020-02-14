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


#ifndef ALGORITHMS_DATAIO_EIGENMATRIXFROMSCIRUNASCIIFORMATCONVERTER_H
#define ALGORITHMS_DATAIO_EIGENMATRIXFROMSCIRUNASCIIFORMATCONVERTER_H

#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Utils/ProgressReporter.h>
#include <Core/Algorithms/DataIO/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace DataIO {
namespace internal
{

  class SCISHARE EigenMatrixFromScirunAsciiFormatConverter
  {
  public:
    explicit EigenMatrixFromScirunAsciiFormatConverter(const Utility::ProgressReporter* reporter = 0);
    Core::Datatypes::MatrixHandle make(const std::string& matFile);

    Core::Datatypes::SparseRowMatrixHandle makeSparse(const std::string& matFile);

    boost::optional<std::string> getMatrixContentsLine(const std::string& matStr);

    std::string readFile(const std::string& filename);

    Core::Datatypes::DenseMatrixHandle makeDense(const std::string& matFile);
    Core::Datatypes::DenseColumnMatrixHandle makeColumn(const std::string& matFile);

    typedef std::vector<int> Indices;
    typedef std::vector<double> Data;
    typedef boost::tuple<int, int, int, Indices, Indices, Data> SparseData;
    typedef boost::tuple<std::string,std::string,std::string,std::string,std::string,std::string> RawSparseData;
    typedef boost::tuple<int, int, Data> DenseData;
    typedef boost::tuple<std::string,std::string,std::string> RawDenseData;

    boost::optional<RawDenseData> parseDenseMatrixString(const std::string& matString);
    boost::optional<RawDenseData> parseColumnMatrixString(const std::string& matString);
    DenseData convertRaw(const RawDenseData& data);
    boost::optional<RawSparseData> parseSparseMatrixString(const std::string& matString);
    SparseData convertRaw(const RawSparseData& data);
  private:
    const Utility::ProgressReporter* reporter_;
  };

}}}}}

#endif

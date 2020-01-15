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


#ifndef CORE_ALGORITHMS_DATAIO_DATAIOALGO_H
#define CORE_ALGORITHMS_DATAIO_DATAIOALGO_H 1

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Legacy/DataIO/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {

class SCISHARE DataIOAlgo : public AlgorithmBase {
public:
  explicit DataIOAlgo(Core::Logging::LoggerHandle pr);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  bool ReadField(const std::string& filename, FieldHandle& field, const std::string& importer = "");
  bool WriteField(const std::string& filename, FieldHandle field, const std::string& exporter = "");

  bool ReadMatrix(const std::string& filename, Datatypes::MatrixHandle& matrix, const std::string& importer = "");
  bool WriteMatrix(const std::string& filename, Datatypes::MatrixHandle matrix, const std::string& exporter = "");
#endif

  bool readNrrd(const std::string& filename, NrrdDataHandle& nrrd, const std::string& importer = "");
  bool writeNrrd(const std::string& filename, NrrdDataHandle nrrd, const std::string& exporter = "");

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  bool ReadBundle(const std::string& filename, Datatypes::BundleHandle& bundle, const std::string& importer = "");
  bool WriteBundle(const std::string& filename, Datatypes::BundleHandle bundle, const std::string& exporter = "");

  bool ReadColorMap(const std::string& filename, Datatypes::ColorMapHandle& colorMap, const std::string& importer = "");
  bool WriteColorMap(const std::string& filename, Datatypes::ColorMapHandle colorMap, const std::string& exporter = "");
#endif

  virtual AlgorithmOutput run(const AlgorithmInput& input) const override;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  bool ReadColorMap2(const std::string& filename, ColorMap2Handle& colorMap2, const std::string& importer = "");
  bool WriteColorMap2(const std::string& filename, ColorMap2Handle colorMap2, const std::string& exporter = "");

  bool ReadPath(const std::string& filename, PathHandle& path, const std::string& importer = "");
  bool WritePath(const std::string& filename, PathHandle path, const std::string& exporter = "");
#endif
private:
  Core::Logging::LoggerHandle pr_;
};

}}}

#endif

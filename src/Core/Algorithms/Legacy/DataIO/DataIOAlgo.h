/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Util/AlgoLibrary.h>

#include <Core/Datatypes/Bundle.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/NrrdData.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Geom/Path.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Volume/ColorMap2.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>



#include <string>
#include <sstream>


#include <Core/Algorithms/DataIO/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE DataIOAlgo : public AlgoLibrary {
public:
  DataIOAlgo(ProgressReporter* pr) : AlgoLibrary(pr) {}

  bool ReadField(const std::string& filename, FieldHandle& field, const std::string& importer = "");
  bool WriteField(const std::string& filename, FieldHandle& field, const std::string& exporter = "");

  bool ReadMatrix(const std::string& filename, MatrixHandle& matrix, const std::string& importer = "");
  bool WriteMatrix(const std::string& filename, MatrixHandle& matrix, const std::string& exporter = "");
   
  bool ReadNrrd(const std::string& filename, NrrdDataHandle& matrix, const std::string& importer = "");
  bool WriteNrrd(const std::string& filename, NrrdDataHandle& matrix, const std::string& exporter = "");

  bool ReadBundle(const std::string& filename, BundleHandle& matrix, const std::string& importer = "");
  bool WriteBundle(const std::string& filename, BundleHandle& matrix, const std::string& exporter = "");
  
  bool ReadColorMap(const std::string& filename, ColorMapHandle& matrix, const std::string& importer = "");
  bool WriteColorMap(const std::string& filename, ColorMapHandle& matrix, const std::string& exporter = "");

  bool ReadColorMap2(const std::string& filename, ColorMap2Handle& matrix, const std::string& importer = "");
  bool WriteColorMap2(const std::string& filename, ColorMap2Handle& matrix, const std::string& exporter = "");

  bool ReadPath(const std::string& filename, PathHandle& matrix, const std::string& importer = "");
  bool WritePath(const std::string& filename, PathHandle& matrix, const std::string& exporter = "");

  bool FileExists(const std::string& filename);
  bool CreateDir(const std::string& dirname);
  
};

} // end namespace SCIRunAlgo

#endif

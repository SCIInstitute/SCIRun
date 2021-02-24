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

#include <Modules/DataIO/ReadFile.h>
#include <Core/ImportExport/Nrrd/NrrdIEPlugin.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Logging/Log.h>
#include <boost/filesystem.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::DataIO;

MODULE_INFO_DEF(AutoReadFile, DataIO, SCIRun)

AutoReadFile::AutoReadFile() : Module(staticInfo_)
{
  INITIALIZE_PORT(Matrix);
  INITIALIZE_PORT(Field);
}

void AutoReadFile::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Variables::Filename, std::string("<load any file>"));
}

namespace detail
{
  using Filename = boost::filesystem::path;
  using FileExtension = std::string;
  template <typename DataHandle>
  using ReadFunc = std::function<DataHandle(const Filename&)>;
  template <typename DataHandle>
  using ReadFuncList = std::vector<ReadFunc<DataHandle>>;
  template <typename DataHandle>
  using PrioritizedReaderMap = std::map<FileExtension, ReadFuncList<DataHandle>>;

  //TODO
  //class HasLogger
  static LoggerHandle nullLogger;

  template <class DataHandle>
  class PluginReader
  {
  public:
    explicit PluginReader(const std::string& pluginFileTypeName) : pluginFileTypeName_(pluginFileTypeName) {}
    DataHandle operator()(const Filename& filename) const
    {
      auto pl = mgr_.get_plugin(pluginFileTypeName_);
      if (pl)
        return pl->readFile(filename.string(), nullLogger);
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE("No custom reader found to handle that file type.");
    }
  private:
    std::string pluginFileTypeName_;
    typename StaticIEPluginGetter<typename DataHandle::element_type>::Manager mgr_;
  };

  template <class DataHandle>
  class BuiltInReader
  {
  public:
    DataHandle operator()(const Filename& name) const
    {
      auto filename = name.string();
      auto stream = auto_istream(filename, nullLogger);
      if (!stream)
      {
        THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Error reading file '" + filename + "'.");
      }

      DataHandle handle;
      Pio(*stream, handle);

      if (!handle || stream->error())
      {
        THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Error reading data from file '" + filename + "'.");
      }
      return handle;
    }
  };

  template <class DataHandle>
  struct PrioritizedReaderMapBuilder
  {
    static PrioritizedReaderMap<DataHandle> value;
  };

  template <class DataHandle>
  class QuickReader
  {
  public:
    DataHandle read(const Filename& filename) const
    {
      auto ext = filename.extension().string();
      auto readerListIter = prioritizedReaderMap.find(ext);
      if (readerListIter != prioritizedReaderMap.end())
      {
        for (const auto& reader : readerListIter->second)
        {
          auto val = reader(filename);
          if (val)
            return val;
        }
        THROW_ALGORITHM_INPUT_ERROR_SIMPLE("No available reader could handle the file type.");
      }
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE("No reader found to handle that file type.");
    }
  private:
    PrioritizedReaderMap<DataHandle> prioritizedReaderMap = PrioritizedReaderMapBuilder<DataHandle>::value;
  };

  template <>
  PrioritizedReaderMap<MatrixHandle> PrioritizedReaderMapBuilder<MatrixHandle>::value =
  {
    // .mat type: try reading as Matlab first, then SCIRun format
    {".mat", { PluginReader<MatrixHandle>("Matlab Matrix"), BuiltInReader<MatrixHandle>() }},
    {".igb", { PluginReader<MatrixHandle>("IGBFile") }},
    {".txt", { PluginReader<MatrixHandle>("SimpleTextFile") }},
    {"", { PluginReader<MatrixHandle>("ECGSimFile"),
          PluginReader<MatrixHandle>("ECGSimFileBinary"),
          PluginReader<MatrixHandle>("SimpleTextFile") }},
  };

  template <>
  PrioritizedReaderMap<FieldHandle> PrioritizedReaderMapBuilder<FieldHandle>::value =
  {
    {".fld", { BuiltInReader<FieldHandle>() }},
    {".mat", { PluginReader<FieldHandle>("Matlab Field") }},
    {".elem", { PluginReader<FieldHandle>("TetVolField"),
               PluginReader<FieldHandle>("CARPMesh"),
               PluginReader<FieldHandle>("JHUFileToTetVol") }},
    {".lon", { PluginReader<FieldHandle>("CARPMesh") }},
    {".pts", { PluginReader<FieldHandle>("TriSurfField"),
              PluginReader<FieldHandle>("TetVolField"),
              PluginReader<FieldHandle>("CARPMesh"),
              PluginReader<FieldHandle>("CVRTI_FacPtsFileToTriSurf"),
              PluginReader<FieldHandle>("CurveField"),
              PluginReader<FieldHandle>("JHUFileToTetVol"),
              PluginReader<FieldHandle>("PointCloudField") }},
    {".fac", { PluginReader<FieldHandle>("TriSurfField"),
              PluginReader<FieldHandle>("CVRTI_FacPtsFileToTriSurf") }},
    {".tri", { PluginReader<FieldHandle>("TriSurfField"),
              PluginReader<FieldHandle>("CVRTI_FacPtsFileToTriSurf"),
              PluginReader<FieldHandle>("EcgsimFileToTriSurf") }},
    {".pos", { PluginReader<FieldHandle>("TriSurfField"),
              PluginReader<FieldHandle>("TetVolField"),
              PluginReader<FieldHandle>("CurveField"),
              PluginReader<FieldHandle>("CVRTI_FacPtsFileToTriSurf"),
              PluginReader<FieldHandle>("JHUFileToTetVol"),
              PluginReader<FieldHandle>("PointCloudField") }},
    {".edge", { PluginReader<FieldHandle>("CurveField") }},
    {".nhdr", { PluginReader<FieldHandle>("NrrdFile"),
               PluginReader<FieldHandle>("NrrdFile[DataOnElements,InvertParity]"),
               PluginReader<FieldHandle>("NrrdFile[DataOnElements]"),
               PluginReader<FieldHandle>("NrrdFile[DataOnNodes,InvertParity]"),
               PluginReader<FieldHandle>("NrrdFile[DataOnNodes]"), }},
    {".nrrd", { PluginReader<FieldHandle>("NrrdFile"),
               PluginReader<FieldHandle>("NrrdFile[DataOnElements,InvertParity]"),
               PluginReader<FieldHandle>("NrrdFile[DataOnElements]"),
               PluginReader<FieldHandle>("NrrdFile[DataOnNodes,InvertParity]"),
               PluginReader<FieldHandle>("NrrdFile[DataOnNodes]"), }},
    {".obj", { PluginReader<FieldHandle>("ObjToField") }},
    {".txt", { PluginReader<FieldHandle>("PointCloudField") }},
    {".tet", { PluginReader<FieldHandle>("TetVolField"),
              PluginReader<FieldHandle>("JHUFileToTetVol") }},
    {".stl", { PluginReader<FieldHandle>("TriSurfFieldSTL[ASCII]"),
              PluginReader<FieldHandle>("TriSurfFieldSTL[Binary]") }},
    {".m", { PluginReader<FieldHandle>("TriSurfFieldToM") }},
    {".vtk", {PluginReader<FieldHandle>("VtkToTriSurfField") }}
  };
}

void AutoReadFile::execute()
{
  if (needToExecute())
  {
    auto filename = get_state()->getValue(Variables::Filename).toFilename();

    if (filename.empty())
    {
      THROW_ALGORITHM_INPUT_ERROR("Empty filename, try again.");
    }

    if (!boost::filesystem::exists(filename))
    {
      THROW_ALGORITHM_INPUT_ERROR("File does not exist.");
    }

    bool asMatrix = oport_connected(Matrix);
    bool asField = oport_connected(Field);
    if (asMatrix && asField)
    {
      THROW_ALGORITHM_INPUT_ERROR("Please specify which type of file this is by connecting to only one output port.");
    }

    using namespace detail;
    if (asMatrix)
    {
      QuickReader<MatrixHandle> matrixReader;
      auto m = matrixReader.read(filename);
      sendOutput(Matrix, m);
    }
    else if (asField)
    {
      QuickReader<FieldHandle> fieldReader;
      auto f = fieldReader.read(filename);
      sendOutput(Field, f);
    }
    else
    {
      remark("No file loaded as no output port was connected.");
      return;
    }

    remark("Loaded file " + filename.string() + " as " + (asMatrix ? "matrix." : "field."));
  }
}

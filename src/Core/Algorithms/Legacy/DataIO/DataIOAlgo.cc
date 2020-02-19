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


#include <Core/Algorithms/Legacy/DataIO/DataIOAlgo.h>
#include <Core/Persistent/Persistent.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

DataIOAlgo::DataIOAlgo(LoggerHandle pr) : pr_(pr) {}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool DataIOAlgo::ReadField(const std::string& filename, FieldHandle& field, const std::string& importer)
{
  if (importer.empty())
  {
    PiostreamPtr stream = auto_istream(filename, pr_);
    if (!stream)
    {
      error("Error reading file '" + filename + "'.");
      return (false);
    }

    // Read the file
    Pio(*stream, field);
    if (!field || stream->error())
    {
      error("Error reading data from file '" + filename +"'.");
      return false;
    }

    return true;
  }
  else
  {
    FieldIEPluginManager mgr;
    FieldIEPlugin *pl = mgr.get_plugin(importer);
    if (pl)
    {
      field = pl->filereader(pr_, filename.c_str());
      if (field) return (true);
    }
    else
    {
      error("Unknown field reader plug-in");
      return false;
    }
  }
  return false;
}


bool DataIOAlgo::ReadMatrix(const std::string& filename, MatrixHandle& matrix, const std::string& importer)
{
  if (importer == "")
  {
    PiostreamPtr stream = auto_istream(filename, pr_);
    if (!stream)
    {
      error("Error reading file '" + filename + "'.");
      return (false);
    }

    // Read the file
    Pio(*stream, matrix);
    if (!matrix || stream->error())
    {
      error("Error reading data from file '" + filename +"'.");
      return (false);
    }
    return (true);
  }
  else
  {
    MatrixIEPluginManager mgr;
    MatrixIEPlugin *pl = mgr.get_plugin(importer);
    if (pl)
    {
      matrix = pl->fileReader_(pr_, filename.c_str());
      if (matrix.get_rep()) return (true);
    }
    else
    {
      error("Unknown matrix reader plug-in");
      return (false);
    }
  }
  return (false);
}


bool DataIOAlgo::ReadBundle(const std::string& filename, BundleHandle& bundle, const std::string& importer)
{
  if (importer != "")
  {
    error("Error no external importers are defined for bundles");
    return (false);
  }

  PiostreamPtr stream = auto_istream(filename, pr_);
  if (!stream)
  {
    error("Error reading file '" + filename + "'.");
    return (false);
  }

  // Read the file
  Pio(*stream, bundle);
  if (!bundle.get_rep() || stream->error())
  {
    error("Error reading data from file '" + filename +"'.");
    return (false);
  }

  return (true);
}
#endif

bool DataIOAlgo::readNrrd(const std::string& filename, NrrdDataHandle& nrrd, const std::string& importer)
{
  if (!importer.empty())
  {
    error("Error no external importers are defined for nrrds");
    return (false);
  }

  // Somehow we do not have support in the wrapper class for reading and writing nrrds

  if (filename.size() > 4)
  {
    const std::string ext = filename.substr(filename.size()-5,5);
    if (ext == ".nhdr" || ext == ".nrrd")
    {
      nrrd.reset(new NrrdData);
      if (!nrrd) return (false);

      NrrdData::lock_teem();
      if (nrrdLoad(nrrd->getNrrd(), airStrdup(filename.c_str()), 0))
      {
        char *err = biffGetDone(NRRD);
        error("Could not read nrrd '" + filename + "' because teem crashed for the following reason: " + err);
        free(err);
        NrrdData::unlock_teem();
        return (false);
      }

      NrrdData::unlock_teem();

      return (true);
    }
  }


  PiostreamPtr stream = auto_istream(filename, pr_);
  if (!stream)
  {
    error("Error reading file '" + filename + "'.");
    return (false);
  }

  // Read the file through Pio
  Pio(*stream, nrrd);
  if (!nrrd || stream->error())
  {
    error("Error reading data from file '" + filename +"'.");
    return (false);
  }

  return (true);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool DataIOAlgo::ReadColorMap(const std::string& filename, ColorMapHandle& colormap, const std::string& importer)
{
  if (!importer.empty())
  {
    error("Error no external importers are defined for colormaps");
    return (false);
  }

  PiostreamPtr stream = auto_istream(filename, pr_);
  if (!stream)
  {
    error("Error reading file '" + filename + "'.");
    return (false);
  }

  // Read the file
  Pio(*stream, colormap);
  if (!colormap.get_rep() || stream->error())
  {
    error("Error reading data from file '" + filename +"'.");
    return (false);
  }

  return (true);
}

bool DataIOAlgo::ReadColorMap2(const std::string& filename, ColorMap2Handle& colormap, const std::string& importer)
{
  if (importer != "")
  {
    error("Error no external importers are defined for colormaps");
    return (false);
  }

  PiostreamPtr stream = auto_istream(filename, pr_);
  if (!stream)
  {
    error("Error reading file '" + filename + "'.");
    return (false);
  }

  // Read the file
  Pio(*stream, colormap);
  if (!colormap.get_rep() || stream->error())
  {
    error("Error reading data from file '" + filename +"'.");
    return (false);
  }

  return (true);
}


bool DataIOAlgo::ReadPath(const std::string& filename, PathHandle& path, const std::string& importer)
{
  if (importer != "")
  {
    error("Error no external importers are defined for colormaps");
    return (false);
  }

  PiostreamPtr stream = auto_istream(filename, pr_);
  if (!stream)
  {
    error("Error reading file '" + filename + "'.");
    return (false);
  }

  // Read the file
  Pio(*stream, path);
  if (!path.get_rep() || stream->error())
  {
    error("Error reading data from file '" + filename +"'.");
    return (false);
  }

  return (true);
}


bool DataIOAlgo::WriteField(const std::string& filename, FieldHandle& field, const std::string& exporter)
{
  if (field.get_rep() == 0) return (false);

  if ((exporter == "text")||(exporter == "Text"))
  {
    PiostreamPtr stream = auto_ostream(filename, "Text", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, field);
    }
  }
  else if (exporter == "")
  {
    PiostreamPtr stream = auto_ostream(filename, "Binary", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, field);
    }
  }
  else
  {
    FieldIEPluginManager mgr;
    FieldIEPlugin *pl = mgr.get_plugin(exporter);
    if (pl)
    {
      pl->filewriter(pr_, field, filename.c_str());
      if (field.get_rep()) return (true);
    }
    else
    {
      error("Could not find requested exporter");
      return (false);
    }
    return (false);
  }
  return (true);
}


bool DataIOAlgo::WriteMatrix(const std::string& filename, MatrixHandle& matrix, const std::string& exporter)
{
  if (matrix.get_rep() == 0) return (false);

  if ((exporter == "text")||(exporter == "Text"))
  {
    PiostreamPtr stream = auto_ostream(filename, "Text", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, matrix);
    }
  }
  else if (exporter == "")
  {
    PiostreamPtr stream = auto_ostream(filename, "Binary", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, matrix);
    }
  }
  else
  {
    MatrixIEPluginManager mgr;
    MatrixIEPlugin *pl = mgr.get_plugin(exporter);
    if (pl)
    {
      pl->fileWriter_(pr_, matrix, filename.c_str());
      if (matrix.get_rep()) return (true);
    }
    else
    {
      error("Could not find requested exporter");
      return (false);
    }
    return (false);
  }
  return (true);
}


bool DataIOAlgo::WriteBundle(const std::string& filename, BundleHandle& bundle, const std::string& exporter)
{
  if (bundle.get_rep() == 0) return (false);

  if ((exporter == "text")||(exporter == "Text"))
  {
    PiostreamPtr stream = auto_ostream(filename, "Text", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, bundle);
    }
  }
  else if (exporter == "")
  {
    PiostreamPtr stream = auto_ostream(filename, "Binary", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, bundle);
    }
  }
  else
  {
    error("No exporters are supported for bundles");
    return (false);
  }
  return (true);
}
#endif


bool DataIOAlgo::writeNrrd(const std::string& filename, NrrdDataHandle nrrd, const std::string& exporter)
{
  if (!nrrd) return (false);

  if ((exporter == "text")||(exporter == "Text"))
  {
    PiostreamPtr stream = auto_ostream(filename, "Text", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, nrrd);
    }
  }
  else if (exporter.empty())
  {

    if (filename.size() > 4)
    {
      if (filename.substr(filename.size()-5,5) == ".nhdr" ||
          filename.substr(filename.size()-5,5) == ".nrrd")
      {
        NrrdData::lock_teem();

        NrrdIoState *nio = nrrdIoStateNew();
        // set encoding to be raw
        nio->encoding = nrrdEncodingArray[nrrdEncodingTypeGzip];
        // set format to be nrrd
        nio->format = nrrdFormatArray[nrrdFormatTypeNRRD];
        // set endian to be endian of machine
        nio->endian = AIR_ENDIAN;
        nio->zlibLevel = 6;
        if (filename.substr(filename.size()-5,5) == ".nhdr")
        {
          if (nio->format != nrrdFormatNRRD)
          {
            nio->format = nrrdFormatNRRD;
          }
        }


        if (nrrdSave( airStrdup(filename.c_str()), nrrd->getNrrd(), nio))
        {
          // Ugly...
          char *err = biffGetDone(NRRD);
          error("Could not save nrrd '" + filename + "' because teem crashed for the following reason: " + err);
          free(err);

          NrrdData::unlock_teem();

          return (false);
        }

        NrrdData::unlock_teem();
        return (true);
      }
    }

    // DO TO: ADD THE NERD CODE
    PiostreamPtr stream = auto_ostream(filename, "Binary", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, nrrd);
    }
  }
  else
  {
    error("No exporters are supported for nrrds");
    return (false);
  }
  return (true);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool DataIOAlgo::WriteColorMap(const std::string& filename, ColorMapHandle& colormap, const std::string& exporter)
{
  if (colormap.get_rep() == 0) return (false);

  if ((exporter == "text")||(exporter == "Text"))
  {
    PiostreamPtr stream = auto_ostream(filename, "Text", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, colormap);
    }
  }
  else if (exporter == "")
  {
    PiostreamPtr stream = auto_ostream(filename, "Binary", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, colormap);
    }
  }
  else
  {
    error("No exporters are supported for colormaps");
    return (false);
  }
  return (true);
}

bool DataIOAlgo::WriteColorMap2(const std::string& filename, ColorMap2Handle& colormap, const std::string& exporter)
{
  if (colormap.get_rep() == 0) return (false);

  if ((exporter == "text")||(exporter == "Text"))
  {
    PiostreamPtr stream = auto_ostream(filename, "Text", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, colormap);
    }
  }
  else if (exporter == "")
  {
    PiostreamPtr stream = auto_ostream(filename, "Binary", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, colormap);
    }
  }
  else
  {
    error("No exporters are supported for colormaps");
    return (false);
  }
  return (true);
}

bool DataIOAlgo::WritePath(const std::string& filename, PathHandle& path, const std::string& exporter)
{
  if (path.get_rep() == 0) return (false);

  if ((exporter == "text")||(exporter == "Text"))
  {
    PiostreamPtr stream = auto_ostream(filename, "Text", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, path);
    }
  }
  else if (exporter == "")
  {
    PiostreamPtr stream = auto_ostream(filename, "Binary", pr_);
    if (stream->error())
    {
      error("Could not open file for writing" + filename);
      return (false);
    }
    else
    {
      // Write the file
      Pio(*stream, path);
    }
  }
  else
  {
    error("No exporters are supported for paths");
    return (false);
  }
  return (true);
}
#endif

AlgorithmOutput DataIOAlgo::run(const AlgorithmInput& input) const
{
  throw "not implemented";
}

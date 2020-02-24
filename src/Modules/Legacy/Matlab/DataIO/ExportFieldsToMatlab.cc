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


#include <Modules/Legacy/Matlab/DataIO/ExportFieldsToMatlab.h>
#include <sstream>
#include <string>
#include <vector>

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <boost/filesystem/operations.hpp>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun::Modules::Matlab;
using namespace SCIRun::MatlabIO;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Matlab;

ALGORITHM_PARAMETER_DEF(Matlab, FieldNames);
ALGORITHM_PARAMETER_DEF(Matlab, FieldFormats);

MODULE_INFO_DEF(ExportFieldsToMatlab, Matlab, SCIRun)

ExportFieldsToMatlab::ExportFieldsToMatlab() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Filename);
}

void ExportFieldsToMatlab::setStateDefaults()
{
  get_state()->setValue(Variables::Filename, std::string());
  get_state()->setValue(Parameters::FieldNames, Variable::List());
  get_state()->setValue(Parameters::FieldFormats, Variable::List());
}

void ExportFieldsToMatlab::execute()
{
  auto filenameInputOption = getOptionalInput(Filename);
  auto fields = getRequiredDynamicInputs(InputField);

  //TODO: buggy with dynamic ports and same values
  //if (needToExecute())
  {
    auto state = get_state();

    if (filenameInputOption && *filenameInputOption)
    {
      auto filename = (*filenameInputOption)->value();
      state->setValue(Variables::Filename, filename);
    }


    auto filename = state->getValue(Variables::Filename).toFilename();

    if (filename.empty())
    {
      error("ExportDatatypesToMatlab: No file name was specified");
      return;
    }

    // Make sure we have a .mat extension
    if (filename.extension() != ".mat")
      filename += ".mat";

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // Make sure the path to the new file exists
    // If not make it and as well convert filename
    // to absolute path name
    if (!exists(filename))
    {
      error("Could not generate path to file");
      return;
    }
    //filename = filename.get_abs_filename();
    //guifilename_.set(filename);
    //get_ctx()->reset();


    if (!overwrite()) return;
#endif

    std::vector<std::string> fieldnames;

    auto iPorts = inputPorts();
    if (fields.size() != iPorts.size() - 2)
      warning("Problem in state of dynamic ports");
    auto fieldPortNameIterator = iPorts.begin();

    for (int i = 0; i < fields.size(); ++i)
    {
      fieldnames.push_back(state->getValue(Name((*fieldPortNameIterator++)->id().toString())).toString());
    }

    auto fieldformats = toStringVector(state->getValue(Parameters::FieldFormats).toVector());

    // Check the validity of the matrixnames
    matlabconverter translate(getLogger());

    for (size_t p = 0; p < fieldnames.size(); ++p)
    {
      if (!translate.isvalidmatrixname(fieldnames[p]))
      {
        error("ExportFieldsToMatlab: The matrix name specified is invalid");
        return;
      }
      for (auto q = 0; q < p; ++q)
      {
        if (fieldnames[q] == fieldnames[p])
        {
          error("ExportFieldsToMatlab: A matrix name is used twice");
          return;
        }
      }
    }

    try
    {
      matlabfile mfile;   // matlab file object contains all function for reading and writing matlab arrayd
      matlabarray ma;		// matlab style formatted array (can be stored in a matlabfile object)
      mfile.open(filename.string(),"w");   // open file for writing

      // Add an information tag to the data, so the origin of the file is known
      // There are 116 bytes of free data storage at the header of the file.
      // Do not start the file with 'SCI ', otherwise the file looks like a
      // native SCIRun file which uses the same extension.

      mfile.setheadertext("Matlab V5 compatible file generated by SCIRun [module ExportFieldsToMatlab version 1.1]");

      for (auto&& tup : zip(fields, fieldnames, fieldformats))
      {
        FieldHandle field;
        std::string name, format;
        boost::tie(field, name, format) = tup;
        // Convert the SCIRun matrixobject to a matlab object
        if (format == "struct array")
        {
          // translate the matrix into a matlab structured array, which
          // can also store some data from the property manager
          translate.converttostructmatrix();
        }

        if (format == "numeric array")
        {
          // only store the numeric parts of the data
          translate.converttonumericmatrix();
        }

        translate.sciFieldTOmlArray(field, ma);

        if (ma.isempty())
        {
          warning("One of the matrices is empty");
          continue; // Do not write empty matrices
        }
        // Every thing seems OK, so proceed and store the matrix in the file
        mfile.putmatlabarray(ma, name);
      }

      mfile.close();
    }
    catch (matlabconverter::error_type&)
    {
      error("ExportFieldsToMatlab: Error in the SCIRun to Matlab converter");
    }
    catch (matlabfile::could_not_open_file&)
    {
      error("ExportFieldsToMatlab: Could not open file");
    }
    catch (matlabfile::invalid_file_format&)
    {
      error("ExportFieldsToMatlab: Invalid file format");
    }
    catch (matlabfile::io_error&)
    {   // IO error from ferror
      error("ExportFieldsToMatlab: IO error");
    }
    catch (matlabfile::unknown_type&)
    {
      error("ExportFieldsToMatlab: Unknow type encountered");
    }
    catch (matlabfile::empty_matlabarray&)
    {
      error("ExportFieldsToMatlab: Empty Matlab array encountered");
    }
    catch (matlabfile::out_of_range&)
    {
      error("ExportFieldsToMatlab: Out of Range error");
    }
    catch (matlabfile::invalid_file_access&)
    {
      error("ExportFieldsToMatlab: Invalid file access");
    }
    catch (matlabfile::compression_error&)
    {
      error("ExportFieldsToMatlab: Compression error");
    }
    catch (matlabfile::internal_error&)
    {
      error("ExportFieldsToMatlab: Internal error");
    }
    catch (matlabfile::matfileerror&)
    {   // All other errors are classified as internal
      // matfileerrror is the base class on which all
      // other exceptions are based.
      error("ExportFieldsToMatlab: Matlab file writer error");
    }
  }
}

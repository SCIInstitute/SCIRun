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


/*
 * This module reads a matlab file and converts it to a SCIRun field
 *
 */

#include <Modules/Legacy/Matlab/DataIO/ImportFieldsFromMatlab.h>
#include <vector>

// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/String.h>

#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Matlab;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::MatlabIO;
using namespace Algorithms;
using namespace Matlab;

ALGORITHM_PARAMETER_DEF(Matlab, FieldInfoStrings);
ALGORITHM_PARAMETER_DEF(Matlab, PortChoices);

MODULE_INFO_DEF(ImportFieldsFromMatlab, Matlab, SCIRun)

ImportFieldsFromMatlab::ImportFieldsFromMatlab() : MatlabFileIndexModule(staticInfo_)
{
  INITIALIZE_PORT(Field1);
  INITIALIZE_PORT(Field2);
  INITIALIZE_PORT(Field3);
  INITIALIZE_PORT(Field4);
  INITIALIZE_PORT(Field5);
  INITIALIZE_PORT(Field6);
  INITIALIZE_PORT(Filename);
  INITIALIZE_PORT(FilenameOut);
}

void ImportFieldsFromMatlab::setStateDefaults()
{
  auto nones = makeHomogeneousVariableList([](size_t) { return std::string("<none>"); }, NUMPORTS);
  get_state()->setValue(Parameters::PortChoices, nones);
  get_state()->setValue(Variables::Filename, std::string());
  get_state()->connectSpecificStateChanged(Variables::Filename, [this]() { indexmatlabfile(); });
}

void ImportFieldsFromMatlab::postStateChangeInternalSignalHookup()
{
  indexmatlabfile();
}

void ImportFieldsFromMatlab::execute()
{
  executeImpl(Filename, FilenameOut);
}

DatatypeHandle ImportFieldsFromMatlab::processMatlabData(const matlabarray& ma) const
{
  FieldHandle mh;
  matlabconverter translate(getLogger());
  translate.mlArrayTOsciField(ma, mh);
  return mh;
}

int ImportFieldsFromMatlab::indexMatlabFile(matlabconverter& converter, const matlabarray& mlarray, std::string& infostring) const
{
  return converter.sciFieldCompatible(mlarray, infostring);
}

void MatlabFileIndexModule::executeImpl(const StringPortName<0>& filenameIn, const StringPortName<6>& filenameOut)
{
  auto fileOption = getOptionalInput(filenameIn);

  if (needToExecute())
  {
    auto state = get_state();
    if (fileOption && *fileOption)
  	{
      state->setValue(Variables::Filename, (*fileOption)->value());
  	}

    auto filename = state->getValue(Variables::Filename).toFilename().string();

    if (filename.empty())
    {
      error("No file name was specified");
      return;
    }

    indexmatlabfile();

    auto choices = toStringVector(state->getValue(Parameters::PortChoices).toVector());

    try
    {
      ScopedMatlabFileReader smfr(filename);
      for (int p = 0; p < numOutputPorts() - 1; ++p)
      {
        // Now read the matrix from file
        // The next function will open, read, and close the file
        // Any error will be exported as an exception.
        // The matlab classes are all based in the matfilebase class
        // which carries the definitions of the exceptions. These
        // definitions are inherited by all other "matlab classes"

        auto ma = readmatlabarray(smfr.mfile, choices[p]);

        // An empty array means something must have gone wrong
        // Or there is no data to put on this port.
        // Do not translate empty arrays, but continue to the
        // next output port.

        if (ma.isempty())
        {
          continue;
        }

        // The data is still in matlab format and the next function
        // creates a SCIRun matrix object

        auto data = processMatlabData(ma);

        send_output_handle(outputPorts()[p]->id(), data);
      }

      StringHandle filenameH(new String(filename));
      sendOutput(filenameOut, filenameH);
    }
    catch (matlabfile::could_not_open_file&)
    {
      error("Could not open file");
    }
    catch (matlabfile::invalid_file_format&)
    {
      error("Invalid file format");
    }
    catch (matlabfile::io_error&)
    {
      error("IO error");
    }
    catch (matlabfile::out_of_range&)
    {
      error("Out of range");
    }
    catch (matlabfile::invalid_file_access&)
    {
      error("Invalid file access");
    }
    catch (matlabfile::empty_matlabarray&)
    {
      error("Empty matlab array");
    }
    catch (matlabfile::matfileerror&)
    {
      error("Internal error in reader");
    }
  }
}

void MatlabFileIndexModule::indexmatlabfile()
{
  auto state = get_state();
  auto filename = state->getValue(Variables::Filename).toFilename().string();
  if (!filename.empty())
  {
    Variable::List matrixinfotexts;
    matlabconverter translate(getLogger());
    try
    {
      matlabfile mfile;
      // Open the .mat file
      // This function also scans through the file and makes
      // sure it is amat file and counts the number of arrays

      mfile.open(filename, "r");

      // all matlab data is stored in a matlabarray object
      matlabarray ma;
      int cindex = 0;		// compability index, which matlab array fits the SCIRun matrix best?
      int maxindex = 0;		// highest index found so far

      // Scan the file and see which matrices are compatible
      // Only those will be shown (you cannot select incompatible matrices).

      for (int p = 0; p < mfile.getnummatlabarrays(); p++)
      {
        ma = mfile.getmatlabarrayinfo(p); // do not load all the data fields
        std::string infotext;
        if ((cindex = indexMatlabFile(translate, ma, infotext)))
        {
          // in case we need to propose a matrix to load, select
          // the one that is most compatible with the data
          if (cindex > maxindex)
          {
            maxindex = cindex;
          }

          matrixinfotexts.emplace_back(Name(ma.getname()), infotext);
        }
      }

      mfile.close();

      state->setTransientValue(Parameters::FieldInfoStrings, matrixinfotexts);
    }
    catch (matlabfile::could_not_open_file&)
    {
      warning("Could not open file");
    }
    catch (matlabfile::invalid_file_format&)
    {
      warning("Invalid file format");
    }
    catch (matlabfile::io_error&)
    {
      warning("IO error");
    }
    catch (matlabfile::out_of_range&)
    {
      warning("Out of range");
    }
    catch (matlabfile::invalid_file_access&)
    {
      warning("Invalid file access");
    }
    catch (matlabfile::empty_matlabarray&)
    {
      warning("Empty matlab array");
    }
    catch (matlabfile::matfileerror&)
    {
      warning("Internal error in reader");
    }
  }
}

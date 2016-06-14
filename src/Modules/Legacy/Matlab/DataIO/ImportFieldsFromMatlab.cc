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

/*
 * This module reads a matlab file and converts it to a SCIRun matrix
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

using namespace SCIRun::Modules::Matlab;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::MatlabIO;
using namespace Algorithms;
using namespace Matlab;

ALGORITHM_PARAMETER_DEF(Matlab, FieldInfoStrings);
ALGORITHM_PARAMETER_DEF(Matlab, PortChoices);

const ModuleLookupInfo ImportFieldsFromMatlab::staticInfo_("ImportFieldsFromMatlab", "Matlab", "SCIRun");

ImportFieldsFromMatlab::ImportFieldsFromMatlab() : Module(staticInfo_)
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
}

namespace
{
  struct ScopedMatlabFileReader
  {
    explicit ScopedMatlabFileReader(const std::string& filename)
    {
      mfile.open(filename, "r");
    }
    ~ScopedMatlabFileReader()
    {
      mfile.close();
    }
    matlabfile mfile;
  };

  matlabarray readmatlabarray(matlabfile& mfile, const std::string& matlabName)
  {
    matlabarray marray;

    if (matlabName.empty())
    {
      // return an empty array
      return(marray);
    }

    if (matlabName == "<none>")
    {
      // return an empty array
      return(marray);
    }
    
    return mfile.getmatlabarray(matlabName);
  }
}

void ImportFieldsFromMatlab::postStateChangeInternalSignalHookup()
{
  indexmatlabfile();
}

void ImportFieldsFromMatlab::execute()
{
  auto fileOption = getOptionalInput(Filename);
  auto state = get_state();
  if (fileOption && *fileOption)
	{
    state->setValue(Variables::Filename, (*fileOption)->value());
	}

  auto filename = state->getValue(Variables::Filename).toFilename().string();

  if (filename.empty())
  {
    error("ImportFieldsFromMatlab: No file name was specified");
    return;
  }

  indexmatlabfile();

  update_state(Executing);

  auto choices = toStringVector(state->getValue(Parameters::PortChoices).toVector());

  try
  {
    ScopedMatlabFileReader smfr(filename);
    for (int p=0; p < NUMPORTS; ++p)
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

      FieldHandle mh;
      matlabconverter translate(getLogger());
      translate.mlArrayTOsciField(ma,mh);

      send_output_handle(outputPorts()[p]->id(), mh);
    }

    StringHandle filenameH(new String(filename));
    sendOutput(Filename, filenameH);
  }
  catch (matlabfile::could_not_open_file&)
  {
    error("ImportFieldsFromMatlab: Could not open file");
  }
  catch (matlabfile::invalid_file_format&)
  {
    error("ImportFieldsFromMatlab: Invalid file format");
  }
  catch (matlabfile::io_error&)
  {
    error("ImportFieldsFromMatlab: IO error");
  }
  catch (matlabfile::out_of_range&)
  {
    error("ImportFieldsFromMatlab: Out of range");
  }
  catch (matlabfile::invalid_file_access&)
  {
    error("ImportFieldsFromMatlab: Invalid file access");
  }
  catch (matlabfile::empty_matlabarray&)
  {
    error("ImportFieldsFromMatlab: Empty matlab array");
  }
  catch (matlabfile::matfileerror&)
  {
    error("ImportFieldsFromMatlab: Internal error in reader");
  }
}

void ImportFieldsFromMatlab::indexmatlabfile()
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
        if ((cindex = translate.sciFieldCompatible(ma, infotext)))
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
      warning("ImportFieldsFromMatlab: Could not open file");
    }
    catch (matlabfile::invalid_file_format&)
    {
      warning("ImportFieldsFromMatlab: Invalid file format");
    }
    catch (matlabfile::io_error&)
    {
      warning("ImportFieldsFromMatlab: IO error");
    }
    catch (matlabfile::out_of_range&)
    {
      warning("ImportFieldsFromMatlab: Out of range");
    }
    catch (matlabfile::invalid_file_access&)
    {
      warning("ImportFieldsFromMatlab: Invalid file access");
    }
    catch (matlabfile::empty_matlabarray&)
    {
      warning("ImportFieldsFromMatlab: Empty matlab array");
    }
    catch (matlabfile::matfileerror&)
    {
      warning("ImportFieldsFromMatlab: Internal error in reader");
    }
  }
}

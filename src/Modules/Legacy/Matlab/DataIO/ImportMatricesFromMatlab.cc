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

#include <Modules/Legacy/Matlab/DataIO/ImportMatricesFromMatlab.h>

// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>

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

MODULE_INFO_DEF(ImportMatricesFromMatlab, Matlab, SCIRun)

ImportMatricesFromMatlab::ImportMatricesFromMatlab() : MatlabFileIndexModule(staticInfo_)
{
  INITIALIZE_PORT(Matrix1);
  INITIALIZE_PORT(Matrix2);
  INITIALIZE_PORT(Matrix3);
  INITIALIZE_PORT(Matrix4);
  INITIALIZE_PORT(Matrix5);
  INITIALIZE_PORT(Matrix6);
  INITIALIZE_PORT(Filename);
  INITIALIZE_PORT(FilenameOut);
}

void ImportMatricesFromMatlab::setStateDefaults()
{
  auto nones = makeHomogeneousVariableList([](size_t) { return std::string("<none>"); }, NUMPORTS);
  get_state()->setValue(Parameters::PortChoices, nones);
  get_state()->setValue(Variables::Filename, std::string());
}

void ImportMatricesFromMatlab::postStateChangeInternalSignalHookup()
{
  indexmatlabfile();
}

void ImportMatricesFromMatlab::execute()
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
    error("ImportMatricesFromMatlab: No file name was specified");
    return;
  }

  indexmatlabfile();

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

      MatrixHandle mh;
      matlabconverter translate(getLogger());
      translate.mlArrayTOsciMatrix(ma, mh);

      send_output_handle(outputPorts()[p]->id(), mh);
    }

    StringHandle filenameH(new String(filename));
    sendOutput(Filename, filenameH);
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

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
  get_state()->connectSpecificStateChanged(Variables::Filename, [this]() { indexmatlabfile(); });
}

void ImportMatricesFromMatlab::postStateChangeInternalSignalHookup()
{
  indexmatlabfile();
}

void ImportMatricesFromMatlab::execute()
{
  executeImpl(Filename, FilenameOut);
}

DatatypeHandle ImportMatricesFromMatlab::processMatlabData(const matlabarray& ma) const
{
  MatrixHandle mh;
  matlabconverter translate(getLogger());
  translate.mlArrayTOsciMatrix(ma, mh);
  return mh;
}

int ImportMatricesFromMatlab::indexMatlabFile(matlabconverter& converter, const matlabarray& mlarray, std::string& infostring) const
{
  return converter.sciMatrixCompatible(mlarray, infostring);
}

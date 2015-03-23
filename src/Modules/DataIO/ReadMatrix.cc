/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// @todo Documentation Modules/DataIO/ReadMatrix.cc

#include <Modules/DataIO/ReadMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

OriginalReadMatrixModule::OriginalReadMatrixModule() : Module(ModuleLookupInfo("OriginalReadMatrix", "DataIO", "SCIRun"))
{
  INITIALIZE_PORT(Filename);
  INITIALIZE_PORT(FileLoaded);
  INITIALIZE_PORT(MatrixLoaded);
}

/// @todo: unit test. Requires algorithm injection/factory for mocking, to be able to isolate the "optional file argument" part.
void OriginalReadMatrixModule::execute()
{
  /// @todo: this will be a common pattern for file loading. Perhaps it will be a base class method someday...

  auto fileOption = getOptionalInput(Filename);

  if (fileOption && *fileOption)
  {
    get_state()->setValue(SCIRun::Core::Algorithms::Variables::Filename, (*fileOption)->value());
  }
  auto path = get_state()->getValue(Variables::Filename).toFilename();
  filename_ = path.string();

  if (needToExecute())
  {
    algo().set(Variables::Filename, filename_);
    auto output = algo().run_generic(makeNullInput());
    sendOutputFromAlgorithm(MatrixLoaded, output);
    sendOutput(FileLoaded, boost::make_shared<String>(filename_));
  }
}


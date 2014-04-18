/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Modules/Math/CreateMatrix.cc

#include <iostream>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Modules/Math/CreateMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

SCIRun::Core::Algorithms::AlgorithmParameterName CreateMatrixModule::TextEntry("TextEntry");

CreateMatrixModule::CreateMatrixModule() : Module(ModuleLookupInfo("CreateMatrix", "Math", "SCIRun")) 
{
  INITIALIZE_PORT(EnteredMatrix);
}

void CreateMatrixModule::setStateDefaults()
{
  auto state = get_state();
  state->setValue(TextEntry, std::string());
}

void CreateMatrixModule::execute()
{
  DenseMatrixHandle matrix(boost::make_shared<DenseMatrix>());
  try
  {
	  std::string matrixString = get_state()->getValue(TextEntry).getString();
	
    if (!matrixString.empty())
    {
      matrixString += "\n";
      std::istringstream reader(matrixString);

      reader >> *matrix;
    }
  }
  catch (...)
  {
  	THROW_ALGORITHM_INPUT_ERROR("Matrix parsing failed.");
  }
  sendOutput(EnteredMatrix, matrix);
}

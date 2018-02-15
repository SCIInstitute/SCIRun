/*
 For more information, please see: http://software.sci.utah.edu
 The MIT License
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Math/BasicPlotter.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

MODULE_INFO_DEF(BasicPlotter, Math, SCIRun)

ALGORITHM_PARAMETER_DEF(Math, PlotTitle);
ALGORITHM_PARAMETER_DEF(Math, DataTitle);
ALGORITHM_PARAMETER_DEF(Math, XAxisLabel);
ALGORITHM_PARAMETER_DEF(Math, YAxisLabel);
ALGORITHM_PARAMETER_DEF(Math, VerticalAxisVisible);
ALGORITHM_PARAMETER_DEF(Math, HorizontalAxisVisible);
ALGORITHM_PARAMETER_DEF(Math, VerticalAxisPosition);
ALGORITHM_PARAMETER_DEF(Math, HorizontalAxisPosition);

BasicPlotter::BasicPlotter() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(IndependentVariable);
  INITIALIZE_PORT(DependentVariables);
}

void BasicPlotter::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::PlotTitle, std::string("Plot title"));
  state->setValue(Parameters::DataTitle, std::string("Data title"));
  state->setValue(Parameters::XAxisLabel, std::string("x axis"));
  state->setValue(Parameters::YAxisLabel, std::string("y axis"));
  state->setValue(Parameters::VerticalAxisVisible, true);
  state->setValue(Parameters::HorizontalAxisVisible, true);
  state->setValue(Parameters::VerticalAxisPosition, 0.0);
  state->setValue(Parameters::HorizontalAxisPosition, 0.0);
}

void BasicPlotter::execute()
{
  auto input_matrix = getRequiredInput(InputMatrix);

  if (needToExecute())
  {
    if (input_matrix->empty())
    {
      THROW_INVALID_ARGUMENT("Empty matrix input.");
    }
    get_state()->setTransientValue(Variables::InputMatrix, input_matrix);
  }
}

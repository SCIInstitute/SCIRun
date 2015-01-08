/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>
//#include <Modules/Legacy/Forward/BuildBEMatrixImpl.h>
//#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Inverse;
using namespace SCIRun::Dataflow::Networks;

const ModuleLookupInfo SolveInverseProblemWithTikhonov::staticInfo_("SolveInverseProblemWithTikhonov", "Inverse", "SCIRun");

SolveInverseProblemWithTikhonov::SolveInverseProblemWithTikhonov() : Module(staticInfo_)
{
  INITIALIZE_PORT(ForwardMatrix);
  INITIALIZE_PORT(WeightingInSourceSpace);
  INITIALIZE_PORT(MeasuredPotentials);
  INITIALIZE_PORT(WeightingInSensorSpace);
  INITIALIZE_PORT(InverseSolution);
  INITIALIZE_PORT(RegularizationParameter);
  INITIALIZE_PORT(RegInverse);
}

void SolveInverseProblemWithTikhonov::setStateDefaults()
{
  //TODO
}

void SolveInverseProblemWithTikhonov::execute()
{
  //auto inputs = getRequiredDynamicInputs(Surface);

  if (needToExecute())
  {
    auto state = get_state();
    // auto fieldNames = state->getValue(Parameters::FieldNameList).toVector();
    // auto boundaryConditions = state->getValue(Parameters::BoundaryConditionList).toVector();
    // auto outsideConds = state->getValue(Parameters::OutsideConductivityList).toVector();
    // auto insideConds = state->getValue(Parameters::InsideConductivityList).toVector();
    //
    // BuildBEMatrixImpl impl(fieldNames, boundaryConditions, outsideConds, insideConds, this);
    //
    // MatrixHandle transferMatrix = impl.executeImpl(inputs);
    // auto fieldTypes = impl.getInputTypes();
    // state->setTransientValue(Parameters::FieldTypeList, fieldTypes);
    //
    // sendOutput(BEM_Forward_Matrix, transferMatrix);
  }
}

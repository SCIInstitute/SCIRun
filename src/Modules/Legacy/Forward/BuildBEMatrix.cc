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
 *  BuildBEMatrix.cc:
 *
 *  Written by:
 *   Saeed Babaeizadeh - Northeastern University
 *   Michael Callahan - Department of Computer Science - University of Utah
 *   May, 2003
 *
 *  Updated by:
 *   Burak Erem - Northeastern University
 *   January, 2012
 *
 */

#include <Modules/Legacy/Forward/BuildBEMatrix.h>
#include <Modules/Legacy/Forward/BuildBEMatrixImpl.h>
#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Forward;

MODULE_INFO_DEF(BuildBEMatrix, Forward, SCIRun)

BuildBEMatrix::BuildBEMatrix() : Module(staticInfo_)
{
  INITIALIZE_PORT(Surface);
  INITIALIZE_PORT(BEM_Forward_Matrix);
}

void BuildBEMatrix::setStateDefaults()
{
  get_state()->setValue(Parameters::FieldNameList, VariableList());
  get_state()->setValue(Parameters::BoundaryConditionList, VariableList());
  get_state()->setValue(Parameters::OutsideConductivityList, VariableList());
  get_state()->setValue(Parameters::InsideConductivityList, VariableList());
}

void BuildBEMatrix::execute()
{
  auto inputs = getRequiredDynamicInputs(Surface);

  if (needToExecute())
  {
    auto state = get_state();
    auto fieldNames = state->getValue(Parameters::FieldNameList).toVector();
    auto boundaryConditions = state->getValue(Parameters::BoundaryConditionList).toVector();
    auto outsideConds = state->getValue(Parameters::OutsideConductivityList).toVector();
    auto insideConds = state->getValue(Parameters::InsideConductivityList).toVector();

    BuildBEMatrixImpl impl(fieldNames, boundaryConditions, outsideConds, insideConds, this);
    MatrixHandle transferMatrix = impl.executeImpl(inputs);
    auto fieldTypes = impl.getInputTypes();
    state->setTransientValue(Parameters::FieldTypeList, fieldTypes);

    sendOutput(BEM_Forward_Matrix, transferMatrix);
  }
}

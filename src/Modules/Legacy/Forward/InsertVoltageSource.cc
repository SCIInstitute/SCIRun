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
 *  InsertVoltageSource: Insert a voltage source
 *
 *  Written by:
 *   David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   January 2002
 *
 */

#include <Modules/Legacy/Forward/InsertVoltageSource.h>
#include <Core/Algorithms/Legacy/Forward/InsertVoltageSourceAlgo.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Forward;

MODULE_INFO_DEF(InsertVoltageSource, Forward, SCIRun)

InsertVoltageSource::InsertVoltageSource() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputFEMesh);
  INITIALIZE_PORT(VoltageSource);
  INITIALIZE_PORT(OutputFEMesh);
  INITIALIZE_PORT(OutputDirichletMatrix);
}

void InsertVoltageSource::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::InterpolateOutside, true);
  state->setValue(Parameters::GroundFirst, false);
}

void InsertVoltageSource::execute()
{
  auto inputField =  getRequiredInput(InputFEMesh);
  auto voltageSource = getRequiredInput(VoltageSource);

  FieldInformation fi(inputField);
  if (fi.is_pointcloudmesh())
  {
    error("FEMesh is a point cloud mesh, the FE mesh needs to have elements");
    return;
  }

  FieldInformation fis(voltageSource);

  if (fis.is_nodata())
  {
    error("VoltageSource needs to contain data");
    return;
  }

  auto state = get_state();
  auto groundFirst = state->getValue(Parameters::GroundFirst).toBool();
  auto outside = state->getValue(Parameters::InterpolateOutside).toBool();

  if (groundFirst)
  {
    if (voltageSource->vmesh()->num_nodes() == 0)
    {
      error("VoltageSource field does not have any nodes");
      return;
    }
  }

  if (needToExecute())
  {
    FieldHandle outputField(inputField->clone());
    DenseMatrixHandle dirichletMatrix;
    InsertVoltageSourceAlgo algo(groundFirst, outside);
    algo.ExecuteAlgorithm(voltageSource, outputField, dirichletMatrix);

    sendOutput(OutputFEMesh, outputField);
    sendOutput(OutputDirichletMatrix, dirichletMatrix);
  }
}

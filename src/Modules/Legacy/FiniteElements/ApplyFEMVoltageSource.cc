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


/*
*  ApplyFEMVoltageSource.cc:  Builds the RHS of the FE matrix for voltage sources
*
*  Written by:
*   David Weinstein
*   University of Utah
*   May 1999
*  Modified by:
*   Alexei Samsonov, March 2001
*   Frank B. Sachse, February 2006
*
*/

#include <Modules/Legacy/FiniteElements/ApplyFEMVoltageSource.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::FiniteElements;

const ModuleLookupInfo ApplyFEMVoltageSource::staticInfo_("ApplyFEMVoltageSource", "FiniteElements", "SCIRun");
const AlgorithmParameterName ApplyFEMVoltageSource::ApplyDirichlet("ApplyDirichlet");

ApplyFEMVoltageSource::ApplyFEMVoltageSource() : Module(staticInfo_)
{
  INITIALIZE_PORT(Mesh);
  INITIALIZE_PORT(StiffnessMatrix);
  INITIALIZE_PORT(RHS);
  INITIALIZE_PORT(Dirichlet);
  INITIALIZE_PORT(ForwardMatrix);
  INITIALIZE_PORT(OutputRHS);
}

void ApplyFEMVoltageSource::setStateDefaults()
{
  auto state = get_state();
  state->setValue(ApplyDirichlet, 0);
}

void ApplyFEMVoltageSource::execute()
{
  auto inputMesh = getRequiredInput(Mesh);
  auto stiffnessMatrix = getRequiredInput(StiffnessMatrix);
  auto rhsMatrix = getOptionalInput(RHS);
  auto dirichlet = getOptionalInput(Dirichlet);

  if (needToExecute())
  {
    auto state = get_state();
    auto applyDirichlet = state->getValue(ApplyDirichlet).toBool();
    std::cout << "applyDirichlet: " << applyDirichlet << std::endl;
  }

  /*
  auto inputField =  getRequiredInput(InputFEMesh);
  auto voltageSource = getRequiredInput(VoltageSource);

  if (needToExecute())
  {
  auto state = get_state();
  //auto useLinear = state->getValue
  sendOutput(OutputFEMesh, inputField);

  }
  */
}

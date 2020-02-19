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
#include <Core/Algorithms/Legacy/FiniteElements/ApplyFEM/ApplyFEMVoltageSourceAlgo.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::FiniteElements;

MODULE_INFO_DEF(ApplyFEMVoltageSource, FiniteElements, SCIRun)
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

  auto state = get_state();
  auto applyDirichlet = state->getValue(ApplyDirichlet).toBool();

  DenseMatrixHandle odirichletMatrix;

  if (applyDirichlet)
  {
    if (!dirichlet)
    {
      warning("There are no dirichlet boundary conditions");
      odirichletMatrix.reset(new DenseMatrix(1, 2));
      (*odirichletMatrix) << 0, 0.0;
    }
    else
    {
      odirichletMatrix.reset((*dirichlet)->clone());
    }
  }
  else
  {
    odirichletMatrix.reset(new DenseMatrix(1, 2));
    (*odirichletMatrix) << 0, 0.0;
  }

  if (!matrixIs::sparse(stiffnessMatrix))
  {
    error("Input stiffness matrix wasn't sparse.");
    return;
  }

  if (stiffnessMatrix->nrows() != stiffnessMatrix->ncols())
  {
    error("Input stiffness matrix wasn't square.");
    return;
  }

  if (needToExecute())
  {
    unsigned int nsize = stiffnessMatrix->ncols();
    MatrixHandle stiffCopy(stiffnessMatrix->clone());
    SparseRowMatrixHandle mat(castMatrix::toSparse(stiffCopy));
    DenseColumnMatrixHandle rhs(new DenseColumnMatrix(nsize));

    if (rhsMatrix)
    {
      DenseColumnMatrixHandle rhsIn(castMatrix::toColumn((*rhsMatrix)));

      if (rhsIn && (rhsIn->nrows() == nsize))
      {
        for (unsigned int i = 0; i < nsize; i++)
          (*rhs)[i] = (*rhsIn)[i];
      }
      else
      {
        rhs->setZero(nsize);
      }
    }
    else
    {
      rhs->setZero(nsize);
    }

    ApplyFEMVoltageSourceAlgo algo;
    algo.ExecuteAlgorithm(odirichletMatrix, rhs, mat);

    MatrixHandle forwardMatrix(mat);
    sendOutput(ForwardMatrix, forwardMatrix);

    MatrixHandle outputrhs(rhs);
    sendOutput(OutputRHS, outputrhs);
  }
}

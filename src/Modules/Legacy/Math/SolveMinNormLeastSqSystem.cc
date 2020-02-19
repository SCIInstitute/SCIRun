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


///
///@class SolveMinNormLeastSqSystem
///@brief Select a row or column of a matrix
///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  June 1999
///
///@details
/// This module computes the minimal norm, least squared solution to a
///  nx3 linear system.
/// Given four input ColumnMatrices (v0,v1,v2,b),
///  find the three coefficients (w0,w1,w2) that minimize:
///  | (w0v0 + w1v1 + w2v2) - b |.
/// If more than one minimum exists (the system is under-determined),
///  choose the coefficients such that (w0,w1,w2) has minimum norm.
/// We output the vector (w0,w1,w2) as a row-matrix,
///  and we output the ColumnMatrix (called x), which is: | w0v0 + w1v1 + w2v2 |.
///

#include <Modules/Legacy/Math/SolveMinNormLeastSqSystem.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Math/Mat.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixIO.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

SolveMinNormLeastSqSystem::SolveMinNormLeastSqSystem()
: Module(ModuleLookupInfo("SolveMinNormLeastSqSystem", "Math", "SCIRun"), false)
{
  INITIALIZE_PORT(BasisVector1);
  INITIALIZE_PORT(BasisVector2);
  INITIALIZE_PORT(BasisVector3);
  INITIALIZE_PORT(TargetVector);
  INITIALIZE_PORT(WeightVector);
  INITIALIZE_PORT(ResultVector);
}

void SolveMinNormLeastSqSystem::execute()
{
  const int numInputs = 4;
  std::vector<DenseColumnMatrixHandle> in(numInputs);
  in[0] = getRequiredInput(BasisVector1);
  in[1] = getRequiredInput(BasisVector2);
  in[2] = getRequiredInput(BasisVector3);
  in[3] = getRequiredInput(TargetVector);

  int size = in[0]->rows();
  for (int i = 1; i < numInputs; i++)
  {
    if ( in[i]->rows() != size )
    {
      THROW_ALGORITHM_INPUT_ERROR("ColumnMatrices are different sizes");
    }
  }

  double *A[3];
  for (int i = 0; i < 3; i++)
  {
    A[i] = in[i]->data();
  }
  auto b = in[3]->data();
  DenseColumnMatrixHandle bprime(new DenseColumnMatrix(size));
  DenseColumnMatrixHandle x(new DenseColumnMatrix(3));

  min_norm_least_sq_3(A, b, x->data(), bprime->data(), size);

  sendOutput(WeightVector, x);
  sendOutput(ResultVector, bprime);
}

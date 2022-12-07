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


/// @todo Documentation Modules/Legacy/Math/AddKnownsToLinearSystem.cc

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Legacy/Math/AddKnownsToLinearSystem.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun;

MODULE_INFO_DEF(AddKnownsToLinearSystem, Math, SCIRun)

AddKnownsToLinearSystem::AddKnownsToLinearSystem() : Module(staticInfo_, false)
{
 INITIALIZE_PORT(LHS_Matrix);
 INITIALIZE_PORT(RHS_Vector);
 INITIALIZE_PORT(X_Vector);
 INITIALIZE_PORT(OutPutLHSMatrix);
 INITIALIZE_PORT(OutPutRHSVector);
}

void AddKnownsToLinearSystem::execute()
{
  auto lhs = getRequiredInput(LHS_Matrix);;
  auto rhs = getOptionalInput(RHS_Vector);;
  auto x = getRequiredInput(X_Vector);

  if (needToExecute())
  {
    auto output = algo().run(withInputData((LHS_Matrix,lhs)(RHS_Vector, optionalAlgoInput(rhs))(X_Vector,x)));

    sendOutputFromAlgorithm(OutPutLHSMatrix,output);
    sendOutputFromAlgorithm(OutPutRHSVector,output);
  }
}

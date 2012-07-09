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

#include <iostream>
#include <stdexcept>
#include <Modules/Math/EvaluateLinearAlgebraBinary.h>
#include <Algorithms/Math/EvaluateLinearAlgebraBinary.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/DenseMatrix.h> //TODO DAN: try to remove this--now it's needed to convert pointers, but actually this module shouldn't need the full def of DenseMatrix.

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Algorithms::Math;
using namespace SCIRun::Domain::Datatypes;
using namespace SCIRun::Domain::Networks;
//TODO DAN

EvaluateLinearAlgebraBinaryModule::EvaluateLinearAlgebraBinaryModule() :
Module("EvaluateLinearAlgebraBinary")
{
}


std::string EvaluateLinearAlgebraBinaryModule::inputPortNames[] = {"InputLHS_VIA_TEMPLATE_REC", "InputRHS_VIA_TEMPLATE_REC"};


void EvaluateLinearAlgebraBinaryModule::execute()
{
  DatatypeHandleOption lhs = get_input_handle(0);
  if (!lhs)
    throw std::logic_error("TODO DAN Input data (lhs) required, need to move this check to Module base class!");
  DatatypeHandleOption rhs = get_input_handle(1);
  if (!rhs)
    throw std::logic_error("TODO DAN Input data (rhs) required, need to move this check to Module base class!");

  DenseMatrixHandle lhsInput = boost::dynamic_pointer_cast<DenseMatrix>(*lhs); //TODO DAN: clean
  DenseMatrixHandle rhsInput = boost::dynamic_pointer_cast<DenseMatrix>(*rhs); //TODO DAN: clean
  if (!lhsInput || !rhsInput)
  {
    //TODO DAN log error? send null? check standard practice.
    return;
  }

  ModuleStateHandle state = get_state();
  EvaluateLinearAlgebraBinaryAlgorithm::Parameters oper = boost::any_cast<EvaluateLinearAlgebraBinaryAlgorithm::Parameters>((*state)["Operation"]);

  EvaluateLinearAlgebraBinaryAlgorithm algo; //TODO DAN inject
  DenseMatrixHandle output = algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(lhsInput, rhsInput), oper); 

  send_output_handle(0, output);
}
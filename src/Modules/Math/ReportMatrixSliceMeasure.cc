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

#include <Modules/Math/ReportMatrixSliceMeasure.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Module.h>
#include <Core/Algorithms/Math/ReportMatrixSliceMeasureAlgo.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

/// @class ReportMatrixSliceMeasure
/// @brief This module computes a measure on each row or column of the input matrix
/// and stores the result in the output matrix.

const ModuleLookupInfo ReportMatrixSliceMeasure::staticInfo_("ReportMatrixSliceMeasure", "Math", "SCIRun");

ReportMatrixSliceMeasure::ReportMatrixSliceMeasure() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputMatrix);
}

void ReportMatrixSliceMeasure::setStateDefaults()
{
  setStateIntFromAlgo(Variables::Operator);
  setStateIntFromAlgo(Variables::Method);
}

void
ReportMatrixSliceMeasure::execute()
{
  auto input = getRequiredInput(InputMatrix);
  
  if (needToExecute())
  {
    
    setAlgoIntFromState(Variables::Operator);
    setAlgoIntFromState(Variables::Method);
    
    auto output = algo().run_generic(withInputData((InputMatrix, input)));
    sendOutputFromAlgorithm(OutputMatrix, output);
    
  }
}




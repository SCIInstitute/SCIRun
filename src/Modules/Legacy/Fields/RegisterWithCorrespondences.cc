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

#include <Modules/Legacy/Fields/RegisterWithCorrespondences.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Legacy/Fields/RegisterWithCorrespondences.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>


using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Fields;

/// @class RegisterWithCorrespondences
/// @brief This module allows you to morph using a thin plate spline algorithm
/// one point set or mesh to another point set or mesh. 

const ModuleLookupInfo RegisterWithCorrespondences::staticInfo_("RegisterWithCorrespondences","ChangeFieldData","SCIRun");

RegisterWithCorrespondences::RegisterWithCorrespondences() : Module(staticInfo_)
{
	INITIALIZE_PORT(InputField);
	INITIALIZE_PORT(Correspondences1);
	INITIALIZE_PORT(Correspondences2);
	INITIALIZE_PORT(OutputField);
}

void RegisterWithCorrespondences::setStateDefaults() {
	setStateIntFromAlgo(Variables::Operator);
}

void RegisterWithCorrespondences::execute()
{
	auto input1 = getRequiredInput(InputField);
	auto input2 = getRequiredInput(Correspondences1);
	auto input3 = getRequiredInput(Correspondences2);
  if (needToExecute())
  {
    setAlgoIntFromState(Variables::Operator);
    auto output = algo().run(withInputData((InputField, input1)(Correspondences1, input2)(Correspondences2, input3)));
    sendOutputFromAlgorithm(OutputField, output);
  }
}


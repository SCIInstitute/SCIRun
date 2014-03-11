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

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Fields/InterfaceWithCleaver.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

InterfaceWithCleaverModule::InterfaceWithCleaverModule() : Module(ModuleLookupInfo("InterfaceWithCleaver", "NewField", "SCIRun"))
{
    INITIALIZE_PORT(InputField1);
    INITIALIZE_PORT(InputField2);
    INITIALIZE_PORT(OutputField);
}


void InterfaceWithCleaverModule::setStateDefaults()
{
  setStateBoolFromAlgo(InterfaceWithCleaverAlgorithm::VerboseCheckBox);
  setStateBoolFromAlgo(InterfaceWithCleaverAlgorithm::PaddingCheckBox);
  setStateBoolFromAlgo(InterfaceWithCleaverAlgorithm::AbsoluteVolumeScalingRadioButton);
  setStateBoolFromAlgo(InterfaceWithCleaverAlgorithm::RelativeVolumeScalingRadioButton);
  setStateDoubleFromAlgo(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_X);
  setStateDoubleFromAlgo(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Y); 
  setStateDoubleFromAlgo(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Z);  
}

void InterfaceWithCleaverModule::execute()
{
  auto field1 = getRequiredInput(InputField1);
  auto field2 = getRequiredInput(InputField2);

  auto state = get_state();
  
  setAlgoBoolFromState(InterfaceWithCleaverAlgorithm::VerboseCheckBox);
  //setAlgoBoolFromState(InterfaceWithCleaverAlgorithm::PaddingCheckBox);
  setAlgoBoolFromState(InterfaceWithCleaverAlgorithm::AbsoluteVolumeScalingRadioButton);
  setAlgoBoolFromState(InterfaceWithCleaverAlgorithm::RelativeVolumeScalingRadioButton);
  setAlgoDoubleFromState(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_X);
  setAlgoDoubleFromState(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Y);
  setAlgoDoubleFromState(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Z);

  algo().set(InterfaceWithCleaverAlgorithm::PaddingCheckBox, true);

  auto output = algo().run_generic(make_input((InputField1, field1)(InputField2, field2)));
  
  sendOutputFromAlgorithm(OutputField,output);
}

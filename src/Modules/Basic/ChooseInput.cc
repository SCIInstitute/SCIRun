/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Basic/ChooseInput.h>

using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::FlowControl;

const ModuleLookupInfo ChooseInput::staticInfo_("ChooseInput", "Flow Control", "SCIRun");

ALGORITHM_PARAMETER_DEF(FlowControl, PortIndex);
ALGORITHM_PARAMETER_DEF(FlowControl, PortMax);

ChooseInput::ChooseInput()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(Input);
  INITIALIZE_PORT(Output);
}

void ChooseInput::setStateDefaults()
{
  get_state()->setValue(Parameters::PortIndex, 0);
  get_state()->setTransientValue(Parameters::PortMax, 0);
}

void ChooseInput::execute()
{
  auto inputs = getRequiredDynamicInputs(Input);
  auto index = get_state()->getValue(Parameters::PortIndex).toInt();
  if (index < inputs.size())
  {
    sendOutput(Output, inputs[index]);
  }
  else
  {
    error("Port index out of range: " + boost::lexical_cast<std::string>(index));
  }
}

void ChooseInput::portAddedSlot(const ModuleId& mid, const PortId&)
{
  //TODO: redesign with non-virtual slot method and virtual hook that ensures module id is the same as this
  if (mid == id_)
  {
    portChangeImpl();
  }
}

void ChooseInput::portRemovedSlot(const ModuleId& mid, const PortId&)
{
  //TODO: redesign with non-virtual slot method and virtual hook that ensures module id is the same as this
  if (mid == id_)
  {
    portChangeImpl();
  }
}

void ChooseInput::portChangeImpl()
{
  int inputs = num_input_ports() - 1; // -1 for empty end
  get_state()->setTransientValue(Parameters::PortMax, inputs);
}
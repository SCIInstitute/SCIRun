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


#ifndef DATAFLOW_NETWORK_MODULEPORTDESCRIPTIONTAGSVARIADIC_H
#define DATAFLOW_NETWORK_MODULEPORTDESCRIPTIONTAGSVARIADIC_H

#include <Dataflow/Network/ModulePortDescriptionTags.h>
#include <Dataflow/Network/share.h>

namespace SCIRun::Modules
{
  #define INPUT_PORT2(index, name, type) static constexpr inputPortName(int i) { return (i == index) ? #name : "[undefined]"; } \
    Dataflow::Networks::StaticPortName< ATTACH_NAMESPACE(type), index > name;


  template <typename Input, typename ...Inputs>
  struct HasInputPorts
  {
    static constexpr size_t NumIPorts2 = sizeof...(Inputs) + 1;
    static constexpr std::deque<Dataflow::Networks::InputPortDescription> inputPortDescriptions()
    {
      return PortDesc<Input>::get() + HasInputPorts<Inputs...>::inputPortDescriptions();
    }
  };

  template <typename ...Inputs>
  struct HasInputPorts
  {
    static constexpr size_t NumIPorts2 = sizeof...(Inputs);
    static constexpr std::vector<Dataflow::Networks::InputPortDescription> inputPortDescriptions();
  };

/*
  template <class ModuleType>
  struct IPortDescriber2<ModuleType>
  {
    static constexpr std::vector<Dataflow::Networks::InputPortDescription> inputs()
    {
      return ModuleType::inputPortDescription(ModuleType::inputPort0Name(), ModuleType::inputPort1Name(), ModuleType::inputPort2Name(), ModuleType::inputPort3Name(), ModuleType::inputPort4Name());
    }
  };

  template <class ModuleType>
  struct IPortDescriber2<ModuleType, int port...>
  {
    static constexpr std::vector<Dataflow::Networks::InputPortDescription> inputs()
    {
      return ModuleType::inputPortDescription(ModuleType::inputPortName(port)...);
    }
  };
*/

}

#endif

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

#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <Core/Dataflow/Network/HardCodedModuleFactory.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <Core/Dataflow/Network/Module.h>

using namespace SCIRun::Domain::Networks;
using namespace boost::assign;

ModuleHandle HardCodedModuleFactory::create(const ModuleDescription& desc)
{
  Module::Builder builder;
  builder.with_name(desc.lookupInfo_.module_name_);
  BOOST_FOREACH(const InputPortDescription& input, desc.input_ports_)
  {
    builder.add_input_port(Port::ConstructionParams(input.name, input.datatype, input.color));
  }
  BOOST_FOREACH(const OutputPortDescription& output, desc.output_ports_)
  {
    builder.add_output_port(Port::ConstructionParams(output.name, output.datatype, output.color));
  }
  if (desc.lookupInfo_.module_name_.find("ComputeSVD") != std::string::npos)
    builder.disable_ui();
  return builder.build();
}

ModuleDescription HardCodedModuleFactory::lookupDescription(const ModuleLookupInfo& info)
{
  std::string name = info.module_name_;
  ModuleDescription d;
  d.lookupInfo_ = info;
  if (name.find("ComputeSVD") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("Input1", "Matrix", "blue");
    d.output_ports_ += OutputPortDescription("Output1", "Matrix", "blue"), OutputPortDescription("Output2", "Matrix", "blue"), OutputPortDescription("Output2", "Matrix", "blue");
  }
  else if (name.find("ReadMatrix") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("Input1", "String", "darkGreen");
    d.output_ports_ += OutputPortDescription("Output1", "Matrix", "blue"), OutputPortDescription("Output2", "String", "darkGreen");
  }
  else if (name.find("WriteMatrix") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("Input1", "Matrix", "blue"), InputPortDescription("Input2", "String", "darkGreen");
  }
  else if (name.find("SendScalar") != std::string::npos)
  {
    d.output_ports_ += OutputPortDescription("Output", "Scalar", "cyan");
  }
  else if (name.find("ReceiveScalar") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("Input", "Scalar", "cyan");
  }
  return d;
}


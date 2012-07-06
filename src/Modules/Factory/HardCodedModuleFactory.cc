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
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/factory.hpp>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <Core/Dataflow/Network/Module.h>

#include <Modules/Basic/ReceiveScalar.h>
#include <Modules/Basic/SendScalar.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Math/EvaluateLinearAlgebraBinary.h>
#include <Modules/Math/ReportMatrixInfo.h>

//TODO
#include <Core/Dataflow/Network/Tests/SimpleSourceSink.h>
#include <Modules/Factory/Share.h>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace boost::assign;

HardCodedModuleFactory::HardCodedModuleFactory()
{
  Module::Builder::use_sink_type(boost::factory<SimpleSink*>());
  Module::Builder::use_source_type(boost::factory<SimpleSource*>());
}

void HardCodedModuleFactory::setStateFactory(SCIRun::Domain::Networks::ModuleStateFactoryHandle stateFactory)
{
  stateFactory_ = stateFactory;
}

ModuleHandle HardCodedModuleFactory::create(const ModuleDescription& desc)
{
  Module::Builder builder;
  
  if (desc.lookupInfo_.module_name_ == "SendScalar")
    builder.using_func(boost::factory<SendScalarModule*>());
  else if (desc.lookupInfo_.module_name_ == "ReceiveScalar")
    builder.using_func(boost::factory<ReceiveScalarModule*>());
  else if (desc.lookupInfo_.module_name_ == "SendTestMatrix")
    builder.using_func(boost::factory<SendTestMatrixModule*>());
  else if (desc.lookupInfo_.module_name_ == "ReceiveTestMatrix")
    builder.using_func(boost::factory<ReceiveTestMatrixModule*>());
  else if (desc.lookupInfo_.module_name_ == "ReportMatrixInfo")
    builder.using_func(boost::factory<ReportMatrixInfoModule*>());
  else if (desc.lookupInfo_.module_name_ == "EvaluateLinearAlgebraUnary")
    builder.using_func(boost::factory<EvaluateLinearAlgebraUnaryModule*>());
  else if (desc.lookupInfo_.module_name_ == "EvaluateLinearAlgebraBinary")
    builder.using_func(boost::factory<EvaluateLinearAlgebraBinaryModule*>());
  else
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
  else if (name.find("SendTestMatrix") != std::string::npos)
  {
    d.output_ports_ += OutputPortDescription("Output", "Matrix", "blue");
  }
  else if (name.find("ReceiveTestMatrix") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("Input", "Matrix", "blue");
  }
  else if (name.find("ReportMatrixInfo") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("Input", "Matrix", "blue");
  }
  else if (name.find("EvaluateLinearAlgebraUnary") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("Input", "Matrix", "blue");
    d.output_ports_ += OutputPortDescription("Result", "Matrix", "blue");
  }
  else if (name.find("EvaluateLinearAlgebraBinary") != std::string::npos)
  {
    d.input_ports_ += InputPortDescription("InputLHS", "Matrix", "blue"), InputPortDescription("InputRHS", "Matrix", "blue");
    d.output_ports_ += OutputPortDescription("Result", "Matrix", "blue");
  }
  else
  {
    std::cout << "NOTE: Module " << name << " does not have any ports defined yet!" << std::endl;
  }
  return d;
}


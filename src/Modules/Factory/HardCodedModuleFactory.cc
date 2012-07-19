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
#include <Modules/Math/AppendMatrix.h>
#include <Modules/DataIO/ReadMatrix.h>
#include <Modules/DataIO/WriteMatrix.h>

#include <Core/Dataflow/Network/Tests/SimpleSourceSink.h>
#include <Modules/Factory/Share.h>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::DataIO;
using namespace boost::assign;

HardCodedModuleFactory::HardCodedModuleFactory()
{
  Module::Builder::use_sink_type(boost::factory<SimpleSink*>());
  Module::Builder::use_source_type(boost::factory<SimpleSource*>());
}

void HardCodedModuleFactory::setStateFactory(SCIRun::Domain::Networks::ModuleStateFactoryHandle stateFactory)
{
  stateFactory_ = stateFactory;
  Module::defaultStateFactory_ = stateFactory_;
}

ModuleHandle HardCodedModuleFactory::create(const ModuleDescription& desc)
{
  Module::Builder builder;
  
  if (desc.maker_)
    builder.using_func(desc.maker_);
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
  ModuleDescription description;
  description.lookupInfo_ = info;
  if (name.find("ComputeSVD") != std::string::npos)
  {
    description.input_ports_ += InputPortDescription("Input", "Matrix", "blue");
    description.output_ports_ += OutputPortDescription("U", "Matrix", "blue"), OutputPortDescription("S", "Matrix", "blue"), OutputPortDescription("V", "Matrix", "blue");
    description.maker_ = 0;
  }
  else if (name.find("ReadMatrix") != std::string::npos)
  {
    description.input_ports_ += ReadMatrixModule::inputPortDescription(ReadMatrixModule::inputPort0Name());
      //InputPortDescription("Input1", "String", "darkGreen");
    description.output_ports_ = ReadMatrixModule::outputPortDescription(ReadMatrixModule::outputPort0Name(), ReadMatrixModule::outputPort1Name());
      //OutputPortDescription("Output1", "Matrix", "blue"), OutputPortDescription("Output2", "String", "darkGreen");
    description.maker_ = boost::factory<ReadMatrixModule*>();
  }
  else if (name.find("WriteMatrix") != std::string::npos)
  {
    description.input_ports_ = WriteMatrixModule::inputPortDescription(WriteMatrixModule::inputPort0Name(), WriteMatrixModule::inputPort1Name());
      //InputPortDescription("Input1", "Matrix", "blue"), InputPortDescription("Input2", "String", "darkGreen");
    description.maker_ = boost::factory<WriteMatrixModule*>();
  }
  else if (name.find("SendScalar") != std::string::npos)
  {
    description.output_ports_ += SendScalarModule::outputPortDescription(SendScalarModule::outputPort0Name());
      //OutputPortDescription("Output", "Scalar", "cyan");
    description.maker_ = boost::factory<SendScalarModule*>();
  }
  else if (name.find("ReceiveScalar") != std::string::npos)
  {
    description.input_ports_ += ReceiveScalarModule::inputPortDescription(ReceiveScalarModule::inputPort0Name());
    description.maker_ = boost::factory<ReceiveScalarModule*>();
  }
  else if (name.find("SendTestMatrix") != std::string::npos)
  {
    description.output_ports_ += SendTestMatrixModule::outputPortDescription(SendTestMatrixModule::outputPort0Name());
      //OutputPortDescription("Output", "Matrix", "blue");
    description.maker_ = boost::factory<SendTestMatrixModule*>();
  }
  else if (name.find("ReceiveTestMatrix") != std::string::npos)
  {
    description.input_ports_ += ReceiveTestMatrixModule::inputPortDescription(ReceiveTestMatrixModule::inputPort0Name());
    description.maker_ = boost::factory<ReceiveTestMatrixModule*>();
  }
  else if (name.find("ReportMatrixInfo") != std::string::npos)
  {
    description.input_ports_ += ReportMatrixInfoModule::inputPortDescription(ReportMatrixInfoModule::inputPort0Name());
    description.maker_ = boost::factory<ReportMatrixInfoModule*>();
  }
  else if (name.find("AppendMatrix") != std::string::npos)
  {
    //description.input_ports_ = AppendMatrixModule::inputPortDescription(AppendMatrixModule::inputPort0Name(), AppendMatrixModule::inputPort1Name());
    //description.output_ports_ += AppendMatrixModule::outputPortDescription(AppendMatrixModule::outputPort0Name());
    //description.maker_ = boost::factory<AppendMatrixModule*>();
  }
  else if (name.find("EvaluateLinearAlgebraUnary") != std::string::npos)
  {
    description.input_ports_ += EvaluateLinearAlgebraUnaryModule::inputPortDescription(EvaluateLinearAlgebraUnaryModule::inputPort0Name());
      //InputPortDescription("Input", "Matrix", "blue");
    description.output_ports_ += EvaluateLinearAlgebraUnaryModule::outputPortDescription(EvaluateLinearAlgebraUnaryModule::outputPort0Name());
      //OutputPortDescription("Result", "Matrix", "blue");
    description.maker_ = boost::factory<EvaluateLinearAlgebraUnaryModule*>();
  }
  else if (name.find("EvaluateLinearAlgebraBinary") != std::string::npos)
  {
    description.input_ports_ = EvaluateLinearAlgebraBinaryModule::inputPortDescription(EvaluateLinearAlgebraBinaryModule::inputPort0Name(), EvaluateLinearAlgebraBinaryModule::inputPort1Name());
    description.output_ports_ += EvaluateLinearAlgebraBinaryModule::outputPortDescription(EvaluateLinearAlgebraBinaryModule::outputPort0Name());
      //OutputPortDescription("Result", "Matrix", "blue");
    description.maker_ = boost::factory<EvaluateLinearAlgebraBinaryModule*>();
  }
  else if (name.find("CreateMatrix") != std::string::npos)
  {
    //description.output_ports_ += OutputPortDescription("Result", "Matrix", "blue");
    //description.maker_ = boost::factory<CreateMatrixModule*>();
  }
  else if (name.find("CreateString") != std::string::npos)
  {
    //description.output_ports_ += OutputPortDescription("Result", "String", "????");
    //description.maker_ = boost::factory<CreateStringModule*>();
  }
  else if (name.find("ShowString") != std::string::npos)
  {
    //description.input_ports_ = EvaluateLinearAlgebraBinaryModule::inputPortDescription(EvaluateLinearAlgebraBinaryModule::inputPort0Name(), EvaluateLinearAlgebraBinaryModule::inputPort1Name());
    //description.output_ports_ += OutputPortDescription("Result", "Matrix", "blue");
    //description.maker_ = boost::factory<ShowStringModule*>();
  }
  else if (name.find("MatrixAsVectorField") != std::string::npos)
  {
    //description.input_ports_ = EvaluateLinearAlgebraBinaryModule::inputPortDescription(EvaluateLinearAlgebraBinaryModule::inputPort0Name(), EvaluateLinearAlgebraBinaryModule::inputPort1Name());
    //description.output_ports_ += OutputPortDescription("Result", "Matrix", "blue");
    //description.maker_ = boost::factory<MatrixAsVectorFieldModule*>();
  }
  else if (name.find("ViewScene") != std::string::npos)
  {
    //description.input_ports_ = EvaluateLinearAlgebraBinaryModule::inputPortDescription(EvaluateLinearAlgebraBinaryModule::inputPort0Name(), EvaluateLinearAlgebraBinaryModule::inputPort1Name());
    //description.output_ports_ += OutputPortDescription("Result", "Matrix", "blue");
    //description.maker_ = boost::factory<ViewScene*>();
  }
  else
  {
    std::cout << "NOTE: Module " << name << " does not have any ports defined yet!" << std::endl;
  }
  return description;
}


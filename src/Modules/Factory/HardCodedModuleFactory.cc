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
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>

#include <Modules/Basic/ReceiveScalar.h>
#include <Modules/Basic/SendScalar.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/PrintDatatype.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Math/EvaluateLinearAlgebraBinary.h>
#include <Modules/Math/ReportMatrixInfo.h>
#include <Modules/Math/AppendMatrix.h>
#include <Modules/Math/CreateMatrix.h>
#include <Modules/Math/SolveLinearSystem.h>
#include <Modules/Fields/CreateLatVolMesh.h>
#include <Modules/Fields/CreateScalarFieldDataBasic.h>
#include <Modules/Fields/ReportFieldInfo.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Modules/Fields/FieldToMesh.h>
#include <Modules/DataIO/ReadMatrix.h>
#include <Modules/DataIO/WriteMatrix.h>
#include <Modules/DataIO/ReadMesh.h>
#include <Modules/DataIO/ReadField.h>
#include <Modules/DataIO/WriteField.h>
#include <Modules/String/CreateString.h>
#include <Modules/Visualization/ShowString.h>
#include <Modules/Visualization/ShowField.h>
#include <Modules/Visualization/MatrixAsVectorField.h>
#include <Modules/Render/ViewScene.h>

#include <Dataflow/Network/Tests/SimpleSourceSink.h>
#include <Modules/Factory/Share.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Modules::StringProcessing;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Modules::Render;
using namespace boost::assign;

HardCodedModuleFactory::HardCodedModuleFactory()
{
  Module::Builder::use_sink_type(boost::factory<SimpleSink*>());
  Module::Builder::use_source_type(boost::factory<SimpleSource*>());
}

void HardCodedModuleFactory::setStateFactory(SCIRun::Dataflow::Networks::ModuleStateFactoryHandle stateFactory)
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

  //TODO: eliminate
  if (desc.lookupInfo_.module_name_.find("ComputeSVD") != std::string::npos)
    builder.disable_ui();

  ModuleHandle module = builder.build();

  return module;
}

struct ModuleLookupInfoLess
{
  bool operator()(const ModuleLookupInfo& lhs, const ModuleLookupInfo& rhs) const
  {
    return lhs.module_name_ < rhs.module_name_;
  }
};

class ModuleDescriptionLookup
{
public:
  ModuleDescriptionLookup()
  {
    //TODO: make EVEN MORE generic...macros? xml?

    addModuleDesc<ReadMatrixModule>("ReadMatrix", "DataIO", "SCIRun", "...", "...");
    addModuleDesc<WriteMatrixModule>("WriteMatrix", "DataIO", "SCIRun", "...", "...");
    addModuleDesc<ReadFieldModule>("ReadField", "DataIO", "SCIRun", "...", "...");
    addModuleDesc<WriteFieldModule>("WriteField", "DataIO", "SCIRun", "...", "...");
    addModuleDesc<ReadMeshModule>("ReadMesh", "Testing", "SCIRun", "...", "...");
    addModuleDesc<ReadMatrixModule>("SendScalar", "Testing", "SCIRun", "...", "...");
    addModuleDesc<ReceiveScalarModule>("ReceiveScalar", "Testing", "SCIRun", "...", "...");
    addModuleDesc<PrintDatatypeModule>("PrintDatatype", "String", "SCIRun", "...", "...");
    //addModuleDesc<SendTestMatrixModule>("SendTestMatrix", "Testing", "SCIRun", "...", "...");
    //addModuleDesc<ReceiveTestMatrixModule>("ReceiveTestMatrix", "Testing", "SCIRun", "...", "...");
    addModuleDesc<ReportMatrixInfoModule>("ReportMatrixInfo", "Math", "SCIRun", "...", "...");
    addModuleDesc<ReportFieldInfoModule>("ReportFieldInfo", "MiscField", "SCIRun", "...", "...");
    addModuleDesc<AppendMatrixModule>("AppendMatrix", "Math", "SCIRun", "...", "...");
    addModuleDesc<EvaluateLinearAlgebraUnaryModule>("EvaluateLinearAlgebraUnary", "Math", "SCIRun", "...", "...");
    addModuleDesc<EvaluateLinearAlgebraBinaryModule>("EvaluateLinearAlgebraBinary", "Math", "SCIRun", "...", "...");
    addModuleDesc<CreateMatrixModule>("CreateMatrix", "Math", "SCIRun", "...", "...");
    addModuleDesc<SolveLinearSystemModule>("SolveLinearSystem", "Math", "SCIRun", "...", "...");
    addModuleDesc<CreateStringModule>("CreateString", "String", "SCIRun", "...", "...");
    //addModuleDesc<ShowStringModule>("ShowString", "String", "SCIRun", "...", "...");
    addModuleDesc<ShowFieldModule>("ShowField", "Visualization", "SCIRun", "...", "...");
    addModuleDesc<CreateLatVolMesh>("CreateLatVolMesh", "NewField", "SCIRun", "...", "...");
    addModuleDesc<MatrixAsVectorFieldModule>("MatrixAsVectorField", "Math", "SCIRun", "...", "...");
    addModuleDesc<CreateLatVol>("CreateLatVol", "NewField", "SCIRun", "...", "...");
    addModuleDesc<FieldToMesh>("FieldToMesh", "MiscField", "SCIRun", "...", "...");
    addModuleDesc<CreateScalarFieldDataBasic>("CreateScalarFieldDataBasic", "NewField", "SCIRun", "...", "...");
    addModuleDesc<ViewScene>("ViewScene", "Render", "SCIRun", "...", "...");
  }


  ModuleDescription lookupDescription(const ModuleLookupInfo& info)
  {
    auto iter = lookup_.find(info);
    if (iter == lookup_.end())
    {
      //TODO: log
      std::ostringstream ostr;
      ostr << "Error: Undefined module \"" << info.module_name_ << "\"";
      THROW_INVALID_ARGUMENT(ostr.str());
    }
    return iter->second;
  }
private:
  std::map<ModuleLookupInfo, ModuleDescription, ModuleLookupInfoLess> lookup_;

  template <class ModuleType>
  void addModuleDesc(const std::string& name, const std::string& package, const std::string& category, const std::string& status, const std::string& desc)
  {
    ModuleLookupInfo info;
    info.module_name_ = name;
    info.package_name_ = package;
    info.category_name_ = category;
    
    ModuleDescription description;
    description.lookupInfo_ = info;

    description.input_ports_ = IPortDescriber<ModuleType::NumIPorts, ModuleType>::inputs();
    description.output_ports_ = OPortDescriber<ModuleType::NumOPorts, ModuleType>::outputs();
    description.maker_ = boost::factory<ModuleType*>();

    lookup_[info] = description;
  }
};

ModuleDescription HardCodedModuleFactory::lookupDescription(const ModuleLookupInfo& info)
{
  static ModuleDescriptionLookup lookup;
  return lookup.lookupDescription(info);
}


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

namespace SCIRun {
  namespace Modules {
    namespace Factory {

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

          addModuleDesc<ReadMatrixModule>("ReadMatrix", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
          addModuleDesc<WriteMatrixModule>("WriteMatrix", "DataIO", "SCIRun", "Functional, outputs text files or binary .mat only.", "...");
          addModuleDesc<ReadFieldModule>("ReadField", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
          addModuleDesc<WriteFieldModule>("WriteField", "DataIO", "SCIRun", "Functional, outputs binary .fld only.", "...");
          addModuleDesc<ReadMeshModule>("ReadMesh", "Testing", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
          addModuleDesc<SendScalarModule>("SendScalar", "Testing", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
          addModuleDesc<ReceiveScalarModule>("ReceiveScalar", "Testing", "SCIRun", "...", "...");
          addModuleDesc<PrintDatatypeModule>("PrintDatatype", "String", "SCIRun", "...", "...");
          //addModuleDesc<SendTestMatrixModule>("SendTestMatrix", "Testing", "SCIRun", "...", "...");
          //addModuleDesc<ReceiveTestMatrixModule>("ReceiveTestMatrix", "Testing", "SCIRun", "...", "...");
          addModuleDesc<ReportMatrixInfoModule>("ReportMatrixInfo", "Math", "SCIRun", "Functional, needs GUI work.", "...");
          addModuleDesc<ReportFieldInfoModule>("ReportFieldInfo", "MiscField", "SCIRun", "Same as v4", "...");
          addModuleDesc<AppendMatrixModule>("AppendMatrix", "Math", "SCIRun", "Fully functional.", "...");
          addModuleDesc<EvaluateLinearAlgebraUnaryModule>("EvaluateLinearAlgebraUnary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
          addModuleDesc<EvaluateLinearAlgebraBinaryModule>("EvaluateLinearAlgebraBinary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
          addModuleDesc<CreateMatrixModule>("CreateMatrix", "Math", "SCIRun", "Functional, needs GUI work.", "...");
          addModuleDesc<SolveLinearSystemModule>("SolveLinearSystem", "Math", "SCIRun", "One multithreaded algorithm available.", "...");
          addModuleDesc<CreateStringModule>("CreateString", "String", "SCIRun", "Functional, needs GUI work.", "...");
          //addModuleDesc<ShowStringModule>("ShowString", "String", "SCIRun", "...", "...");
          addModuleDesc<ShowFieldModule>("ShowField", "Visualization", "SCIRun", "Some basic options available, still work in progress.", "...");
          addModuleDesc<CreateLatVolMesh>("CreateLatVolMesh", "NewField", "SCIRun", "Can create lat vol meshes of arbitrary size.", "...");
          addModuleDesc<MatrixAsVectorFieldModule>("MatrixAsVectorField", "Math", "SCIRun", "...", "...");
          addModuleDesc<CreateLatVol>("CreateLatVol", "NewField", "SCIRun", "Official ported v4 module.", "...");
          addModuleDesc<FieldToMesh>("FieldToMesh", "MiscField", "SCIRun", "New, working.", "Returns underlying mesh from a field.");
          addModuleDesc<CreateScalarFieldDataBasic>("CreateScalarFieldDataBasic", "NewField", "SCIRun", "Set field data via python.", "...");
          addModuleDesc<ViewScene>("ViewScene", "Render", "SCIRun", "Can display meshes and fields, pan/rotate/zoom.", "...");
        }

        std::vector<ModuleDescription> viewCurrentModules() const
        {
          std::vector<ModuleDescription> descs;
          std::transform(lookup_.begin(), lookup_.end(), std::back_inserter(descs), [](const Lookup::value_type& p) { return p.second; });
          return descs;
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
        typedef std::map<ModuleLookupInfo, ModuleDescription, ModuleLookupInfoLess> Lookup;
        Lookup lookup_;

        template <class ModuleType>
        void addModuleDesc(const std::string& name, const std::string& category, const std::string& package, const std::string& status, const std::string& desc)
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
          description.moduleStatus_ = status;
          description.moduleInfo_ = desc;

          lookup_[info] = description;
        }
      };

      class HardCodedModuleFactoryImpl
      {
      public:
        ModuleDescriptionLookup lookup;
      };

    }}}

HardCodedModuleFactory::HardCodedModuleFactory() : impl_(new HardCodedModuleFactoryImpl)
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

ModuleDescription HardCodedModuleFactory::lookupDescription(const ModuleLookupInfo& info)
{
  return impl_->lookup.lookupDescription(info);
}

std::vector<ModuleDescription> HardCodedModuleFactory::getAllAvailableModuleDescriptions() const
{
  return impl_->lookup.viewCurrentModules();
}
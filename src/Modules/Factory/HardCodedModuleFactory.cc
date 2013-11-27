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
#include <Modules/Basic/DynamicPortTester.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Math/EvaluateLinearAlgebraBinary.h>
#include <Modules/Math/ReportMatrixInfo.h>
#include <Modules/Math/AppendMatrix.h>
#include <Modules/Math/CreateMatrix.h>
#include <Modules/Math/SolveLinearSystem.h>
#include <Modules/Fields/CreateScalarFieldDataBasic.h>
#include <Modules/Fields/ReportFieldInfo.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Modules/Legacy/Fields/GetFieldBoundary.h>
#include <Modules/Legacy/Fields/CalculateSignedDistanceToField.h>
#include <Modules/Legacy/Fields/CalculateGradients.h>
#include <Modules/Legacy/Fields/CalculateVectorMagnitudes.h>
#include <Modules/Legacy/Fields/ConvertQuadSurfToTriSurf.h>
#include <Modules/Legacy/Fields/AlignMeshBoundingBoxes.h>
#include <Modules/Legacy/Fields/GetFieldNodes.h>
#include <Modules/Legacy/Fields/SetFieldNodes.h>
#include <Modules/Legacy/Fields/GetFieldData.h>
#include <Modules/Legacy/Math/SolveMinNormLeastSqSystem.h>
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
#include <Modules/FiniteElements/TDCSSimulator.h>
#include <Modules/Render/ViewScene.h>
#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
#include <Modules/BrainStimulator/SetConductivitiesToTetMesh.h>
#include <Modules/BrainStimulator/SetupRHSforTDCSandTMS.h>
#include <Modules/BrainStimulator/GenerateROIStatistics.h>
#include <Modules/Legacy/Math/AddKnownsToLinearSystem.h>
#include <Modules/Legacy/FiniteElements/BuildTDCSMatrix.h>

#include <Dataflow/Network/SimpleSourceSink.h>
#include <Modules/Factory/share.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Modules::StringProcessing;
using namespace SCIRun::Modules::BrainStimulator;
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
        ModuleDescriptionLookup() : includeTestingModules_(false)
        {
          //TODO: is BUILD_TESTING off when we build releases?
        #ifdef BUILD_TESTING
          includeTestingModules_ = true;
        #endif
          //TODO: make EVEN MORE generic...macros? xml?
          //TODO: at least remove duplication of Name,Package,Category here since we should be able to infer from header somehow.

          addModuleDesc<ReadMatrixModule>("ReadMatrix", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
          addModuleDesc<WriteMatrixModule>("WriteMatrix", "DataIO", "SCIRun", "Functional, outputs text files or binary .mat only.", "...");
          addModuleDesc<ReadFieldModule>("ReadField", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
          addModuleDesc<WriteFieldModule>("WriteField", "DataIO", "SCIRun", "Functional, outputs binary .fld only.", "...");
          addModuleDesc<PrintDatatypeModule>("PrintDatatype", "String", "SCIRun", "...", "...");
          addModuleDesc<ReportMatrixInfoModule>("ReportMatrixInfo", "Math", "SCIRun", "Functional, needs GUI work.", "...");
          addModuleDesc<ReportFieldInfoModule>("ReportFieldInfo", "MiscField", "SCIRun", "Same as v4", "...");
          addModuleDesc<AppendMatrixModule>("AppendMatrix", "Math", "SCIRun", "Fully functional.", "...");
          addModuleDesc<EvaluateLinearAlgebraUnaryModule>("EvaluateLinearAlgebraUnary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
          addModuleDesc<EvaluateLinearAlgebraBinaryModule>("EvaluateLinearAlgebraBinary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
          addModuleDesc<CreateMatrixModule>("CreateMatrix", "Math", "SCIRun", "Functional, needs GUI work.", "...");
          addModuleDesc<SolveLinearSystemModule>("SolveLinearSystem", "Math", "SCIRun", "Four multi-threaded algorithms available.", "...");
          addModuleDesc<CreateStringModule>("CreateString", "String", "SCIRun", "Functional, needs GUI work.", "...");
          //addModuleDesc<ShowStringModule>("ShowString", "String", "SCIRun", "...", "...");
          addModuleDesc<ShowFieldModule>("Some basic options available, still work in progress.", "...");
          addModuleDesc<CreateLatVol>("CreateLatVol", "NewField", "SCIRun", "Official ported v4 module.", "...");
          //addModuleDesc<FieldToMesh>("FieldToMesh", "MiscField", "SCIRun", "New, working.", "Returns underlying mesh from a field.");
          addModuleDesc<ViewScene>("Can display meshes and fields, pan/rotate/zoom.", "...");

          addModuleDesc<GetFieldBoundary>("GetFieldBoundary", "NewField", "SCIRun", "First real ported module", "...");
          addModuleDesc<CalculateSignedDistanceToField>("CalculateSignedDistanceToField", "ChangeFieldData", "SCIRun", "Second real ported module", "...");
          addModuleDesc<CalculateGradients>("CalculateGradients", "ChangeFieldData", "SCIRun", "Real ported module", "...");
          addModuleDesc<ConvertQuadSurfToTriSurf>("ConvertQuadSurfToTriSurf", "ChangeMesh", "SCIRun", "Real ported module", "...");
          addModuleDesc<AlignMeshBoundingBoxes>("AlignMeshBoundingBoxes", "ChangeMesh", "SCIRun", "Real ported module", "...");
          addModuleDesc<GetFieldNodes>("GetFieldNodes", "ChangeMesh", "SCIRun", "Real ported module", "...");
          addModuleDesc<SetFieldNodes>("SetFieldNodes", "ChangeMesh", "SCIRun", "Real ported module", "...");
          addModuleDesc<TDCSSimulatorModule>("tDCSSimulator", "FiniteElements", "SCIRun", "Dummy module for design purposes", "...");
          addModuleDesc<SolveMinNormLeastSqSystem>("SolveMinNormLeastSqSystem", "Math", "SCIRun", "Real ported module", "...");

          //TODO: possibly use different build setting for these.
          if (includeTestingModules_)
          {
            addModuleDesc<ReadMeshModule>("ReadMesh", "Testing", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
            addModuleDesc<SendScalarModule>("SendScalar", "Testing", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
            addModuleDesc<ReceiveScalarModule>("ReceiveScalar", "Testing", "SCIRun", "...", "...");
            addModuleDesc<SendTestMatrixModule>("SendTestMatrix", "Testing", "SCIRun", "...", "...");
            addModuleDesc<ReceiveTestMatrixModule>("ReceiveTestMatrix", "Testing", "SCIRun", "...", "...");
            addModuleDesc<MatrixAsVectorFieldModule>("MatrixAsVectorField", "Testing", "SCIRun", "...", "...");
            addModuleDesc<CreateScalarFieldDataBasic>("CreateScalarFieldDataBasic", "Testing", "SCIRun", "Set field data via python.", "...");
            addModuleDesc<DynamicPortTester>("DynamicPortTester", "Testing", "SCIRun", "...", "...");
          }

          addModuleDesc<BuildTDCSMatrix>("BuildTDCSMatrix", "FiniteElements", "SCIRun", " in progress ", "Generates tDCS Forward Matrix ");
          addModuleDesc<ElectrodeCoilSetupModule>("ElectrodeCoilSetup", "BrainStimulator", "SCIRun", " in progress ", " Place tDCS electrodes and TMS coils ");
          addModuleDesc<SetConductivitiesToTetMeshModule>("SetConductivitiesToTetMesh", "BrainStimulator", "SCIRun", " in progress ", " Sets conveniently conductivity profile for tetrahedral mesh ");
          addModuleDesc<GenerateROIStatisticsModule>("GenerateROIStatistics", "BrainStimulator", "SCIRun", " in progress ", " Roi statistics ");   
          addModuleDesc<SetupRHSforTDCSandTMSModule>("SetupRHSforTDCSandTMS", "BrainStimulator", "SCIRun", " in progress ", " set RHS for tDCS and TMS ");        
          addModuleDesc<AddKnownsToLinearSystem>("AddKnownsToLinearSystem", "Math", "SCIRun", " in progress ", " adds knowns to linear systems ");        
          addModuleDesc<CalculateVectorMagnitudes>("CalculateVectorMagnitudes", "ChangeFieldData", "SCIRun", "Real ported module", "...");
          addModuleDesc<GetFieldData>("GetFieldData", "ChangeMesh", "SCIRun", "Real ported module", "...");
   
	}

        ModuleDescriptionMap descMap_;

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
        bool includeTestingModules_;

        //TODO: remove this function and use static MLI from each module
        template <class ModuleType>
        void addModuleDesc(const std::string& name, const std::string& category, const std::string& package, const std::string& status, const std::string& desc)
        {
          ModuleLookupInfo info(name, category, package);
          addModuleDesc<ModuleType>(info, status, desc);
        }

        template <class ModuleType>
        void addModuleDesc(const ModuleLookupInfo& info, const std::string& status, const std::string& desc)
        {
          ModuleDescription description;
          description.lookupInfo_ = info;

          description.input_ports_ = IPortDescriber<ModuleType::NumIPorts, ModuleType>::inputs();
          description.output_ports_ = OPortDescriber<ModuleType::NumOPorts, ModuleType>::outputs();
          description.maker_ = boost::factory<ModuleType*>();
          description.moduleStatus_ = status;
          description.moduleInfo_ = desc;

          lookup_[info] = description;

          descMap_[info.package_name_][info.category_name_][info.module_name_] = description;
        }

        template <class ModuleType>
        void addModuleDesc(const std::string& status, const std::string& desc)
        {
          addModuleDesc<ModuleType>(ModuleType::staticInfo_, status, desc);
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

void HardCodedModuleFactory::setAlgorithmFactory(SCIRun::Core::Algorithms::AlgorithmFactoryHandle algoFactory)
{
  Module::defaultAlgoFactory_ = algoFactory;
}

ModuleHandle HardCodedModuleFactory::create(const ModuleDescription& desc)
{
  Module::Builder builder;
  
  if (desc.maker_)
    builder.using_func(desc.maker_).setStateDefaults();
  else
    builder.with_name(desc.lookupInfo_.module_name_);

  BOOST_FOREACH(const InputPortDescription& input, desc.input_ports_)
  {
    builder.add_input_port(Port::ConstructionParams(input.id, input.datatype, input.isDynamic));
  }
  BOOST_FOREACH(const OutputPortDescription& output, desc.output_ports_)
  {
    builder.add_output_port(Port::ConstructionParams(output.id, output.datatype, output.isDynamic));
  }

  ModuleHandle module = builder.build();

  return module;
}

ModuleDescription HardCodedModuleFactory::lookupDescription(const ModuleLookupInfo& info)
{
  return impl_->lookup.lookupDescription(info);
}

const ModuleDescriptionMap& HardCodedModuleFactory::getAllAvailableModuleDescriptions() const
{
  return impl_->lookup.descMap_;
}

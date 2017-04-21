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

#ifndef DATAFLOW_NETWORK_MODULE_H
#define DATAFLOW_NETWORK_MODULE_H

#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/atomic.hpp>
#include <atomic>
#include <vector>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Datatypes/DatatypeFwd.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/PortManager.h>
#include <Dataflow/Network/DefaultModuleFactories.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {


      template <class Type, size_t N>
      struct PortNameBase
      {
        explicit PortNameBase(const PortId& id) : id_(id) {}

        operator PortId() const
        {
          return toId();
        }

        PortId toId() const
        {
          if (id_.name.empty())
            BOOST_THROW_EXCEPTION(DataPortException() << SCIRun::Core::ErrorMessage("Port name not initialized!"));
          return id_;
        }
        operator std::string() const
        {
          return toId().name;
        }

        PortId id_;
      };

      template <class Type, size_t N>
      struct StaticPortName : PortNameBase<Type,N>
      {
        explicit StaticPortName(const PortId& id = PortId(0, "[not defined yet]")) : PortNameBase<Type,N>(id) {}
      };

      template <class Type, size_t N>
      struct DynamicPortName : PortNameBase<Type,N>
      {
        explicit DynamicPortName(const PortId& id = PortId(0, "[not defined yet]")) : PortNameBase<Type,N>(id) {}
      };


  class SCISHARE Module : public ModuleInterface,
    public Core::Logging::LegacyLoggerInterface,
    public StateChangeObserver,
    boost::noncopyable
  {
  public:
    explicit Module(const ModuleLookupInfo& info,
      bool hasUi = true,
      Core::Algorithms::AlgorithmFactoryHandle algoFactory = DefaultModuleFactories::defaultAlgoFactory_,
      ModuleStateFactoryHandle stateFactory = DefaultModuleFactories::defaultStateFactory_,
      ReexecuteStrategyFactoryHandle reexFactory = DefaultModuleFactories::defaultReexFactory_,
      const std::string& version = "1.0");
    virtual ~Module() override;

    /*** User-interface ****/
    virtual ModuleStateHandle get_state() override final;
    virtual const ModuleStateHandle get_state() const override final;
    virtual void enqueueExecuteAgain(bool upstream) override final;
    virtual void error(const std::string& msg) const override final;
    virtual void warning(const std::string& msg) const override final { getLogger()->warning(msg); }
    virtual void remark(const std::string& msg) const override final { getLogger()->remark(msg); }
    virtual void status(const std::string& msg) const override final { getLogger()->status(msg); }
    virtual bool needToExecute() const override final;

    /*** Dev-interface ****/
    virtual boost::signals2::connection connectExecuteSelfRequest(const ExecutionSelfRequestSignalType::slot_type& subscriber) override final;
    virtual void set_state(ModuleStateHandle state) override final;
    virtual ModuleExecutionState& executionState() override final;
    virtual std::string helpPageUrl() const override;
    std::string newHelpPageUrl() const; // location in flux, but new v5 modules only have one of these
    //for serialization
    virtual const ModuleLookupInfo& get_info() const override final { return info_; }
    virtual void set_id(const std::string& id) override final;
    virtual bool executeWithSignals() NOEXCEPT override final;
    bool has_ui() const override;
    void setUiVisible(bool visible) override;
    virtual size_t num_input_ports() const override final;
    virtual size_t num_output_ports() const override final;
    // override this for modules that changed packages, to point to correct wiki page
    virtual std::string legacyPackageName() const override { return get_packagename(); }
    // override this for modules that changed names, to point to correct wiki page
    virtual std::string legacyModuleName() const override { return get_module_name(); }
    virtual bool hasInputPort(const PortId& id) const override final;
    virtual bool hasOutputPort(const PortId& id) const override final;
    virtual InputPortHandle getInputPort(const PortId& id) override final;
    virtual OutputPortHandle getOutputPort(const PortId& id) const override final;
    virtual std::vector<InputPortHandle> findInputPortsWithName(const std::string& name) const override final;
    virtual std::vector<OutputPortHandle> findOutputPortsWithName(const std::string& name) const override final;
    virtual std::vector<InputPortHandle> inputPorts() const override final;
    virtual std::vector<OutputPortHandle> outputPorts() const override final;
    virtual bool isStoppable() const override final;
    virtual bool hasDynamicPorts() const override;
    bool oport_connected(const PortId& id) const;
    bool inputsChanged() const;
    virtual std::string get_module_name() const override final { return info_.module_name_; }
    std::string get_categoryname() const { return info_.category_name_; }
    std::string get_packagename() const { return info_.package_name_; }
    ModuleId get_id() const override { return id_; }

    virtual ModuleReexecutionStrategyHandle getReexecutionStrategy() const override final;
    virtual void setReexecutionStrategy(ModuleReexecutionStrategyHandle caching) override final;
    virtual Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc getUpdaterFunc() const override final;
    virtual void setUpdaterFunc(Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc func) override final;
    virtual void setUiToggleFunc(UiToggleFunc func) override final;

    virtual boost::signals2::connection connectExecuteBegins(const ExecuteBeginsSignalType::slot_type& subscriber) override final;
    virtual boost::signals2::connection connectExecuteEnds(const ExecuteEndsSignalType::slot_type& subscriber) override final;
    virtual boost::signals2::connection connectErrorListener(const ErrorSignalType::slot_type& subscriber) override final;

    virtual void addPortConnection(const boost::signals2::connection& con) override final;

    virtual Core::Algorithms::AlgorithmHandle getAlgorithm() const override final;
    virtual void setLogger(Core::Logging::LoggerHandle log) override final;
    virtual Core::Logging::LoggerHandle getLogger() const override final;
    virtual const MetadataMap& metadata() const override final;
    virtual bool executionDisabled() const override final;
    virtual void setExecutionDisabled(bool disable) override final;

    template <class T, class D, size_t N>
    void sendOutput(const StaticPortName<T,N>& port, boost::shared_ptr<D> data);

    static const int TraitFlags;
    /*******/

    //for unit testing. Need to restrict access somehow.
    static void resetIdGenerator();

  protected:
/*** User-interface ****/
    // Throws if input is not present or null.
    template <class T, size_t N>
    boost::shared_ptr<T> getRequiredInput(const StaticPortName<T,N>& port);

    template <class T, size_t N>
    boost::optional<boost::shared_ptr<T>> getOptionalInput(const StaticPortName<T,N>& port);

    template <class T, size_t N>
    std::vector<boost::shared_ptr<T>> getRequiredDynamicInputs(const DynamicPortName<T,N>& port);

    template <class T, size_t N>
    std::vector<boost::shared_ptr<T>> getOptionalDynamicInputs(const DynamicPortName<T,N>& port);

    template <class T, size_t N>
    void sendOutputFromAlgorithm(const StaticPortName<T,N>& port, const Core::Algorithms::AlgorithmOutput& output);

    Core::Algorithms::AlgorithmBase& algo();

    void setStateBoolFromAlgo(const Core::Algorithms::AlgorithmParameterName& name);
    void setStateIntFromAlgo(const Core::Algorithms::AlgorithmParameterName& name);
    void setStateDoubleFromAlgo(const Core::Algorithms::AlgorithmParameterName& name);
    void setStateListFromAlgo(const Core::Algorithms::AlgorithmParameterName& name);
    void setStateStringFromAlgo(const Core::Algorithms::AlgorithmParameterName& name);
    void setStateStringFromAlgoOption(const Core::Algorithms::AlgorithmParameterName& name);
    void setAlgoBoolFromState(const Core::Algorithms::AlgorithmParameterName& name);
    void setAlgoIntFromState(const Core::Algorithms::AlgorithmParameterName& name);
    void setAlgoDoubleFromState(const Core::Algorithms::AlgorithmParameterName& name);
    void setAlgoStringFromState(const Core::Algorithms::AlgorithmParameterName& name);
    void setAlgoOptionFromState(const Core::Algorithms::AlgorithmParameterName& name);
    void setAlgoListFromState(const Core::Algorithms::AlgorithmParameterName& name);
    //For modules that need to initialize some internal state signal/slots, this needs to be called after set_state to reinitialize.
    virtual void postStateChangeInternalSignalHookup() {}

/*** Dev-interface ****/
    virtual void send_output_handle(const PortId& id, Core::Datatypes::DatatypeHandle data) override final;
    virtual size_t add_input_port(InputPortHandle);
    size_t add_output_port(OutputPortHandle);
    virtual void removeInputPort(const PortId& id);
    void sendFeedbackUpstreamAlongIncomingConnections(const Core::Datatypes::ModuleFeedback& feedback) const;
    std::string stateMetaInfo() const;
    void copyStateToMetadata();

    friend class ModuleBuilder;

  private:
    virtual Core::Datatypes::DatatypeHandleOption get_input_handle(const PortId& id) override final;
    virtual std::vector<Core::Datatypes::DatatypeHandleOption> get_dynamic_input_handles(const PortId& id) override final;
    template <class T>
    boost::shared_ptr<T> getRequiredInputAtIndex(const PortId& id);
    template <class T>
    boost::optional<boost::shared_ptr<T>> getOptionalInputAtIndex(const PortId& id);
    template <class T>
    boost::shared_ptr<T> checkInput(Core::Datatypes::DatatypeHandleOption inputOpt, const PortId& id);

    const ModuleLookupInfo info_;
    ModuleId id_;
    friend class ModuleImpl;
    boost::shared_ptr<class ModuleImpl> impl_;
  };

#include <Dataflow/Network/ModuleTemplateImpl.h>

}}}

#include <Dataflow/Network/ModulePortDescriptionTags.h>
#include <Dataflow/Network/ModuleTraits.h>

#endif

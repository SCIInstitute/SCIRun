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


#ifndef DATAFLOW_NETWORK_MODULE_H
#define DATAFLOW_NETWORK_MODULE_H

#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>
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
#include <Dataflow/Network/PortNames.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

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
      ReexecuteStrategyFactoryHandle reexFactory = DefaultModuleFactories::defaultReexFactory_);
    ~Module() override;

    /*** User-interface ****/
    ModuleStateHandle get_state() override final;
    const ModuleStateHandle cstate() const override final;
    void enqueueExecuteAgain(bool upstream) override final;
    void error(const std::string& msg) const override final;
    bool errorReported() const override final { return getLogger()->errorReported(); }
    void setErrorFlag(bool flag) override final { getLogger()->setErrorFlag(flag); }
    void warning(const std::string& msg) const override final { getLogger()->warning(msg); }
    void remark(const std::string& msg) const override final { getLogger()->remark(msg); }
    void status(const std::string& msg) const override final { getLogger()->status(msg); }
    bool needToExecute() const override final;
    bool alwaysExecuteEnabled() const;
    bool hasDynamicPorts() const override;

    /*** public Dev-interface ****/
    boost::signals2::connection connectExecuteSelfRequest(const ExecutionSelfRequestSignalType::slot_type& subscriber) override final;
    void setState(ModuleStateHandle state) override final;
    ModuleExecutionState& executionState() override final;

    std::string helpPageUrl() const override;
    std::string newHelpPageUrl() const override; // location in flux, but new v5 modules only have one of these
    //for serialization
    const ModuleLookupInfo& info() const override final;
    void setId(const std::string& id) override final;
    bool executeWithSignals() NOEXCEPT override final;
    bool hasUI() const override;
    void setUiVisible(bool visible) override;
    size_t numInputPorts() const override final;
    size_t numOutputPorts() const override final;
    // override this for modules that changed packages, to point to correct wiki page
    std::string legacyPackageName() const override { return get_packagename(); }
    // override this for modules that changed names, to point to correct wiki page
    std::string legacyModuleName() const override { return name(); }
    bool hasInputPort(const PortId& id) const override final;
    bool hasOutputPort(const PortId& id) const override final;
    InputPortHandle getInputPort(const PortId& id) override final;
    OutputPortHandle getOutputPort(const PortId& id) const override final;
    std::vector<InputPortHandle> findInputPortsWithName(const std::string& name) const override final;
    std::vector<OutputPortHandle> findOutputPortsWithName(const std::string& name) const override final;
    std::vector<InputPortHandle> inputPorts() const override final;
    std::vector<OutputPortHandle> outputPorts() const override final;
    bool isStoppable() const override final;
    bool oport_connected(const PortId& id) const;
    bool inputsChanged() const;
    std::string name() const override final;
    std::string get_categoryname() const;
    std::string get_packagename() const;
    ModuleId id() const override;
    bool isDeprecated() const override { return false; }
    std::string replacementModuleName() const override { return ""; }
    ModuleReexecutionStrategyHandle getReexecutionStrategy() const override final;
    void setReexecutionStrategy(ModuleReexecutionStrategyHandle caching) override final;
    Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc getUpdaterFunc() const override final;
    void setUpdaterFunc(Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc func) override final;
    void setUiToggleFunc(UiToggleFunc func) override final;
    boost::signals2::connection connectExecuteBegins(const ExecuteBeginsSignalType::slot_type& subscriber) override final;
    boost::signals2::connection connectExecuteEnds(const ExecuteEndsSignalType::slot_type& subscriber) override final;
    boost::signals2::connection connectErrorListener(const ErrorSignalType::slot_type& subscriber) override final;
    void addPortConnection(const boost::signals2::connection& con) override final;
    Core::Algorithms::AlgorithmHandle getAlgorithm() const override final;
    void setLogger(Core::Logging::LoggerHandle log) override final;
    Core::Logging::LoggerHandle getLogger() const override final;
    const MetadataMap& metadata() const override final;
    bool executionDisabled() const override final;
    void setExecutionDisabled(bool disable) override final;
    bool isImplementationDisabled() const override { return false; }
    void setProgrammableInputPortEnabled(bool enable) override final;
    bool checkForVirtualConnection(const ModuleInterface& downstream) const override { return false; }
    std::string description() const override;
    void setInfoStrings(const ModuleDescription& desc);
    static const int TraitFlags;
    //for unit testing. Need to restrict access somehow.
    static void resetIdGenerator();
    /*******/

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
    // does not throw, only returns non-null
    template <class T, size_t N>
    std::vector<boost::shared_ptr<T>> getValidDynamicInputs(const DynamicPortName<T,N>& port);
  public: //for python
    template <class T, class D, size_t N>
    void sendOutput(const StaticPortName<T, N>& port, boost::shared_ptr<D> data);
  protected:
    template <class T, size_t N, typename F>
    void computeOutputAndSendIfConnected(const StaticPortName<T, N>& port, F evalFunc);
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
    virtual void postStateChangeInternalSignalHookup();

/*** protected Dev-interface ****/
    virtual void send_output_handle(const PortId& id, Core::Datatypes::DatatypeHandle data) override final;
    virtual size_t add_input_port(InputPortHandle);
    size_t add_output_port(OutputPortHandle);
    virtual void removeInputPort(const PortId& id);
    void sendFeedbackUpstreamAlongIncomingConnections(const Core::Datatypes::ModuleFeedback& feedback) const;
    std::string stateMetaInfo() const;
    void copyStateToMetadata();

    friend class ModuleBuilder;

  private:
    Core::Datatypes::DatatypeHandleOption get_input_handle(const PortId& id) override final;
    std::vector<Core::Datatypes::DatatypeHandleOption> get_dynamic_input_handles(const PortId& id) override final;
    void runProgrammablePortInput();
    template <class T>
    boost::shared_ptr<T> getRequiredInputAtIndex(const PortId& id);
    template <class T>
    boost::optional<boost::shared_ptr<T>> getOptionalInputAtIndex(const PortId& id);
    template <class T>
    boost::shared_ptr<T> checkInput(Core::Datatypes::DatatypeHandleOption inputOpt, const PortId& id);

    friend class ModuleImpl;
    boost::shared_ptr<class ModuleImpl> impl_;
  };

  // ReSharper disable once CppUnusedIncludeDirective
#include <Dataflow/Network/ModuleTemplateImpl.h>

}}}

// ReSharper disable once CppUnusedIncludeDirective
#include <Dataflow/Network/ModulePortDescriptionTags.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Dataflow/Network/ModuleTraits.h>

#endif

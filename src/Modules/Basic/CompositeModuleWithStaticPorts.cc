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

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Datatypes/String.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Modules/Basic/CompositeModuleWithStaticPorts.h>
#include <iostream>

using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Python;

MODULE_INFO_DEF(CompositeModuleWithStaticPorts, Flow Control, SCIRun)

ALGORITHM_PARAMETER_DEF(Python, NetworkXml)
ALGORITHM_PARAMETER_DEF(Python, PortSettings)
ALGORITHM_PARAMETER_DEF(Python, ModuleIdList)

class SCIRun::Modules::Basic::CompositeModuleImpl
{
 public:
  NetworkHandle subNet_;

  explicit CompositeModuleImpl(CompositeModuleWithStaticPorts* module) : module_(module) {}
  void initializeSubnet(const std::string& networkXmlFromState);

 private:
  CompositeModuleWithStaticPorts* module_;
};

CompositeModuleWithStaticPorts::CompositeModuleWithStaticPorts()
    : Module(staticInfo_), impl_(new CompositeModuleImpl(this))
{
  INITIALIZE_PORT(Input0) INITIALIZE_PORT(Input1) INITIALIZE_PORT(Input2)
  INITIALIZE_PORT(Input3) INITIALIZE_PORT(Input4) INITIALIZE_PORT(Input5) INITIALIZE_PORT(Input6)  // INITIALIZE_PORT(Input7)
  INITIALIZE_PORT(Output0) INITIALIZE_PORT(Output1) INITIALIZE_PORT(Output2)
  INITIALIZE_PORT(Output3) INITIALIZE_PORT(Output4) INITIALIZE_PORT(Output5)
  INITIALIZE_PORT(Output6) INITIALIZE_PORT(Output7)
}

CompositeModuleWithStaticPorts::~CompositeModuleWithStaticPorts() = default;

void CompositeModuleWithStaticPorts::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::NetworkXml, std::string());
  state->setValue(Parameters::PortSettings, std::string());

  state->connectSpecificStateChanged(Parameters::NetworkXml, [this]()
  {
    const auto xml = get_state()->getValue(Parameters::NetworkXml).toString();
    impl_->initializeSubnet(xml);
  });
}

void CompositeModuleWithStaticPorts::execute()
{
  if (needToExecute())
  {
    if (!impl_->subNet_)
    {
      const auto xml = get_state()->getValue(Parameters::NetworkXml).toString();
      impl_->initializeSubnet(xml);
    }

    auto resultFuture = impl_->subNet_->executeAll();
    resultFuture.wait();
    const auto result = resultFuture.get();
    if (result != 0)
    {
      error("Subnetwork returned error code " + std::to_string(result) + ".");
    }
    else
    {
      remark("Subnetwork executed successfully.");
    }
  }
}

namespace
{
  CompositeModuleInfoMap makeModuleIdList(const NetworkStateInterface& net)
  {
    CompositeModuleInfoMap cmim;
    for (size_t i = 0; i < net.nmodules(); ++i)
    {
      const auto mod = net.module(i);
      cmim[mod->id()] = mod;
    }
    return cmim;
  }
}

void CompositeModuleImpl::initializeSubnet(const std::string& networkXmlFromState)
{
  if (networkXmlFromState.empty())
  {
    module_->get_state()->setTransientValue(Parameters::ModuleIdList, CompositeModuleInfoMap(), true);
    return;
  }

  subNet_ = module_->network()->createSubnetwork();
  if (subNet_)
  {
    module_->remark("Subnet created.");
  }
  else
  {
    module_->error("Subnet null");
    return;
  }
  if (!subNet_->getNetwork())
  {
    module_->error("Subnet's network state is null");
    return;
  }

  try
  {
    std::istringstream istr(networkXmlFromState);
    const auto xml = XMLSerializer::load_xml<NetworkFile>(istr);
    subNet_->loadXmlDataIntoNetwork(xml->network.data());
  }
  catch (...)
  {
    module_->error("Error reading network xml");
    return;
  }
  const auto network = subNet_->getNetwork();

  auto wrapperModuleInputs = module_->inputPorts();
  auto wrapperModuleInputsIterator = wrapperModuleInputs.begin();
  auto wrapperModuleOutputs = module_->outputPorts();
  auto wrapperModuleOutputsIterator = wrapperModuleOutputs.begin();

  module_->remark("Subnet created with " + std::to_string(network->nmodules()) + " modules and " + std::to_string(network->nconnections()) + " connections.");
  std::ostringstream ostr;

  for (size_t i = 0; i < network->nmodules(); ++i)
  {
    const auto subModule = network->module(i);

    for (const auto& inputPort : subModule->inputPorts())
    {
      if (!inputPort->isDynamic() && inputPort->nconnections() == 0 && inputPort->get_typename() != "MetadataObject")
      {
        auto portId = inputPort->id();
        logCritical("Found input port that can be exposed: {} :: {}", subModule->id().id_, portId.toString());

        if (wrapperModuleInputsIterator != wrapperModuleInputs.end())
        {
          logCritical("\t\tperforming port surgery on {} :: {} --> Input{}", subModule->id().id_, portId.toString(), (wrapperModuleInputsIterator - wrapperModuleInputs.begin()));
          ostr << subModule->id().id_ << "::" << portId.toString() << " --> Input" << (wrapperModuleInputsIterator - wrapperModuleInputs.begin()) << std::endl;
          subModule->removeInputPort(portId);
          (*wrapperModuleInputsIterator)->setInternalId(portId);
          subModule->add_input_port(*wrapperModuleInputsIterator);
          ++wrapperModuleInputsIterator;
        }
      }
    }
    for (const auto& outputPort : subModule->outputPorts())
    {
      if (outputPort->nconnections() == 0 && outputPort->get_typename() != "MetadataObject")
      {
        auto portId = outputPort->id();
        logCritical("Found output port that can be exposed: {} :: {}", subModule->id().id_, portId.toString());

        if (wrapperModuleOutputsIterator != wrapperModuleOutputs.end())
        {
          logCritical("\t\tperforming port surgery on {} :: {} --> Output{}", subModule->id().id_, portId.toString(), (wrapperModuleOutputsIterator - wrapperModuleOutputs.begin()));
          ostr << subModule->id().id_ << "::" << portId.toString() << " --> Output" << (wrapperModuleOutputsIterator - wrapperModuleOutputs.begin()) << std::endl;
          subModule->removeOutputPort(portId);
          (*wrapperModuleOutputsIterator)->setInternalId(portId);
          subModule->add_output_port(*wrapperModuleOutputsIterator);
          ++wrapperModuleOutputsIterator;
        }
      }
    }
  }
  module_->get_state()->setValue(Parameters::PortSettings, ostr.str());
  module_->get_state()->setTransientValue(Parameters::ModuleIdList, makeModuleIdList(*network), true);
}

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

#include <Dataflow/Network/NetworkInterface.h>
#include <Modules/Legacy/Fields/CompositeModuleTestGFB_FM.h>
// TODO
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun;
using namespace Dataflow::Networks;
using namespace Dataflow::Engine;

MODULE_INFO_DEF(CompositeModuleTestGFB_FM, MiscField, SCIRun)

class Modules::Fields::CompositeModuleImpl
{
public:
  NetworkHandle subNet_;

  explicit CompositeModuleImpl(CompositeModuleTestGFB_FM* module) : module_(module) {}
  void initializeSubnet();

private:
  CompositeModuleTestGFB_FM* module_;
};

CompositeModuleTestGFB_FM::CompositeModuleTestGFB_FM()
    : Module(staticInfo_, false), impl_(new CompositeModuleImpl(this))
{
  INITIALIZE_PORT(InputField)
  INITIALIZE_PORT(Faired_Mesh)
  INITIALIZE_PORT(Mapping)
}

CompositeModuleTestGFB_FM::~CompositeModuleTestGFB_FM() = default;

void CompositeModuleTestGFB_FM::setStateDefaults()
{
}

void CompositeModuleTestGFB_FM::execute()
{
  impl_->initializeSubnet();

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

void CompositeModuleImpl::initializeSubnet()
{
  if (!subNet_)
  {
    module_->remark("Basic composite module concept part 1");

    subNet_ = module_->network()->createSubnetwork();
    if (subNet_) { module_->remark("Subnet created."); }
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

    const auto xml = XMLSerializer::load_xml<NetworkFile>("X:\\Dev\\r1\\SCIRun\\Release\\xml1.txt");
    subNet_->loadXmlDataIntoNetwork(xml->network.data());
    const auto network = subNet_->getNetwork();

    if (network->nmodules() == 2)
    {
      module_->remark("Created subnet with 2 modules");
    }
    else
    {
      module_->error("Subnet missing modules");
      return;
    }

    auto wrapperModuleInputs = module_->inputPorts();
    auto wrapperModuleInputsIterator = wrapperModuleInputs.begin();
    auto wrapperModuleOutputs = module_->outputPorts();
    auto wrapperModuleOutputsIterator = wrapperModuleOutputs.begin();
    for (size_t i = 0; i < network->nmodules(); ++i) 
    {
      const auto subModule = network->module(i);

      for (const auto& inputPort : subModule->inputPorts())
      {
        if (inputPort->nconnections() == 0 && inputPort->get_typename() != "MetadataObject")
        {
          auto portId = inputPort->id();
          logCritical("Found input port that can be exposed: {} :: {}", subModule->id().id_, portId.toString());

          if (wrapperModuleInputsIterator != wrapperModuleInputs.end())
          {
            logCritical("\t\tperforming port surgery on {} {}", subModule->id().id_, portId.toString());
            subModule->removeInputPort(portId);
            subModule->add_input_port(*wrapperModuleInputsIterator++);
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
            logCritical("\t\tperforming port surgery on {} {}", subModule->id().id_, portId.toString());
            subModule->removeOutputPort(portId);
            subModule->add_output_port(*wrapperModuleOutputsIterator++);  
          }
        }
      }

    }
    
    if (subNet_->getNetwork()->nconnections() == 1)
    {
      module_->remark("Created connection between 2 modules");
    }
    else
    {
      module_->error("Connection error");
    }
  }
}

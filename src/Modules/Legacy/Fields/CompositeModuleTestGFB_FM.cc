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
  auto state = get_state();
  /*
  state->setValue(Variables::FunctionString, std::string(
"<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
<!DOCTYPE boost_serialization>
<boost_serialization signature="serialization::archive" version="18">
<networkFragment class_id="0" tracking_level="0" version="6">
        <networkInfo class_id="1" tracking_level="0" version="0">
                <modules class_id="2" tracking_level="0" version="0">
                        <count>2</count>
                        <item_version>0</item_version>
                        <item class_id="3" tracking_level="0" version="0">
                                <first>FairMesh:0</first>
                                <second class_id="4" tracking_level="0" version="0">
                                        <module class_id="5" tracking_level="0" version="0">
                                                <package_name_>SCIRun</package_name_>
                                                <category_name_>NewField</category_name_>
                                                <module_name_>FairMesh</module_name_>
                                        </module>
                                        <state class_id="6" tracking_level="0" version="0">
                                                <stateMap class_id="7" tracking_level="0"
version="0"> <count>5</count> <item_version>0</item_version> <item class_id="8" tracking_level="0"
version="0"> <first class_id="9" tracking_level="0" version="0"> <name>FairMeshMethod</name>
                                                                </first>
                                                                <second class_id="10"
tracking_level="0" version="0"> <name>FairMeshMethod</name> <value class_id="11" tracking_level="1"
version="0" object_id="_0"> <which>2</which> <value>fast</value>
                                                                        </value>
                                                                </second>
                                                        </item>
                                                        <item>
                                                                <first>
                                                                        <name>FilterCutoff</name>
                                                                </first>
                                                                <second>
                                                                        <name>FilterCutoff</name>
                                                                        <value object_id="_1">
                                                                                <which>1</which>
                                                                                <value>1.00000000000000006e-01</value>
                                                                        </value>
                                                                </second>
                                                        </item>
                                                        <item>
                                                                <first>
                                                                        <name>Lambda</name>
                                                                </first>
                                                                <second>
                                                                        <name>Lambda</name>
                                                                        <value object_id="_2">
                                                                                <which>1</which>
                                                                                <value>6.30700000000000038e-01</value>
                                                                        </value>
                                                                </second>
                                                        </item>
                                                        <item>
                                                                <first>
                                                                        <name>NumIterations</name>
                                                                </first>
                                                                <second>
                                                                        <name>NumIterations</name>
                                                                        <value object_id="_3">
                                                                                <which>0</which>
                                                                                <value>50</value>
                                                                        </value>
                                                                </second>
                                                        </item>
                                                        <item>
                                                                <first>
                                                                        <name>ProgrammableInputPortEnabled</name>
                                                                </first>
                                                                <second>
                                                                        <name>ProgrammableInputPortEnabled</name>
                                                                        <value object_id="_4">
                                                                                <which>3</which>
                                                                                <value>0</value>
                                                                        </value>
                                                                </second>
                                                        </item>
                                                </stateMap>
                                        </state>
                                </second>
                        </item>
                        <item>
                                <first>GetFieldBoundary:0</first>
                                <second>
                                        <module>
                                                <package_name_>SCIRun</package_name_>
                                                <category_name_>NewField</category_name_>
                                                <module_name_>GetFieldBoundary</module_name_>
                                        </module>
                                        <state>
                                                <stateMap>
                                                        <count>1</count>
                                                        <item_version>0</item_version>
                                                        <item>
                                                                <first>
                                                                        <name>ProgrammableInputPortEnabled</name>
                                                                </first>
                                                                <second>
                                                                        <name>ProgrammableInputPortEnabled</name>
                                                                        <value object_id="_5">
                                                                                <which>3</which>
                                                                                <value>0</value>
                                                                        </value>
                                                                </second>
                                                        </item>
                                                </stateMap>
                                        </state>
                                </second>
                        </item>
                </modules>
                <connections class_id="12" tracking_level="0" version="0">
                        <count>1</count>
                        <item_version>0</item_version>
                        <item class_id="13" tracking_level="0" version="0">
                                <moduleId1_>GetFieldBoundary:0</moduleId1_>
                                <port1_ class_id="14" tracking_level="0" version="0">
                                        <name>BoundaryField</name>
                                        <id>0</id>
                                </port1_>
                                <moduleId2_>FairMesh:0</moduleId2_>
                                <port2_>
                                        <name>Input_Mesh</name>
                                        <id>0</id>
                                </port2_>
                        </item>
                </connections>
        </networkInfo>
        <modulePositions class_id="15" tracking_level="0" version="0">
                <count>2</count>
                <item_version>0</item_version>
                <item class_id="16" tracking_level="0" version="0">
                        <first>FairMesh:0</first>
                        <second class_id="17" tracking_level="0" version="0">
                                <first>-4.90000000000000000e+02</first>
                                <second>-3.80000000000000000e+02</second>
                        </second>
                </item>
                <item>
                        <first>GetFieldBoundary:0</first>
                        <second>
                                <first>-5.32000000000000000e+02</first>
                                <second>-4.56000000000000000e+02</second>
                        </second>
                </item>
        </modulePositions>
        <moduleNotes class_id="18" tracking_level="0" version="0">
                <count>0</count>
                <item_version>0</item_version>
        </moduleNotes>
        <connectionNotes>
                <count>0</count>
                <item_version>0</item_version>
        </connectionNotes>
        <moduleTags class_id="19" tracking_level="0" version="0">
                <count>2</count>
                <item_version>0</item_version>
                <item class_id="20" tracking_level="0" version="0">
                        <first>FairMesh:0</first>
                        <second>-1</second>
                </item>
                <item>
                        <first>GetFieldBoundary:0</first>
                        <second>-1</second>
                </item>
        </moduleTags>
        <disabledModules class_id="21" tracking_level="0" version="0">
                <count>0</count>
                <item_version>0</item_version>
        </disabledModules>
        <disabledConnections>
                <count>0</count>
                <item_version>0</item_version>
        </disabledConnections>
        <moduleTagLabels class_id="22" tracking_level="0" version="0">
                <count>0</count>
                <item_version>0</item_version>
        </moduleTagLabels>
        <loadTagGroups>0</loadTagGroups>
        <subnetworks class_id="23" tracking_level="0" version="0">
                <count>0</count>
                <item_version>0</item_version>
        </subnetworks>
</networkFragment>
</boost_serialization>

"  ));
*/
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

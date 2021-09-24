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


#include <Modules/Legacy/Fields/CompositeModuleTestGFB_FM.h>
#include <Modules/Legacy/Fields/GetFieldBoundary.h>
#include <Modules/Legacy/Fields/FairMesh.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ConnectionId.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(CompositeModuleTestGFB_FM, MiscField, SCIRun)

class SCIRun::Modules::Fields::CompositeModuleImpl
{
public:
  NetworkHandle subNet_;

  explicit CompositeModuleImpl(CompositeModuleTestGFB_FM* module) : module_(module) { }
  void initializeSubnet();
private:
  CompositeModuleTestGFB_FM* module_;
};

CompositeModuleTestGFB_FM::CompositeModuleTestGFB_FM() : Module(staticInfo_, false),
  impl_(new CompositeModuleImpl(this))
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Faired_Mesh);
}

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
						<stateMap class_id="7" tracking_level="0" version="0">
							<count>5</count>
							<item_version>0</item_version>
							<item class_id="8" tracking_level="0" version="0">
								<first class_id="9" tracking_level="0" version="0">
									<name>FairMeshMethod</name>
								</first>
								<second class_id="10" tracking_level="0" version="0">
									<name>FairMeshMethod</name>
									<value class_id="11" tracking_level="1" version="0" object_id="_0">
										<which>2</which>
										<value>fast</value>
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

  logCritical("CompositeModuleTestGFB_FM::execute()");
  impl_->subNet_->executeAll(nullptr);
}

void CompositeModuleImpl::initializeSubnet()
{
  if (!subNet_)
  {
    module_->remark("Basic composite module concept part 1");

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
    auto gfb = subNet_->addModule({"GetFieldBoundary", "...", ",,,"});

    dynamic_cast<Module*>(gfb.get())->removeInputPort(dynamic_cast<GetFieldBoundary*>(gfb.get())->InputField.toId());
    dynamic_cast<Module*>(gfb.get())->add_input_port(module_->getInputPort(module_->InputField.toId()));

    auto fm = subNet_->addModule({"FairMesh", "...", ",,,"});
    dynamic_cast<Module*>(fm.get())->removeOutputPort(dynamic_cast<FairMesh*>(fm.get())->Faired_Mesh.toId());
    dynamic_cast<Module*>(fm.get())->add_output_port(module_->getOutputPort(module_->Faired_Mesh.toId()));


    if (subNet_->getNetwork()->nmodules() == 2)
    {
      module_->remark("Created subnet with 2 modules");
    }
    else
    {
      module_->error("Subnet missing modules");
      return;
    }
    auto conn = subNet_->requestConnection(gfb->outputPorts()[0].get(), fm->inputPorts()[0].get());
    if (conn && subNet_->getNetwork()->nconnections() == 1)
    {
      module_->remark("Created connection between 2 modules");
    }
    else
    {
      module_->error("Connection error");
      return;
    }
  }
}

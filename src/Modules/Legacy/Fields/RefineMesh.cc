/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Modules/Legacy/Fields/RefineMesh.cc


#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
//#include <Core/Datatypes/Legacy/Matrix/Matrix.h>
#include <Modules/Legacy/Fields/RefineMesh.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMesh.h> 

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const ModuleLookupInfo RefineMesh::staticInfo_("RefineMesh", "ChangeFieldData", "SCIRun"); 

RefineMesh::RefineMesh()
		:Module(staticInfo_)
{
		INITIALIZE_PORT(InputField);
		INITIALIZE_PORT(OutputField);
}

void RefineMesh::setStateDefaults()
{
		setStateStringFromAlgoOption(Parameters::AddConstraints);
		setStateStringFromAlgoOption(Parameters::RefineMethod);
		setStateStringFromAlgoOption(Parameters::IsoValue);
}

void
RefineMesh::execute()
{
	auto input = getRequiredInput(InputField); 

  if (needToExecute() )
  {
    update_state(Executing);
      
    pushInputMeshFieldInfo(input); 

		setAlgoOptionFromState(Parameters::AddConstraints);
		setAlgoOptionFromState(Parameters::RefineMethod);
		setAlgoOptionFromState(Parameters::IsoValue);

		#if SCIRUN4_CODE_TO_BE_ENABLED_LATER
		if (need_mapping)
		{
		if (!(algo_.run(input_field_handle,output_field_handle,mapping_matrix_handle))) return;
		}
		else
		{
		if (!(algo_.run(input_field_handle,output_field_handle))) return;
		}
		#endif
		remark("Mapping matrix port implementation is not enabled yet--please contact a developer");

		auto output = algo().run(withInputData((InputField, input)));
		sendOutputFromAlgorithm(OutputField, output); 

		#if SCIRUN4_CODE_TO_BE_ENABLED_LATER
		send_output_handle("Mapping", mapping_matrix_handle);
		#endif
  }
}

void RefineMesh::pushInputMeshFieldInfo(FieldHandle input) const
{
		auto state = get_state();
		std::string name = input->properties().get_name();
		if (name.empty())
				name = "--- no name ---";
		state->setValue(Parameters::RefineMethod, name);
		std::string refMethod;
		if (input->vfield()->is_nodata()) refMethod = "Default";
		if (input->vfield()->is_constantdata()) refMethod = "Expand refinement volume to improve element quality";
		state->setValue(Parameters::RefineMethod, refMethod);

		/*auto state = get_state();
		std::string name = input->properties().get_name();
		if (name.empty())
				name = "--- no name ---";
		state->setValue(Parameters::AddConstraints, name);
		std::string addCon;
		if (input->vfield()->is_nodata()) addCon = "Do not add constraint";
		if (input->vfield()->is_constantdata()) addCon = "Do not refine nodes/elements with values less than isovalue";
		if (input->vfield()->is_lineardata()) addCon = "Do not refine nodes/elements with values unequal to isovalue";
		if (input->vfield()->is_quadraticdata()) addCon = "Do not refine nodes/elements with values greater than isovalue";
		if (input->vfield()->is_cubicdata()) addCon = "Do not refine any elements";*/
		//state->setValue(Parameters::AddConstraints, addCon);

		//state->setValue(Parameters::IsoValue, name);
		//std::string isoval;
		//if (input->matrix->) isoval = "0.0";
		//state->setValue(Parameters::InputType, isoval);
}
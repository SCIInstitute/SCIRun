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


/// @todo Documentation Modules/Legacy/Fields/RefineMesh.cc

#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Legacy/Fields/RefineMesh.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>


using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

MODULE_INFO_DEF(RefineMesh, ChangeFieldData, SCIRun)

RefineMesh::RefineMesh() : Module(staticInfo_)
{
		INITIALIZE_PORT(InputField);
		INITIALIZE_PORT(OutputField);
		INITIALIZE_PORT(IsoValueField);
}

void RefineMesh::setStateDefaults()
{
		setStateStringFromAlgoOption(Parameters::AddConstraints);
		setStateStringFromAlgoOption(Parameters::RefineMethod);
		setStateDoubleFromAlgo(Parameters::IsoValue);
}

void
RefineMesh::execute()
{
auto input = getRequiredInput(InputField);
#if SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool need_mapping = oport_connected("Mapping");
#endif
auto isovaluefield = getOptionalInput(IsoValueField);

  if (needToExecute() )
  {
		setAlgoOptionFromState(Parameters::AddConstraints);
		setAlgoOptionFromState(Parameters::RefineMethod);
		setAlgoDoubleFromState(Parameters::IsoValue);

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

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


#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
// For mapping matrices
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshTriSurfAlgoV.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshTetVolAlgoV.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshHexVolAlgoV.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshCurveAlgoV.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshQuadSurfAlgoV.h>

//STL classes needed
#include <algorithm>
#include <set>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;


ALGORITHM_PARAMETER_DEF(Fields, AddConstraints);
ALGORITHM_PARAMETER_DEF(Fields, RefineMethod);
ALGORITHM_PARAMETER_DEF(Fields, IsoValue);

RefineMeshAlgo::RefineMeshAlgo()
{
		using namespace Parameters;
		addOption(AddConstraints,"all","all|greaterthan|unequal|lessthan|none");
		addOption(RefineMethod,"Default","Default|Expand refinement volume to improve element quality");
		addParameter(IsoValue,0.0);
}

AlgorithmOutput RefineMeshAlgo::run(const AlgorithmInput& input) const
{
	auto field = input.get<Field>(Variables::InputField);
  FieldHandle outputField;

  if (!runImpl(field, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
	AlgorithmOutput output;
	output[Variables::OutputField] = outputField;
  return output;
}

// General access function

bool
RefineMeshAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
	REPORT_STATUS(RefineMesh);
  if (!input)
  {
    error("No input field");
    return (false);
  }
	FieldInformation fi(input);
	FieldInformation fo(output);

	const std::string rMethod = getOption(Parameters::RefineMethod);
	const double isoVal = get(Parameters::IsoValue).toDouble();
	const std::string addCon = getOption(Parameters::AddConstraints);

  if (input->vfield()->num_values() == 0)
  {
    error("Input field has no data values. The RefineMesh algorithm requires input fields to contain data.");
    return (false);
  }

  if (addCon == "none")
  {
    output = input;
    return (true);
  }

  if (fi.is_pnt_element() || fi.is_prism_element())
  {
    error("This algorithm does not support point or prism meshes");
    return(false);
  }

  if ((!(fi.is_scalar())) && (addCon != "all"))
  {
    error("Field data needs to be of scalar type");
    return (false);
  }

  if (fi.is_quad_element())
  {
    RefineMeshQuadSurfAlgoV algo;
    algo.setUpdaterFunc(getUpdaterFunc());
    return(algo.runImpl(input, output, addCon, isoVal));
  }

  if (fi.is_hex_element())
  {
    bool convex = false;
    convex = rMethod == "Expand refinement volume to improve element quality";
    RefineMeshHexVolAlgoV algo;
    algo.setUpdaterFunc(getUpdaterFunc());
    return(algo.runImpl(input, output, convex, addCon, isoVal));
  }

  if (fi.is_crv_element())
  {
    RefineMeshCurveAlgoV algo;
    algo.setUpdaterFunc(getUpdaterFunc());
    return(algo.runImpl(input, output, addCon, isoVal));
  }

  if (fi.is_tri_element())
  {
    RefineMeshTriSurfAlgoV algo;
    algo.setUpdaterFunc(getUpdaterFunc());
    return(algo.runImpl(input, output, addCon, isoVal));
  }

  if (fi.is_tet_element())
  {
    RefineMeshTetVolAlgoV algo;
    algo.setUpdaterFunc(getUpdaterFunc());
    return(algo.runImpl(input, output, addCon, isoVal));
  }

  error("No refinement method has been implemented for this type of mesh");
  return (false);
}

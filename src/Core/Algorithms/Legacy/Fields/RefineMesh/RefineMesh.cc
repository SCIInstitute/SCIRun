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
		add_option(AddConstraints,"all","all|lessthan|unequal|greaterthan|none");
		add_option(RefineMethod,"Default","Default|Expand refinement volume to improve element quality"); 
		addParameter(IsoValue,0.0);
}

AlgorithmOutput RefineMeshAlgo::run_generic(const AlgorithmInput& input) const 
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
	ScopedAlgorithmStatusReporter asr(this, "RefineMesh"); 
  if (!input)
  {
    error("No input field");
    return (false);
  }
	FieldInformation fi(input);
	FieldInformation fo(output); 

	const std::string rMethod = get_option(Parameters::RefineMethod);
	const double isoVal = get(Parameters::IsoValue).toDouble();
	const std::string addCon = get_option(Parameters::AddConstraints);
 
  if (input->vfield()->num_values() == 0)
  {
			std::cout<< "numV == 0" <<std::endl;
    error("Input field has no data values. The RefineMesh algorithm requires input fields to contain data."); 
    return (false);
  }
	
  if (addCon == "Do not add constraint")
  {
			std::cout<< "noAddCon? " << addCon <<std::endl;
    output = input;
    return (true); 
  }
	  
  if (fi.is_pnt_element() || fi.is_prism_element())
  {
			std::cout<< "pntElem?" <<std::endl;
    error("This algorithm does not support point or prism meshes");
    return(false);
  }
    
  if ((!(fi.is_scalar()))&&(addCon != "all"))
  {
			std::cout<< "scarlar?" <<std::endl;
    error("Field data needs to be of scalar type");
    return (false);
  }

  if (fi.is_quad_element())
  {
			std::cout <<"surf" << std::endl; 
    RefineMeshQuadSurfAlgoV algo;
    return(algo.runImpl(input,output,addCon,isoVal));
  }
  
  if (fi.is_hex_element())
  {
			std::cout <<"convex" << std::endl; 
			bool convex = get_option(Parameters::RefineMethod) == "Expand refinement volume to improve element quality"; 
    RefineMeshHexVolAlgoV algo;
		return(algo.runImpl(input,output,convex,addCon,isoVal));
  }

  if (fi.is_crv_element())
  {
			std::cout <<"curve" << std::endl; 
			RefineMeshCurveAlgoV algo; 
			return(algo.runImpl(input,output,addCon,isoVal));
  }
  
  if (fi.is_tri_element())
  {
			std::cout <<"trisurf" << std::endl; 
			RefineMeshTriSurfAlgoV algo; 
			return(algo.runImpl(input,output,addCon,isoVal));
  }
  
  if (fi.is_tet_element())
  {
			std::cout <<"tet" << std::endl; 
			RefineMeshTetVolAlgoV algo; 
			return(algo.runImpl(input,output,addCon,isoVal));
  }

  error("No refinement method has been implemented for this type of mesh");
  return (false);
}                           
     
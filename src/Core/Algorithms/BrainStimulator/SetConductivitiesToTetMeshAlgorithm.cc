/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

   Author : Spencer Frisby, Moritz Dannhauer
   Date   : May 2014
*/

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <iostream>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace boost::assign;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::Skin() {return AlgorithmParameterName("Skin");}
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::SoftBone() {return AlgorithmParameterName("Soft Bone");}
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::HardBone() {return AlgorithmParameterName("Hard Bone");}
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::CSF() { return AlgorithmParameterName("CSF");}
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::GM() { return AlgorithmParameterName("GM");}
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::WM() { return AlgorithmParameterName("WM");}
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::Electrode() { return AlgorithmParameterName("Electrode");}
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::InternalAir() { return AlgorithmParameterName("InternalAir");}

AlgorithmInputName  SetConductivitiesToTetMeshAlgorithm::MESH("MESH");
AlgorithmOutputName SetConductivitiesToTetMeshAlgorithm::OUTPUTMESH("OUTPUTMESH");

SetConductivitiesToTetMeshAlgorithm::SetConductivitiesToTetMeshAlgorithm()
{
  ElemLabelLookup += 1,2,3,4,5,6,7,8;
  addParameter(Skin(),      0.43);    /// electrical conductivities (isotropic) default values based on the literature
  addParameter(SoftBone(),     0.02856);
  addParameter(HardBone(),     0.00640);
  addParameter(CSF(),       1.79);
  addParameter(GM(),        0.33);
  addParameter(WM(),        0.142);
  addParameter(Electrode(), 1.4);
  addParameter(InternalAir(), 1e-6);
}

AlgorithmOutput SetConductivitiesToTetMeshAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto mesh  = input.get<Field>(MESH);
 
  AlgorithmOutput output;
  
  FieldHandle output_field = run(mesh);
  
  output[OUTPUTMESH] = output_field;;

  return output;
}

FieldHandle SetConductivitiesToTetMeshAlgorithm::run(FieldHandle fh) const
{
  /// making sure the field is not null
  if (!fh)
    THROW_ALGORITHM_INPUT_ERROR("Field supplied is empty ");
  
  /// making sure the data is on the elem and not the nodes
  FieldInformation fi(fh);
  if (!fi.is_constantdata())
    THROW_ALGORITHM_INPUT_ERROR("This function requires the data to be on the elements ");
  
  /// making sure the field contains data
  VField* vfield = fh->vfield();
  if (vfield->is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("Field supplied contained no data ");
  
  /// making sure the field is not in vector format
  if (!vfield->is_scalar())
    THROW_ALGORITHM_INPUT_ERROR("Function only supports scalar labels. ");
     
  /// array holding conductivities    
  std::vector<double> conductivies = {get(Skin()).getDouble(), get(SoftBone()).getDouble(), get(HardBone()).getDouble(),
  get(CSF()).getDouble(), get(GM()).getDouble(), get(WM()).getDouble(), get(InternalAir()).getDouble(), get(Electrode()).getDouble()};
  
  //check if defined conductivities and lookup table are consistent
  if (conductivies.size()!=ElemLabelLookup.size())
     THROW_ALGORITHM_INPUT_ERROR("Defined conductivities and lookup table are inconsistent! ");
  
  /// replacing field value with conductivity value
  FieldHandle output = CreateField(fi, fh->mesh());
  VField* ofield = output->vfield();
  int val = 0;
  int cnt = 0;
  
  for (VMesh::Elem::index_type i=0; i < vfield->vmesh()->num_elems(); i++) /// loop over all tetrahedral elements
  {
    vfield->get_value(val, i);  //get the data value stored on the current element
    
    bool found=false; /// boolean that indicates if element label was found in lookup
    
    for (size_t j = 0; j < ElemLabelLookup.size(); ++j) /// loop over lookup table and check if the current element has one of the desired labels, if not error
    {   
      if (val==ElemLabelLookup[j]) 
       { 
	ofield->set_value(conductivies[j], i); /// if so, set it to the isotropic conductivity value
	found=true;
        break;
       }
    }
    
    if (!found)
    {
     THROW_ALGORITHM_INPUT_ERROR("Tetrahedral element label could not be found in lookup table. ");
    } 
    
    cnt++;
    if (cnt == 500)
    {
      cnt = 0;
      update_progress_max(i,vfield->vmesh()->num_elems());
    }
  }

  return output;
}

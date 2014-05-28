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

   Author : Spencer Frisby
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

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::Skin("Skin");
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::Skull("Skull");
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::CSF("CSF");
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::GM("GM"); 
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::WM("WM");  
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::Electrode("Electrode");

AlgorithmInputName  SetConductivitiesToTetMeshAlgorithm::MESH("MESH");
AlgorithmInputName  SetConductivitiesToTetMeshAlgorithm::INHOMOGENEOUS_SKULL("INHOMOGENEOUS_SKULL");
AlgorithmInputName  SetConductivitiesToTetMeshAlgorithm::ANISOTROPIC_WM("ANISOTROPIC_WM");
AlgorithmOutputName SetConductivitiesToTetMeshAlgorithm::OUTPUTMESH("OUTPUTMESH");

SetConductivitiesToTetMeshAlgorithm::SetConductivitiesToTetMeshAlgorithm()
{
  addParameter(Skin,      0.0);
  addParameter(Skull,     0.0);
  addParameter(CSF,       0.0);
  addParameter(GM,        0.0);
  addParameter(WM,        0.0);
  addParameter(Electrode, 0.0);
}

AlgorithmOutput SetConductivitiesToTetMeshAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto mesh  = input.get<Field>(MESH);
  auto skull = input.get<Matrix>(INHOMOGENEOUS_SKULL);
  auto wm    = input.get<Matrix>(ANISOTROPIC_WM);

 /* ENSURE_ALGORITHM_INPUT_NOT_NULL(pos_orient, "ELECTRODE_COIL_POSITIONS_AND_NORMAL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri, "ELECTRODE_TRIANGULATION input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri2, "ELECTRODE_TRIANGULATION2 input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil, "COIL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil2, "COIL2 input field");*/
 
  AlgorithmOutput output;
  
  FieldHandle output_field = run(mesh);
  
  output[OUTPUTMESH] = output_field;;

  return output;
}

FieldHandle SetConductivitiesToTetMeshAlgorithm::run(FieldHandle fh) const
{
  // making sure the field is not null
  if (!fh)
    THROW_ALGORITHM_INPUT_ERROR("Field supplied is empty ");
  
  // making sure the data is on the elem and not the nodes
  FieldInformation fi(fh);
  if (!fi.is_constantdata())
    THROW_ALGORITHM_INPUT_ERROR("This function requires the data to be on the elements ");
  
  // making sure the field contains data
  VField* vfield = fh->vfield();
  if (vfield->is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("Field supplied contained no data ");
  
  // making sure the field is not in vector format
  if (vfield->is_vector())
    THROW_ALGORITHM_INPUT_ERROR("Function is not setup to work with vectors at this time ");
  
  // making sure no field value (from the input) is outside the range 1-6
  double ival = 0;
  for (VMesh::Elem::index_type i=0; i < vfield->vmesh()->num_elems(); i++)
  {
    vfield->get_value(ival, i);
    if (ival > 6 || ival < 1)
      THROW_ALGORITHM_INPUT_ERROR("Field values were outside the range 1-6 ");
  }
  
  // array holding conductivities
  double conductivies[] = {get(Skin).getDouble(),
    get(Skull).getDouble(), get(CSF).getDouble(),
    get(GM).getDouble(), get(WM).getDouble(),
    get(Electrode).getDouble()};
  
  // replacing field value with conductivity value
  FieldHandle output = CreateField(fi, fh->mesh());
  VField* ofield = output->vfield();
  int val = 0;
  for (VMesh::Elem::index_type i=0; i < vfield->vmesh()->num_elems(); i++)
  {
    vfield->get_value(val, i);
    switch (val)
    {
      case 1:
        ofield->set_value(conductivies[0], i);
        break;
      case 2:
        ofield->set_value(conductivies[1], i);
        break;
      case 3:
        ofield->set_value(conductivies[2], i);
        break;
      case 4:
        ofield->set_value(conductivies[3], i);
        break;
      case 5:
        ofield->set_value(conductivies[4], i);
        break;
      case 6:
        ofield->set_value(conductivies[5], i);
        break;
    }
  }

  return output;
}

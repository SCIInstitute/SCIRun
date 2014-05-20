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

   Author            : Spencer Frisby
   Last modification : May 2014
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

//void ShowWhatFieldHandleIsMadeOf(FieldHandle input)
//{
//  VField* vfield = input->vfield();
//  if (vfield->is_nodata())
//    std::cout << "There is not data contained in the field" << std::endl;
//  else
//  {
//    // show the number of values of the field, if greater than zero print them out
//    VMesh::size_type n = vfield->vfield()->num_values();
//    std::cout << "Field contains " << n << " values" << std::endl;
//    if (n > 0)
//    {
//      if (vfield->is_vector())
//      {
//        Vector val;
//        for (VMesh::index_type idx = 0; idx<n; idx++)
//        {
//          vfield->get_value(val,idx);
//          std::cout << "x = " << val.x() << ", y = " << val.y() << ", z = " << val.z() << std::endl;
//        }
//      }
//    }
//    if (vfield->is_scalar())
//    {
//      double val = 0;
//      for (VMesh::index_type idx = 0; idx<n; idx++)
//      {
//        vfield->get_value(val,idx);
//        std::cout << val << ((idx == (n-1)) ? "\n" : " ");
//      }
//    }
//  }
//}

SetConductivitiesToTetMeshAlgorithm::SetConductivitiesToTetMeshAlgorithm()
{
  addParameter(skin(),      0);
  addParameter(skull(),     0);
  addParameter(CSF(),       0);
  addParameter(GM(),        0);
  addParameter(WM(),        0);
  addParameter(electrode(), 0);
}

//bool row_select = get(rowCheckBox()).getBool();
//bool col_select = get(columnCheckBox()).getBool();
//index_type row_start = get(rowStartSpinBox()).getInt();
//index_type row_end = get(rowEndSpinBox()).getInt();
//index_type col_start = get(columnStartSpinBox()).getInt();
//index_type col_end = get(columnEndSpinBox()).getInt();

void SetConductivitiesToTetMeshAlgorithm::run(FieldHandle fh)
{
  // making sure the field is not null
  if (!fh)
    THROW_ALGORITHM_INPUT_ERROR("Field was null");
  
  VField* vfield = fh->vfield();
  
  // making sure the field contained data
  if (vfield->is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("Field contained no data");
  
  // TODO: make sure the data is on the elements and not the nodes
  
  std::cout << "fields: " << vfield->num_values() << std::endl;
  
  // # of elems = vfield->vmesh()->num_elems()
  // displaying the field value of the elements
  double val = 0;
  for (VMesh::Elem::index_type i=0; i < vfield->vmesh()->num_elems(); i++)
  {
    vfield->get_value(val, i);
    if (i == 0) std::cout << "elements: ";
    std::cout << val << ((vfield->vmesh()->num_elems() == (i+1)) ? "\n" : " ");
  }

  // array holding conductivies
  int size = 6;
  double conductivies [] = {get(skin()).getDouble(), get(skull()).getDouble(), get(CSF()).getDouble(), get(GM()).getDouble(), get(WM()).getDouble(), get(electrode()).getDouble()};
  for (int i=0; i<size; i++)
  {
    if (i == 0) std::cout << "conductivities: ";
    std::cout << conductivies[i] << ((size == (i+1)) ? "\n" : " ");
  }
  
  
  // TODO: Replace field value with conductivity value
  

}

AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::skin()      { return AlgorithmParameterName("skin");  }
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::skull()     { return AlgorithmParameterName("skull"); }
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::CSF()       { return AlgorithmParameterName("CSF");   }
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::GM()        { return AlgorithmParameterName("GM");    }
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::WM()        { return AlgorithmParameterName("WM");    }
AlgorithmParameterName SetConductivitiesToTetMeshAlgorithm::electrode() { return AlgorithmParameterName("electrode"); }

AlgorithmInputName  SetConductivitiesToTetMeshAlgorithm::MESH("MESH");
AlgorithmInputName  SetConductivitiesToTetMeshAlgorithm::INHOMOGENEOUS_SKULL("INHOMOGENEOUS_SKULL");
AlgorithmInputName  SetConductivitiesToTetMeshAlgorithm::ANISOTROPIC_WM("ANISOTROPIC_WM");
AlgorithmOutputName SetConductivitiesToTetMeshAlgorithm::OUTPUTMESH("OUTPUTMESH");

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
  //output[OUTPUTMESH] = out1;

  return output;
}

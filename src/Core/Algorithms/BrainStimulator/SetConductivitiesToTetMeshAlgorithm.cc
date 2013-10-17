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
*/

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <iostream>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;
    
const AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::ELECTRODE_COIL_POSITIONS_AND_NORMAL("ELECTRODE_COIL_POSITIONS_AND_NORMAL");
const AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::ELECTRODE_TRIANGULATION("ELECTRODE_TRIANGULATION");
const AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::ELECTRODE_TRIANGULATION2("ELECTRODE_TRIANGULATION2");
const AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::COIL("COIL");
const AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::COIL2("COIL2");
const AlgorithmOutputName SetConductivitiesToTetMeshAlgorithm::ELECTRODES_FIELD("ELECTRODES_FIELD");
const AlgorithmOutputName SetConductivitiesToTetMeshAlgorithm::COILS_FIELD("COILS_FIELD");


AlgorithmOutput SetConductivitiesToTetMeshAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto pos_orient = input.get<Field>(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  auto tri = input.get<Field>(ELECTRODE_TRIANGULATION);
  auto tri2 = input.get<Field>(ELECTRODE_TRIANGULATION2);
  auto coil = input.get<Field>(COIL);
  auto coil2 = input.get<Field>(COIL2);
  ENSURE_ALGORITHM_INPUT_NOT_NULL(pos_orient, "ELECTRODE_COIL_POSITIONS_AND_NORMAL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri, "ELECTRODE_TRIANGULATION input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri2, "ELECTRODE_TRIANGULATION2 input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil, "COIL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil2, "COIL2 input field");
  //old-style run call, just put algorithm code here
  //auto outputs = run(boost::make_tuple(lhs, rhs), Option(get(Variables::AppendMatrixOption).getInt()));
  // CODE HERE
  FieldHandle out1,out2;

  //Algorithm starts here:
  //VField* vfield = elc_coil_pos_and_normal->vfield();
   VMesh*  vmesh  = pos_orient->vmesh();
 
   std::cout << "a: " << vmesh->num_nodes() << std::endl;
   //for (int i=0;i<vmesh->num_nodes();;i++)
   //{
   
   
   //}
  //


  AlgorithmOutput output;
  output[ELECTRODES_FIELD] = out1;
  output[COILS_FIELD] = out2;
  return output;
}

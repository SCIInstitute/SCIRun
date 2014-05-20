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

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
#include <iostream>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;
    
AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::MESH("MESH");
AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::INHOMOGENEOUS_SKULL("INHOMOGENEOUS_SKULL");
AlgorithmInputName SetConductivitiesToTetMeshAlgorithm::ANISOTROPIC_WM("ANISOTROPIC_WM");
AlgorithmOutputName SetConductivitiesToTetMeshAlgorithm::OUTPUTMESH("OUTPUTMESH");

AlgorithmOutput SetConductivitiesToTetMeshAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto mesh = input.get<Field>(MESH);
  auto skull = input.get<Matrix>(INHOMOGENEOUS_SKULL);
  auto wm = input.get<Matrix>(ANISOTROPIC_WM);

 /* ENSURE_ALGORITHM_INPUT_NOT_NULL(pos_orient, "ELECTRODE_COIL_POSITIONS_AND_NORMAL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri, "ELECTRODE_TRIANGULATION input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri2, "ELECTRODE_TRIANGULATION2 input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil, "COIL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil2, "COIL2 input field");*/
 
 

  AlgorithmOutput output;
  //output[OUTPUTMESH] = out1;

  return output;
}

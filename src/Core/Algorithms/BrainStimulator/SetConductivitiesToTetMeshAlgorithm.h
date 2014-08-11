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

///
///@file SetConductivitiesToTetMesh.h
///@brief This module assigns electrical conductivities (isotropic) to various mesh types and is part of BrainStimlator package.
///
///@author
/// Spencer Frisby
/// Moritz Dannhauer
///@details
/// The module maps the numbers in round brackets found in the input field and converts them to the values provided in GUI elements right behind (doublespinboxes).
///


#ifndef ALGORITHMS_MATH_SETCONDUCTIVITIESTOTETMESHALGORITHM_H
#define ALGORITHMS_MATH_SETCONDUCTIVITIESTOTETMESHALGORITHM_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Math/AlgorithmFwd.h>
#include <Core/Algorithms/BrainStimulator/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {
  
  class SCISHARE SetConductivitiesToTetMeshAlgorithm : public AlgorithmBase
  {
    public:
      SetConductivitiesToTetMeshAlgorithm();
      FieldHandle run(FieldHandle fh) const;
      static AlgorithmParameterName Skin();
      static AlgorithmParameterName SoftBone();
      static AlgorithmParameterName HardBone();
      static AlgorithmParameterName CSF();
      static AlgorithmParameterName GM();
      static AlgorithmParameterName WM();
      static AlgorithmParameterName Electrode();
      static AlgorithmParameterName InternalAir();
    
      AlgorithmOutput run_generic(const AlgorithmInput& input) const;
      static AlgorithmInputName MESH;
      static AlgorithmOutputName OUTPUTMESH;
      
      std::vector<int> ElemLabelLookup; /// this module checks for those label data stored on the elements
      
  };

}}}}

#endif

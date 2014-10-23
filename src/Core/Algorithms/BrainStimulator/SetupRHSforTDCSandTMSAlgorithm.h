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

///@file SetupRHSforTDCSandTMSAlgorithm
///@brief 
/// This module sets up tDCS by providing the right hand side vector (parameterized by module GUI) and inputs for the modules: AddKnownsToLinearSystem, BuildTDCSMatrix.
///
///@author
/// Moritz Dannhauer, Spencer Frisby
///
///@details
/// .
/// 

#ifndef ALGORITHMS_MATH_SETUPRHSFORTDCSANDTMSALGORITHM_H
#define ALGORITHMS_MATH_SETUPRHSFORTDCSANDTMSALGORITHM_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/BrainStimulator/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {

  ALGORITHM_PARAMETER_DECL(ElectrodeTableValues);
  ALGORITHM_PARAMETER_DECL(ELECTRODE_VALUES);

  class SCISHARE SetupRHSforTDCSandTMSAlgorithm : public AlgorithmBase
  {
  public:
    SetupRHSforTDCSandTMSAlgorithm();
    AlgorithmOutput run_generic(const AlgorithmInput& input) const;

    boost::tuple<Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle> run(FieldHandle mesh, const std::vector<Variable>& elcs, int num_of_elc, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, SCIRun::Core::Datatypes::DenseMatrixHandle elc_sponge_location) const;
    
    static AlgorithmParameterName refnode();
    static AlgorithmParameterName number_of_electrodes();
    
    static AlgorithmInputName MESH;
    static AlgorithmInputName SCALP_TRI_SURF_MESH;
    static AlgorithmInputName ELECTRODE_TRI_SURF_MESH;
    static AlgorithmInputName ELECTRODE_SPONGE_LOCATION_AVR;    
    static AlgorithmOutputName ELECTRODE_ELEMENT;
    static AlgorithmOutputName ELECTRODE_ELEMENT_TYPE;
    static AlgorithmOutputName ELECTRODE_ELEMENT_DEFINITION;
    static AlgorithmOutputName ELECTRODE_CONTACT_IMPEDANCE;
    static AlgorithmOutputName LHS_KNOWNS;
    static AlgorithmOutputName RHS;
    
    static Core::Algorithms::AlgorithmParameterName ElecrodeParameterName(int i);
  
  private:  
    const double identical_node_location_differce = 1e-10; /// should be a bit bigger than machine precision 
    SCIRun::Core::Datatypes::DenseMatrixHandle create_rhs(FieldHandle mesh, const std::vector<Variable>& elcs, int num_of_elc) const;
    boost::tuple<Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle> create_lhs(FieldHandle mesh, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, SCIRun::Core::Datatypes::DenseMatrixHandle elc_sponge_location) const;
  };

}}}}

#endif

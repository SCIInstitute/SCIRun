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


///@file SetupTDCSAlgorithm
///@brief
/// This module sets up tDCS by providing the right hand side vector (parameterized by module GUI) and inputs for the modules: AddKnownsToLinearSystem, BuildTDCSMatrix.
///
///@author
/// Moritz Dannhauer, Spencer Frisby
///
///@details
/// .
///

#ifndef ALGORITHMS_MATH_SetupTDCSAlgorithm_H
#define ALGORITHMS_MATH_SetupTDCSAlgorithm_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/BrainStimulator/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {
  ALGORITHM_PARAMETER_DECL(ImpedanceTableValues);
  ALGORITHM_PARAMETER_DECL(ElectrodeTableValues);
  ALGORITHM_PARAMETER_DECL(SurfaceAreaValues);
  ALGORITHM_PARAMETER_DECL(ELECTRODE_VALUES);
  ALGORITHM_PARAMETER_DECL(IMPEDANCE_VALUES);
  ALGORITHM_PARAMETER_DECL(refnode);
  ALGORITHM_PARAMETER_DECL(normal_dot_product_bound);
  ALGORITHM_PARAMETER_DECL(pointdistancebound);
  ALGORITHM_PARAMETER_DECL(number_of_electrodes);
  ALGORITHM_PARAMETER_DECL(GetContactSurface);

  class SCISHARE SetupTDCSAlgorithm : public AlgorithmBase
  {
  public:
    SetupTDCSAlgorithm();
    virtual AlgorithmOutput run(const AlgorithmInput& input) const override;

    boost::tuple<Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle,
    Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, FieldHandle, Datatypes::DenseMatrixHandle, std::vector<double>> run(FieldHandle mesh, const std::vector<Variable>& elcs, const std::vector<Variable>& impelc,int num_of_elc, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, SCIRun::Core::Datatypes::DenseMatrixHandle elc_sponge_location) const;

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
    static AlgorithmOutputName SELECTMATRIXINDECES;
    static AlgorithmOutputName ELECTRODE_SPONGE_SURF;

    static Core::Algorithms::AlgorithmParameterName ElectrodeParameterName(int i);
    static Core::Algorithms::AlgorithmParameterName ElectrodeImpedanceParameterName(int i);
    static const int max_number_of_electrodes;
  private:
    static const int special_label;
    static const double electode_current_summation_bound;
    SCIRun::Core::Datatypes::DenseMatrixHandle create_rhs(FieldHandle mesh, FieldHandle elc_tri_surf, const std::vector<Variable>& elcs, int num_of_elc) const;
    boost::tuple<Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, Datatypes::DenseMatrixHandle, FieldHandle, std::vector<double>> create_lhs(FieldHandle mesh, const std::vector<Variable>& impelc, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, SCIRun::Core::Datatypes::DenseMatrixHandle elc_sponge_location) const;
  };

}}}}

#endif

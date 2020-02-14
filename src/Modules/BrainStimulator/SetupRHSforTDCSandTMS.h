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


///@file SetupRHSforTDCSandTMS
///@brief
/// This module sets up TDCS problem by providing the right hand side vector (controlled by GUI) and inputs for the modules: AddKnownsToLinearSystem, BuildTDCSMatrix.
///
///@author
/// Moritz Dannhauer, Spencer Frisby
///
///@details
/// .The module gets 4 inputs the tetrahedral mesh (first), triangle surfaces (as part of the first input) for the scalp (second) and electrodes sponges (third) as well as the a matrix (output of ElectrodeCoilSetup)
///  that contains the center location and thickness of the electrode sponge. The first output contains the knowns (reference zeros node) of the linear system (third input of AddKnownsToLinearSystem).
///  The zero potential node is set to be the first mesh node by default, which can be chosen arbitrarily in the GUI by the user. The second until fifth output prepares the inputs for the BuildTDCSMatrix module that
///  combines the FEM stiffness matrix with the boundary conditions of the complete electrode model. The electrical impedances can be provided in the GUI for each electrode individually.
///  The module needs to identify the surface of the sponge which makes up the major part of the algorithmic module implementation.
///  The sixth output creates the right hand side (RHS) vector that can be provided by the GUI. The electrical charges (ingoing and outgoing current intensities) need to sum up zero.
///  If you execute the module once the module will graphically only present the number of electrodes defined in the input data.
///  The complete electrode model solves the nodal and electrode potentials in an agumented linear system the potential solution (output of SolveLinearSystem) needs to adjuested.
///  Therefore, a seventh output is provided that contains the relevant indeces to be used for the module 'SelectSubMatrix'.
///  Since the identification of the contact surface between electrode sponge and the electrode itself (the sponge surface top is assumed to be that) is a difficult geometrical process the eighth
///  output provides a triangle mesh that can be used for visual inspection (->ShowField->ViewScene).

#ifndef MODULES_BRAINSTIMULATOR_SetupRHSforTDCSandTMS_H
#define MODULES_BRAINSTIMULATOR_SetupRHSforTDCSandTMS_H

#include <Dataflow/Network/Module.h>
#include <Modules/BrainStimulator/share.h>
//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
namespace SCIRun {
  namespace Modules {
    namespace BrainStimulator {

class SCISHARE SetupTDCS : public SCIRun::Dataflow::Networks::Module,
  public Has4InputPorts<FieldPortTag, FieldPortTag, FieldPortTag, MatrixPortTag>,
  public Has8OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, FieldPortTag>
{
  public:
    SetupTDCS();

    virtual void execute() override;
    virtual void setStateDefaults() override;

    INPUT_PORT(0, MESH, Field);
    INPUT_PORT(1, SCALP_TRI_SURF_MESH, Field);
    INPUT_PORT(2, ELECTRODE_TRI_SURF_MESH, Field);
    INPUT_PORT(3, ELECTRODE_SPONGE_LOCATION_AVR, Matrix);

    OUTPUT_PORT(0, LHS_KNOWNS, Matrix);
    OUTPUT_PORT(1, ELECTRODE_ELEMENT, Matrix);
    OUTPUT_PORT(2, ELECTRODE_ELEMENT_TYPE, Matrix);
    OUTPUT_PORT(3, ELECTRODE_ELEMENT_DEFINITION, Matrix);
    OUTPUT_PORT(4, ELECTRODE_CONTACT_IMPEDANCE, Matrix);
    OUTPUT_PORT(5, RHS, Matrix);
    OUTPUT_PORT(6, SELECTMATRIXINDECES, Matrix);
    OUTPUT_PORT(7, ELECTRODE_SPONGE_SURF, Field);

    NEW_BRAIN_STIMULATOR_MODULE

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
};

}}}

#endif

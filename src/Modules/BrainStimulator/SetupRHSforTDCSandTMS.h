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

///@file SetupRHSforTDCSandTMS
///@brief 
/// This module sets up TDCS by providing the right hand side vector (controlled by GUI) and inputs for the modules: AddKnownsToLinearSystem, BuildTDCSMatrix. 
///
///@author
/// Moritz Dannhauer, Spencer Frisby
///
///@details
/// .
/// 

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

class SCISHARE SetupRHSforTDCSandTMSModule : public SCIRun::Dataflow::Networks::Module,
  public Has4InputPorts<FieldPortTag, FieldPortTag, FieldPortTag, MatrixPortTag>,
  public Has6OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>
{
  public:
    SetupRHSforTDCSandTMSModule();

    virtual void execute();
    virtual void setStateDefaults();

    INPUT_PORT(0, MESH, LegacyField);
    INPUT_PORT(1, SCALP_TRI_SURF_MESH, LegacyField);
    INPUT_PORT(2, ELECTRODE_TRI_SURF_MESH, LegacyField);
    INPUT_PORT(3, ELECTRODE_SPONGE_LOCATION_AVR, Matrix);
    
    OUTPUT_PORT(0, LHS_KNOWNS, Matrix);
    OUTPUT_PORT(1, ELECTRODE_ELEMENT, Matrix);
    OUTPUT_PORT(2, ELECTRODE_ELEMENT_TYPE, Matrix);
    OUTPUT_PORT(3, ELECTRODE_ELEMENT_DEFINITION, Matrix);
    OUTPUT_PORT(4, ELECTRODE_CONTACT_IMPEDANCE, Matrix);
    OUTPUT_PORT(5, RHS, Matrix);
};

}}}

#endif

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

///@file SimulateForwardMagneticField
///@brief 
/// This module sets up TDCS by providing the right hand side vector (controlled by GUI) and inputs for the modules: AddKnownsToLinearSystem, BuildTDCSMatrix. 
///
///@author
/// ported from SCIRun4 by Moritz Dannhauer
///
///@details
/// .
/// 

#ifndef MODULES_BRAINSTIMULATOR_SimulateForwardMagneticField_H
#define MODULES_BRAINSTIMULATOR_SimulateForwardMagneticField_H

#include <Dataflow/Network/Module.h>
#include <Modules/BrainStimulator/share.h>
//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
namespace SCIRun {
  namespace Modules {
    namespace BrainStimulator {

class SCISHARE SimulateForwardMagneticFieldModule : public SCIRun::Dataflow::Networks::Module,
  public Has4InputPorts<FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag>,
  public Has2OutputPorts<FieldPortTag, FieldPortTag>
{
  public:
    SimulateForwardMagneticFieldModule();

    virtual void execute();
    virtual void setStateDefaults();

    INPUT_PORT(0, ElectricField, LegacyField);
    INPUT_PORT(1, ConductivityTensor, LegacyField);
    INPUT_PORT(2, DipoleSources, LegacyField);
    INPUT_PORT(3, DetectorLocations, LegacyField);
    
    OUTPUT_PORT(0, MagneticField, LegacyField);
    OUTPUT_PORT(1, MagneticFieldMagnitudes, LegacyField);

};

}}}

#endif

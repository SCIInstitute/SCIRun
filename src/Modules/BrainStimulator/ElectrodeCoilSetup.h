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

#ifndef MODULES_BRAINSTIMULATOR_ELECTRODECOILSETUP_H
#define MODULES_BRAINSTIMULATOR_ELECTRODECOILSETUP_H

#include <Dataflow/Network/Module.h>
#include <Modules/BrainStimulator/share.h>

//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
using namespace SCIRun::Core::Datatypes;

namespace SCIRun {
  namespace Modules {
    namespace BrainStimulator {

class SCISHARE ElectrodeCoilSetupModule : public SCIRun::Dataflow::Networks::Module,
  public Has3InputPorts<FieldPortTag, MatrixPortTag, DynamicPortTag<FieldPortTag>>,
  public Has3OutputPorts<MatrixPortTag, FieldPortTag, FieldPortTag>
{
  public:
    ElectrodeCoilSetupModule();

    virtual void execute();
    virtual void setStateDefaults();
    
    virtual bool hasDynamicPorts() const override { return true; }
    
    INPUT_PORT(0, SCALP_SURF, LegacyField);
    INPUT_PORT(1, LOCATIONS, Matrix);
    INPUT_PORT_DYNAMIC(2, ELECTRODECOILPROTOTYPES, LegacyField);    
    OUTPUT_PORT(0, ELECTRODE_SPONGE_LOCATION_AVR, Matrix);
    OUTPUT_PORT(1, ELECTRODES_FIELD, LegacyField);
    OUTPUT_PORT(2, COILS_FIELD, LegacyField);
    
};

}}}

#endif

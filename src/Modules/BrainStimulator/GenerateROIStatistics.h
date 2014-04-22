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

#ifndef MODULES_BRAINSTIMULATOR_GenerateROIStatistics_H
#define MODULES_BRAINSTIMULATOR_GenerateROIStatistics_H

#include <Dataflow/Network/Module.h>
#include <Modules/BrainStimulator/share.h>
//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
namespace SCIRun {
  namespace Modules {
    namespace BrainStimulator {

class SCISHARE GenerateROIStatisticsModule : public SCIRun::Dataflow::Networks::Module,
  public Has5InputPorts<FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag>,
  public Has2OutputPorts<FieldPortTag, FieldPortTag>
{
  public:
    GenerateROIStatisticsModule();

    virtual void execute();
    virtual void setStateDefaults();

    INPUT_PORT(0, ELECTRODE_COIL_POSITIONS_AND_NORMAL, LegacyField);
    INPUT_PORT(1, ELECTRODE_TRIANGULATION, LegacyField);
    INPUT_PORT(2, ELECTRODE_TRIANGULATION2, LegacyField);
    INPUT_PORT(3, COIL, LegacyField);
    INPUT_PORT(4, COIL2, LegacyField);
    
    OUTPUT_PORT(0, ELECTRODES_FIELD, LegacyField);
    OUTPUT_PORT(1, COILS_FIELD, LegacyField);
};

}}}

#endif

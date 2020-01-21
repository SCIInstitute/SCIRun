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


///@file ModelTMSCoil
///@brief
/// Generates
/// a) spiral wire segments of single/figure-of-8 TMS coil with repsective current values
/// b) Generates wire segments of single/figure-of-8 TMS coil (one ring wire for each wing) with repsective current values
/// c) Point cloud with vector data (= magnetic dipoles) describing magnetic field output of single/figure-of-8 TMS coil
///
///@author
/// Implementation: Petar Petrov for SCIRun 4.7
/// Converted to SCIRun5 by Moritz Dannhauer

#ifndef MODULES_BRAINSTIMULATOR_ModelTMSCoil_H
#define MODULES_BRAINSTIMULATOR_ModelTMSCoil_H

#include <Dataflow/Network/Module.h>
#include <Modules/BrainStimulator/share.h>

namespace SCIRun {
  namespace Modules {
    namespace BrainStimulator {

class SCISHARE ModelTMSCoil : public SCIRun::Dataflow::Networks::Module,
  public HasNoInputPorts,
  public Has1OutputPort<FieldPortTag>
{
  public:
    ModelTMSCoil();

    virtual void execute() override;
    virtual void setStateDefaults() override;

    OUTPUT_PORT(0, Mesh, Field);

    NEW_BRAIN_STIMULATOR_MODULE

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
};

}}}

#endif

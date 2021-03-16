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


///
///@file SetConductivitiesToTetMesh.h
///@brief This module assigns electrical conductivities (isotropic) to various mesh types and is part of BrainStimlator package.
///
///@author
/// Spencer Frisby
/// Moritz Dannhauer
///@details
/// The SetConductivitiesToTetMesh module uses a single field input and assigns conductivities to output field that are provided by the GUI elements to work in the BrainStimulator environment (uses SI units).
/// The module looks for the numbers in round brackets (range visible in GUI) of the input field and sends them to the algorithm to be converted to the conductivities (specified in GUI).
///

#ifndef MODULES_BRAINSTIMULATOR_SetConductivitiesToTetMesh_H
#define MODULES_BRAINSTIMULATOR_SetConductivitiesToTetMesh_H

#include <Dataflow/Network/Module.h>
#include <Modules/BrainStimulator/share.h>

namespace SCIRun {
namespace Modules {
namespace BrainStimulator {

class SCISHARE SetConductivitiesToMesh : public SCIRun::Dataflow::Networks::Module,
  public Has1InputPort<FieldPortTag>,
  public Has1OutputPort<FieldPortTag>
{
  public:
    SetConductivitiesToMesh();

    void execute() override;
    void setStateDefaults() override;

    INPUT_PORT(0, InputField, Field);
    OUTPUT_PORT(0, OutputField, Field);

    NEW_BRAIN_STIMULATOR_MODULE

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
};

}}}

#endif

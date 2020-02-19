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


///@file ElectrodeCoilSetup
///@brief With this module the user is able to position tDCS electrodes and TMS coils to setup a simulation scenario.
///
///
///@author
/// Moritz Dannhauer
///
///@details
/// The module gets the triangulated scalp surface as the first input and a matrix of possible locations as the second. The third and any following input fields (dynamic input ports)
/// are considered to be prototypes of electrodes or TMS coils that need to be positioned. After the first execution using a minimum of 3 (valid) inputs the positions appear as table rows
/// in the GUI. The GUI allows to assign a prototype (first table column), normal direction etc. as well as the type of stimulation (second table column). Based on the choices made in the
/// first (prototype, "Input #") and second column (stimulation type, "Type") information are gathered from the rest of the row. An defined tDCS electrode or TMS coil will be moved using
/// its location ("X","Y","Z"), its normal ("NX","NY","NZ") and can be rotated along its normal by the angle table cell defined in degrees. Throughout this table, multiple electrodes and TMS coils
/// can be located at their final destinations. For a tDCS electrode specification, the placed and oriented prototype (contains electrode dimension) is used to cut the scalp that is covered by the
/// electrode. This piece of scalp and its normals are used to generate the electrode sponge shape by using the thickness table parameter. The module contains three outputs: 1) the averaged scalp
/// sponge locations (+thickness, as a DenseMatrix) which are designed to be the last input of the SetupTDCS module, 2) a collection of all valid tDCS electrodes and 3) a collection of all
/// valid TMS coil inputs that are represented by rotated points and dipole normals approximating the magnetic field.

#ifndef MODULES_BRAINSTIMULATOR_ELECTRODECOILSETUP_H
#define MODULES_BRAINSTIMULATOR_ELECTRODECOILSETUP_H

#include <Dataflow/Network/Module.h>
#include <Modules/BrainStimulator/share.h>

//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////

namespace SCIRun {
  namespace Modules {
    namespace BrainStimulator {

class SCISHARE ElectrodeCoilSetup : public SCIRun::Dataflow::Networks::Module,
  public Has3InputPorts<FieldPortTag, MatrixPortTag, DynamicPortTag<FieldPortTag>>,
  public Has4OutputPorts<MatrixPortTag, FieldPortTag, FieldPortTag, FieldPortTag>
{
  public:
    ElectrodeCoilSetup();

    virtual void execute() override;
    virtual void setStateDefaults() override;

    virtual bool hasDynamicPorts() const override { return true; }

    INPUT_PORT(0, SCALP_SURF, Field);
    INPUT_PORT(1, LOCATIONS, Matrix);
    INPUT_PORT_DYNAMIC(2, ELECTRODECOILPROTOTYPES, Field);
    OUTPUT_PORT(0, ELECTRODE_SPONGE_LOCATION_AVR, Matrix);
    OUTPUT_PORT(1, MOVED_ELECTRODES_FIELD, Field);
    OUTPUT_PORT(2, FINAL_ELECTRODES_FIELD, Field);
    OUTPUT_PORT(3, COILS_FIELD, Field);

    NEW_BRAIN_STIMULATOR_MODULE

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
};

}}}

#endif

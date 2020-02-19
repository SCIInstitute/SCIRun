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


///@file
/// GenerateROIStatisticsModule.cc
///@brief
/// This module allows to perform statistical analysis of simulation results and is part of the BrainStimulator package.
///
///@author
/// Moritz Dannhauer
///
///@details
/// The GUI presents two tables that are functionally different.
/// First, the upper table is for output only and presents any statistical results where each ROI is one row in the table. In its default state the module will analyze
/// and present results for all ROIs that are correctly provided as inputs. Second, the lower table is meant to be able to define an individual ROI based on several critia (location,
/// material label number and a radius that defines a spherical ROI around it). Once valid parameter are provided (in lower table, e.g. radius > 0) the results will be presented in the upper table after
/// module execution.
/// Two inputs have to be provided to be able to run the module it. The first input is a field that contains the simulation data defined on a mesh (tested with tetrahedra) elements.
/// The third input represents the atlas that is identical in terms of geometry but contains label numbers (>0) to define anatomical regions. The anatomical regions can be used to
/// compute descriptive statistics (avr., stddev., min, max). All other inputs are optional. You can provide anatomical names as a string (separated by ";", e.g. region1;region2) that must be ordered
/// in a way that they match up with the ordered label numbers. An addtional (latvol) input can be provided to define a coordinate space to query anatomical statistics of common
/// and self-defined neurological spaces such as Talairach or MNI space. The module is therefore generic since any anatomical data are provided by inputs only.
/// Using the second and seventh input, GUI labels describing the analysis (the physical unit the first input is in; the name of the cordinate space system that is used) can be provided.
/// The user is responsible for providing appropriate inputs that are registered etc.
/// You can use the tutorial walkthrough and example networks to explore its functionality.

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

class SCISHARE GenerateROIStatistics : public SCIRun::Dataflow::Networks::Module,
  public Has6InputPorts<FieldPortTag, StringPortTag, FieldPortTag, StringPortTag, FieldPortTag, StringPortTag>,
  public Has1OutputPort<MatrixPortTag>
{
  public:
    GenerateROIStatistics();

    virtual void execute() override;
    virtual void setStateDefaults() override;

    INPUT_PORT(0, MeshDataOnElements, Field);
    INPUT_PORT(1, PhysicalUnit, String);
    INPUT_PORT(2, AtlasMesh, Field);
    INPUT_PORT(3, AtlasMeshLabels, String);
    INPUT_PORT(4, CoordinateSpace, Field);
    INPUT_PORT(5, CoordinateSpaceLabel, String);

    OUTPUT_PORT(0, StatisticalResults, Matrix);

    NEW_BRAIN_STIMULATOR_MODULE

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
};

}}}

#endif

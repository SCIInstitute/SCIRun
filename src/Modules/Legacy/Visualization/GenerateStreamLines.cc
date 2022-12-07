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


///  File   : GenerateStreamLines.cc
///  Author : Allen R. Sanderson
///  Date   : July 2006

#include <Modules/Legacy/Visualization/GenerateStreamLines.h>
#include <Core/Algorithms/Legacy/Fields/StreamLines/GenerateStreamLines.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun;

MODULE_INFO_DEF(GenerateStreamLines, Visualization, SCIRun)

GenerateStreamLines::GenerateStreamLines() : Module(staticInfo_)
{
  INITIALIZE_PORT(Vector_Field);
  INITIALIZE_PORT(Seed_Points);
  INITIALIZE_PORT(Streamlines);
}

void GenerateStreamLines::setStateDefaults()
{
  auto state = get_state();
  setStateStringFromAlgoOption(Parameters::StreamlineDirection);
  setStateStringFromAlgoOption(Parameters::StreamlineValue);
  setStateIntFromAlgo(Parameters::StreamlineMaxSteps);
  setStateDoubleFromAlgo(Parameters::StreamlineStepSize);
  setStateDoubleFromAlgo(Parameters::StreamlineTolerance);
  setStateStringFromAlgoOption(Parameters::StreamlineMethod);
  setStateBoolFromAlgo(Parameters::AutoParameters);
  setStateBoolFromAlgo(Parameters::RemoveColinearPoints);
  setStateBoolFromAlgo(Parameters::UseMultithreading);
}

void GenerateStreamLines::execute()
{
  auto input = getRequiredInput(Vector_Field);
  auto seeds = getRequiredInput(Seed_Points);

  if (needToExecute())
  {
    setAlgoDoubleFromState(Parameters::StreamlineStepSize);
    setAlgoDoubleFromState(Parameters::StreamlineTolerance);
    setAlgoOptionFromState(Parameters::StreamlineDirection);
    setAlgoOptionFromState(Parameters::StreamlineValue);
    setAlgoIntFromState(Parameters::StreamlineMaxSteps);
    setAlgoBoolFromState(Parameters::RemoveColinearPoints);
    setAlgoBoolFromState(Parameters::AutoParameters);
    setAlgoOptionFromState(Parameters::StreamlineMethod);
    setAlgoBoolFromState(Parameters::UseMultithreading);

    auto output = algo().run(withInputData((Vector_Field, input)(Seed_Points, seeds)));

    #ifdef NEED_ALGO_OUTPUT
    gui_tolerance_.set(algo_.get_scalar("tolerance"));
    gui_step_size_.set(algo_.get_scalar("step_size"));
    #endif

    sendOutputFromAlgorithm(Streamlines, output);
  }
}

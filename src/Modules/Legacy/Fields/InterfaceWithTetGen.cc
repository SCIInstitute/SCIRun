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


#include <Modules/Legacy/Fields/InterfaceWithTetGen.h>
#ifdef WITH_TETGEN
#include <Modules/Legacy/Fields/InterfaceWithTetGenImpl.h>
#endif
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Fields, PiecewiseFlag);
ALGORITHM_PARAMETER_DEF(Fields, AssignFlag);
ALGORITHM_PARAMETER_DEF(Fields, SetNonzeroAttributeFlag);
ALGORITHM_PARAMETER_DEF(Fields, SuppressSplitFlag);
ALGORITHM_PARAMETER_DEF(Fields, SetSplitFlag);
ALGORITHM_PARAMETER_DEF(Fields, QualityFlag);
ALGORITHM_PARAMETER_DEF(Fields, SetRatioFlag);
ALGORITHM_PARAMETER_DEF(Fields, VolConstraintFlag);
ALGORITHM_PARAMETER_DEF(Fields, SetMaxVolConstraintFlag);
ALGORITHM_PARAMETER_DEF(Fields, MinRadius);
ALGORITHM_PARAMETER_DEF(Fields, MaxVolConstraint);
ALGORITHM_PARAMETER_DEF(Fields, DetectIntersectionsFlag);
ALGORITHM_PARAMETER_DEF(Fields, MoreSwitches);

MODULE_INFO_DEF(InterfaceWithTetGen, NewField, SCIRun)

InterfaceWithTetGen::InterfaceWithTetGen()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(Main);
  INITIALIZE_PORT(Points);
  INITIALIZE_PORT(Region_Attribs);
  INITIALIZE_PORT(Regions);
  INITIALIZE_PORT(TetVol);
}

void InterfaceWithTetGen::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::PiecewiseFlag, true);
  state->setValue(Parameters::AssignFlag, true);
  state->setValue(Parameters::SetNonzeroAttributeFlag, false);
  state->setValue(Parameters::SuppressSplitFlag, true);
  state->setValue(Parameters::SetSplitFlag, false);
  state->setValue(Parameters::QualityFlag, true);
  state->setValue(Parameters::SetRatioFlag, false);
  state->setValue(Parameters::VolConstraintFlag, false);
  state->setValue(Parameters::SetMaxVolConstraintFlag, false);
  state->setValue(Parameters::MinRadius, 2.0);
  state->setValue(Parameters::MaxVolConstraint, 0.1);
  state->setValue(Parameters::DetectIntersectionsFlag, false);
  state->setValue(Parameters::MoreSwitches, std::string());
}

void InterfaceWithTetGen::execute()
{
#ifdef WITH_TETGEN
  auto first_surface = getRequiredInput(Main);
  auto rest = getOptionalDynamicInputs(Regions);
  std::deque<FieldHandle> surfaces(rest.begin(), rest.end());
  surfaces.push_front(first_surface);

  auto points = getOptionalInput(Points);
  auto region_attribs = getOptionalInput(Region_Attribs);

  if (needToExecute())
  {
    auto state = get_state();
    InterfaceWithTetGenInput inputs;

    inputs.piecewiseFlag_ = state->getValue(Parameters::PiecewiseFlag).toBool();
    inputs.assignFlag_ = state->getValue(Parameters::AssignFlag).toBool();
    inputs.setNonzeroAttributeFlag_ = state->getValue(Parameters::SetNonzeroAttributeFlag).toBool();
    inputs.suppressSplitFlag_ = state->getValue(Parameters::SuppressSplitFlag).toBool();
    inputs.setSplitFlag_ = state->getValue(Parameters::SetSplitFlag).toBool();
    inputs.qualityFlag_ = state->getValue(Parameters::QualityFlag).toBool();
    inputs.setRatioFlag_ = state->getValue(Parameters::SetRatioFlag).toBool();
    inputs.volConstraintFlag_ = state->getValue(Parameters::VolConstraintFlag).toBool();
    inputs.setMaxVolConstraintFlag_ = state->getValue(Parameters::SetMaxVolConstraintFlag).toBool();
    inputs.minRadius_ = state->getValue(Parameters::MinRadius).toDouble();
    inputs.maxVolConstraint_ = state->getValue(Parameters::MaxVolConstraint).toDouble();
    inputs.detectIntersectionsFlag_ = state->getValue(Parameters::DetectIntersectionsFlag).toBool();
    inputs.moreSwitches_ = state->getValue(Parameters::MoreSwitches).toString();

    InterfaceWithTetGenImpl impl(this, inputs);
    auto result = impl.runImpl(surfaces, points.get_value_or(nullptr), region_attribs.get_value_or(nullptr));
    sendOutput(TetVol, result);
  }
#else
  error("This module needs the build flag WITH_TETGEN enabled in order to work.");
#endif
}

/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Math/CreateGeometricTransform.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Module.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

///@file  CreateGeometricTransform.cc
///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  March 1999

const ModuleLookupInfo CreateGeometricTransform::staticInfo_("CreateGeometricTransform", "Math", "SCIRun");
ALGORITHM_PARAMETER_DEF(Math, TransformType);
ALGORITHM_PARAMETER_DEF(Math, MultiplyRadioButton);
ALGORITHM_PARAMETER_DEF(Math, TranslateVectorX);
ALGORITHM_PARAMETER_DEF(Math, TranslateVectorY);
ALGORITHM_PARAMETER_DEF(Math, TranslateVectorZ);
ALGORITHM_PARAMETER_DEF(Math, ScalePointX);
ALGORITHM_PARAMETER_DEF(Math, ScalePointY);
ALGORITHM_PARAMETER_DEF(Math, ScalePointZ);
ALGORITHM_PARAMETER_DEF(Math, LogScale);
ALGORITHM_PARAMETER_DEF(Math, LogScaleX);
ALGORITHM_PARAMETER_DEF(Math, LogScaleY);
ALGORITHM_PARAMETER_DEF(Math, LogScaleZ);
ALGORITHM_PARAMETER_DEF(Math, RotatePointX);
ALGORITHM_PARAMETER_DEF(Math, RotatePointY);
ALGORITHM_PARAMETER_DEF(Math, RotatePointZ);
ALGORITHM_PARAMETER_DEF(Math, RotateAxisX);
ALGORITHM_PARAMETER_DEF(Math, RotateAxisY);
ALGORITHM_PARAMETER_DEF(Math, RotateAxisZ);
ALGORITHM_PARAMETER_DEF(Math, RotateTheta);
ALGORITHM_PARAMETER_DEF(Math, ShearVectorX);
ALGORITHM_PARAMETER_DEF(Math, ShearVectorY);
ALGORITHM_PARAMETER_DEF(Math, ShearVectorZ);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneA);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneB);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneC);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneD);
ALGORITHM_PARAMETER_DEF(Math, FieldMapX);
ALGORITHM_PARAMETER_DEF(Math, FieldMapY);
ALGORITHM_PARAMETER_DEF(Math, FieldMapZ);
ALGORITHM_PARAMETER_DEF(Math, UniformScale);
ALGORITHM_PARAMETER_DEF(Math, ResizeSeparably);
ALGORITHM_PARAMETER_DEF(Math, IgnoreChanges);

CreateGeometricTransform::CreateGeometricTransform() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(Widget);
}

void CreateGeometricTransform::setStateDefaults()
{
  auto state = get_state();
  using namespace Parameters;
  state->setValue(TransformType, 0);
  state->setValue(MultiplyRadioButton, 1);
  state->setValue(TranslateVectorX, 0.0);
  state->setValue(TranslateVectorY, 0.0);
  state->setValue(TranslateVectorZ, 0.0);
  state->setValue(ScalePointX, 0.0);
  state->setValue(ScalePointY, 0.0);
  state->setValue(ScalePointZ, 0.0);
  state->setValue(LogScale, 0.0);
  state->setValue(LogScaleX, 0.0);
  state->setValue(LogScaleY, 0.0);
  state->setValue(LogScaleZ, 0.0);
  state->setValue(RotatePointX, 0.0);
  state->setValue(RotatePointY, 0.0);
  state->setValue(RotatePointZ, 0.0);
  state->setValue(RotateAxisX, 0.0);
  state->setValue(RotateAxisY, 0.0);
  state->setValue(RotateAxisZ, 1.0);
  state->setValue(RotateTheta, 0.0);
  state->setValue(ShearVectorX, 0.0);
  state->setValue(ShearVectorY, 0.0);
  state->setValue(ShearVectorZ, 0.0);
  state->setValue(ShearPlaneA, 0.0);
  state->setValue(ShearPlaneB, 0.0);
  state->setValue(ShearPlaneC, 0.0);
  state->setValue(ShearPlaneD, 1.0);
  std::string x = "x+";
  std::string y = "y+";
  std::string z = "z+";
  state->setValue(FieldMapX, x);
  state->setValue(FieldMapY, y);
  state->setValue(FieldMapZ, z);
  state->setValue(UniformScale, 1.0);
  state->setValue(ResizeSeparably, true);
  state->setValue(IgnoreChanges, true);
}

void CreateGeometricTransform::execute()
{
  auto input = getRequiredInput(InputMatrix);
  
  if (needToExecute())
  {
    
    
    
  }
}

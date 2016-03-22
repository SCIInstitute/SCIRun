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
using namespace SCIRun::Core::Datatypes;

///@file  CreateGeometricTransform.cc
///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  March 1999

const ModuleLookupInfo CreateGeometricTransformModule::staticInfo_("CreateGeometricTransform", "Math", "SCIRun");

CreateGeometricTransformModule::CreateGeometricTransformModule() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(Widget);
}

void CreateGeometricTransformModule::setStateDefaults()
{
  auto state = get_state();
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
  state->setValue(RotateAxisZ, 0.0);
  state->setValue(RotateTheta, 0.0);
  state->setValue(ShearVectorX, 0.0);
  state->setValue(ShearVectorY, 0.0);
  state->setValue(ShearVectorZ, 0.0);
  state->setValue(ShearPlaneA, 0.0);
  state->setValue(ShearPlaneB, 0.0);
  state->setValue(ShearPlaneC, 0.0);
  state->setValue(ShearPlaneD, 1.0);
  state->setValue(FieldMapX, "x+");
  state->setValue(FieldMapY, "y+");
  state->setValue(FieldMapZ, "z+");
  state->setValue(UniformScale, 1.0);
  state->setValue(ResizeSeparably, true);
  state->setValue(IgnoreChanges, true);
}

void CreateGeometricTransformModule::execute()
{
  auto input = getRequiredInput(InputMatrix);
  
  if (needToExecute())
  {
    
    
    
  }
}

const AlgorithmParameterName CreateGeometricTransformModule::TransformType("TransformType");
const AlgorithmParameterName CreateGeometricTransformModule::MultiplyRadioButton("MultiplyRadioButton");
const AlgorithmParameterName CreateGeometricTransformModule::TranslateVectorX("TranslateVectorX");
const AlgorithmParameterName CreateGeometricTransformModule::TranslateVectorY("TranslateVectorY");
const AlgorithmParameterName CreateGeometricTransformModule::TranslateVectorZ("TranslateVectorZ");
const AlgorithmParameterName CreateGeometricTransformModule::ScalePointX("ScalePointX");
const AlgorithmParameterName CreateGeometricTransformModule::ScalePointY("ScalePointY");
const AlgorithmParameterName CreateGeometricTransformModule::ScalePointZ("ScalePointZ");
const AlgorithmParameterName CreateGeometricTransformModule::LogScale("LogScale");
const AlgorithmParameterName CreateGeometricTransformModule::LogScaleX("LogScaleX");
const AlgorithmParameterName CreateGeometricTransformModule::LogScaleY("LogScaleY");
const AlgorithmParameterName CreateGeometricTransformModule::LogScaleZ("LogScaleZ");
const AlgorithmParameterName CreateGeometricTransformModule::RotatePointX("RotatePointX");
const AlgorithmParameterName CreateGeometricTransformModule::RotatePointY("RotatePointY");
const AlgorithmParameterName CreateGeometricTransformModule::RotatePointZ("RotatePointZ");
const AlgorithmParameterName CreateGeometricTransformModule::RotateAxisX("RotateAxisX");
const AlgorithmParameterName CreateGeometricTransformModule::RotateAxisY("RotateAxisY");
const AlgorithmParameterName CreateGeometricTransformModule::RotateAxisZ("RotateAxisZ");
const AlgorithmParameterName CreateGeometricTransformModule::RotateTheta("RotateTheta");
const AlgorithmParameterName CreateGeometricTransformModule::ShearVectorX("ShearVectorX");
const AlgorithmParameterName CreateGeometricTransformModule::ShearVectorY("ShearVectorY");
const AlgorithmParameterName CreateGeometricTransformModule::ShearVectorZ("ShearVectorZ");
const AlgorithmParameterName CreateGeometricTransformModule::ShearPlaneA("ShearPlaneA");
const AlgorithmParameterName CreateGeometricTransformModule::ShearPlaneB("ShearPlaneB");
const AlgorithmParameterName CreateGeometricTransformModule::ShearPlaneC("ShearPlaneC");
const AlgorithmParameterName CreateGeometricTransformModule::ShearPlaneD("ShearPlaneD");
const AlgorithmParameterName CreateGeometricTransformModule::FieldMapX("FieldMapX");
const AlgorithmParameterName CreateGeometricTransformModule::FieldMapY("FieldMapY");
const AlgorithmParameterName CreateGeometricTransformModule::FieldMapZ("FieldMapZ");
const AlgorithmParameterName CreateGeometricTransformModule::UniformScale("UniformScale");
const AlgorithmParameterName CreateGeometricTransformModule::ResizeSeparably("ResizeSeparably");
const AlgorithmParameterName CreateGeometricTransformModule::IgnoreChanges("IgnoreChanges");


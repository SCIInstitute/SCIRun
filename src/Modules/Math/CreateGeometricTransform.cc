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

const ModuleLookupInfo CreateGeometricTransform::staticInfo_("CreateGeometricTransform", "Math", "SCIRun");

CreateGeometricTransform::CreateGeometricTransform() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(Widget);
}

void CreateGeometricTransform::setStateDefaults()
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

const AlgorithmParameterName CreateGeometricTransform::TransformType("TransformType");
const AlgorithmParameterName CreateGeometricTransform::MultiplyRadioButton("MultiplyRadioButton");
const AlgorithmParameterName CreateGeometricTransform::TranslateVectorX("TranslateVectorX");
const AlgorithmParameterName CreateGeometricTransform::TranslateVectorY("TranslateVectorY");
const AlgorithmParameterName CreateGeometricTransform::TranslateVectorZ("TranslateVectorZ");
const AlgorithmParameterName CreateGeometricTransform::ScalePointX("ScalePointX");
const AlgorithmParameterName CreateGeometricTransform::ScalePointY("ScalePointY");
const AlgorithmParameterName CreateGeometricTransform::ScalePointZ("ScalePointZ");
const AlgorithmParameterName CreateGeometricTransform::LogScale("LogScale");
const AlgorithmParameterName CreateGeometricTransform::LogScaleX("LogScaleX");
const AlgorithmParameterName CreateGeometricTransform::LogScaleY("LogScaleY");
const AlgorithmParameterName CreateGeometricTransform::LogScaleZ("LogScaleZ");
const AlgorithmParameterName CreateGeometricTransform::RotatePointX("RotatePointX");
const AlgorithmParameterName CreateGeometricTransform::RotatePointY("RotatePointY");
const AlgorithmParameterName CreateGeometricTransform::RotatePointZ("RotatePointZ");
const AlgorithmParameterName CreateGeometricTransform::RotateAxisX("RotateAxisX");
const AlgorithmParameterName CreateGeometricTransform::RotateAxisY("RotateAxisY");
const AlgorithmParameterName CreateGeometricTransform::RotateAxisZ("RotateAxisZ");
const AlgorithmParameterName CreateGeometricTransform::RotateTheta("RotateTheta");
const AlgorithmParameterName CreateGeometricTransform::ShearVectorX("ShearVectorX");
const AlgorithmParameterName CreateGeometricTransform::ShearVectorY("ShearVectorY");
const AlgorithmParameterName CreateGeometricTransform::ShearVectorZ("ShearVectorZ");
const AlgorithmParameterName CreateGeometricTransform::ShearPlaneA("ShearPlaneA");
const AlgorithmParameterName CreateGeometricTransform::ShearPlaneB("ShearPlaneB");
const AlgorithmParameterName CreateGeometricTransform::ShearPlaneC("ShearPlaneC");
const AlgorithmParameterName CreateGeometricTransform::ShearPlaneD("ShearPlaneD");
const AlgorithmParameterName CreateGeometricTransform::UniformScale("UniformScale");
const AlgorithmParameterName CreateGeometricTransform::ResizeSeparably("ResizeSeparably");
const AlgorithmParameterName CreateGeometricTransform::IgnoreChanges("IgnoreChanges");


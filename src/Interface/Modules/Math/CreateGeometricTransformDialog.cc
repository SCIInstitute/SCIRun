/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Interface/Modules/Math/CreateGeometricTransformDialog.h>
#include <Modules/Math/CreateGeometricTransform.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

CreateGeometricTransformDialog::CreateGeometricTransformDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: ModuleDialogGeneric(state, parent)
{
	setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();
  WidgetStyleMixin::tabStyle(transformTabWidget_);

  auto tabIndex = state_->getValue(CreateGeometricTransformModule::TransformType).toInt();
  if (tabIndex)
  {
    transformTabWidget_->setCurrentIndex(tabIndex);
  }

  addRadioButtonGroupManager({ preMultiplyRadioButton_, postMultiplyRadioButton_ }, CreateGeometricTransformModule::MultiplyRadioButton);
  addDoubleSpinBoxManager(translateVectorXDoubleSpinBox_, CreateGeometricTransformModule::TranslateVectorX);
  addDoubleSpinBoxManager(translateVectorYDoubleSpinBox_, CreateGeometricTransformModule::TranslateVectorY);
  addDoubleSpinBoxManager(translateVectorZDoubleSpinBox_, CreateGeometricTransformModule::TranslateVectorZ);
  addDoubleSpinBoxManager(scaleFixedPointXDoubleSpinBox_, CreateGeometricTransformModule::ScalePointX);
  addDoubleSpinBoxManager(scaleFixedPointYDoubleSpinBox_, CreateGeometricTransformModule::ScalePointY);
  addDoubleSpinBoxManager(scaleFixedPointZDoubleSpinBox_, CreateGeometricTransformModule::ScalePointZ);
  addDoubleSpinBoxManager(logScaleDoubleSpinBox_, CreateGeometricTransformModule::LogScale);
  addDoubleSpinBoxManager(logScaleXDoubleSpinBox_, CreateGeometricTransformModule::LogScaleX);
  addDoubleSpinBoxManager(logScaleYDoubleSpinBox_, CreateGeometricTransformModule::LogScaleY);
  addDoubleSpinBoxManager(logScaleZDoubleSpinBox_, CreateGeometricTransformModule::LogScaleZ);
  addDoubleSpinBoxManager(rotationFixedPointXDoubleSpinBox_, CreateGeometricTransformModule::RotatePointX);
  addDoubleSpinBoxManager(rotationFixedPointYDoubleSpinBox_, CreateGeometricTransformModule::RotatePointY);
  addDoubleSpinBoxManager(rotationFixedPointZDoubleSpinBox_, CreateGeometricTransformModule::RotatePointZ);
  addDoubleSpinBoxManager(rotateXAxisDoubleSpinBox_, CreateGeometricTransformModule::RotateAxisX);
  addDoubleSpinBoxManager(rotateYAxisDoubleSpinBox_, CreateGeometricTransformModule::RotateAxisY);
  addDoubleSpinBoxManager(rotateZAxisDoubleSpinBox_, CreateGeometricTransformModule::RotateAxisZ);
  addDoubleSpinBoxManager(rotateThetaDoubleSpinBox_, CreateGeometricTransformModule::RotateTheta);
  addDoubleSpinBoxManager(shearVectorXDoubleSpinBox_, CreateGeometricTransformModule::ShearVectorX);
  addDoubleSpinBoxManager(shearVectorYDoubleSpinBox_, CreateGeometricTransformModule::ShearVectorY);
  addDoubleSpinBoxManager(shearVectorZDoubleSpinBox_, CreateGeometricTransformModule::ShearVectorZ);
  addDoubleSpinBoxManager(shearFixedPlaneADoubleSpinBox_, CreateGeometricTransformModule::ShearPlaneA);
  addDoubleSpinBoxManager(shearFixedPlaneBDoubleSpinBox_, CreateGeometricTransformModule::ShearPlaneB);
  addDoubleSpinBoxManager(shearFixedPlaneCDoubleSpinBox_, CreateGeometricTransformModule::ShearPlaneC);
  addDoubleSpinBoxManager(shearFixedPlaneDDoubleSpinBox_, CreateGeometricTransformModule::ShearPlaneD);
  addDynamicLabelManager(fieldMapXLabel_, CreateGeometricTransformModule::FieldMapX);
  addDynamicLabelManager(fieldMapYLabel_, CreateGeometricTransformModule::FieldMapY);
  addDynamicLabelManager(fieldMapZLabel_, CreateGeometricTransformModule::FieldMapZ);
  addDoubleSpinBoxManager(uniformScaleDoubleSpinBox_, CreateGeometricTransformModule::UniformScale);
  addCheckBoxManager(resizeSeparablyCheckBox_, CreateGeometricTransformModule::ResizeSeparably);
  addCheckBoxManager(ignoreChangesCheckBox_, CreateGeometricTransformModule::IgnoreChanges);
}

void CreateGeometricTransformDialog::pullSpecial()
{
}

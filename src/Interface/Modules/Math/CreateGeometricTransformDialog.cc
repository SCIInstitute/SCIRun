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


#include <Interface/Modules/Math/CreateGeometricTransformDialog.h>
#include <Modules/Math/CreateGeometricTransform.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;

CreateGeometricTransformDialog::CreateGeometricTransformDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: ModuleDialogGeneric(state, parent)
{
	setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();
  WidgetStyleMixin::tabStyle(transformTabWidget_);

  auto tabIndex = state_->getValue(Parameters::TransformType).toInt();
  if (tabIndex >= 0)
  {
    transformTabWidget_->setCurrentIndex(tabIndex);
  }
  else
  {
    resetValues();
    resetFieldMap();
  }

  addRadioButtonGroupManager({ preMultiplyRadioButton_, postMultiplyRadioButton_ }, Parameters::MultiplyRadioButton);
  addDoubleSpinBoxManager(translateVectorXDoubleSpinBox_, Parameters::TranslateVectorX);
  addDoubleSpinBoxManager(translateVectorYDoubleSpinBox_, Parameters::TranslateVectorY);
  addDoubleSpinBoxManager(translateVectorZDoubleSpinBox_, Parameters::TranslateVectorZ);
  addDoubleSpinBoxManager(scaleFixedPointXDoubleSpinBox_, Parameters::ScalePointX);
  addDoubleSpinBoxManager(scaleFixedPointYDoubleSpinBox_, Parameters::ScalePointY);
  addDoubleSpinBoxManager(scaleFixedPointZDoubleSpinBox_, Parameters::ScalePointZ);
  addDoubleSpinBoxManager(logScaleDoubleSpinBox_, Parameters::LogScale);
  addDoubleSpinBoxManager(logScaleXDoubleSpinBox_, Parameters::LogScaleX);
  addDoubleSpinBoxManager(logScaleYDoubleSpinBox_, Parameters::LogScaleY);
  addDoubleSpinBoxManager(logScaleZDoubleSpinBox_, Parameters::LogScaleZ);
  addDoubleSpinBoxManager(rotationFixedPointXDoubleSpinBox_, Parameters::RotatePointX);
  addDoubleSpinBoxManager(rotationFixedPointYDoubleSpinBox_, Parameters::RotatePointY);
  addDoubleSpinBoxManager(rotationFixedPointZDoubleSpinBox_, Parameters::RotatePointZ);
  addDoubleSpinBoxManager(rotateXAxisDoubleSpinBox_, Parameters::RotateAxisX);
  addDoubleSpinBoxManager(rotateYAxisDoubleSpinBox_, Parameters::RotateAxisY);
  addDoubleSpinBoxManager(rotateZAxisDoubleSpinBox_, Parameters::RotateAxisZ);
  addDoubleSpinBoxManager(rotateThetaDoubleSpinBox_, Parameters::RotateTheta);
  addDoubleSpinBoxManager(shearVectorXDoubleSpinBox_, Parameters::ShearVectorX);
  addDoubleSpinBoxManager(shearVectorYDoubleSpinBox_, Parameters::ShearVectorY);
  addDoubleSpinBoxManager(shearVectorZDoubleSpinBox_, Parameters::ShearVectorZ);
  addDoubleSpinBoxManager(shearFixedPlaneADoubleSpinBox_, Parameters::ShearPlaneA);
  addDoubleSpinBoxManager(shearFixedPlaneBDoubleSpinBox_, Parameters::ShearPlaneB);
  addDoubleSpinBoxManager(shearFixedPlaneCDoubleSpinBox_, Parameters::ShearPlaneC);
  addDoubleSpinBoxManager(shearFixedPlaneDDoubleSpinBox_, Parameters::ShearPlaneD);
  addDynamicLabelManager(fieldMapXLabel_, Parameters::FieldMapX);
  addDynamicLabelManager(fieldMapYLabel_, Parameters::FieldMapY);
  addDynamicLabelManager(fieldMapZLabel_, Parameters::FieldMapZ);
  addDoubleSpinBoxManager(uniformScaleDoubleSpinBox_, Parameters::UniformScale);
  addCheckBoxManager(resizeSeparablyCheckBox_, Parameters::ResizeSeparably);
  addCheckBoxManager(ignoreChangesCheckBox_, Parameters::IgnoreChanges);

  connect(transformTabWidget_, SIGNAL(currentChanged(int)), this, SLOT(changeTransformType(int)));
  connect(resetPushButton_, SIGNAL(clicked()), this, SLOT(resetValues()));
  connect(resetFieldMapPushButton_, SIGNAL(clicked()), this, SLOT(resetFieldMap()));
  connect(resetWidgetPushButton_, SIGNAL(clicked()), this, SLOT(resetWidget()));
  connect(cycleUpPushButton_, SIGNAL(clicked()), this, SLOT(cycleUp()));
  connect(cycleDownPushButton_, SIGNAL(clicked()), this, SLOT(cycleDown()));
  connect(swapXYPushButton_, SIGNAL(clicked()), this, SLOT(swapXY()));
  connect(swapYZPushButton_, SIGNAL(clicked()), this, SLOT(swapYZ()));
  connect(swapXZPushButton_, SIGNAL(clicked()), this, SLOT(swapXZ()));
  connect(flipXPushButton_, SIGNAL(clicked()), this, SLOT(flipX()));
  connect(flipYPushButton_, SIGNAL(clicked()), this, SLOT(flipY()));
  connect(flipZPushButton_, SIGNAL(clicked()), this, SLOT(flipZ()));
  connect(applyTransformPushButton_, SIGNAL(clicked()), this, SLOT(applyTransform()));
  connect(compositeTransformPushButton_, SIGNAL(clicked()), this, SLOT(compositeTransform()));
  connect(logCalculatorLineEdit_, SIGNAL(textEdited(const QString&)), this, SLOT(calculateLog(const QString&)));
}

void CreateGeometricTransformDialog::pullSpecial()
{
}

void CreateGeometricTransformDialog::changeTransformType(int index)
{
  state_->setValue(Parameters::TransformType, index);
}

void CreateGeometricTransformDialog::resetValues()
{
  state_->setValue(Parameters::TransformType, 0);
  state_->setValue(Parameters::MultiplyRadioButton, 1);
  state_->setValue(Parameters::TranslateVectorX, 0.0);
  state_->setValue(Parameters::TranslateVectorY, 0.0);
  state_->setValue(Parameters::TranslateVectorZ, 0.0);
  state_->setValue(Parameters::ScalePointX, 0.0);
  state_->setValue(Parameters::ScalePointY, 0.0);
  state_->setValue(Parameters::ScalePointZ, 0.0);
  state_->setValue(Parameters::LogScale, 0.0);
  state_->setValue(Parameters::LogScaleX, 0.0);
  state_->setValue(Parameters::LogScaleY, 0.0);
  state_->setValue(Parameters::LogScaleZ, 0.0);
  state_->setValue(Parameters::RotatePointX, 0.0);
  state_->setValue(Parameters::RotatePointY, 0.0);
  state_->setValue(Parameters::RotatePointZ, 0.0);
  state_->setValue(Parameters::RotateAxisX, 0.0);
  state_->setValue(Parameters::RotateAxisY, 0.0);
  state_->setValue(Parameters::RotateAxisZ, 1.0);
  state_->setValue(Parameters::RotateTheta, 0.0);
  state_->setValue(Parameters::ShearVectorX, 0.0);
  state_->setValue(Parameters::ShearVectorY, 0.0);
  state_->setValue(Parameters::ShearVectorZ, 0.0);
  state_->setValue(Parameters::ShearPlaneA, 0.0);
  state_->setValue(Parameters::ShearPlaneB, 0.0);
  state_->setValue(Parameters::ShearPlaneC, 0.0);
  state_->setValue(Parameters::ShearPlaneD, 1.0);
  state_->setValue(Parameters::UniformScale, 1.0);
  state_->setValue(Parameters::ResizeSeparably, true);
  state_->setValue(Parameters::IgnoreChanges, true);

  transformTabWidget_->setCurrentIndex(state_->getValue(Parameters::TransformType).toInt());
}

void CreateGeometricTransformDialog::resetFieldMap()
{
  std::string x = "x+";
  std::string y = "y+";
  std::string z = "z+";
  state_->setValue(Parameters::FieldMapX, x);
  state_->setValue(Parameters::FieldMapY, y);
  state_->setValue(Parameters::FieldMapZ, z);
  state_->setValue(Parameters::PermuteX, 1);
  state_->setValue(Parameters::PermuteY, 2);
  state_->setValue(Parameters::PermuteZ, 3);
}

void CreateGeometricTransformDialog::resetWidget()
{
  state_->setValue(Parameters::UniformScale, 1.0);
}

void CreateGeometricTransformDialog::cycleUp()
{
  std::string temp = state_->getValue(Parameters::FieldMapX).toString();
  state_->setValue(Parameters::FieldMapX, state_->getValue(Parameters::FieldMapY).toString());
  state_->setValue(Parameters::FieldMapY, state_->getValue(Parameters::FieldMapZ).toString());
  state_->setValue(Parameters::FieldMapZ, temp);

  int value = state_->getValue(Parameters::PermuteX).toInt();
  state_->setValue(Parameters::PermuteX, state_->getValue(Parameters::PermuteY).toInt());
  state_->setValue(Parameters::PermuteY, state_->getValue(Parameters::PermuteZ).toInt());
  state_->setValue(Parameters::PermuteZ, value);
}

void CreateGeometricTransformDialog::cycleDown()
{
  std::string temp = state_->getValue(Parameters::FieldMapX).toString();
  state_->setValue(Parameters::FieldMapX, state_->getValue(Parameters::FieldMapZ).toString());
  state_->setValue(Parameters::FieldMapZ, state_->getValue(Parameters::FieldMapY).toString());
  state_->setValue(Parameters::FieldMapY, temp);

  int value = state_->getValue(Parameters::PermuteX).toInt();
  state_->setValue(Parameters::PermuteX, state_->getValue(Parameters::PermuteZ).toInt());
  state_->setValue(Parameters::PermuteZ, state_->getValue(Parameters::PermuteY).toInt());
  state_->setValue(Parameters::PermuteY, value);
}

void CreateGeometricTransformDialog::swapXY()
{
  std::string temp = state_->getValue(Parameters::FieldMapX).toString();
  state_->setValue(Parameters::FieldMapX, state_->getValue(Parameters::FieldMapY).toString());
  state_->setValue(Parameters::FieldMapY, temp);

  int value = state_->getValue(Parameters::PermuteX).toInt();
  state_->setValue(Parameters::PermuteX, state_->getValue(Parameters::PermuteY).toInt());
  state_->setValue(Parameters::PermuteY, value);
}

void CreateGeometricTransformDialog::swapYZ()
{
  std::string temp = state_->getValue(Parameters::FieldMapY).toString();
  state_->setValue(Parameters::FieldMapY, state_->getValue(Parameters::FieldMapZ).toString());
  state_->setValue(Parameters::FieldMapZ, temp);

  int value = state_->getValue(Parameters::PermuteY).toInt();
  state_->setValue(Parameters::PermuteY, state_->getValue(Parameters::PermuteZ).toInt());
  state_->setValue(Parameters::PermuteZ, value);
}

void CreateGeometricTransformDialog::swapXZ()
{
  std::string temp = state_->getValue(Parameters::FieldMapX).toString();
  state_->setValue(Parameters::FieldMapX, state_->getValue(Parameters::FieldMapZ).toString());
  state_->setValue(Parameters::FieldMapZ, temp);

  int value = state_->getValue(Parameters::PermuteX).toInt();
  state_->setValue(Parameters::PermuteX, state_->getValue(Parameters::PermuteZ).toInt());
  state_->setValue(Parameters::PermuteZ, value);
}

void CreateGeometricTransformDialog::flipX()
{
  auto x = state_->getValue(Parameters::FieldMapX).toString();
  auto value = state_->getValue(Parameters::PermuteX).toInt();
  if (x[1] == '+')
  {
    x[1] = '-';
  }
  else
  {
    x[1] = '+';
  }
  state_->setValue(Parameters::FieldMapX, x);
  state_->setValue(Parameters::PermuteX, -value);
}

void CreateGeometricTransformDialog::flipY()
{
  auto y = state_->getValue(Parameters::FieldMapY).toString();
  auto value = state_->getValue(Parameters::PermuteY).toInt();
  if (y[1] == '+')
  {
    y[1] = '-';
  }
  else
  {
    y[1] = '+';
  }
  state_->setValue(Parameters::FieldMapY, y);
  state_->setValue(Parameters::PermuteY, -value);
}

void CreateGeometricTransformDialog::flipZ()
{
  auto z = state_->getValue(Parameters::FieldMapZ).toString();
  auto value = state_->getValue(Parameters::PermuteZ).toInt();
  if (z[1] == '+')
  {
    z[1] = '-';
  }
  else
  {
    z[1] = '+';
  }
  state_->setValue(Parameters::FieldMapZ, z);
  state_->setValue(Parameters::PermuteZ, -value);
}

void CreateGeometricTransformDialog::applyTransform()
{

}

void CreateGeometricTransformDialog::compositeTransform()
{

}

void CreateGeometricTransformDialog::calculateLog(const QString& text)
{
  auto val = log10(text.toDouble());
  QString result = QString::number(val, 'f', 5);
  logCalculatorResultLabel_->setText(result);
}

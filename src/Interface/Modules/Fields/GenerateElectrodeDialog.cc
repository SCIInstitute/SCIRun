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


#include <Interface/Modules/Fields/GenerateElectrodeDialog.h>
#include <Modules/Legacy/Fields/GenerateElectrode.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Datatypes/Color.h>
#include <QColorDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;

GenerateElectrodeDialog::GenerateElectrodeDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addComboBoxManager(TypeComboBox_, Parameters::ElectrodeType);
  addComboBoxManager(ProjectionComboBox_, Parameters::ElectrodeProjection);
  addDoubleSpinBoxManager(LengthDoubleSpinBox_, Parameters::ElectrodeLength);
  addDoubleSpinBoxManager(WidthDoubleSpinBox_, Parameters::ElectrodeWidth);
  addDoubleSpinBoxManager(ThicknessDoubleSpinBox_, Parameters::ElectrodeThickness);
  addSpinBoxManager(NumberOfControlPointsSpinBox_, Parameters::NumberOfControlPoints);
  addSpinBoxManager(ResolutionSpinBox_, Parameters::ElectrodeResolution);
  addCheckBoxManager(MoveAllCheckBox_, Parameters::MoveAll);
  addCheckBoxManager(UseFieldNodesCheckBox_, Parameters::UseFieldNodes);
  addDoubleSpinBoxManager(WidthDoubleSpinBox_, Parameters::GEProbeSize);

  connect(TypeComboBox_, COMBO_BOX_ACTIVATED_STRING, this, &GenerateElectrodeDialog::enableWidgets);
  connect(colorChooserPushButton_, &QPushButton::clicked, this, &GenerateElectrodeDialog::assignDefaultMeshColor);
  connectButtonToExecuteSignal(colorChooserPushButton_);
  connect(AddPointPushButton_, &QPushButton::clicked, this, &GenerateElectrodeDialog::AddPoint);
  connect(RemovePointPushButton_, &QPushButton::clicked, this, &GenerateElectrodeDialog::RemovePoint);
  connectButtonToExecuteSignal(AddPointPushButton_);
  connectButtonToExecuteSignal(RemovePointPushButton_);
}

void GenerateElectrodeDialog::enableWidgets(const QString& mode)
{
  WidthDoubleSpinBox_->setReadOnly(mode != "planar");
  ProjectionComboBox_->setEnabled(mode == "planar");
}

void GenerateElectrodeDialog::pullSpecial()
{
  ColorRGB color(state_->getValue(Parameters::GEProbeColor).toString());
  // check for old saved color format: integers 0-255.
  defaultMeshColor_ = QColor(
    static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
    static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
    static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));

  enableWidgets(QString::fromStdString(state_->getValue(Parameters::ElectrodeType).toString()));
}

void GenerateElectrodeDialog::assignDefaultMeshColor()
{
  auto newColor = QColorDialog::getColor(defaultMeshColor_, this, "Choose default mesh color");
  if (newColor.isValid())
  {
    defaultMeshColor_ = newColor;
    pushColor();
  }
}

void GenerateElectrodeDialog::pushColor()
{
  state_->setValue(Parameters::GEProbeColor, ColorRGB(defaultMeshColor_.redF(), defaultMeshColor_.greenF(), defaultMeshColor_.blueF()).toString());
}

//void GenerateElectrodeDialog::toggleSpinBoxes()
//{
//  bboxScaleDoubleSpinBox_->setEnabled(!bboxScaleDoubleSpinBox_->isEnabled());
//  sizeDoubleSpinBox_->setEnabled(!sizeDoubleSpinBox_->isEnabled());
//}

void GenerateElectrodeDialog::AddPoint()
{
    int numpoints = state_->getValue(Parameters::NumberOfControlPoints).toInt();
    state_->setValue(Parameters::NumberOfControlPoints,numpoints+1);
}


void GenerateElectrodeDialog::RemovePoint()
{
    int numpoints = state_->getValue(Parameters::NumberOfControlPoints).toInt();
    state_->setValue(Parameters::NumberOfControlPoints,numpoints-1);
}

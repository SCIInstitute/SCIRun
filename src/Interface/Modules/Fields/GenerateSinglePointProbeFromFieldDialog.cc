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


#include <Interface/Modules/Fields/GenerateSinglePointProbeFromFieldDialog.h>
#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Datatypes/Color.h>
#include <QColorDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;

GenerateSinglePointProbeFromFieldDialog::GenerateSinglePointProbeFromFieldDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addComboBoxManager(moveToComboBox_, Parameters::MoveMethod);
  addDoubleSpinBoxManager(xLocationDoubleSpinBox_, Parameters::XLocation);
  addDoubleSpinBoxManager(yLocationDoubleSpinBox_, Parameters::YLocation);
  addDoubleSpinBoxManager(zLocationDoubleSpinBox_, Parameters::ZLocation);
  addDoubleSpinBoxManager(sizeDoubleSpinBox_, Parameters::ProbeSize);
  addLineEditManager(fieldValueLineEdit_, Parameters::FieldValue);
  addSpinBoxManager(fieldNodeSpinBox_, Parameters::FieldNode);
  addSpinBoxManager(fieldElemSpinBox_, Parameters::FieldElem);
  addCheckBoxManager(snapToNodeCheckBox_, Parameters::SnapToNode);
  addCheckBoxManager(snapToElementCheckBox_, Parameters::SnapToElement);

  connect(moveToComboBox_, SIGNAL(activated(const QString&)), this, SLOT(enableWidgets(const QString&)));
  connect(colorChooserPushButton_, SIGNAL(clicked()), this, SLOT(assignDefaultMeshColor()));
  connectButtonToExecuteSignal(colorChooserPushButton_);
}

void GenerateSinglePointProbeFromFieldDialog::enableWidgets(const QString& mode)
{
  xLocationDoubleSpinBox_->setReadOnly(mode != "Location");
  yLocationDoubleSpinBox_->setReadOnly(mode != "Location");
  zLocationDoubleSpinBox_->setReadOnly(mode != "Location");
  fieldNodeSpinBox_->setReadOnly(mode != "Node");
  fieldElemSpinBox_->setReadOnly(mode != "Element");
  snapToNodeCheckBox_->setVisible(mode == "Node");
  snapToElementCheckBox_->setVisible(mode == "Element");
}

void GenerateSinglePointProbeFromFieldDialog::pullSpecial()
{
  ColorRGB color(state_->getValue(Parameters::ProbeColor).toString());
  // check for old saved color format: integers 0-255.
  defaultMeshColor_ = QColor(
    static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
    static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
    static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));

  enableWidgets(QString::fromStdString(state_->getValue(Parameters::MoveMethod).toString()));
}

void GenerateSinglePointProbeFromFieldDialog::assignDefaultMeshColor()
{
  auto newColor = QColorDialog::getColor(defaultMeshColor_, this, "Choose default mesh color");
  if (newColor.isValid())
  {
    defaultMeshColor_ = newColor;
    pushColor();
  }
}

void GenerateSinglePointProbeFromFieldDialog::pushColor()
{
  state_->setValue(Parameters::ProbeColor, ColorRGB(defaultMeshColor_.redF(), defaultMeshColor_.greenF(), defaultMeshColor_.blueF()).toString());
}

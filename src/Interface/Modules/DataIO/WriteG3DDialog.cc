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

#include <Interface/Modules/DataIO/WriteG3DDialog.h>
#include <Modules/DataIO/WriteG3D.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/ImportExport/GenericIEPlugin.h>
#include <Core/Datatypes/Color.h>
#include <iostream>
#include <QFileDialog>
#include <QColorDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

WriteG3DDialog::WriteG3DDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addCheckBoxManager(transparentCheckBox_, WriteG3D::EnableTransparency);
  addDoubleSpinBoxManager(transparencyDoubleSpinBox_, WriteG3D::TransparencyValue);
  addRadioButtonGroupManager({ defaultColorRadioButton_, colorMapRadioButton_, rgbConversionRadioButton_ }, WriteG3D::Coloring);

  connect(colorPushButton_, SIGNAL(clicked()), this, SLOT(assignDefaultColor()));
  connect(saveFileButton_, SIGNAL(clicked()), this, SLOT(saveFile()));
  connect(fileNameLineEdit_, SIGNAL(editingFinished()), this, SLOT(pushFileNameToState()));
  connect(fileNameLineEdit_, SIGNAL(returnPressed()), this, SLOT(pushFileNameToState()));
  WidgetStyleMixin::setStateVarTooltipWithStyle(fileNameLineEdit_, Variables::Filename.name());
  WidgetStyleMixin::setStateVarTooltipWithStyle(this, Variables::FileTypeName.name());
  WidgetStyleMixin::setStateVarTooltipWithStyle(saveFileButton_, Variables::FileTypeName.name());
}

void WriteG3DDialog::pullSpecial()
{
  fileNameLineEdit_->setText(QString::fromStdString(state_->getValue(Variables::Filename).toString()));

  ColorRGB color(state_->getValue(WriteG3D::DefaultColor).toString());
  // check for old saved color format: integers 0-255.
  defaultColor_ = QColor(
    static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
    static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
    static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));
  
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(defaultColor_.red()) + "," +
    QString::number(defaultColor_.green()) + "," + QString::number(defaultColor_.blue()) + "); }";
  defaultColorDisplayLabel_->setStyleSheet(styleSheet);
}

void WriteG3DDialog::pushFileNameToState()
{
  state_->setValue(Variables::Filename, fileNameLineEdit_->text().trimmed().toStdString());
}

void WriteG3DDialog::saveFile()
{
  auto types = state_->getValue(Variables::FileTypeList).toString();
  auto file = QFileDialog::getSaveFileName(this, "Save Field File", dialogDirectory(), QString::fromStdString(types), &selectedFilter_);
  if (file.length() > 0)
  {
    auto typeName = SCIRun::fileTypeDescriptionFromDialogBoxFilter(selectedFilter_.toStdString());
    state_->setValue(Variables::FileTypeName, typeName);
    fileNameLineEdit_->setText(file);
    updateRecentFile(file);
    pushFileNameToState();
  }
}

void WriteG3DDialog::assignDefaultColor()
{
  auto newColor = QColorDialog::getColor(defaultColor_, this, "Choose default color");
  if (newColor.isValid())
  {
    state_->setValue(WriteG3D::DefaultColor, ColorRGB(newColor.redF(), newColor.greenF(), newColor.blueF()).toString());
  }
}
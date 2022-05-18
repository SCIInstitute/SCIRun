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


#include <Interface/Modules/Fields/CreateImageDialog.h>
#include <Modules/Legacy/Fields/CreateImage.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
typedef SCIRun::Modules::Fields::CreateImage CreateImageModule;

CreateImageDialog::CreateImageDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addSpinBoxManager(width_, CreateImageModule::Width);
  addSpinBoxManager(height_, CreateImageModule::Height);
  addDoubleSpinBoxManager(padPercentageSpinBox_, CreateImageModule::PadPercent);

  addComboBoxManager(mode_, CreateImageModule::Mode);
  // these don't really do anything with the module, or they are broken. Fix will require module code refactoring
  mode_->setVisible(false);
  modeLabel_->setVisible(false);
  index_->setVisible(false);
  indexLabel_->setVisible(false);

  addComboBoxManager(axis_, CreateImageModule::Axis);

  addDoubleSpinBoxManager(centerX_, CreateImageModule::CenterX);
  addDoubleSpinBoxManager(centerY_, CreateImageModule::CenterY);
  addDoubleSpinBoxManager(centerZ_, CreateImageModule::CenterZ);

  addDoubleSpinBoxManager(normalX_, CreateImageModule::NormalX);
  addDoubleSpinBoxManager(normalY_, CreateImageModule::NormalY);
  addDoubleSpinBoxManager(normalZ_, CreateImageModule::NormalZ);

  addDoubleSpinBoxManager(position_, CreateImageModule::Position);
  connect(axis_, COMBO_BOX_ACTIVATED_STRING, this, &CreateImageDialog::enableWidgets);

  addComboBoxManager(dataLocation_, CreateImageModule::DataLocation);
}

void CreateImageDialog::enableWidgets(const QString& mode)
{
  const auto useCustom = mode == "Custom";
  for (auto& spinBox : { centerX_, centerY_, centerZ_, normalX_, normalY_, normalZ_ })
  {
    spinBox->setEnabled(useCustom);
  }
  position_->setEnabled(!useCustom);
}

void CreateImageDialog::pullSpecial()
{
  enableWidgets(QString::fromStdString(state_->getValue(CreateImageModule::Axis).toString()));
}

/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Interface/Modules/Fields/CreateLatVolDialog.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
typedef SCIRun::Modules::Fields::CreateLatVol CreateLatVolModule;

CreateLatVolDialog::CreateLatVolDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  //executeButton_->setEnabled(false);
  
  //connect(executeButton_, SIGNAL(clicked()), this, SIGNAL(executeButtonPressed()));
  //TODO: here is where to start on standardizing module dialog buttons.
  //connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(pushMatrixToState()));
  //push();

  connect(xSizeSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  connect(ySizeSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  connect(zSizeSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  connect(elementSizeNormalizedButton_, SIGNAL(clicked()), this, SLOT(push()));
  connect(elementSizeOneButton_, SIGNAL(clicked()), this, SLOT(push()));
  connect(dataAtNodesButton_, SIGNAL(clicked()), this, SLOT(push()));
  connect(dataAtCellsButton_, SIGNAL(clicked()), this, SLOT(push()));
  connect(dataAtNoneButton_, SIGNAL(clicked()), this, SLOT(push()));
}

void CreateLatVolDialog::push()
{
  if (!pulling_)
  {
    state_->setValue(CreateLatVolModule::XSize, xSizeSpinBox_->value());
    state_->setValue(CreateLatVolModule::YSize, ySizeSpinBox_->value());
    state_->setValue(CreateLatVolModule::ZSize, zSizeSpinBox_->value());
    state_->setValue(CreateLatVolModule::ElementSizeNormalized, elementSizeNormalizedButton_->isChecked());
    state_->setValue(CreateLatVolModule::DataAtLocation, getDataAtLocation());
  }
}

std::string CreateLatVolDialog::getDataAtLocation() const
{
  //TODO: need to standardize these options at the algo/module level
  if (dataAtNodesButton_->isChecked())
    return "Nodes";
  if (dataAtCellsButton_->isChecked())
    return "Cells";
  if (dataAtNoneButton_->isChecked())
    return "None";
  return "Unknown";
}

void CreateLatVolDialog::pull()
{
  Pulling p(this);
  int newValue = state_->getValue(CreateLatVolModule::XSize).getInt();
  if (newValue != xSizeSpinBox_->value())
    xSizeSpinBox_->setValue(newValue);
  newValue = state_->getValue(CreateLatVolModule::YSize).getInt();
  if (newValue != ySizeSpinBox_->value())
    ySizeSpinBox_->setValue(newValue);
  newValue = state_->getValue(CreateLatVolModule::ZSize).getInt();
  if (newValue != zSizeSpinBox_->value())
    zSizeSpinBox_->setValue(newValue);
  elementSizeNormalizedButton_->setChecked(state_->getValue(CreateLatVolModule::ElementSizeNormalized).getBool());
  elementSizeOneButton_->setChecked(!elementSizeNormalizedButton_->isChecked());

  std::string loc = state_->getValue(CreateLatVolModule::DataAtLocation).getString();
  dataAtNodesButton_->setChecked(loc == "Nodes");
  dataAtCellsButton_->setChecked(loc == "Cells");
  dataAtNoneButton_->setChecked(loc == "None");
}

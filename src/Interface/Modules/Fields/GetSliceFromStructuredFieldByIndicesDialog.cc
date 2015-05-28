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

#include <Interface/Modules/Fields/GetSliceFromStructuredFieldByIndicesDialog.h>
#include <Modules/Legacy/Fields/GetSliceFromStructuredFieldByIndices.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

GetSliceFromStructuredFieldByIndicesDialog::GetSliceFromStructuredFieldByIndicesDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  //connect(compartmentRadioButton_, SIGNAL(clicked()), this, SLOT(push()));
  //connect(compartmentsRangeRadioButton_, SIGNAL(clicked()), this, SLOT(push()));

  using namespace Parameters;
  addSpinBoxManager(iAxisSpinBox_, Index_i);
  addSpinBoxManager(jAxisSpinBox_, Index_j);
  addSpinBoxManager(kAxisSpinBox_, Index_k);
  addRadioButtonGroupManager({ iAxisRadioButton_, jAxisRadioButton_, kAxisRadioButton_ }, Axis_ijk);

  createExecuteInteractivelyToggleAction();

  //TODO: hook up with dialog execution model choice
  connect(iAxisHorizontalSlider_, SIGNAL(valueChanged(int)), this, SIGNAL(executeActionTriggered()));
  connect(jAxisHorizontalSlider_, SIGNAL(valueChanged(int)), this, SIGNAL(executeActionTriggered()));
  connect(kAxisHorizontalSlider_, SIGNAL(valueChanged(int)), this, SIGNAL(executeActionTriggered()));

  //connect()
}

void GetSliceFromStructuredFieldByIndicesDialog::pullSpecial()
{
  using namespace Parameters;

  //qDebug() << "iAxis max: " << state_->getValue(Dim_i).toInt();
  iAxisSpinBox_->setMaximum(state_->getValue(Dim_i).toInt());
  jAxisSpinBox_->setMaximum(state_->getValue(Dim_j).toInt());
  kAxisSpinBox_->setMaximum(state_->getValue(Dim_k).toInt());

  iAxisHorizontalSlider_->setMaximum(state_->getValue(Dim_i).toInt());
  jAxisHorizontalSlider_->setMaximum(state_->getValue(Dim_j).toInt());
  kAxisHorizontalSlider_->setMaximum(state_->getValue(Dim_k).toInt());
}
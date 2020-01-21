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

  using namespace Parameters;
  addSpinBoxManager(iAxisSpinBox_, Index_i);
  addSpinBoxManager(jAxisSpinBox_, Index_j);
  addSpinBoxManager(kAxisSpinBox_, Index_k);
  addRadioButtonGroupManager({ iAxisRadioButton_, jAxisRadioButton_, kAxisRadioButton_ }, Axis_ijk);
  addCheckBoxManager(spinBoxExecuteCheckBox_, SpinBoxReexecute);
  addCheckBoxManager(axisExecuteCheckBox_, AxisReexecute);
  addCheckBoxManager(sliderExecuteCheckBox_, SliderReexecute);

  connect(iAxisHorizontalSlider_, SIGNAL(sliderReleased()), this, SLOT(sliderIndexChanged()));
  connect(jAxisHorizontalSlider_, SIGNAL(sliderReleased()), this, SLOT(sliderIndexChanged()));
  connect(kAxisHorizontalSlider_, SIGNAL(sliderReleased()), this, SLOT(sliderIndexChanged()));
  connect(iAxisSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(spinBoxClicked(int)));
  connect(jAxisSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(spinBoxClicked(int)));
  connect(kAxisSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(spinBoxClicked(int)));
  connect(iAxisRadioButton_, SIGNAL(clicked()), this, SLOT(axisButtonClicked()));
  connect(jAxisRadioButton_, SIGNAL(clicked()), this, SLOT(axisButtonClicked()));
  connect(kAxisRadioButton_, SIGNAL(clicked()), this, SLOT(axisButtonClicked()));
}

void GetSliceFromStructuredFieldByIndicesDialog::pullSpecial()
{
  using namespace Parameters;

  auto iMax = state_->getValue(Dim_i).toInt() - 1;
  auto jMax = state_->getValue(Dim_j).toInt() - 1;
  auto kMax = state_->getValue(Dim_k).toInt() - 1;

  iAxisSpinBox_->setMaximum(iMax);
  iAxisSpinBox_->setEnabled(iAxisRadioButton_->isChecked());
  jAxisSpinBox_->setMaximum(jMax);
  jAxisSpinBox_->setEnabled(jAxisRadioButton_->isChecked());
  kAxisSpinBox_->setMaximum(kMax);
  kAxisSpinBox_->setEnabled(kAxisRadioButton_->isChecked());

  iAxisHorizontalSlider_->setMaximum(iMax);
  iAxisHorizontalSlider_->setEnabled(iAxisRadioButton_->isChecked());
  jAxisHorizontalSlider_->setMaximum(jMax);
  jAxisHorizontalSlider_->setEnabled(jAxisRadioButton_->isChecked());
  kAxisHorizontalSlider_->setMaximum(kMax);
  kAxisHorizontalSlider_->setEnabled(kAxisRadioButton_->isChecked());
}

void GetSliceFromStructuredFieldByIndicesDialog::axisButtonClicked()
{
  auto axisButton = qobject_cast<QRadioButton*>(sender());

  {
    auto i = axisButton->objectName().contains("iAxis");
    iAxisSpinBox_->setEnabled(i);
    iAxisHorizontalSlider_->setEnabled(i);
  }
  {
    auto j = axisButton->objectName().contains("jAxis");
    jAxisSpinBox_->setEnabled(j);
    jAxisHorizontalSlider_->setEnabled(j);
  }
  {
    auto k = axisButton->objectName().contains("kAxis");
    kAxisSpinBox_->setEnabled(k);
    kAxisHorizontalSlider_->setEnabled(k);
  }
  if (axisExecuteCheckBox_->isChecked())
    Q_EMIT executeFromStateChangeTriggered();
}

void GetSliceFromStructuredFieldByIndicesDialog::spinBoxClicked(int value)
{
  auto spinBox = qobject_cast<QSpinBox*>(sender());
  bool reexecute = false;
  if (spinBox->objectName().contains("iAxis"))
    reexecute = iAxisRadioButton_->isChecked();
  else if (spinBox->objectName().contains("jAxis"))
    reexecute = jAxisRadioButton_->isChecked();
  else if (spinBox->objectName().contains("kAxis"))
    reexecute = kAxisRadioButton_->isChecked();

  if (!pulling_ && spinBoxExecuteCheckBox_->isChecked() && reexecute)
    Q_EMIT executeFromStateChangeTriggered();
}

void GetSliceFromStructuredFieldByIndicesDialog::sliderIndexChanged()
{
  auto slider = qobject_cast<QSlider*>(sender());
  bool reexecute = false;
  if (slider->objectName().contains("iAxis"))
  {
    ScopedWidgetSignalBlocker i(iAxisSpinBox_);
    iAxisSpinBox_->setValue(slider->value());
    state_->setValue(Parameters::Index_i, slider->value());
    reexecute |= iAxisRadioButton_->isChecked();
  }
  else if (slider->objectName().contains("jAxis"))
  {
    ScopedWidgetSignalBlocker j(jAxisSpinBox_);
    jAxisSpinBox_->setValue(slider->value());
    state_->setValue(Parameters::Index_j, slider->value());
    reexecute |= jAxisRadioButton_->isChecked();
  }
  else if (slider->objectName().contains("kAxis"))
  {
    ScopedWidgetSignalBlocker k(kAxisSpinBox_);
    kAxisSpinBox_->setValue(slider->value());
    state_->setValue(Parameters::Index_k, slider->value());
    reexecute |= kAxisRadioButton_->isChecked();
  }

  if (sliderExecuteCheckBox_->isChecked() && reexecute)
    Q_EMIT executeFromStateChangeTriggered();
}

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


#include <Interface/Modules/Fields/ExtractSimpleIsosurfaceDialog.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/ExtractSimpleIsosurfaceAlgo.h>
#include <Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

ExtractSimpleIsosurfaceDialog::ExtractSimpleIsosurfaceDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  addDoubleSpinBoxManager(singleDoubleSpinBox_, Parameters::SingleIsoValue);
  addTextEditManager(listTextEdit_, Parameters::ListOfIsovalues);
  addSpinBoxManager(quantitySpinBox_, Parameters::QuantityOfIsovalues);
  addTextEditManager(isovalListFromQuantityTextEdit_, Parameters::IsovalueListString);
  addRadioButtonGroupManager({inclusiveRadioButton_, exclusiveRadioButton_}, Parameters::IsovalueListInclusiveExclusive);
  addRadioButtonGroupManager({manualMinMaxRadioButton_, fieldMinMaxRadioButton_}, Parameters::IsovalueQuantityFromField);
  addDoubleSpinBoxManager(manualMinDoubleSpinBox_, Parameters::ManualMinimumIsovalue);
  addDoubleSpinBoxManager(manualMaxDoubleSpinBox_, Parameters::ManualMaximumIsovalue);
  WidgetStyleMixin::tabStyle(tabWidget);
  addTabManager(tabWidget, Parameters::IsovalueChoice);
  connect(singleHorizontalSlider_, SIGNAL(sliderReleased()), this, SLOT(sliderChanged()));
}

void ExtractSimpleIsosurfaceDialog::sliderChanged()
{
  //qDebug() << __FUNCTION__ << value;
  auto value = singleHorizontalSlider_->value();
  auto percent = static_cast<double>(value) / (singleHorizontalSlider_->maximum() - singleHorizontalSlider_->minimum());
  auto minmax = transient_value_cast<std::pair<double, double>>(state_->getTransientValue("fieldMinMax"));
  auto newFieldValue = percent * (minmax.second - minmax.first) + minmax.first;
  singleDoubleSpinBox_->setValue(newFieldValue);
  singleDoubleSpinBox_->setToolTip(QString("Min: %1, Max: %2").arg(minmax.first).arg(minmax.second));

  if (executeOnReleaseCheckBox_->isChecked())
    Q_EMIT executeFromStateChangeTriggered();
}

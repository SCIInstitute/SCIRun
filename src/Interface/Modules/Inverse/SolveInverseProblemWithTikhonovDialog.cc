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

#include <Interface/Modules/Inverse/SolveInverseProblemWithTikhonovDialog.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

typedef SCIRun::Modules::Inverse::SolveInverseProblemWithTikhonov SolveInverseProblemWithTikhonovModule;

SolveInverseProblemWithTikhonovDialog::SolveInverseProblemWithTikhonovDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  lambdaMethod_.insert(StringPair("Direct entry", "single"));
  lambdaMethod_.insert(StringPair("Slider", "slider"));
  lambdaMethod_.insert(StringPair("L-curve", "lcurve"));

  WidgetStyleMixin::tabStyle(inputTabWidget_);

  addDoubleLineEditManager(lCurveLambdaLineEdit_, SolveInverseProblemWithTikhonovModule::LambdaCorner);
  addSpinBoxManager(lambdaNumberSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaNum);
  addDoubleSpinBoxManager(lambdaDoubleSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaFromDirectEntry);
  addDoubleSpinBoxManager(lambdaMinDoubleSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaMin);
  addDoubleSpinBoxManager(lambdaMaxDoubleSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaMax);
  addDoubleSpinBoxManager(lambdaResolutionDoubleSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaResolution);
  addDoubleSpinBoxManager(lCurveMinDoubleSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaMin);
  addDoubleSpinBoxManager(lCurveMaxDoubleSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaMax);

  addDoubleSpinBoxManager(lambdaSliderDoubleSpinBox_, SolveInverseProblemWithTikhonovModule::LambdaSliderValue);

  addRadioButtonGroupManager({ autoRadioButton_, underRadioButton_, overRadioButton_ }, SolveInverseProblemWithTikhonovModule::TikhonovCase);
  addRadioButtonGroupManager({ solutionConstraintRadioButton_, squaredSolutionRadioButton_ }, SolveInverseProblemWithTikhonovModule::TikhonovSolutionSubcase);
  addRadioButtonGroupManager({ residualConstraintRadioButton_, squaredResidualSolutionRadioButton_ }, SolveInverseProblemWithTikhonovModule::TikhonovResidualSubcase);

  addComboBoxManager(lambdaMethodComboBox_, SolveInverseProblemWithTikhonovModule::RegularizationMethod, lambdaMethod_);
  addTextEditManager(lCurveTextEdit_, SolveInverseProblemWithTikhonovModule::LCurveText);

  connect(lambdaSlider_, SIGNAL(valueChanged(int)), this, SLOT(setSpinBoxValue(int)));
  connect(lambdaSliderDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));
  connect(lambdaMinDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMin(double)));
  connect(lambdaMaxDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMax(double)));
  connect(lambdaResolutionDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderStep(double)));
}

void SolveInverseProblemWithTikhonovDialog::setSpinBoxValue(int value)
{
  lambdaSliderDoubleSpinBox_->setValue(value);
}

void SolveInverseProblemWithTikhonovDialog::setSliderValue(double value)
{
  if (value <= lambdaSlider_->maximum() && value >= lambdaSlider_->minimum())
    lambdaSlider_->setValue(static_cast<int>(value));
}

void SolveInverseProblemWithTikhonovDialog::setSliderMin(double value)
{
  lambdaSlider_->setMinimum(static_cast<int>(value));
}

void SolveInverseProblemWithTikhonovDialog::setSliderMax(double value)
{
  lambdaSlider_->setMaximum(static_cast<int>(value));
}

void SolveInverseProblemWithTikhonovDialog::setSliderStep(double value)
{
  lambdaSlider_->setSingleStep(static_cast<int>(value));
}

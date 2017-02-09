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

#include <Interface/Modules/Inverse/SolveInverseProblemWithTikhonovSVDDialog.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Inverse;

typedef SCIRun::Modules::Inverse::SolveInverseProblemWithTikhonovSVD SolveInverseProblemWithTikhonovSVDModule;

SolveInverseProblemWithTikhonovSVDDialog::SolveInverseProblemWithTikhonovSVDDialog(const std::string& name, ModuleStateHandle state,
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

  addDoubleLineEditManager(lCurveLambdaLineEdit_, SolveInverseProblemWithTikhonovSVDModule::LambdaCorner);
  addSpinBoxManager(lambdaNumberSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaNum);
  addDoubleSpinBoxManager(lambdaDoubleSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaFromDirectEntry);
  addDoubleSpinBoxManager(lambdaMinDoubleSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaMin);
  addDoubleSpinBoxManager(lambdaMaxDoubleSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaMax);
  addDoubleSpinBoxManager(lambdaResolutionDoubleSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaResolution);
  addDoubleSpinBoxManager(lCurveMinDoubleSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaMin);
  addDoubleSpinBoxManager(lCurveMaxDoubleSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaMax);

  addDoubleSpinBoxManager(lambdaSliderDoubleSpinBox_, SolveInverseProblemWithTikhonovSVDModule::LambdaSliderValue);

  addRadioButtonGroupManager({ autoRadioButton_, underRadioButton_, overRadioButton_ }, SolveInverseProblemWithTikhonovSVDModule::TikhonovCase);
  addRadioButtonGroupManager({ solutionConstraintRadioButton_, squaredSolutionRadioButton_ }, Parameters::TikhonovSolutionSubcase);
  addRadioButtonGroupManager({ residualConstraintRadioButton_, squaredResidualSolutionRadioButton_ }, Parameters::TikhonovResidualSubcase);

  addComboBoxManager(lambdaMethodComboBox_, SolveInverseProblemWithTikhonovSVDModule::RegularizationMethod, lambdaMethod_);
  addTextEditManager(lCurveTextEdit_, SolveInverseProblemWithTikhonovSVDModule::LCurveText);

  connect(lambdaSlider_, SIGNAL(valueChanged(int)), this, SLOT(setSpinBoxValue(int)));
  connect(lambdaSliderDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));
  connect(lambdaMinDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMin(double)));
  connect(lambdaMaxDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMax(double)));
  connect(lambdaResolutionDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderStep(double)));
}

void SolveInverseProblemWithTikhonovSVDDialog::setSpinBoxValue(int value)
{
  lambdaSliderDoubleSpinBox_->setValue(value);
}

void SolveInverseProblemWithTikhonovSVDDialog::setSliderValue(double value)
{
  if (value <= lambdaSlider_->maximum() && value >= lambdaSlider_->minimum())
    lambdaSlider_->setValue(static_cast<int>(value));
}

void SolveInverseProblemWithTikhonovSVDDialog::setSliderMin(double value)
{
  lambdaSlider_->setMinimum(static_cast<int>(value));
}

void SolveInverseProblemWithTikhonovSVDDialog::setSliderMax(double value)
{
  lambdaSlider_->setMaximum(static_cast<int>(value));
}

void SolveInverseProblemWithTikhonovSVDDialog::setSliderStep(double value)
{
  lambdaSlider_->setSingleStep(static_cast<int>(value));
}

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


#include <Interface/Modules/Inverse/SolveInverseProblemWithTikhonovSVDDialog.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>

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

  GuiStringTranslationMap lambdaMethod_;
  lambdaMethod_.insert(StringPair("Direct entry", "single"));
  lambdaMethod_.insert(StringPair("Slider", "slider"));
  lambdaMethod_.insert(StringPair("L-curve", "lcurve"));

  addSpinBoxManager(lambdaNumberSpinBox_, Parameters::LambdaNum);
  addDoubleSpinBoxManager(lambdaDoubleSpinBox_, Parameters::LambdaFromDirectEntry);
  addDoubleSpinBoxManager(lambdaMinDoubleSpinBox_, Parameters::LambdaMin);
  addDoubleSpinBoxManager(lambdaMaxDoubleSpinBox_, Parameters::LambdaMax);
  addDoubleSpinBoxManager(lambdaResolutionDoubleSpinBox_, Parameters::LambdaResolution);
  addDoubleLineEditManager(lCurveMinLineEdit_, Parameters::LambdaMin);
  addDoubleLineEditManager(lCurveMaxLineEdit_, Parameters::LambdaMax);

  addDoubleSpinBoxManager(lambdaSliderDoubleSpinBox_, Parameters::LambdaSliderValue);

  addComboBoxManager(lambdaMethodComboBox_, Parameters::RegularizationMethod, lambdaMethod_);

  connect(lambdaSlider_, SIGNAL(valueChanged(int)), this, SLOT(setSpinBoxValue(int)));
  connect(lambdaSliderDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));
  connect(lambdaMinDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMin(double)));
  connect(lambdaMaxDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMax(double)));
  connect(lambdaResolutionDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderStep(double)));

  WidgetStyleMixin::tabStyle(tabWidget);
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

void SolveInverseProblemWithTikhonovSVDDialog::pullAndDisplayInfo()
{
  auto str = transient_value_cast<std::string>(state_->getTransientValue("LambdaCurveInfo"));
  lCurveTextEdit_->setPlainText(QString::fromStdString(str));
  auto lambda = transient_value_cast<double>(state_->getTransientValue("LambdaCorner"));
  lCurveLambdaLineEdit_->setText(QString::number(lambda));
  lCurvePlotWidgetHelper_.updatePlot(state_, plotTab_);
}

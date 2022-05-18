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


#include <Interface/Modules/Inverse/SolveInverseProblemWithTSVDDialog.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTSVD.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Inverse;

typedef SCIRun::Modules::Inverse::SolveInverseProblemWithTSVD SolveInverseProblemWithTSVDModule;

SolveInverseProblemWithTSVDDialog::SolveInverseProblemWithTSVDDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addSpinBoxManager(lambdaNumberSpinBox_, Parameters::LambdaNum);
  addDoubleSpinBoxManager(lambdaDoubleSpinBox_, Parameters::LambdaFromDirectEntry);
  addDoubleSpinBoxManager(lambdaMinDoubleSpinBox_, Parameters::LambdaMin);
  addDoubleSpinBoxManager(lambdaMaxDoubleSpinBox_, Parameters::LambdaMax);
  addDoubleSpinBoxManager(lambdaResolutionDoubleSpinBox_, Parameters::LambdaResolution);
  addDoubleLineEditManager(lCurveMinLineEdit_, Parameters::LambdaMin);
  addDoubleLineEditManager(lCurveMaxLineEdit_, Parameters::LambdaMax);

  addDoubleSpinBoxManager(lambdaSliderDoubleSpinBox_, Parameters::LambdaSliderValue);

  addComboBoxManager(lambdaMethodComboBox_, Parameters::RegularizationMethod,
    {{"Direct entry", "single"},
    {"Slider", "slider"},
    {"L-curve", "lcurve"}});

  connect(lambdaSlider_, &QSlider::valueChanged, this, &SolveInverseProblemWithTSVDDialog::setSpinBoxValue);
  connect(lambdaSliderDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SolveInverseProblemWithTSVDDialog::setSliderValue);
  connect(lambdaMinDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SolveInverseProblemWithTSVDDialog::setSliderMin);
  connect(lambdaMaxDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SolveInverseProblemWithTSVDDialog::setSliderMax);
  connect(lambdaResolutionDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SolveInverseProblemWithTSVDDialog::setSliderStep);

  WidgetStyleMixin::tabStyle(tabWidget);
}


void SolveInverseProblemWithTSVDDialog::setSpinBoxValue(int value)
{
  lambdaSliderDoubleSpinBox_->setValue(value);
}

void SolveInverseProblemWithTSVDDialog::setSliderValue(double value)
{
  if (value <= lambdaSlider_->maximum() && value >= lambdaSlider_->minimum())
    lambdaSlider_->setValue(static_cast<int>(value));
}

void SolveInverseProblemWithTSVDDialog::setSliderMin(double value)
{
  lambdaSlider_->setMinimum(static_cast<int>(value));
}

void SolveInverseProblemWithTSVDDialog::setSliderMax(double value)
{
  lambdaSlider_->setMaximum(static_cast<int>(value));
}

void SolveInverseProblemWithTSVDDialog::setSliderStep(double value)
{
  lambdaSlider_->setSingleStep(static_cast<int>(value));
}

void SolveInverseProblemWithTSVDDialog::pullAndDisplayInfo()
{
  auto str = transient_value_cast<std::string>(state_->getTransientValue("LambdaCurveInfo"));
  lCurveTextEdit_->setPlainText(QString::fromStdString(str));
  auto lambda = transient_value_cast<double>(state_->getTransientValue("LambdaCorner"));
  lCurveLambdaLineEdit_->setText(QString::number(lambda));
  lCurvePlotWidgetHelper_.updatePlot(state_, plotTab_);
}

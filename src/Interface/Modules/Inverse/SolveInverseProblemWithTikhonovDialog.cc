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


#include <Interface/Modules/Inverse/SolveInverseProblemWithTikhonovDialog.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Inverse;
using namespace SCIRun::Core::Datatypes;

typedef SCIRun::Modules::Inverse::SolveInverseProblemWithTikhonov SolveInverseProblemWithTikhonovModule;

SolveInverseProblemWithTikhonovDialog::SolveInverseProblemWithTikhonovDialog(const std::string& name, ModuleStateHandle state,
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

  WidgetStyleMixin::tabStyle(inputTabWidget_);

  addSpinBoxManager(lambdaNumberSpinBox_, Parameters::LambdaNum);
  addDoubleSpinBoxManager(lambdaDoubleSpinBox_, Parameters::LambdaFromDirectEntry);
  addDoubleSpinBoxManager(lambdaMinDoubleSpinBox_, Parameters::LambdaMin);
  addDoubleSpinBoxManager(lambdaMaxDoubleSpinBox_, Parameters::LambdaMax);
  addDoubleSpinBoxManager(lambdaResolutionDoubleSpinBox_, Parameters::LambdaResolution);
  addDoubleLineEditManager(lCurveMinLineEdit_, Parameters::LambdaMin);
  addDoubleLineEditManager(lCurveMaxLineEdit_, Parameters::LambdaMax);

  addDoubleSpinBoxManager(lambdaSliderDoubleSpinBox_, Parameters::LambdaSliderValue);

  addRadioButtonGroupManager({ autoRadioButton_, underRadioButton_, overRadioButton_ }, Parameters::regularizationChoice);
  addRadioButtonGroupManager({ solutionConstraintRadioButton_, squaredSolutionRadioButton_ }, Parameters::regularizationSolutionSubcase);
  addRadioButtonGroupManager({ residualConstraintRadioButton_, squaredResidualSolutionRadioButton_ }, Parameters::regularizationResidualSubcase);

  addComboBoxManager(lambdaMethodComboBox_, Parameters::RegularizationMethod, lambdaMethod_);

  connect(lambdaSlider_, SIGNAL(valueChanged(int)), this, SLOT(setSpinBoxValue(int)));
  connect(lambdaSliderDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));
  connect(lambdaMinDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMin(double)));
  connect(lambdaMaxDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderMax(double)));
  connect(lambdaResolutionDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSliderStep(double)));

  WidgetStyleMixin::tabStyle(tabWidget);
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

void SolveInverseProblemWithTikhonovDialog::pullAndDisplayInfo()
{
  auto str = transient_value_cast<std::string>(state_->getTransientValue("LambdaCurveInfo"));
  lCurveTextEdit_->setPlainText(QString::fromStdString(str));
  auto lambda = transient_value_cast<double>(state_->getTransientValue("LambdaCorner"));
  lCurveLambdaLineEdit_->setText(QString::number(lambda));
  lCurvePlotWidgetHelper_.updatePlot(state_, plotTab_);
}

void LCurvePlotWidgetHelper::updatePlot(ModuleStateHandle state, QWidget* plotTab)
{
  auto data = transient_value_cast<DenseMatrixHandle>(state->getTransientValue("LambdaCurve"));
  auto cornerData = transient_value_cast<std::vector<double>>(state->getTransientValue("LambdaCornerPlot"));

  if (data)
  {
    QPolygonF points;
    auto log10L = [](double d) { return log10(d); };
    auto logX = data->col(1).unaryExpr(log10L);
    auto logY = data->col(2).unaryExpr(log10L);
    double maxX = logX.maxCoeff();
    double maxY = logY.maxCoeff();
    double minX = logX.minCoeff();
    double minY = logY.minCoeff();

    for (int i = 0; i < data->nrows(); ++i)
    {
      points << QPointF(logX(i), logY(i));
    }


    if (plot_)
    {
      plotTab->layout()->removeWidget(plot_);
    }

    plot_ = new QwtPlot(plotTab);
    plot_->setCanvasBackground( Qt::white );
    plot_->setAxisScale( QwtPlot::xBottom, minX * 0.9, maxX * 1.1 );
    plot_->setAxisTitle(QwtPlot::xBottom, "log ||Ax - y||");
    plot_->setAxisScale( QwtPlot::yLeft, minY * 0.9, maxY * 1.1 );
    plot_->setAxisTitle(QwtPlot::yLeft, "log ||Rx||");
    plot_->insertLegend( new QwtLegend() );

    auto grid = new QwtPlotGrid();
    grid->attach( plot_ );

    auto curve = new QwtPlotCurve();
    curve->setPen( Qt::yellow, 2 ),
    curve->setTitle( "L Curve" );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    curve->attach( plot_ );
    curve->setSamples( points );

    if (cornerData.size() == 6)
    {
      auto corner = new QwtPlotCurve();
      corner->setPen( Qt::green, 2 ),
      corner->setTitle( "L Corner" );
      corner->setRenderHint( QwtPlotItem::RenderAntialiased, true );
      corner->attach( plot_ );
      QPolygonF cornerPoints;
      cornerPoints << QPointF(0, cornerData[2]);
      cornerPoints << QPointF(cornerData[3], cornerData[2]);
      cornerPoints << QPointF(cornerData[3], 0);

      corner->setSamples(cornerPoints);
    }

    plotTab->layout()->addWidget(plot_);
  }
}

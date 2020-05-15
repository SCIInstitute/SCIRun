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


#include <Interface/Modules/Math/BasicPlotterDialog.h>
#include <Interface/Modules/Math/PlotDialog.h>
#include <Modules/Math/BasicPlotter.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Logging/Log.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

#ifndef Q_MOC_RUN
#include <Core/Utils/StringUtil.h>
#endif

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;

BasicPlotterDialog::BasicPlotterDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: ModuleDialogGeneric(state, parent), dataColors_(labelColorMax_), dataLabels_(labelColorMax_)
{
	setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();

	addDoubleSpinBoxManager(verticalAxisSpinBox_, Parameters::VerticalAxisPosition);
  addDoubleSpinBoxManager(horizontalAxisSpinBox_, Parameters::HorizontalAxisPosition);
	//addCheckBoxManager(verticalAxisGroupBox_, Parameters::VerticalAxisVisible);
  //addCheckBoxManager(horizontalAxisGroupBox_, Parameters::HorizontalAxisVisible);
	addLineEditManager(titleLineEdit_, Parameters::PlotTitle);
	addLineEditManager(xAxisLineEdit_, Parameters::XAxisLabel);
	addLineEditManager(yAxisLineEdit_, Parameters::YAxisLabel);
	addCheckBoxManager(showPointsCheckBox_, Parameters::ShowPointSymbols);
	addCheckBoxManager(transposeDataCheckBox_, Parameters::TransposeData);
	addComboBoxManager(curveStyleComboBox_, Parameters::CurveStyle);
	connect(curveStyleComboBox_, SIGNAL(activated(const QString&)), this, SLOT(setCurveStyle(const QString&)));

	connect(showPlotPushButton_, SIGNAL(clicked()), this, SLOT(showPlot()));
	connect(exportPlotPushButton_, SIGNAL(clicked()), this, SLOT(exportPlot()));
	connect(dataColorPushButton_, SIGNAL(clicked()), this, SLOT(assignDataColor()));
  connect(backgroundColorPushButton_, SIGNAL(clicked()), this, SLOT(assignBackgroundColor()));
  connect(dataSeriesComboBox_, SIGNAL(activated(int)), this, SLOT(switchDataSeries(int)));
  connect(dataLineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(assignDataLabel(const QString&)));
  dataSeriesComboBox_->setDisabled(true);

  plotDialog_ = new PlotDialog(this);

	//broken in Qt5
	backgroundColorLabel_->hide();
	backgroundColorPushButton_->hide();
}

BasicPlotterDialog::~BasicPlotterDialog()
{
	delete plotDialog_;
}

void BasicPlotterDialog::pullSpecial()
{
  dataColors_ = colorsFromState(Parameters::PlotColors);
  dataColors_.resize(labelColorMax_);
  dataLabels_ = state_->getValue(Parameters::DataTitle).toVector();
  dataLabels_.resize(labelColorMax_);
  {
    ScopedWidgetSignalBlocker q(dataLineEdit_);
    dataLineEdit_->setText(QString::fromStdString(dataLabels_[dataSeriesIndex_].toString()));
  }
  backgroundColor_ = colorFromState(Parameters::PlotBackgroundColor);
  plotDialog_->plot()->setCanvasBackground(backgroundColor_);

	if (plotDialog_->isVisible())
		updatePlot();
}

void BasicPlotterDialog::showPlot()
{
	updatePlot();
	plotDialog_->show();
	plotDialog_->raise();
}

void BasicPlotterDialog::updatePlot()
{
  plotData();

  plotDialog_->updatePlot(titleLineEdit_->text(), xAxisLineEdit_->text(), yAxisLineEdit_->text(),
    boost::make_optional(horizontalAxisGroupBox_->isChecked(), horizontalAxisSpinBox_->value()),
    boost::make_optional(verticalAxisGroupBox_->isChecked(), verticalAxisSpinBox_->value()));
}

void BasicPlotterDialog::plotData()
{
  auto showPoints = showPointsCheckBox_->isChecked();
  auto data = transient_value_cast<DenseMatrixHandle>(state_->getTransientValue(Variables::InputMatrix));
	if (data)
	{
    auto plot = plotDialog_->plot();
		plot->clearCurves();
		plot->addCurve(data, dataLineEdit_->text(), dataColors_[dataSeriesIndex_], true, showPoints);
		plot->addLegend();
	}
}

void BasicPlotterDialog::assignDataColor()
{
  auto newColor = QColorDialog::getColor(dataColors_[dataSeriesIndex_], this, "Choose data color");
  if (newColor.isValid())
  {
		dataColors_[dataSeriesIndex_] = newColor;
    colorsToState(Parameters::PlotColors, dataColors_);
		updatePlot();
  }
}

void BasicPlotterDialog::assignBackgroundColor()
{
  auto newColor = QColorDialog::getColor(backgroundColor_, this, "Choose background color");
  if (newColor.isValid())
  {
		backgroundColor_ = newColor;
    colorToState(Parameters::PlotBackgroundColor, backgroundColor_);
    plotDialog_->plot()->setCanvasBackground(backgroundColor_);
  }
}

void BasicPlotterDialog::assignDataLabel(const QString& label)
{
  dataLabels_[dataSeriesIndex_].setValue(label.toStdString());
  state_->setValue(Parameters::DataTitle, dataLabels_);
  updatePlot();
}

void BasicPlotterDialog::exportPlot()
{
  plotDialog_->plot()->exportPlot();
}

void BasicPlotterDialog::switchDataSeries(int index)
{
  dataSeriesIndex_ = index;
  {
    ScopedWidgetSignalBlocker q(dataLineEdit_);
    dataLineEdit_->setText(QString::fromStdString(dataLabels_[dataSeriesIndex_].toString()));
  }
}

void BasicPlotterDialog::setCurveStyle(const QString& style)
{
	plotDialog_->plot()->setCurveStyle(style);
}

void BasicPlotterDialog::updateFromPortChange(int, const std::string&, DynamicPortChange type)
{
  if (type == DynamicPortChange::INITIAL_PORT_CONSTRUCTION)
    return;

	if (type == DynamicPortChange::USER_REMOVED_PORT)
	{
		state_->setTransientValue(Variables::InputMatrix, nullptr);
		plotDialog_->plot()->clearCurves();
    auto log = ModuleLog::Instance().get();
    if (log)
      log->warn("[{}] Curves and plot data cleared. Re-execute to replot connected data.", windowTitle().toStdString());
	}
}

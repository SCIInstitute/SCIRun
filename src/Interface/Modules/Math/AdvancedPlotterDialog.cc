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


#include <Interface/Modules/Math/AdvancedPlotterDialog.h>
#include <Interface/Modules/Math/PlotDialog.h>
#include <Modules/Math/BasicPlotter.h>
#include <Modules/Math/AdvancedPlotter.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

AdvancedPlotterDialog::AdvancedPlotterDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: BasicPlotterDialog(name, state, parent)
{
  dataSeriesComboBox_->setDisabled(false);
}

void AdvancedPlotterDialog::plotData()
{
  auto independents = transient_value_cast<std::vector<DenseMatrixHandle>>(state_->getTransientValue(Parameters::IndependentVariablesVector));
  auto dependents = transient_value_cast<std::vector<DenseMatrixHandle>>(state_->getTransientValue(Parameters::DependentVariablesVector));
  auto plot = plotDialog_->plot();
  auto showPoints = showPointsCheckBox_->isChecked();
  plot->clearCurves();
	plot->removeLegend();
  bool addLegend = true;
  for (auto&& tup : zip(independents, dependents))
  {
    DenseMatrixHandle x, y;
    boost::tie(x, y) = tup;
		bool useFirstXColumnOnly = x->ncols() != y->ncols();
		auto independentColumn = [useFirstXColumnOnly](int c)
		{
			return useFirstXColumnOnly ? 0 : c;
		};
    for (int c = 0; c < y->ncols(); ++c)
      plot->addCurve(x->col(independentColumn(c)), y->col(c), QString::fromStdString(dataLabels_[c % labelColorMax_].toString()),
        dataColors_[c % labelColorMax_], c < labelColorMax_, showPoints);
    if (y->ncols() > labelColorMax_)
      addLegend = false;
  }
  if (addLegend)
    plot->addLegend();
}

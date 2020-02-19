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


#include <Interface/Modules/Math/SolveLinearSystemDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

#if 0 //TODO: make compiler symbol for WITH_QWT_WIDGETS
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#endif

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;


namespace SCIRun {
  namespace Gui {
    class SolveLinearSystemDialogImpl
    {
    public:
      SolveLinearSystemDialogImpl()
      {
        solverNameLookup_.insert(StringPair("Conjugate Gradient (SCI)", "cg"));
        solverNameLookup_.insert(StringPair("BiConjugate Gradient (SCI)", "bicg"));
        solverNameLookup_.insert(StringPair("Jacobi (SCI)", "jacobi"));
        solverNameLookup_.insert(StringPair("MINRES (SCI)", "minres"));
      }
      GuiStringTranslationMap solverNameLookup_;
    };
  }}

SolveLinearSystemDialog::SolveLinearSystemDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), impl_(new SolveLinearSystemDialogImpl)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addSpinBoxManager(maxIterationsSpinBox_, Variables::MaxIterations);
  addDoubleSpinBoxManager(targetErrorSpinBox_, Variables::TargetError);

#if 0 //TODO: make compiler symbol for WITH_QWT_WIDGETS
  //TODO: fix parenting, all these objects leak
	QwtPlot *myPlot = this->qwtPlot;
	QwtLegend *myLegend = new QwtLegend;
	QwtPlotCurve *curve1 = new QwtPlotCurve("Current Target");
	QwtPlotCurve *curve2 = new QwtPlotCurve("Target Error");
	QwtPlotCurve *curve3 = new QwtPlotCurve("CurrentError");
	double x[5] = { 0, 10, 15, 20, 25 };
	double y[5] = { 0, 2, 4, 16, 64 };

	curve1->setSamples(&x[0], &y[0], (size_t)5);
	curve1->setPen(Qt::blue);
	curve1->setLegendAttribute(QwtPlotCurve::LegendShowLine);
	curve1->setItemAttribute(QwtPlotItem::Legend, true);

	curve2->setPen(Qt::darkRed);
	curve2->setLegendAttribute(QwtPlotCurve::LegendShowLine);
	curve2->setItemAttribute(QwtPlotItem::Legend, true);

	curve3->setPen(Qt::darkGreen);
	curve3->setLegendAttribute(QwtPlotCurve::LegendShowLine);
	curve3->setItemAttribute(QwtPlotItem::Legend, true);

	myPlot->setAxisTitle(2, QString("Iteration"));
	myPlot->setAxisTitle(0, QString("Error (RMS)"));
	myPlot->setTitle(QString("Convergence"));
	myPlot->insertLegend(myLegend, QwtPlot::RightLegend);

	curve1->attach(myPlot);
	myPlot->replot();
	myPlot->show();
#endif

  addComboBoxManager(preconditionerComboBox_, Variables::Preconditioner);
  addComboBoxManager(methodComboBox_, Variables::Method, impl_->solverNameLookup_);
}

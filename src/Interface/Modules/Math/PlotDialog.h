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

#ifndef INTERFACE_MODULES_MATH_PLOTDIALOG_H
#define INTERFACE_MODULES_MATH_PLOTDIALOG_H

#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <qwt_plot.h>
#include <Interface/Modules/Math/share.h>

class QwtPlotMarker;
class QwtPlotCurve;
class QwtPlotMagnifier;

namespace SCIRun
{
  namespace Gui
  {
    class Plot;

    class PlotDialog : public QDialog
    {
    public:
      explicit PlotDialog(QWidget* parent = nullptr);
      void updatePlot(const QString& title, const QString& xAxis, const QString& yAxis,
        const boost::optional<double>& horizAxisOpt,
        const boost::optional<double>& vertAxisOpt);
      ~PlotDialog();
      Plot* plot() { return plot_; }
    private:
      Plot* plot_{nullptr};
    };

    class Plot : public QwtPlot
    {
      Q_OBJECT
    public:
      explicit Plot( QWidget *parent = nullptr );
      void makeVerticalAxis(bool show, double position);
      void makeHorizontalAxis(bool show, double position);

      void addCurve(Core::Datatypes::DenseMatrixHandle data, const QString& title, const QColor& color, bool showLegend, bool showPoints);
      template <typename Column>
      void addCurve(const Column& x, const Column& y, const QString& title, const QColor& color, bool showLegend, bool showPoints)
      {
        addCurveImpl(makePoints(x, y), title, color, showLegend, showPoints);
      }
      void clearCurves();
      void addLegend();
      void exportPlot();
    public Q_SLOTS:
      void adjustZoom(const QString& type);
    private Q_SLOTS:
      void showItem(const QVariant&, bool on);
    private:
      template <typename Column>
      QPolygonF makePoints(const Column& x, const Column& y)
      {
        QPolygonF points;
        for (int i = 0; i < x.size(); ++i)
        {
          points << QPointF(x(i), y(i));
        }
        return points;
      }
      template <typename Column>
      void setBounds(const Column& x, const Column& y)
      {
        double maxX = x.maxCoeff();
        double maxY = y.maxCoeff();
        double minX = x.minCoeff();
        double minY = y.minCoeff();
        setAxisScale( xBottom, minX, maxX );
        setAxisScale( yLeft, minY, maxY );
      }
      void addCurveImpl(const QPolygonF& points, const QString& title, const QColor& color, bool showLegend, bool showPoints);

      QwtPlotMarker* verticalAxis_ {nullptr};
      QwtPlotMarker* horizontalAxis_ {nullptr};
      QwtPlotMagnifier* magnifier_ {nullptr};
      std::vector<QwtPlotCurve*> curves_;
    };
  }
}
#endif

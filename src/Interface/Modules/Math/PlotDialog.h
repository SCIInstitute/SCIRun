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


#ifndef INTERFACE_MODULES_MATH_PLOTDIALOG_H
#define INTERFACE_MODULES_MATH_PLOTDIALOG_H

#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
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
      Q_OBJECT
    public:
      explicit PlotDialog(QWidget* parent = nullptr);
      void updatePlot(const QString& title, const QString& xAxis, const QString& yAxis,
        const boost::optional<double>& horizAxisOpt,
        const boost::optional<double>& vertAxisOpt);
      ~PlotDialog();
      Plot* plot() { return plot_; }
    public Q_SLOTS:
      void message(const QString& s);
    private:
      Plot* plot_{nullptr};
      QStatusBar* statusBar_{nullptr};
    };

    struct PointLess
    {
      bool operator()(const QPointF& p1, const QPointF& p2) const
      {
        if (p1.x() == p2.x())
          return p1.y() < p2.y();
        return p1.x() < p2.x();
      }
    };

    using PlotPointMap = std::map<QPointF, int, PointLess>;

    class SpecialMapPlotCanvas : public QwtPlotCanvas
    {
      Q_OBJECT
    public:
      explicit SpecialMapPlotCanvas(const PlotPointMap& pointCurveMap, QwtPlot* plot = nullptr)
        : QwtPlotCanvas(plot), pointCurveMap_(pointCurveMap)
      {}
    Q_SIGNALS:
      void curveSelected(int index, const QString& message);
    protected:
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
    private:
      const PlotPointMap& pointCurveMap_;
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
      void removeLegend();
      void exportPlot();
      void setCurveStyle(const QString& style);
    public Q_SLOTS:
      void adjustZoom(const QString& type);
      void highlightCurve(int index, const QString& message);
    Q_SIGNALS:
      void hasMessage(const QString& message);
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
      void updateCurveStyle(QwtPlotCurve* curve);

      QwtPlotMarker* verticalAxis_ {nullptr};
      QwtPlotMarker* horizontalAxis_ {nullptr};
      QwtPlotMagnifier* magnifier_ {nullptr};
      std::vector<QwtPlotCurve*> curves_;

      PlotPointMap pointCurveMap_;
      int justSelected_{-1};
      QPen previousPen_;
      QString curveStyle_;
    };
  }
}
#endif

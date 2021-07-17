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


#ifndef INTERFACE_MODULES_CREATESTANDARDCOLORMAPDIALOG_H
#define INTERFACE_MODULES_CREATESTANDARDCOLORMAPDIALOG_H

#include "Interface/Modules/Visualization/ui_CreateStandardColorMap.h"
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Core/Datatypes/ColorMap.h>
#include <Interface/Modules/Visualization/share.h>

namespace SCIRun {
  namespace Gui {
    class ColorMapPreviewPoint : public QGraphicsEllipseItem
    {
    public:
      ColorMapPreviewPoint(qreal x, qreal y);
      ColorMapPreviewPoint(const QPointF& p);
      QPointF center() const { return center_; }
    private:
      QPointF center_;
    };

    struct SortedByXCoordinate
    {
      bool operator()(const QPointF& p1, const QPointF& p2) const
      {
        return p1.x() < p2.x();
      }
      bool operator()(const ColorMapPreviewPoint* p1, const ColorMapPreviewPoint* p2) const
      {
        return p1->center().x() < p2->center().x();
      }
    };

    using LogicalAlphaPointSet = std::set<QPointF, SortedByXCoordinate>;
    using PreviewPointSet = std::set<ColorMapPreviewPoint*, SortedByXCoordinate>;

    class AlphaFunctionManager
    {
    public:
      AlphaFunctionManager(const boost::atomic<bool>& pulling);
      void clear();
      void insert(const QPointF& p);
      void erase(const QPointF& p);
      size_t size() const;
      bool alreadyExists(const QPointF& p) const;
      bool empty() const;
      bool equals(const LogicalAlphaPointSet& other) const { return other == alphaPoints_; }
    private:
      LogicalAlphaPointSet alphaPoints_;
      static const size_t ALPHA_SAMPLES = 10; //TODO cbright: once alpha values are visible, increase this number
      static const size_t ALPHA_VECTOR_LENGTH = ALPHA_SAMPLES + 2; // 0.5 added on both ends
      const boost::atomic<bool>& dialogPulling_;
      void printSet() const;
    public:
      auto begin() const -> decltype(alphaPoints_.begin()) { return alphaPoints_.begin(); }
      auto end() const -> decltype(alphaPoints_.end()) { return alphaPoints_.end(); }
      std::pair<double, double> getPointRange(const QPointF& p);
    };

    class ColormapPreview : public QGraphicsView
    {
    Q_OBJECT
    public:
      explicit ColormapPreview(QGraphicsScene* scene,
        SCIRun::Dataflow::Networks::ModuleStateHandle state,
        const boost::atomic<bool>& pulling, QWidget* parent = nullptr);
      void addDefaultLine();
      ColorMapPreviewPoint* getItemAtPoint(const QPointF& point) const;
      void addPoint(const QPointF& point);
      void addPoint(const QPointF& point, std::pair<double, double>& range);
      void updateSize();
      void addPointsAndLineFromFile(const LogicalAlphaPointSet& pointsToLoad);
      void updateFromState();
      void drawAlphaPolyline();
      void redraw();
      void deletePointItems();
      LogicalAlphaPointSet getPointsFromState();
      Core::Algorithms::Variable::List getPointsAsVariableList();
    public Q_SLOTS:
      void clearAlphaPointGraphics();
    Q_SIGNALS:
      void clicked(int x, int y);
    protected:
      void mousePressEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    private:
      double convertToAlphaValue(double v);
      void removeCurrentLine();
      void removePoint(const QPointF& point);
      void removeItem(ColorMapPreviewPoint* item);
      QPointF& flipYAxis(QPointF& pos) const;
      QPointF previewToAlphaPoint(QPointF pos) const;
      QPointF alphaToPreviewPoint(QPointF pos) const;
      QPointF& scalePointToWindow(QPointF& p) const;
      QPointF& normalizePoint(QPointF& pos) const;
      double clickableWidth() const;
      double clickableHeight() const;
      void pushToState();

      SharedPointer<QGraphicsPathItem> alphaPath_;
      SharedPointer<ColorMapPreviewPoint> selectedPoint_;
      std::pair<double, double> selectedPointRange_;
      AlphaFunctionManager alphaManager_;
      const boost::atomic<bool>& dialogPulling_;
      const static int BORDER_SIZE_ = 3;
      PreviewPointSet previewPoints_;
      Dataflow::Networks::ModuleStateHandle state_;
    };

    class SCISHARE CreateStandardColorMapDialog : public ModuleDialogGeneric,
      public Ui::CreateStandardColorMap
    {
      Q_OBJECT

    public:
      CreateStandardColorMapDialog(const std::string& name,
        SCIRun::Dataflow::Networks::ModuleStateHandle state,
        QWidget* parent = nullptr);
    protected:
      void pullSpecial() override;
      void resizeEvent(QResizeEvent *event) override;
      void showEvent(QShowEvent *event) override;
    private Q_SLOTS:
      void selectCustomColorMin();
      void selectCustomColorMax();
      void updateColorMapPreview();
      QString buildGradientString(const SCIRun::Core::Datatypes::ColorMap& cm) const;
      void setShiftSlider(double d);
      void setResolutionSlider(int i);
      void setShiftSpinner(int i);
      void onInvertCheck(bool b);
    private:
      QGraphicsScene* scene_;
      ColormapPreview* previewColorMap_;
      QColor customColors_[2] = {{0, 0, 0}, {0, 0, 0}};
    };
  }
}

#endif

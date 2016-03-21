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

#ifndef INTERFACE_MODULES_CREATESTANDARDCOLORMAPDIALOG_H
#define INTERFACE_MODULES_CREATESTANDARDCOLORMAPDIALOG_H

#include "Interface/Modules/Visualization/ui_CreateStandardColorMap.h"
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Core/Datatypes/ColorMap.h>
#include <Interface/Modules/Visualization/share.h>

namespace SCIRun {
  namespace Gui {

    struct SortedByXCoordinate
    {
      bool operator()(const QPointF& p1, const QPointF& p2) const
      {
        return p1.x() < p2.x();
      }
    };

    class AlphaFunctionManager
    {
    public:
      AlphaFunctionManager(const QPointF& start, const QPointF& end, SCIRun::Dataflow::Networks::ModuleStateHandle state, const boost::atomic<bool>& pulling);
      void clear();
      void insertEndpoints();
      void insert(const QPointF& p);
      bool alreadyExists(const QPointF& p) const;
    private:
      void updateAlphaFunction();
      void pushToState();
      std::pair<QPointF,QPointF> alphaLineEndpointsAtColor(double color) const;
      double pointYToAlpha(double y) const;
      QPointF colorToPoint(double color) const;
      double interpolateAlphaLineValue(const QPointF& leftEndpoint, const QPointF& rightEndpoint, double color) const;

      SCIRun::Dataflow::Networks::ModuleStateHandle state_;
      std::set<QPointF, SortedByXCoordinate> alphaPoints_;
      const size_t ALPHA_SAMPLES = 10;
      const size_t ALPHA_VECTOR_LENGTH = ALPHA_SAMPLES + 2; // 0.5 added on both ends
      const double DEFAULT_ALPHA = 0.5;
      const QPointF defaultStart_;
      const QPointF defaultEnd_;
      std::vector<double> alphaFunction_;
      const boost::atomic<bool>& dialogPulling_;
    public:
      auto begin() const -> decltype(alphaPoints_.begin()) { return alphaPoints_.begin(); }
      auto end() const -> decltype(alphaPoints_.end()) { return alphaPoints_.end(); }
    };

    class ColormapPreview : public QGraphicsView
    {
    Q_OBJECT
    public:
      explicit ColormapPreview(QGraphicsScene* scene,
        SCIRun::Dataflow::Networks::ModuleStateHandle state,
        const boost::atomic<bool>& pulling,
        QWidget* parent = nullptr);
    void addDefaultLine();
    void addPoint(const QPointF& point);
    void addEndpoints() { alphaManager_.insertEndpoints(); }
    public Q_SLOTS:
      void clearAlphaPointGraphics();
    Q_SIGNALS:
      void clicked(int x, int y);
    protected:
      virtual void mousePressEvent(QMouseEvent* event) override;
    private:
      void removeDefaultLine();
      void drawAlphaPolyline();

      QGraphicsItem* alphaPath_;
      const QPointF defaultStart_;
      const QPointF defaultEnd_;
      AlphaFunctionManager alphaManager_;
      const boost::atomic<bool>& dialogPulling_;
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
      virtual void pullSpecial() override;
    private Q_SLOTS:
      void updateColorMapPreview();
      void updateColorMapPreview(const QString& s);
      QString buildGradientString(const SCIRun::Core::Datatypes::ColorMap& cm) const;
      void setShiftSlider(double d);
      void setResolutionSlider(int i);
      void setShiftSpinner(int i);
      void onInvertCheck(bool b);
    private:
      QGraphicsScene* scene_;
      ColormapPreview* previewColorMap_;
    };
  }
}

#endif

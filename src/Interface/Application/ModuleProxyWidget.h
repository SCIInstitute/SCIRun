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


#ifndef INTERFACE_APPLICATION_MODULEPROXYWIDGET_H
#define INTERFACE_APPLICATION_MODULEPROXYWIDGET_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Interface/Application/Note.h>
#include <QGraphicsProxyWidget>

class QTimeLine;

namespace SCIRun
{
  namespace Gui
  {
    class ModuleWidget;

    class LoopDiamondPolygon : public QGraphicsPolygonItem
    {
    public:
      explicit LoopDiamondPolygon(QGraphicsItem* parent = nullptr);
      void mouseMoveEventPublic(QGraphicsSceneMouseEvent *event)
      {
        mouseMoveEvent(event);
      }
    private:
      QPolygonF shape_;
    };

    class ModuleProxyWidget : public QGraphicsProxyWidget, public NoteDisplayHelper
    {
	    Q_OBJECT

    public:
      explicit ModuleProxyWidget(ModuleWidget* module, QGraphicsItem* parent = nullptr);
      ~ModuleProxyWidget();
      ModuleWidget* getModuleWidget();
      void createStartupNote();
      void adjustHeight(int delta);
      void adjustWidth(int delta);
      void setBackgroundPolygon(LoopDiamondPolygon* p);

      //TODO: move to utility
      static void ensureItemVisible(QGraphicsItem* item);

    public Q_SLOTS:
      void highlightIfSelected();
      void setDefaultNotePosition(NotePosition position);
      void setDefaultNoteSize(int size);
      void createPortPositionProviders();
      void snapToGrid();
      void highlightPorts(int state);
      void ensureThisVisible();
      void showAndColor(const QColor& color);
      void updateNote(const Note& note);

    Q_SIGNALS:
      void selected();
      void widgetMoved(const SCIRun::Dataflow::Networks::ModuleId& id, double newX, double newY);
      void tagChanged(int tag);
    protected:
      void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
      void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
      void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
      void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
      void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
      QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    private Q_SLOTS:
      void disableModuleGUI(bool disabled);
      void loadAnimate(qreal val);
      void colorAnimate(qreal val);
      void findInNetwork();
    private:
      void showAndColorImpl(const QColor& color, int milliseconds);
      bool isSubwidget(QWidget* alienWidget) const;
      void updatePressedSubWidget(QGraphicsSceneMouseEvent* event);

      ModuleWidget* module_;
      QColor animateColor_;
      bool grabbedByWidget_, isSelected_;
      QWidget* pressedSubWidget_;
      QPointF position_;
      QPointF cachedPosition_;
      bool doHighlight_;
      int stackDepth_;
      QSizeF originalSize_;
      QTimeLine* timeLine_;
      QGraphicsEffect* previousEffect_{nullptr};
      LoopDiamondPolygon* backgroundShape_ {nullptr};
    };

    class SubnetPortsBridgeProxyWidget : public QGraphicsProxyWidget
    {
    public:
      explicit SubnetPortsBridgeProxyWidget(class SubnetPortsBridgeWidget* ports, QGraphicsItem* parent = nullptr);
      void updateConnections();
    private:
      class SubnetPortsBridgeWidget* ports_;
    };
  }
}

//#define MODULE_POSITION_LOGGING

#endif

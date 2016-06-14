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

#ifndef INTERFACE_APPLICATION_MODULEPROXYWIDGET_H
#define INTERFACE_APPLICATION_MODULEPROXYWIDGET_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Interface/Application/Note.h>
#include <QGraphicsProxyWidget>

namespace SCIRun
{
  namespace Gui
  {
    class ModuleWidget;

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

    public Q_SLOTS:
      void highlightIfSelected();
      void setDefaultNotePosition(NotePosition position);
      void createPortPositionProviders();
      void snapToGrid();
      void highlightPorts(int state);

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
      virtual void setNoteGraphicsContext() override;
    private Q_SLOTS:
      void updateNote(const Note& note);
      void ensureThisVisible();
      void disableModuleGUI(bool disabled);
    private:
      void ensureItemVisible(QGraphicsItem* item);
      bool isSubwidget(QWidget* alienWidget) const;
      void updatePressedSubWidget(QGraphicsSceneMouseEvent* event);

      ModuleWidget* module_;
      bool grabbedByWidget_, isSelected_;
      QWidget* pressedSubWidget_;
      QPointF position_;
      QPointF cachedPosition_;
      bool doHighlight_;
      int stackDepth_;
      QSizeF originalSize_;
    };

    // arbitrary values
    static const int TagDataKey = 123;
    static const int TagLayerKey = 100;
    static const int CurrentTagKey = 101;
    static const int NoTag = -1;
    static const int AllTags = -50;
    static const int ShowGroups = -100;
    static const int HideGroups = -101;
  }
}

#endif

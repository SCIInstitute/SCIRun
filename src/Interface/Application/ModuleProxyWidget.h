/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef MODULEPROXY_H
#define MODULEPROXY_H

#include <QGraphicsProxyWidget>

namespace SCIRun
{
  namespace Gui
  {

    class Module;

class ModuleProxyWidget : public QGraphicsProxyWidget
{
	Q_OBJECT
	
public:
  explicit ModuleProxyWidget(Module* module, QGraphicsItem* parent = 0);
  void createPortPositionProviders();
public slots:
  void highlightIfSelected();
signals:
  void selected();
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);


  //void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
  //void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
  //void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
  //void dropEvent(QGraphicsSceneDragDropEvent *event);


  QVariant itemChange(GraphicsItemChange change, const QVariant& value);
private:
  bool isSubwidget(QWidget* alienWidget) const;
  void updatePressedSubWidget(QGraphicsSceneMouseEvent* event);
  void addPort();
  
  Module* module_;
  bool grabbedByWidget_;
  QWidget* pressedSubWidget_;
  //int backupZ_;
};

  }
}

#endif
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


/// author James Hughes
/// date   September 2012
/// brief  Not sure this file should go in Modules/Render. But it is an
///        auxiliary file to the ViewScene render module.

#ifndef INTERFACE_MODULES_GLWIDGET_H
#define INTERFACE_MODULES_GLWIDGET_H

// For windows.
/// \todo Make this definition specific to windows.
#define NOMINMAX

#ifndef Q_MOC_RUN
#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#endif
#include <QOpenGLWidget>

namespace SCIRun {
namespace Gui {

class QtGLContext;

class GLWidget : public QOpenGLWidget
{
  Q_OBJECT

public:
  GLWidget(QWidget* parent);
  ~GLWidget();

  Render::RendererPtr getSpire() const {return graphics_;}

  /// Required function for single threaded interfaces that have multiple
  /// contexts running on the same thread.

  void setLockZoom(bool lock);
  void setLockPanning(bool lock);
  void setLockRotation(bool lock);
  void requestFrame() {frameRequested_ = true;}

Q_SIGNALS:
  void fatalError(const QString& message);
  void finishedFrame();

public Q_SLOTS:
  // Only use when not using threading.
  void updateRenderer();

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;
  void closeEvent(QCloseEvent *evt) override;

private:
  Render::RendererPtr                   graphics_          {};  ///< Interface to spire.
  QTimer*                               timer_             {};
  bool                                  frameRequested_    {false};
  double                                frameTime_         {0.0};
};

} // end of namespace Gui
} // end of namespace SCIRun

#endif // SPIRE_GLWIDGET_H

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

/// \author James Hughes
/// \date   September 2012
/// \brief  Not sure this file should go in Modules/Render. But it is an
///         auxiliary file to the ViewScene render module.

#include <Interface/Modules/Render/GLWidget.h>
#include <iostream>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QtDebug>
#include <Core/Application/Application.h>

namespace SCIRun {
namespace Gui {

const int RendererUpdateInMS = 35;
const double updateTime = RendererUpdateInMS / 1000.0;

//------------------------------------------------------------------------------
GLWidget::GLWidget(QtGLContext* context, QWidget* parent) :
    QGLWidget(context, parent),
    mContext(new GLContext(this)),
    mCurrentTime(0.0)
{
  /// \todo Implement this intelligently. This function is called everytime
  ///       there is a new graphics context.
  std::vector<std::string> shaderSearchDirs;

  mContext->makeCurrent();

  // Call gl platform init.
  CPM_GL_PLATFORM_NS::glPlatformInit();
  
  auto frameInitLimitFromCommandLine = Core::Application::Instance().parameters()->frameInitLimit();
  if (frameInitLimitFromCommandLine)
  {
    std::cout << "Renderer frame init limit changed to " << *frameInitLimitFromCommandLine << std::endl;
  }
  const int frameInitLimit = frameInitLimitFromCommandLine.get_value_or(100);

  mGraphics.reset(new Render::SRInterface(mContext, frameInitLimit));

  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(RendererUpdateInMS);

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
GLWidget::~GLWidget()
{
  // Need to inform module that the context is being destroyed.
  if (mGraphics != nullptr)
  {
    //std::cout << "Terminating spire." << std::endl;
    mGraphics.reset();
  }
}

//------------------------------------------------------------------------------
void GLWidget::initializeGL()
{
}

//------------------------------------------------------------------------------
SCIRun::Render::SRInterface::MouseButton GLWidget::getSpireButton(QMouseEvent* event)
{
  SCIRun::Render::SRInterface::MouseButton btn = SCIRun::Render::SRInterface::MOUSE_NONE;
  if (event->buttons() & Qt::LeftButton)
    btn = Render::SRInterface::MOUSE_LEFT;
  else if (event->buttons() & Qt::RightButton)
    btn = Render::SRInterface::MOUSE_RIGHT;
  else if (event->buttons() & Qt::MidButton)
    btn = Render::SRInterface::MOUSE_MIDDLE;

  return btn;
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  // Extract appropriate key.
  SCIRun::Render::SRInterface::MouseButton btn = getSpireButton(event);
  mGraphics->inputMouseMove(glm::ivec2(event->x(), event->y()), btn);
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  Q_EMIT mousePressSignalForTestingGeometryObjectFeedback(event->x(), event->y());
  SCIRun::Render::SRInterface::MouseButton btn = getSpireButton(event);
  mGraphics->inputMouseDown(glm::ivec2(event->x(), event->y()), btn);
  event->ignore();
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
  SCIRun::Render::SRInterface::MouseButton btn = getSpireButton(event);
  mGraphics->inputMouseUp(glm::ivec2(event->x(), event->y()), btn);
  event->ignore();
}

//------------------------------------------------------------------------------
void GLWidget::wheelEvent(QWheelEvent * event)
{
  mGraphics->inputMouseWheel(event->delta());
}

//------------------------------------------------------------------------------
void GLWidget::keyPressEvent(QKeyEvent* event)
{
  std::cout << "key down" << std::endl;
  mGraphics->inputShiftKeyDown(event->key() == Qt::Key_Shift);
  event->ignore();
}

//------------------------------------------------------------------------------
void GLWidget::keyReleaseEvent(QKeyEvent* event)
{
  mGraphics->inputShiftKeyDown(false);
  event->ignore();
}

//------------------------------------------------------------------------------
void GLWidget::resizeGL(int width, int height)
{
  mGraphics->eventResize(static_cast<size_t>(width),
                         static_cast<size_t>(height));
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  //qDebug() << "Close event for window.";
  if (mGraphics != nullptr)
  {
    //std::cout << "Terminating spire." << std::endl;
    mGraphics.reset();
  }
  QGLWidget::closeEvent(evt);
}

//------------------------------------------------------------------------------
void GLWidget::makeCurrent()
{
  mContext->makeCurrent();
}

//------------------------------------------------------------------------------
void GLWidget::updateRenderer()
{
  mCurrentTime += updateTime;

  try
  {
    mGraphics->doFrame(mCurrentTime, updateTime);
    mContext->swapBuffers();
  }
  catch (const SCIRun::Render::SRInterfaceFailure& e)
  {
    Q_EMIT fatalError(e.what());
    mTimer->stop();
  }
}

} // namespace Gui
} // namespace SCIRun

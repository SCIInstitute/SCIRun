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

/// \author James Hughes
/// \date   September 2012
/// \brief  Not sure this file should go in Modules/Render. But it is an 
///         auxiliary file to the ViewScene render module.

#include <iostream>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QtDebug>
#include <Core/Application/Application.h>

#include <Interface/Modules/Render/GLWidget.h>

using namespace SCIRun::Gui;

//------------------------------------------------------------------------------
GLWidget::GLWidget(QtGLContext* context) :
    QGLWidget(context),
    mContext(new GLContext(this))
{
  /// \todo Implement this intelligently. This function is called everytime
  ///       there is a new graphics context.
  std::vector<std::string> shaderSearchDirs;

  mContext->makeCurrent();

  // Call gl platform init.
  CPM_GL_PLATFORM_NS::glPlatformInit();
  
  auto shadersInBinDirectory = SCIRun::Core::Application::Instance().executablePath() / "Shaders";
  shaderSearchDirs.push_back(shadersInBinDirectory.string());

  mGraphics = std::shared_ptr<SRInterface>(
      new SRInterface(mContext, shaderSearchDirs));
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(35);

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
SRInterface::MouseButton GLWidget::getSpireButton(QMouseEvent* event)
{
  SRInterface::MouseButton btn = SRInterface::MOUSE_NONE;
  if (event->buttons() & Qt::LeftButton)
    btn = SRInterface::MOUSE_LEFT;
  else if (event->buttons() & Qt::RightButton)
    btn = SRInterface::MOUSE_RIGHT;
  else if (event->buttons() & Qt::MidButton)
    btn = SRInterface::MOUSE_MIDDLE;
  
  return btn;
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  // Extract appropriate key.
  SRInterface::MouseButton btn = getSpireButton(event);
  mGraphics->inputMouseMove(glm::ivec2(event->x(), event->y()), btn);
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  SRInterface::MouseButton btn = getSpireButton(event);
  mGraphics->inputMouseDown(glm::ivec2(event->x(), event->y()), btn);
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
  SRInterface::MouseButton btn = getSpireButton(event);
  mGraphics->inputMouseUp(glm::ivec2(event->x(), event->y()), btn);
}

//------------------------------------------------------------------------------
void GLWidget::wheelEvent(QWheelEvent * event)
{
  mGraphics->inputMouseWheel(event->delta());
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
  qDebug() << "Close event for window.";
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
  mGraphics->doFrame();
  mContext->swapBuffers();
}


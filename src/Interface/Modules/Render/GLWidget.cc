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
#include <ctime>

using namespace SCIRun;
using namespace Core::Datatypes;

namespace SCIRun {
namespace Gui {

const int RendererUpdateInMS = 1000 / 60;
const double updateTime = RendererUpdateInMS / 1000.0;

//------------------------------------------------------------------------------
GLWidget::GLWidget(QWidget* parent) :
    QOpenGLWidget(parent)
{
  mGraphics.reset(new Render::SRInterface());

  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(RendererUpdateInMS);}

//------------------------------------------------------------------------------
GLWidget::~GLWidget()
{
  // Need to inform module that the context is being destroyed.
  if (mGraphics != nullptr)
  {
    mGraphics.reset();
  }
}

//------------------------------------------------------------------------------
void GLWidget::initializeGL()
{
	spire::glPlatformInit();
}

void GLWidget::paintGL()
{
  //set to 200ms to force promise fullfilment every frame if a good frame as been requested
  double lUpdateTime = mFrameRequested ? 0.2 : updateTime;
  mCurrentTime += lUpdateTime;
  mGraphics->doFrame(mCurrentTime, lUpdateTime);

  if(mFrameRequested && !mGraphics->hasShaderPromise())
  {
    mFrameRequested = false;
    finishedFrame();
  }
}

//------------------------------------------------------------------------------
MouseButton GLWidget::getSpireButton(QMouseEvent* event)
{
  auto btn = MouseButton::NONE;
  if (event->buttons() & Qt::LeftButton)
    btn = MouseButton::LEFT;
  else if (event->buttons() & Qt::RightButton)
    btn = MouseButton::RIGHT;
  else if (event->buttons() & Qt::MidButton)
    btn = MouseButton::MIDDLE;

  return btn;
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  // Extract appropriate key.
  auto btn = getSpireButton(event);
  mGraphics->inputMouseMove(btn, glm::ivec2(event->x(), event->y()));
  event->ignore();
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  makeCurrent();
  event->ignore();
}

//------------------------------------------------------------------------------
void GLWidget::wheelEvent(QWheelEvent * event)
{
  mGraphics->inputMouseWheel(event->delta());
  event->ignore();
}

//------------------------------------------------------------------------------
void GLWidget::keyPressEvent(QKeyEvent* event)
{
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
  makeCurrent();
  mGraphics->eventResize(static_cast<size_t>(width),
                         static_cast<size_t>(height));
  //updateRenderer();
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  if (mGraphics != nullptr)
  {
    mGraphics.reset();
  }
  QOpenGLWidget::closeEvent(evt);
}

//------------------------------------------------------------------------------
void GLWidget::updateRenderer()
{
  if(isValid())
  {
    update();
  }
}

} // namespace Gui
} // namespace SCIRun

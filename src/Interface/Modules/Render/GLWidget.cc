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

#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <Core/Application/Application.h>

#include "GLWidget.h"

using namespace SCIRun::Gui;
using Spire::Vector2;

//------------------------------------------------------------------------------
GLWidget::GLWidget(const QGLFormat& format) :
    QGLWidget(format),
    mContext(new GLContext(this))
{
  /// \todo Implement this intelligently. This function is called everytime
  ///       there is a new graphics context.
  std::vector<std::string> shaderSearchDirs;
  
  auto shadersInBinDirectory = SCIRun::Core::Application::Instance().executablePath() / "Shaders";
  shaderSearchDirs.push_back(shadersInBinDirectory.string());

#ifdef SPIRE_USE_STD_THREADS
  mGraphics = std::shared_ptr<Spire::SCIRun::SRInterface>(
      new Spire::SCIRun::SRInterface(
          std::dynamic_pointer_cast<Spire::Context>(mContext),
          shaderSearchDirs, true));
#else
  mGraphics = std::shared_ptr<Spire::SCIRun::SRInterface>(
      new Spire::SCIRun::SRInterface(
          std::dynamic_pointer_cast<Spire::Context>(mContext),
          shaderSearchDirs, false));
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(35);
#endif

  /// \todo Where should we store common shader names?
  mGraphics->getStuPipe()->addPersistentShader(
      "UniformColor", 
      { {"UniformColor.vs", Spire::StuInterface::VERTEX_SHADER}, 
        {"UniformColor.fs", Spire::StuInterface::FRAGMENT_SHADER},
      });

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
GLWidget::~GLWidget()
{
  // Need to inform module that the context is being destroyed.
  std::cout << "Shutting down graphics." << std::endl;
  if (mGraphics != nullptr)
  {
    mGraphics->terminate();
    mGraphics.reset();
  }
}

//------------------------------------------------------------------------------
void GLWidget::initializeGL()
{
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mGraphics->inputMouseMove(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mGraphics->inputMouseDown(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mGraphics->inputMouseUp(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::wheelEvent(QWheelEvent * event)
{
  mGraphics->inputMouseWheel(event->delta());
}

//------------------------------------------------------------------------------
void GLWidget::resizeGL(int width, int height)
{
  mGraphics->eventResize(static_cast<int32_t>(width),
                         static_cast<int32_t>(height));
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  // Kill off the graphics thread.
  mGraphics->terminate();
  mGraphics.reset();
}

//------------------------------------------------------------------------------
void GLWidget::updateRenderer()
{
  mContext->makeCurrent();
  mGraphics->doFrame();
  mContext->swapBuffers();
}



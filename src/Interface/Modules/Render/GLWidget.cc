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

#include "GLWidget.h"

using namespace SCIRun::Gui;

//------------------------------------------------------------------------------
GLWidget::GLWidget(const QGLFormat& format) :
    QGLWidget(format),
    mContext(new GLContext(this))
{
  std::vector<std::string> shaderSearchDirs;

  // Create a threaded spire renderer. This should be created at the module
  // level once it has access to the context, should be passed using Transients.
  mGraphics = std::shared_ptr<Spire::SCIRun::SRInterface>(
      new Spire::SCIRun::SRInterface(
          std::dynamic_pointer_cast<Spire::Context>(mContext),
          shaderSearchDirs, true));

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
GLWidget::~GLWidget()
{
  // Need to inform module that the context is being destroyed.
  mGraphics.reset();
}

//------------------------------------------------------------------------------
void GLWidget::initializeGL()
{
  /// \todo Implement this intelligently. This function is called everytime
  ///       there is a new graphics context.
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  /// \todo Send mouse move event to SCIRun's spire interface.
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  //mLastMousePos.x = event->x();
  //mLastMousePos.y = event->y();

  /// \todo Send mouse down event to SCIRun's spire interface.
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
  /// \todo Send mouse up event to SCIRun's spire interface.
}

//------------------------------------------------------------------------------
void GLWidget::resizeGL(int width, int height)
{
  /// \todo Inform Spire that the GL window has changed in dimensions.
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  // Kill off the graphics thread.
  mGraphics->terminate();
  mGraphics.reset();
  //QGLWidget::closeEvent(evt);
}


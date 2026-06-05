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

#ifndef INTERFACE_MODULES_RENDER_OFFSCREENGLRENDERER_H
#define INTERFACE_MODULES_RENDER_OFFSCREENGLRENDERER_H

// Only QImage is needed in this header (returned by value). The OpenGL Qt
// classes are used as pointer members, so forward-declare them rather than
// including their headers here: on Windows those headers pull in <gl.h>, and
// if that lands before glew.h (included via gl-platform in the .cc) MSVC fails
// with "gl.h included before glew.h".
#include <QImage>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <Interface/Modules/Render/share.h>

class QOffscreenSurface;
class QOpenGLContext;
class QOpenGLFramebufferObject;

namespace SCIRun {
namespace Gui {

// Wraps SRInterface with a QOffscreenSurface + QOpenGLContext so regression
// tests can render without a visible window or display server.
class SCISHARE OffscreenGLRenderer
{
public:
  OffscreenGLRenderer(int width, int height);
  ~OffscreenGLRenderer();

  bool isValid() const;
  QOpenGLContext* context() const { return context_; }
  Render::RendererPtr renderer() const { return spire_; }

  // Renders one frame and returns the pixel data. Returns a null QImage if
  // the context is not valid.
  QImage renderToImage();

private:
  QOffscreenSurface*         surface_ {nullptr};
  QOpenGLContext*            context_ {nullptr};
  QOpenGLFramebufferObject*  fbo_     {nullptr};
  Render::RendererPtr        spire_;
  bool                       initialized_ {false};

  void initialize();
};

} // namespace Gui
} // namespace SCIRun

#endif // INTERFACE_MODULES_RENDER_OFFSCREENGLRENDERER_H

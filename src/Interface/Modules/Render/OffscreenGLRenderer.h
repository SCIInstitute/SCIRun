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

#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <Interface/Modules/Render/share.h>

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

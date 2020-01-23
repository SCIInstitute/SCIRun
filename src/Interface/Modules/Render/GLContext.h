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
/// date   December 2012
/// brief  Not sure this file should go in Modules/Render. But it is an
///        auxiliary file to the ViewScene render module.

#ifndef INTERFACE_MODULES_RENDER_GLCONTEXT_H
#define INTERFACE_MODULES_RENDER_GLCONTEXT_H

// For windows.
#define NOMINMAX

#include <Interface/Modules/Render/GLContextPlatformCompatibility.h>
#include <Interface/Modules/Render/share.h>

namespace SCIRun {
namespace Gui {

/// Context that will be sent to spire.
class SCISHARE GLContext
{
public:
  GLContext(QGLWidget* glWidget);
  virtual ~GLContext();

  /// Mandatory override from Context.
  virtual void makeCurrent();

  /// Mandatory override from Context.
  virtual void swapBuffers();

private:

  QGLWidget* mGLWidget;
};

} // end of namespace SCIRun
} // end of namespace Gui

#endif

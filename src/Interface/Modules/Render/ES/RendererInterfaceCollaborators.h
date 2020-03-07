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

#ifndef INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERINTERFACECOLLABORATORS_H
#define INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERINTERFACECOLLABORATORS_H

#include <Interface/Modules/Render/share.h>

namespace SCIRun
{
  namespace Render
  {
    enum class MouseButton
    {
      MOUSE_NONE,
      MOUSE_LEFT,
      MOUSE_RIGHT,
      MOUSE_MIDDLE,
    };

    enum class MouseMode
    {
      MOUSE_OLDSCIRUN,
      MOUSE_NEWSCIRUN
    };

    enum class MatFactor
    {
      MAT_AMBIENT,
      MAT_DIFFUSE,
      MAT_SPECULAR,
      MAT_SHINE
    };

    enum class FogFactor
    {
      FOG_INTENSITY,
      FOG_START,
      FOG_END
    };

    struct SCISHARE ClippingPlane
    {
      bool visible, showFrame, reverseNormal;
      double x, y, z, d;
    };
  }
}

#endif

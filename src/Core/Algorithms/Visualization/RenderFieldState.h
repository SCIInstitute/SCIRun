/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ALGORITHMS_VISUALIZATION_RENDER_FIELD_STATE_H
#define CORE_ALGORITHMS_VISUALIZATION_RENDER_FIELD_STATE_H

#include <Core/Datatypes/Material.h>
#include <Core/Datatypes/Color.h>

namespace SCIRun {

// Add render state information here.
class RenderState
{
public:

  enum ActionFlags
  {
    IS_ON = 0,
    HAS_DATA,
    USE_TRANSPARENCY,
    NORMALIZE_DATA,

    USE_DEFAULT_COLOR,
    USE_COLORMAP,
    USE_COLOR_CONVERT,

    SMALL_IS_DOT,

    DIRTY,

    // Node flags
    USE_SPHERE,

    // Edge flags
    USE_CYLINDER,

    // Face flags
    USE_NORMALS,
    USE_TEXTURE,
    IS_DOUBLE_SIDED,

    // Text flags
    BACKFACES_CULL,
    ALWAYS_VISIBLE,

    // Scalar data flags

    // Vector data flags
    BIDIRECTIONAL,

    // Tensor data flags

    // Secondary / tertiary data flags
    USE_ALPHA,
    USE_VALUE,
    USE_MAJOR_RADIUS,
    USE_MINOR_RADIUS,
    USE_PITCH,

    MAX_ACTION_FLAGS
  };

  enum GlyphType
  {
    POINT_GLYPH,
    SPHERE_GLYPH,
    BOX_GLYPH,
    AXIS_GLYPH,
    LINE_GLYPH,
    NEEDLE_GLYPH,
    ARROW_GLYPH,
    CONE_GLYPH,
    DISK_GLYPH,
    RING_GLYPH
  };

  RenderState()
  {
    for (int i = 0; i < MAX_ACTION_FLAGS; ++i)
    {
      mFlags[i] = false;
    }

    // Default settings.
    mFlags[USE_NORMALS] = true;
  }

  void set(ActionFlags flag, bool truth)
  {
    mFlags[flag] = truth;
  }

  bool get(ActionFlags flag) const
  {
    return mFlags[flag];
  }

  // Render state flags.
  bool mFlags[MAX_ACTION_FLAGS];

  Core::Datatypes::ColorRGB defaultColor;
};


} // namespace SCIRun

#endif


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


#ifndef INTERFACE_MODULES_RENDER_ES_COMP_RENDER_COLOR_MAP_GEOM_H
#define INTERFACE_MODULES_RENDER_ES_COMP_RENDER_COLOR_MAP_GEOM_H

#include <gl-shaders/GLShader.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-render/util/Shader.hpp>
#include <es-render/comp/StaticVBOMan.hpp>

namespace SCIRun {
namespace Render {

struct RenderColorMapGeom
{
  // -- Data --
  static const int MaxNumAttributes = 5;
  ren::ShaderVBOAttribs<MaxNumAttributes> attribs;

  // -- Functions --
  RenderColorMapGeom() {}

  static const char* getName() {return "RenderColorMapGeom";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Shouldn't need to serialize these values. They are context specific.
    // Maybe? Will need to figure out as I go along.
    return true;
  }
};

} // namespace Render
} // namespace SCIRun

#endif

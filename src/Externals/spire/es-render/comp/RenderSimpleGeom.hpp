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


#ifndef SPIRE_RENDER_COMPONENT_RENDER_SIMPLE_GEOM_HPP
#define SPIRE_RENDER_COMPONENT_RENDER_SIMPLE_GEOM_HPP

#include <es-log/trace-log.h>
#include <gl-shaders/GLShader.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include "StaticVBOMan.hpp"
#include <spire/scishare.h>

namespace ren {

/// \todo Transition this class to use the template ShaderVBOAttribs class
///       under utils (utils/Shader.hpp).
struct RenderSimpleGeom
{
  // -- Data --
  static const int MaxNumAttributes = 5;

  int attribSize;     ///< How many elements in 'appliedAttribs' are valid.
  size_t stride;      ///< Stride between elements vertex buffer.
  spire::ShaderAttributeApplied appliedAttribs[MaxNumAttributes];

  // -- Functions --

  RenderSimpleGeom()
  {
    attribSize = -1;
  }

  static const char* getName() {return "ren:RenderSimpleGeom";}

  // Returns true if this simple geom instance has been appropriately setup
  // to handle rendering.
  bool isSetUp() const;

  void setAsUninitialized()
  {
    attribSize = -1;
  }

  // Constructs attribute array given the VBOMan, vbo glid, and shader glid.
  // You will need to const-cast this component as this function modifies
  // the component in-place.
  void checkAttribArray(GLuint vboID, GLuint shaderID,
                        const StaticVBOMan& vboMan);

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Shouldn't need to serialize these values. They are context specific.
    // Maybe? Will need to figure out as I go along.
    return true;
  }
};

} // namespace ren

#endif

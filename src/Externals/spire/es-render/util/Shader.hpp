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


#ifndef SPIRE_ES_RENDER_UTIL_SHADER_HPP
#define SPIRE_ES_RENDER_UTIL_SHADER_HPP

#include <es-log/trace-log.h>
#include <es-cereal/CerealCore.hpp>
#include <gl-shaders/GLShader.hpp>

#include "../comp/StaticVBOMan.hpp"
#include "../VBOMan.hpp"
#include <spire/scishare.h>

namespace ren {

void addShaderVSFS(spire::CerealCore& core, uint64_t entityID,
                   const std::string& shader);

// Class to help construct attributes given a VBO and a shader.
// NOTE: The given VBOID *must* be present in the static VBO man.
// Otherwise this class will not know how to compare the shader attributes
// and the VBO attributes.
template <int MaxNumAttributes = 5>
class ShaderVBOAttribs
{
public:
  ShaderVBOAttribs() : mAttribSize(-1), mStride(0) {}
  ShaderVBOAttribs(GLuint vboID, GLuint shaderID, const StaticVBOMan& vboMan)
  {
    setup(vboID, shaderID, vboMan);
  }
  ShaderVBOAttribs(GLuint vboID, GLuint shaderID, const VBOMan& vboMan)
  {
    setup(vboID, shaderID, vboMan);
  }

  /// Bind pre-applied attributes setup with the 'setup' function.
  void bind() const
  {
    if (isSetup()) {
      spire::bindPreappliedAttrib(mAppliedAttribs,
                                    static_cast<size_t>(mAttribSize), mStride);
    } else {
      std::cerr << "Attempted to bind uninitialized attributes!" << std::endl;
    }
  }

  /// Unbind pre-applied atttributes.
  void unbind() const
  {
    if (isSetup()) {
      spire::unbindPreappliedAttrib(mAppliedAttribs,
                                      static_cast<size_t>(mAttribSize));
    } else {
      std::cerr << "Attempted to unbind unitialized attributes!" << std::endl;
    }

  }

  /// Sets up this class 'ShaderVBOAttribs' such that it attributes can be
  /// applied before rendering.
  void setup(GLuint vboID, GLuint shaderID, const StaticVBOMan& vboMan)
  {
    setup(vboID, shaderID, *(vboMan.instance_));
  }

  /// Sets up this class 'ShaderVBOAttribs' such that it attributes can be
  /// applied before rendering.
  void setup(GLuint vboID, GLuint shaderID, const VBOMan& vboMan)
  {
    /// NOTE: If this statement proves to be a performance problem (because
    ///       we are looking up the shader's attributes using OpenGL), then
    ///       we can cache the attributes in the shader manager and import
    ///       them using the StaticShaderMan component.
    std::vector<spire::ShaderAttribute> attribs =
        spire::getProgramAttributes(shaderID);
    spire::sortAttributesAlphabetically(attribs);

    // Lookup the VBO and its attributes by GL id.
    std::vector<spire::ShaderAttribute> vboAttribs =
        vboMan.getVBOAttributes(vboID);

    if (vboAttribs.size() < attribs.size())
    {
      std::cerr << "ren::RenderSimpleGeom: Unable to satisfy shader! Not enough attributes." << std::endl;
    }

    // Use the shader and VBO data to construct an applied state.
    std::tuple<size_t, size_t> sizes = buildPreappliedAttrib(
        &vboAttribs[0], vboAttribs.size(),
        &attribs[0], attribs.size(),
        mAppliedAttribs, MaxNumAttributes
        );

    mAttribSize = static_cast<int>(std::get<0>(sizes));
    mStride = std::get<1>(sizes);
  }

  /// Returns true if the shader and VBO attributes have been merged together
  /// into our appliedAttributes array.
  bool isSetup() const
  {
    return (mAttribSize != -1);
  }

  void setAsUninitialized()
  {
    mAttribSize = -1;
  }

private:
  int     mAttribSize;   ///< How many elements in 'appliedAttribs' are valid.
  size_t  mStride;       ///< Stride between elements in the vertex buffer.

  spire::ShaderAttributeApplied mAppliedAttribs[MaxNumAttributes];
};

} // namespace ren

#endif

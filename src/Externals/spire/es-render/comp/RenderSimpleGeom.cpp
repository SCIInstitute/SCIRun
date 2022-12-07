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


#include "RenderSimpleGeom.hpp"
#include "../VBOMan.hpp"
#include "StaticVBOMan.hpp"
#include <glm/glm.hpp>

namespace ren {

void RenderSimpleGeom::checkAttribArray(GLuint vboID, GLuint shaderID,
                                        const StaticVBOMan& vboMan)
{
  if (!isSetUp())
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
        vboMan.instance_->getVBOAttributes(vboID);

    if (vboAttribs.size() < attribs.size())
    {
      std::cerr << "ren::RenderSimpleGeom: Unable to satisfy shader! Not enough attributes." << std::endl;
    }

    // Use the shader and VBO data to construct an applied state.
    std::tuple<size_t, size_t> sizes = buildPreappliedAttrib(
        &vboAttribs[0], vboAttribs.size(),
        &attribs[0], attribs.size(),
        appliedAttribs, MaxNumAttributes
        );

    attribSize = static_cast<int>(std::get<0>(sizes));
    stride = std::get<1>(sizes);
  }
}

bool RenderSimpleGeom::isSetUp() const
{
  return (attribSize != -1);
}

} // namespace ren

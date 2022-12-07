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


#ifndef SPIRE_ES_RENDER_UTIL_LINES_HPP
#define SPIRE_ES_RENDER_UTIL_LINES_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/CerealCore.hpp>
#include "es-render/comp/VBO.hpp"
#include "es-render/comp/IBO.hpp"
#include <spire/scishare.h>

/// Utilities related to constructing and attaching geometry composed of
/// lines.

namespace ren {

/// Returns the VBO and IBO id of a unit line square. If the asset
/// already exists in the system then the ids of that vbo and ibo are returned
/// instead of generating a new VBO and IBO. The quad's vertices in object space
/// are at each of the positive and negative units -1,-1 ... 1,1.
/// \return Fully prepared VBO and IBO components.
std::pair<ren::VBO, ren::IBO> getLineUnitSquare(spire::CerealCore& core);

/// Generates a shader from memory, if it does not already exist in the system
/// returns the ID to the shader program.
GLuint getColorLineShader(spire::CerealCore& core);

// If you want to place the line unit square, use the tex quad transformations
// found in TexQuad.hpp. They are the same as this line unit square transform
// would be.

} // namespace ren

#endif

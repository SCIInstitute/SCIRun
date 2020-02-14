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


#ifndef INTERFACE_MODULES_RENDER_ES_SRUTIL_H
#define INTERFACE_MODULES_RENDER_ES_SRUTIL_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace SCIRun {
namespace Render {

// Misc SCIRun utilities.

/// \todo Possibly parameterize uint16_t output type for ibo using templates.

/// Normal rendering. Expects position and normals to be floats.
/// \param  vboData     The raw VBO that gets sent to spire.
/// \param  stride      Stride between vertices.
/// \param  normLength  Length of the normal.
/// \param  posOffset   Offset to position.
/// \param  normOffset  Offset to the normal.
/// \param  out_vboData Output VBO data. Format: Position. Where the 3 elements
///                     inside the position are floats.
/// \param  out_iboData Output IBO data. Format: uint16_t, GL_LINE.
/// \return The number of elements (GL_LINE) in the ibo data.
size_t buildNormalRenderingForVBO(std::shared_ptr<std::vector<uint8_t>> vboData,
                                  size_t stride, float normLength,
                                  std::vector<uint8_t>& out_vboData,
                                  std::vector<uint8_t>& out_iboData,
                                  size_t posOffset = 0,
                                  size_t normOffset = sizeof(float) * 3);

} // namespace Render
} // namespace SCIRun

#endif

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


#ifndef SPIRE_ES_RENDER_UTIL_TEXQUAD_HPP
#define SPIRE_ES_RENDER_UTIL_TEXQUAD_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/CerealCore.hpp>
#include "es-render/comp/VBO.hpp"
#include "es-render/comp/IBO.hpp"
#include <spire/scishare.h>

/// Utilities related to constructing and attaching textured quads to entity
/// ids.

namespace ren {

// Adds a textured quad to the given entity ID. Keep in mind, duplicate
// render assets on a single entity ID is not handled well. This is easily
// fixable with a sequence ID on the assets themselves. But I don't know if
// that is the correct approach to take from the design perspective. Maybe
// some sort of 'subobject' approach? I don't rightly know.

/// Returns the VBO and IBO id of a unit textured quad asset. If the asset
/// already exists in the system then the ids of that vbo and ibo are returned
/// instead of generating a new VBO and IBO. The quad's vertices in object space
/// are at each of the positive and negative units -1,-1 ... 1,1.
/// Texture coordinates range from [0 to 1] on both the U and V axes.
/// \return Fully prepared VBO and IBO components.
std::pair<ren::VBO, ren::IBO> getTexUnitQuad(spire::ESCoreBase& core);

/// Function will add a unit quad VBO and IBO alongside the given shader and
/// texture into the entityID. The texture is placed in unit 0 with uniform
/// name uTX0. Rendering and transform is up to you.
/// You will still have to add the renderer (something like simple geom),
/// uniforms, and transforms.
void addTexQuad(spire::CerealCore& core, uint64_t entityID, const std::string& texture);

/// Returns a mat4 that will position a tex quad generated with 'getTexUnitQuad'
/// at the given position and dimensions. This function will apply scaling
/// to the returned matrix.
glm::mat4 getTexQuadTransform(const glm::vec3& center, float width, float height);
glm::mat4 getTexQuadTransform(const glm::vec2& topLeft, const glm::vec2& bottomRight, float z);

/// The following two functions are identical to the two functions above except
/// that these functions will apply the calculated transform to the object
/// using add component... This use addComponent and does not check for an
/// existing transform component!
/// \todo These should be renamed to *add* tex quad transform. Because that is
///       what they are doing.
void setTexQuadTransform(spire::CerealCore& core, uint64_t entityID,
                         const glm::vec3& center, float width, float height);
void setTexQuadTransform(spire::CerealCore& core, uint64_t entityID,
                         const glm::vec2& topLeft, const glm::vec2& bottomRight, float z);

} // namespace ren

#endif

#ifndef IAUNS_ES_RENDER_UTIL_TEXQUAD_HPP
#define IAUNS_ES_RENDER_UTIL_TEXQUAD_HPP

#include <glm/glm.hpp>
#include <es-cereal/CerealCore.hpp>
#include "es-render/comp/VBO.hpp"
#include "es-render/comp/IBO.hpp"

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
std::pair<ren::VBO, ren::IBO> getTexUnitQuad(CPM_ES_NS::ESCoreBase& core);

/// Function will add a unit quad VBO and IBO alongside the given shader and
/// texture into the entityID. The texture is placed in unit 0 with uniform
/// name uTX0. Rendering and transform is up to you.
/// You will still have to add the renderer (something like simple geom),
/// uniforms, and transforms.
void addTexQuad(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID, const std::string& texture);

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
void setTexQuadTransform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                         const glm::vec3& center, float width, float height);
void setTexQuadTransform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                         const glm::vec2& topLeft, const glm::vec2& bottomRight, float z);

} // namespace ren 

#endif 


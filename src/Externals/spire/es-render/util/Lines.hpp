#ifndef IAUNS_ES_RENDER_UTIL_LINES_HPP
#define IAUNS_ES_RENDER_UTIL_LINES_HPP

#include <glm/glm.hpp>
#include <es-cereal/CerealCore.hpp>
#include "es-render/comp/VBO.hpp"
#include "es-render/comp/IBO.hpp"

/// Utilities related to constructing and attaching geometry composed of
/// lines.

namespace ren {

/// Returns the VBO and IBO id of a unit line square. If the asset
/// already exists in the system then the ids of that vbo and ibo are returned
/// instead of generating a new VBO and IBO. The quad's vertices in object space
/// are at each of the positive and negative units -1,-1 ... 1,1.
/// \return Fully prepared VBO and IBO components.
std::pair<ren::VBO, ren::IBO> getLineUnitSquare(CPM_ES_CEREAL_NS::CerealCore& core);

/// Generates a shader from memory, if it does not already exist in the system
/// returns the ID to the shader program.
GLuint getColorLineShader(CPM_ES_CEREAL_NS::CerealCore& core);

// If you want to place the line unit square, use the tex quad transformations
// found in TexQuad.hpp. They are the same as this line unit square transform
// would be.

} // namespace ren 

#endif 


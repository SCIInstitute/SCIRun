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
    std::vector<CPM_GL_SHADERS_NS::ShaderAttribute> attribs = 
        CPM_GL_SHADERS_NS::getProgramAttributes(shaderID);
    CPM_GL_SHADERS_NS::sortAttributesAlphabetically(attribs);

    // Lookup the VBO and its attributes by GL id.
    std::vector<CPM_GL_SHADERS_NS::ShaderAttribute> vboAttribs =
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


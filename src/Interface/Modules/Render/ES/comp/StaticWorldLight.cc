#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "StaticWorldLight.h"

namespace shaders = CPM_GL_SHADERS_NS;

namespace SCIRun {
namespace Render {

void StaticWorldLight::checkUniformArray(GLuint shaderID)
{
  // Obtain uniforms from shader and decide which of the uniforms we can
  // provide automatically.
  std::vector<shaders::ShaderUniform> shaderUniforms = shaders::getProgramUniforms(shaderID);
  for (const shaders::ShaderUniform& uniform : shaderUniforms)
  {
    if (uniform.nameInCode == "uLightDirWorld")
    {
      hasLightUniform = true;
      uniformLocation = uniform.uniformLoc;
      break;
    }
  }
}

void StaticWorldLight::applyUniform() const
{
  if (hasLightUniform)
  {
    GL(glUniform3f(uniformLocation, lightDir.x, lightDir.y, lightDir.z));
  }
}

} // namespace Render
} // namespace SCIRun



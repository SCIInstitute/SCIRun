#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "LightingUniforms.h"

namespace shaders = CPM_GL_SHADERS_NS;

namespace SCIRun {
namespace Render {

LightingUniforms::LightingUniforms()
{
  for (int i = 0; i < LIGHT_NUM; ++i)
  {
    hasLightUniform[i] = false;
    uniformLocation[i] = 0;
  }
}

void LightingUniforms::checkUniformArray(GLuint shaderID)
{
  // Obtain uniforms from shader and decide which of the uniforms we can
  // provide automatically.
  std::vector<shaders::ShaderUniform> shaderUniforms = shaders::getProgramUniforms(shaderID);
  for (const shaders::ShaderUniform& uniform : shaderUniforms)
  {
    if (uniform.nameInCode == "uLightDirWorld")
    {
      hasLightUniform[0] = true;
      uniformLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirWorld0")
    {
      hasLightUniform[0] = true;
      uniformLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirWorld1")
    {
      hasLightUniform[1] = true;
      uniformLocation[1] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirWorld2")
    {
      hasLightUniform[2] = true;
      uniformLocation[2] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirWorld3")
    {
      hasLightUniform[3] = true;
      uniformLocation[3] = uniform.uniformLoc;
    }
  }
}

void LightingUniforms::applyUniform(const std::vector<glm::vec3>& lightDirs) const
{
  for (int i = 0; i < lightDirs.size(); ++i)
  {
    if (i >= LIGHT_NUM)
      break;
    if (hasLightUniform[i])
    {
      GL(glUniform3f(uniformLocation[i], lightDirs[i].x, lightDirs[i].y, lightDirs[i].z));
    }
  }
}

} // namespace Render
} // namespace SCIRun



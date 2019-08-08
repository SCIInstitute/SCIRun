#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "LightingUniforms.h"

namespace shaders = spire;

namespace SCIRun {
namespace Render {

LightingUniforms::LightingUniforms()
{
  for (int i = 0; i < LIGHT_NUM; ++i)
  {
    hasLightUniform[i] = false;
    uniformLocation[i] = 0;
    hasLightColorUniform[i] = false;
    colorUnifLocation[i] = 0;
  }
}

void LightingUniforms::checkUniformArray(GLuint shaderID)
{
  // Obtain uniforms from shader and decide which of the uniforms we can
  // provide automatically.
  std::vector<shaders::ShaderUniform> shaderUniforms = shaders::getProgramUniforms(shaderID);
  for (const shaders::ShaderUniform& uniform : shaderUniforms)
  {
    if (uniform.nameInCode == "uLightDirectionView")
    {
      hasLightUniform[0] = true;
      uniformLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView0")
    {
      hasLightUniform[0] = true;
      uniformLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView1")
    {
      hasLightUniform[1] = true;
      uniformLocation[1] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView2")
    {
      hasLightUniform[2] = true;
      uniformLocation[2] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView3")
    {
      hasLightUniform[3] = true;
      uniformLocation[3] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor0")
    {
      hasLightColorUniform[0] = true;
      colorUnifLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor1")
    {
      hasLightColorUniform[1] = true;
      colorUnifLocation[1] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor2")
    {
      hasLightColorUniform[2] = true;
      colorUnifLocation[2] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor3")
    {
      hasLightColorUniform[3] = true;
      colorUnifLocation[3] = uniform.uniformLoc;
    }
  }
}

void LightingUniforms::applyUniform(const std::vector<glm::vec3>& lightDirs,
  const std::vector<glm::vec3>& lightColors) const
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
  for (int i = 0; i < lightColors.size(); ++i)
  {
    if (i >= LIGHT_NUM)
      break;
    if (hasLightColorUniform[i])
    {
      GL(glUniform3f(colorUnifLocation[i], lightColors[i].x, lightColors[i].y, lightColors[i].z));
    }
  }
}

} // namespace Render
} // namespace SCIRun



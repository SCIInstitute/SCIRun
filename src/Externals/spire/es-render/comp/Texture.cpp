#include <gl-shaders/GLShader.hpp>
#include "Texture.hpp"

namespace shaders = CPM_GL_SHADERS_NS;

namespace ren {

void Texture::checkUniform(GLuint shaderID)
{
  if (isSetUp() == false)
  {
    bool boundUniform = false;
    std::vector<shaders::ShaderUniform> uniforms = 
        shaders::getProgramUniforms(shaderID);
    for (const shaders::ShaderUniform& unif : uniforms)
    {
      if (unif.nameInCode == uniformName)
      {
        /// \todo Check size and type against what we pulled from OpenGL.
        uniformLocation = unif.uniformLoc;
        boundUniform = true;
        break;
      }
    }

    if (boundUniform == false)
    {
      std::cerr << "Unable to find uniform with name: " << uniformName <<
          " in shader with ID: " << shaderID << std::endl;
    }
  }
}

void Texture::applyUniform() const
{
  if (isSetUp())
  {
    // When rendering an object with this program.
    GL(glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(textureUnit)));
    GL(glBindTexture(textureType, glid));
    GL(glUniform1i(uniformLocation, textureUnit));
  }
}

bool Texture::isSetUp() const
{
  return (uniformLocation != -1);
}

} // namespace ren


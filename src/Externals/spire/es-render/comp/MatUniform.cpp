#include <gl-shaders/GLShader.hpp>
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr

#include "MatUniform.hpp"

namespace shaders = CPM_GL_SHADERS_NS;

namespace ren {

void MatUniform::applyUniform() const
{
  const GLfloat* ptr = glm::value_ptr(uniform);
  switch (type)
  {
    case MAT4:
      GL(glUniformMatrix4fv(uniformLocation, 1, false, ptr));
      break;

    case NONE:
      std::cerr << "MatUniform: attempting to set 'none' type" << std::endl;
      break;
  }
}

void MatUniform::checkUniform(GLuint shaderID)
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

bool MatUniform::isSetUp() const
{
  return (uniformLocation != -1);
}

} // namespace ren



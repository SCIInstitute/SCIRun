#include <gl-shaders/GLShader.hpp>
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr

#include "VecUniform.hpp"

namespace shaders = CPM_GL_SHADERS_NS;

namespace ren {

void VecUniform::applyUniform() const
{
  switch (type)
  {
    case SCALAR:
      GL(glUniform1f(uniformLocation, uniform.x));
      break;

    case VEC2:
      GL(glUniform2f(uniformLocation, uniform.x, uniform.y));
      break;

    case VEC3:
      GL(glUniform3f(uniformLocation, uniform.x, uniform.y, uniform.z));
      break;

    case VEC4:
      GL(glUniform4f(uniformLocation, uniform.x, uniform.y, uniform.z, uniform.w));
      break;

    case NONE:
      std::cerr << "VecUniform - applyUniform: attempting to apply NONE." << std::endl;
      break;
  }
}

void VecUniform::checkUniform(GLuint shaderID)
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

bool VecUniform::isSetUp() const
{
  return (uniformLocation != -1);
}

VecUniform::VecUniform(const char* uniformNameIn, const glm::vec4& vec, VecUniformType unifType) :
    uniformLocation(-1)
{
  uniform = vec;
  type = unifType;
  if (std::strlen(uniformNameIn) < MaxUniformNameSize - 1)
  {
    std::strncpy(uniformName, uniformNameIn, MaxUniformNameSize - 1);
  }
  else
  {
    std::cerr << "ren::VecUniform Uniform name too long." << std::endl;
  }
}

} // namespace ren



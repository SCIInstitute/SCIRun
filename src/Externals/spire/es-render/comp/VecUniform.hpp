#ifndef IAUNS_RENDER_COMPONENT_VEC_UNIFORM_HPP
#define IAUNS_RENDER_COMPONENT_VEC_UNIFORM_HPP

#include <cstring>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct VecUniform
{
  // -- Data --
  static const int MaxUniformNameSize = 24;

  enum VecUniformType
  {
    SCALAR,
    VEC2,
    VEC3,
    VEC4,
    NONE
  };

  glm::vec4       uniform;
  VecUniformType  type;
  GLint           uniformLocation;
  char            uniformName[MaxUniformNameSize];

  // -- Functions --
  VecUniform()
  {
    type            = VecUniformType::NONE;
    uniformLocation = -1;
    uniformName[0]  = '\0';
  }

  VecUniform(const char* uniformNameIN, const glm::vec4& vec, VecUniformType unifType);

  // Applies the uniform.
  void applyUniform() const;

  // Checks to see if uniformLocation has been set. If not, then the uniform
  // location in the shader 
  void checkUniform(GLuint shaderID);

  // Returns true if this uniform has been setup for rendering with a particular
  // shader.
  bool isSetUp() const;

  static const char* getName() {return "ren:VecUniform";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Write out appropriate vector uniform.
    return true;
  }
};

} // namespace ren

#endif 

#ifndef IAUNS_RENDER_COMPONENT_MAT_UNIFORM_HPP
#define IAUNS_RENDER_COMPONENT_MAT_UNIFORM_HPP

#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct MatUniform
{
  // -- Data --
  static const int MaxUniformNameSize = 24;

  enum MatUniformType
  {
    MAT4,
    NONE
  };

  glm::mat4       uniform;
  MatUniformType  type;
  GLint           uniformLocation;
  char            uniformName[MaxUniformNameSize];

  // -- Functions --
  MatUniform()
  {
    type            = MatUniformType::NONE;
    uniformLocation = -1;
    uniformName[0]  = '\0';
  }

  // Applies the uniform.
  void applyUniform() const;

  // Checks to see if uniformLocation has been set. If not, then the uniform
  // location in the shader 
  void checkUniform(GLuint shaderID);

  // Returns true if this uniform has been setup for rendering with a particular
  // shader.
  bool isSetUp() const;

  static const char* getName() {return "ren:MatUniform";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize appropriate uniform data (mat4).
    return true;
  }
};

} // namespace ren

#endif 

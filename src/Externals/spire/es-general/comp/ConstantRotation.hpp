#ifndef IAUNS_ES_GENERAL_CONSTANT_ROTATION_HPP
#define IAUNS_ES_GENERAL_CONSTANT_ROTATION_HPP

#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <cereal-glm/CerealGLM.hpp>

namespace gen {

// This component will *not* be present when there is no mouse input available
// for the system. You can check other input sources such as touch input
// or keyboard input.
struct ConstantRotation
{
  // -- Data --
  bool      pretransform;     ///< If true, then if this matrix is X, and the
                              ///< transformation is Y, then the resulting
                              ///< transform is Y * X, otherwise it is X * Y.
  float     radiansPerSec;    ///< Radians per second to rotate around an axis.
  glm::vec3 rotationAxis;     ///< Axis of rotation.

  // -- Functions --
  ConstantRotation()
  {
    pretransform = true;
    radiansPerSec = 0.0f;
    rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
  }

  static const char* getName() {return "gen:ConstantRotation";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("prexform", pretransform);
    s.serialize("axis", rotationAxis);
    s.serialize("rPerSec", radiansPerSec);
    return true;
  }
};

} // namespace gen

#endif 

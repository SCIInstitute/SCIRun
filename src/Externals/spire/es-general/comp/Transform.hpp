#ifndef IAUNS_ES_GENERAL_COMP_TRANSFORM_HPP
#define IAUNS_ES_GENERAL_COMP_TRANSFORM_HPP

#include <es-cereal/ComponentSerialize.hpp>
#include <cereal-glm/CerealGLM.hpp>

namespace gen {

struct Transform
{
  // -- Data --
  glm::mat4 transform;

  // -- Functions --
  void setPosition(const glm::vec3& pos)
  {
    transform[3].x = pos.x;
    transform[3].y = pos.y;
    transform[3].z = pos.z;
  }

  glm::vec3 getPosition()
  {
    return glm::vec3(transform[3].x, transform[3].y, transform[3].z);
  }

  static const char* getName() {return "gen:transform";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("trafo", transform);
    return true;
  }
};

} // namespace gen 

#endif 

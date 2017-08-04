#ifndef SPIRE_COMPONENT_SKINNED_GEOM_HPP
#define SPIRE_COMPONENT_SKINNED_GEOM_HPP

#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct SkinnedGeom
{
  // -- Data --
  GLuint vbo;
  GLuint ibo;
  //SkeletonData* skeleton;

  // -- Functions --
  static const char* getName() {return "ren:SkinnedGeom";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // ?
    return true;
  }
};

} // namespace ren

#endif 

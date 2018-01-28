#ifndef SPIRE_RENDER_COMPONENT_VBO_HPP
#define SPIRE_RENDER_COMPONENT_VBO_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct VBO
{
  // -- Data --
  VBO() : glid(0) {}

  // -- Functions --
  static const char* getName() {return "ren:VBO";}

  GLuint glid;

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Serialize out the asset name (if any), and the attributes associated
    // with this glid. We don't need to store the attributes alongside
    // the glid, just in a central repository. This is very similar to
    // how we handle the shader.

    // Maybe write out the name of the file the VBO is associated with?
    // Maybe write out the VBO itself?
    return true;
  }
};

} // namespace ren

#endif

#ifndef SPIRE_RENDER_COMPONENT_IBO_HPP
#define SPIRE_RENDER_COMPONENT_IBO_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct IBO
{
  // -- Data --
  GLuint glid;

  GLenum  primMode;   ///< GL_TRIANGLE_STRIP...
  GLenum  primType;   ///< GL_UNSIGNED_SHORT...
  GLsizei numPrims;

  // -- Functions --
  IBO()
  {
    glid = 0;
    numPrims = 0;
    primType = 0;
  }

  static const char* getName() {return "ren:IBO";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// Nothing needs to be serialized. This is context specific.
    return true;
  }
};

} // namespace ren

#endif

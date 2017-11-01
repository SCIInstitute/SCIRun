#ifndef SPIRE_RENDER_COMPONENTS_STATIC_GLSTATE_HPP
#define SPIRE_RENDER_COMPONENTS_STATIC_GLSTATE_HPP

#include <es-log/trace-log.h>
#include <gl-state/GLState.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct StaticGLState
{
  // -- Data --
  spire::GLState state;

  // -- Functions --
  static const char* getName() {return "ren:StaticGLState";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize GLState
    return true;
  }
};

} // namespace ren

#endif

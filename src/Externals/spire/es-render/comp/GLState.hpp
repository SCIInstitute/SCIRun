#ifndef SPIRE_RENDER_COMPONENTS_GLSTATE_HPP
#define SPIRE_RENDER_COMPONENTS_GLSTATE_HPP

#include <es-log/trace-log.h>
#include <gl-state/GLState.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct GLState
{
  // -- Data --
  spire::GLState state;

  // -- Functions --

  static const char* getName() {return "ren:GLState";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize a GLState. We possibly want to make this a separate
    ///       function because StaticGLState will also want to serialize
    ///       a GLState.
    return true;
  }

};

} // namespace ren

#endif

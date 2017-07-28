#ifndef IAUNS_RENDER_COMPONENTS_GLSTATE_HPP
#define IAUNS_RENDER_COMPONENTS_GLSTATE_HPP

#include <gl-state/GLState.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct GLState
{
  // -- Data --
  CPM_GL_STATE_NS::GLState state;  

  // -- Functions --

  static const char* getName() {return "ren:GLState";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize a GLState. We possibly want to make this a separate
    ///       function because StaticGLState will also want to serialize
    ///       a GLState.
    return true;
  }

};

} // namespace ren

#endif 

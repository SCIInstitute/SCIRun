#ifndef IAUNS_RENDER_COMPONENTS_STATIC_GLSTATE_HPP
#define IAUNS_RENDER_COMPONENTS_STATIC_GLSTATE_HPP

#include <gl-state/GLState.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct StaticGLState
{
  // -- Data --
  CPM_GL_STATE_NS::GLState state;  

  // -- Functions --
  static const char* getName() {return "ren:StaticGLState";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize GLState
    return true;
  }
};

} // namespace ren

#endif 

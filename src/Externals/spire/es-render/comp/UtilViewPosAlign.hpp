#ifndef SPIRE_RENDER_UTIL_VIEW_POS_ALIGN_HPP
#define SPIRE_RENDER_UTIL_VIEW_POS_ALIGN_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <cereal-glm/CerealGLM.hpp>

namespace ren {

// Careful with this component. It requires that the transform be modified
// using const_cast and not using the default modification system.
struct UtilViewPosAlign
{
  // -- Data --
  // !!NOTE!! Neither of the following variables are implemented. See
  // ViewPosAlignSys. That is where it should be implemented.
  bool offsetInViewCoords;  // True if the offset is relative to the view coordinate system.
  glm::vec3 offset;         // Offset from the view position.

  // -- Functions --
  UtilViewPosAlign()
  {
    offsetInViewCoords = false;
    offset = glm::vec3(0.0f, 0.0f, 0.0f);
  }

  static const char* getName() {return "ren:UtilViewPosAlign";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("offvc", offsetInViewCoords);
    s.serialize("offset", offset);
    return true;
  }
};

} // namespace ren

#endif

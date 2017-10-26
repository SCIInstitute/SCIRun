#ifndef SPIRE_RENDER_COMPONENT_RENDER_SEQUENCE_HPP
#define SPIRE_RENDER_COMPONENT_RENDER_SEQUENCE_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct RenderSequence
{
  // -- Data --
  uint64_t sequence;

  // -- Functions --
  static const char* getName() {return "ren:RenderSequence";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("s", sequence);
    return true;
  }
};

} // namespace ren

#endif

#ifndef SPIRE_ES_GENERAL_STATIC_SCREEN_DIMS_HPP
#define SPIRE_ES_GENERAL_STATIC_SCREEN_DIMS_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace gen {

// This component will *not* be present when there is no mouse input available
// for the system. You can check other input sources such as touch input
// or keyboard input.
struct StaticScreenDims
{
  // -- Data --
  uint32_t width;   // In pixels.
  uint32_t height;  // In pixels.

  // -- Functions --
  StaticScreenDims()
  {
    width = 0;
    height = 0;
  }

  static const char* getName() {return "gen:StaticScreenDims";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("width", width);
    s.serialize("height", height);
    return true;
  }
};

} // namespace gen

#endif

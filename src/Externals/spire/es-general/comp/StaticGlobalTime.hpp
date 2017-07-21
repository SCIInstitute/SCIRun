#ifndef IAUNS_ES_GENERAL_STATIC_GLOBAL_TIME_HPP
#define IAUNS_ES_GENERAL_STATIC_GLOBAL_TIME_HPP

#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace gen {

// This component will *not* be present when there is no mouse input available
// for the system. You can check other input sources such as touch input
// or keyboard input.
struct StaticGlobalTime
{
  // -- Data --
  double globalTime;      // Seconds
  double deltaTime;       // Seconds

  // -- Functions --
  StaticGlobalTime()
  {
    globalTime = 0.0;
    deltaTime = 0.0;
  }

  static const char* getName() {return "gen:StaticGlobalTime";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("time", globalTime);
    s.serialize("delta", deltaTime);
    return true;
  }
};

} // namespace gen

#endif 

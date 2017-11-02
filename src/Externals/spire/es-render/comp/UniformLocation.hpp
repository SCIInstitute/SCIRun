#ifndef SPIRE_RENDER_COMPONENT_UNIFORMLOCATION_HPP
#define SPIRE_RENDER_COMPONENT_UNIFORMLOCATION_HPP

#include <es-log/trace-log.h>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

/// Simple component to help store the locations of uniforms found inside
/// of a shader.
struct UniformLocation
{
  // -- Data --
  int uniformID;          ///< Use this ID to identify your uniform (instead of by name).
  GLint uniformLocation;  ///< Use this to store the uniform location inside of the shader.

  // -- Functions --
  static const char* getName() {return "ren:UniformLocation";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // How is this component used?
    return true;
  }
};

} // namespace ren

#endif

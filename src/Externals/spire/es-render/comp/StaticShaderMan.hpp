#ifndef SPIRE_RENDER_COMPONENT_STATIC_SHADER_MAN_HPP
#define SPIRE_RENDER_COMPONENT_STATIC_SHADER_MAN_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../ShaderMan.hpp"
#include <spire/scishare.h>

class ShaderMan;

namespace ren {

struct StaticShaderMan
{
  // -- Data --
  std::shared_ptr<ShaderMan> instance_;

  // -- Functions --
  StaticShaderMan() : instance_(std::shared_ptr<ShaderMan>(new ShaderMan)) {}
  StaticShaderMan(ShaderMan* s) : instance_(std::shared_ptr<ShaderMan>(s)) {}

  // This assignment operator is only used during modification calls inside
  // of the entity system. We don't care about those calls as they won't
  // affect this static shader man.
  StaticShaderMan& operator=(const StaticShaderMan&)
  {
    // We don't care about the incoming object. We've already created oun own
    // shader man and will continue to use that.
    return *this;
  }

  static const char* getName() {return "ren:StaticShaderMan";}

private:
  friend class spire::CerealHeap<StaticShaderMan>;

  bool serialize(spire::ComponentSerialize&, uint64_t)
  {
    // No need to serialize. But we do want that we were in the component
    // system to be serialized out.
    return true;
  }
};

} // namespace ren

#endif

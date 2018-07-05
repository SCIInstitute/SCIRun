#ifndef SPIRE_ASYNC_STATICFS_HPP
#define SPIRE_ASYNC_STATICFS_HPP

#include <es-log/trace-log.h>
#include <stdexcept>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../Filesystem.hpp"
#include <spire/scishare.h>

namespace spire {

struct StaticFS
{
  // -- Data --

  /// \todo Should be unique ptr. But we need to be able to add move
  ///       constructible components.
  std::shared_ptr<Filesystem> instance;

  // -- Functions --

  StaticFS() : instance(nullptr) {}
  StaticFS(std::shared_ptr<Filesystem> man) : instance(man) {}

  static const char* getName() {return "es-fs:StaticFS";}

  bool serialize(spire::ComponentSerialize&, uint64_t /* entityID */)
  {
    //logRendererError("No serialization should be performed on StaticFS!");
    throw std::runtime_error("Cannot serialize StaticFS!");
  }
};

} // namespace spire

#endif

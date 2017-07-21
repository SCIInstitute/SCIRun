#ifndef IAUNS_ASYNC_STATICFS_HPP
#define IAUNS_ASYNC_STATICFS_HPP

#include <stdexcept>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../Filesystem.hpp"

namespace CPM_ES_FS_NS {

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

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t /* entityID */)
  {
    std::cerr << "No serialization sohuld be performed on StaticFS!" << std::endl;
    throw std::runtime_error("Cannot serialize StaticFS!");
    return true;
  }
};

} // namespace CPM_ES_FS_NS 

#endif 

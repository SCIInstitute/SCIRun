#ifndef SPIRE_ENTITY_SYSTEM_ESCORE_HPP
#define SPIRE_ENTITY_SYSTEM_ESCORE_HPP

#include <es-log/trace-log.h>
#include "ESCoreBase.hpp"

namespace spire {

// This class is simply a lite wrapper around ESCoreBase which exposes
// coreAddComponent and coreAddStaticComponent.
class ESCore : public ESCoreBase
{
public:

  template <typename T>
  void addComponent(uint64_t entityID, const T& component)
  {
    coreAddComponent(entityID, component);
  }

  template <typename T>
  size_t addStaticComponent(T&& component)
  {
    return coreAddStaticComponent(std::forward<T>(component));
  }
};

} // namespace spire

#endif

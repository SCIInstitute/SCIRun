#ifndef SPIRE_ENTITY_SYSTEM_BASESYSTEM_HPP
#define SPIRE_ENTITY_SYSTEM_BASESYSTEM_HPP

#include <es-log/trace-log.h>
#include <vector>
#include <cstdint>
#include <spire/scishare.h>

namespace spire {

class ESCoreBase;

class BaseSystem
{
public:
  BaseSystem()          {}
  virtual ~BaseSystem() {}

  // The following two functions may be convertable to const functions.
  virtual void walkComponents(ESCoreBase& core) = 0;
  virtual bool walkEntity(ESCoreBase& core, uint64_t entityID) = 0;

  // Debug functions related to printing components that a particular
  // entity is missing in relation to the system executing.
  virtual std::vector<uint64_t> getComponents() const = 0;
  virtual bool isComponentOptional(uint64_t component) = 0;
};

} // namespace spire

#endif

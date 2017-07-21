#ifndef IAUNS_ENTITY_SYSTEM_ESCORE_HPP
#define IAUNS_ENTITY_SYSTEM_ESCORE_HPP

#include "ESCoreBase.hpp"

namespace CPM_ES_NS {

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

  //template <typename T>
  //void addComponent(uint64_t entityID, T&& component)
  //{
  //  coreAddComponent(entityID, std::forward<T>(component));
  //}

  template <typename T>
  size_t addStaticComponent(T&& component)
  {
    return coreAddStaticComponent(std::forward<T>(component));
  }
};

} // namespace CPM_ES_NS

#endif


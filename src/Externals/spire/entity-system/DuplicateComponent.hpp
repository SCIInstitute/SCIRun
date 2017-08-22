#ifndef SPIRE_ENTITY_SYSTEM_DUPLICATECOMPONENT_HPP
#define SPIRE_ENTITY_SYSTEM_DUPLICATECOMPONENT_HPP

#include "GenericSystem.hpp"

namespace spire {

template <typename T>
class HasDuplicateComponent : public GenericSystem<true, T>
{
public:
  HasDuplicateComponent(const T& compareComp) :
      mCompare(compareComp),
      mFoundDuplicate(false)
  {}

  bool hasFoundDuplicate() {return mFoundDuplicate;}

  void groupExecute(ESCoreBase& core, uint64_t entityID,
                    const ComponentGroup<T>& comp)
  {
    for (const T& c : comp)
    {
      if (mCompare == c)
      {
        mFoundDuplicate = true;
        return;
      }
    }
  }

private:
  bool mFoundDuplicate;
  const T& mCompare;
};

/// Tests to see if there is already an equivalent component within a given
/// entityID.
template <typename T>
bool hasDuplicateComponent(ESCoreBase& core, uint64_t entityID, const T& component)
{
  HasDuplicateComponent<T> sys(component);
  sys.walkEntity(core, entityID);
  return sys.hasFoundDuplicate();
}

} // namespace spire 

#endif 


#ifndef IAUNS_ENTITY_SYSTEM_BASECOMPONENTCONTAINER_HPP
#define IAUNS_ENTITY_SYSTEM_BASECOMPONENTCONTAINER_HPP

#include <cstdint>

namespace CPM_ES_NS {

// Component base class, used to verify component types at run-time.
class BaseComponentContainer
{
public:
  BaseComponentContainer()           {}
  virtual ~BaseComponentContainer()  {}
  
  virtual void renormalize(bool stableSort) = 0;

  /// Get the least sequence held by the component.
  virtual uint64_t getLowerSequence() const = 0;

  /// Get the upper sequence held by the component.
  virtual uint64_t getUpperSequence() const = 0;

  /// Get number of components.
  virtual uint64_t getNumComponents() const = 0;

  /// Remove components identified with \p sequence. Every component associated
  /// with the sequnece (if there are multiple) will be removed.
  virtual void removeSequence(uint64_t sequence) = 0;

  /// Removes the first component found that is associated with 'sequence'.
  virtual void removeFirstSequence(uint64_t sequence) = 0;

  /// Removes the last component found that is associated with 'sequence'.
  virtual void removeLastSequence(uint64_t sequence) = 0;

  /// Remove the sequence with the the given component index
  virtual void removeSequenceWithIndex(uint64_t sequence, int32_t componentID) = 0;

  /// Remove all components from the container.
  virtual void removeAll() = 0;

  /// Remove all components from the container -- IMMEDIATELY. Use with extreme
  /// care. This breaks the expected mutable / immutable behavior derived
  /// from the renormalization mechanism.
  virtual void removeAllImmediately() = 0;

  /// Returns true if the component system contains only static elements.
  /// These elements values are always the same regardless of the entity
  /// executing. Use for global values.
  virtual bool isStatic() const = 0;

  /// Retrieves the sequence associated with the given index.
  /// Index must be in [0, getNumComponents()).
  /// Be careful when using this function, cache misses are likely if you
  /// aren't walking in-order.
  /// If the index is not present, the function returns 0 (an invalid sequence).
  virtual uint64_t getSequenceFromIndex(int index) const = 0;

  /// Retrieves the number of components associated with a particular sequence.
  /// This function also counts the components *to be added*! Seeing the
  /// components to be added is important when we want debugging information
  /// related to if we have satisfied a particular system.
  virtual int getNumComponentsWithSequence(uint64_t sequence) const = 0;

  static const int StaticEntID;
};

} // namespace CPM_ES_NS 

#endif 

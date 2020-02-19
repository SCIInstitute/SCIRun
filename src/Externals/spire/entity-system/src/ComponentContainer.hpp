/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


#ifndef SPIRE_ENTITY_SYSTEM_COMPONENTCONTAINER_HPP
#define SPIRE_ENTITY_SYSTEM_COMPONENTCONTAINER_HPP

#include <es-log/trace-log.h>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <sstream>
#include "TemplateID.hpp"
#include "BaseComponentContainer.hpp"
#include <spire/scishare.h>

namespace spire {

/// Component container.
/// \todo Add maximum size caps to the container. Should also check size
///       caps for the number of removed components as well.
template <typename T>
class ComponentContainer : public BaseComponentContainer
{
  // SFINAE implementation of possible function calls inside of component
  // structures.
  template<class V>
  static auto maybe_component_construct(V& v, uint64_t sequence, int)
      -> decltype(v.componentConstruct(sequence), void())
  {
    v.componentConstruct(sequence);
  }

  template<class V>
  static void maybe_component_construct(V&, size_t, long){}


  template<class V>
  static auto maybe_component_destruct(V& v, uint64_t sequence, int)
      -> decltype(v.componentDestruct(sequence), void())
  {
    v.componentDestruct(sequence);
  }

  template<class V>
  static void maybe_component_destruct(V&, size_t, long){}

public:
  ComponentContainer() :
      mLastSortedSize(0),
      mUpperSequence(0),
      mLowerSequence(0),
      mIsStatic(false)
  {
    /// \todo Extract information from global table regarding default / max size
    ///       of components.
    //mComponents.reserve();
  }

  virtual ~ComponentContainer()
  {
    // Remove all componnts, and if their destructors exist, call those
    // as well.
    removeAllImmediately();
  }

  std::string toString(std::string prefix) const override
  {
    std::string output = "";
    prefix += "  ";

    for(auto& comp : mComponents)
    {
      output += prefix + "Sequence: " + std::to_string(comp.sequence) + "\n";
    }

    return output;
  }

  /// Item that represents one component paired with a sequence.
  struct ComponentItem
  {
    ComponentItem() : sequence(0)
    {
    }

    ComponentItem(uint64_t seq, const T& comp) :
        sequence(seq),
        component(comp)
    {}

    ComponentItem(uint64_t seq, T&& comp) :
        sequence(seq),
        component(std::forward<T>(comp))
    {}

    bool operator<(const ComponentItem& other) const
    {
      return (sequence < other.sequence);
    }

    bool operator<(uint64_t sequenceIn) const
    {
      return sequence < sequenceIn;
    }

    uint64_t  sequence;   ///< Commonly used element in the first cacheline.
    T         component;  ///< Copy constructable component data.
  };

  /// Returns -1 if no component of the given sequence is found.
  int getComponentItemIndexWithSequence(uint64_t sequence) const
  {
    if (mComponents.size() == 0)
      return -1;

    // You cannot search within static components. They are the same for all
    // entities. Therefore we return the first index, 0.
    if (isStatic())
      return 0;

    auto last = mComponents.cbegin() + mLastSortedSize;
    auto it = std::lower_bound(mComponents.cbegin(), last, sequence);

    if (it->sequence == sequence)
    {
      return static_cast<int>(it - mComponents.cbegin());
    }
    else
    {
      return -1;
    }
  }

  ComponentItem* getComponentItemWithSequence(uint64_t sequence)
  {
    if (mComponents.size() == 0)
      return nullptr;

    if (isStatic())
      return &mComponents.front();

    auto last = mComponents.begin() + mLastSortedSize;

    // lower_bound uses a binary search to find the target component.
    // Our vector is always sorted, so we can pull this off.
    auto it = std::lower_bound(mComponents.begin(), last, sequence);

    if (it->sequence == sequence)
    {
      return &(*it);
    }
    else
    {
      return nullptr;
    }
  }

  /// Retrieves component with the indicated sequence.
  const T* getComponentWithSequence(uint64_t sequence)
  {
    ComponentItem* item = getComponentItemWithSequence(sequence);
    if (item)
      &item->component;
    else
      return item;
  }

  /// Retrieves the component for the given sequence. Returns both a pointer
  /// to the component and its index in the component container. You can
  /// use this index to modify the components value using modifyIndex.
  std::pair<const T*, size_t> getComponent(uint64_t sequence) const
  {
    if (mComponents.size() == 0)
      return std::make_pair(nullptr, 0);

    if (isStatic())
      return std::make_pair(&mComponents.front().component, 0);

    auto last = mComponents.cbegin() + mLastSortedSize;

    // lower_bound uses a binary search to find the target component.
    // Our vector is always sorted, so we can pull this off.
    auto it = std::lower_bound(mComponents.cbegin(), last, sequence);

    if (it->sequence == sequence)
    {
      size_t index = it - mComponents.cbegin();
      return std::make_pair(&it->component, index);
    }
    else
    {
      return std::make_pair(nullptr, 0);
    }
  }

  int getNumComponentsWithSequence(uint64_t sequence) const override
  {
    int numComponents = 0;
    // We can't just use getComponentItemWithSequence because we want to
    // also search added items.

    if (mComponents.size() == 0) { return 0; }

    // lower_bound uses a binary search to find the target component.
    // Our vector is always sorted, so we can pull this off.
    auto it = std::lower_bound(mComponents.cbegin(), mComponents.cend(), sequence);

    if (it->sequence != sequence) { return 0; }

    const ComponentItem* item = &(*it);

    /// \xxx NOTE: When we separated the 'added' array from the primary array
    ///      we want to also search the added array here! This is in the
    ///      contract for the function (see BaseComponentContainer).

    // We are guaranteed that there is at least one component in the mComponents
    // array because nullptr was not returned from the
    // getComponentItemWithSequence function.
    auto curIndex = item - &mComponents[0];
    while (item->sequence == sequence
      && curIndex < static_cast<int>(mComponents.size()))
    {
      ++numComponents;
      ++curIndex;
      ++item;
    }

    return numComponents;
  }

  /// Sorts in added components and removes deleted components.
  /// Neither of these operations (addition or deletion) take affect when the
  /// system is operating. This way, each timestep is completely deterministic.
  /// The same set of data is acted upon by all systems.
  void renormalize(bool stableSort) override
  {
    // Changes should come FIRST. Changes rely on direct indices to values.
    // No additions or removals should come before modifications.

    // Removals should come *after* addition of new components. This keeps
    // logic consistent within one frame in an entire entity was removed.
    // If an entity was adding components to itself, then subsequently deleted,
    // some of it's components would be left dangling if removal was performed
    // first. But, adding before removal has no side effects, especially if
    // stable sort is used. Then removal from the beginning or end of the list
    // is guaranteed to be consistent.

    if (mModifications.size() > 0)
    {
      // Sort modifications to maintain some semblance of cache friendliness
      // and to detect modification conflicts and resolve with priority.
      // Stable sort is not needed, a priority is used to determine who
      // resolves the modifications.
      std::sort(mModifications.begin(), mModifications.end(), modificationCompare);

      // Simple iteration through the modifications to apply them to our
      // components.
      size_t attemptIdx = 0;
      size_t numMods = mModifications.size();
      for (; attemptIdx != numMods;)
      {
        size_t resolvedIndex = attemptIdx;
        while (   (attemptIdx + 1) != numMods
               && mModifications[attemptIdx + 1].componentIndex == mModifications[resolvedIndex].componentIndex)
        {
          ++attemptIdx;
          if (mModifications[attemptIdx].priority > mModifications[resolvedIndex].priority)
            resolvedIndex = attemptIdx;
        }

        // Now we have 1 fully resolved modification.
        if (mModifications[resolvedIndex].componentIndex < mComponents.size())
        {
          mComponents[mModifications[resolvedIndex].componentIndex].component = mModifications[resolvedIndex].value;
        }
        else
        {
          std::cerr << "entity-system - renormalize: Bad index!" << std::endl;
        }
        ++attemptIdx;
      }

      // Clear all modifications.
      mModifications.clear();
    }

    // Check to see if components were added. If so, then sort them into
    // our vector.
    if (mComponents.size() > 0)
    {
      if (mLastSortedSize != static_cast<int>(mComponents.size()))
      {
       // Iterate through the components to-be-constructed array, and construct.
        auto it = mComponents.begin() + mLastSortedSize;
        for (; it != mComponents.end(); ++it)
        {
          // Construct added components
          maybe_component_construct(it->component, it->sequence, 0);
        }

        // Sort the entire vector (not just to mLastSortedSize).
        // We *always* stable sort static components. This way we guarantee
        // the correct ordering.
        if (!stableSort && !isStatic())
          std::sort(mComponents.begin(), mComponents.end());
        else
          std::stable_sort(mComponents.begin(), mComponents.end());

        mLastSortedSize = static_cast<int>(mComponents.size());
      }

      mLowerSequence = mComponents.front().sequence;
      mUpperSequence = mComponents.back().sequence;
    }
    else
    {
      mLastSortedSize = 0;
      mLowerSequence = 0;
      mUpperSequence = 0;
    }

    // Perform requested removals.
    if (mRemovals.size() > 0)
    {
      for (RemovalItem& rem : mRemovals)
      {
        auto last = mComponents.begin() + mLastSortedSize;
        auto it = std::lower_bound(mComponents.begin(), last, rem.sequence);

        if (rem.removeType == REMOVE_ALL)
        {
          while (it != mComponents.end() && it->sequence == rem.sequence)
          {
            // Call components destructor (we need to do this at the end).
            maybe_component_destruct(it->component, rem.sequence, 0);

            it = mComponents.erase(it);
            --mLastSortedSize;
          }
        }
        else if (rem.removeType == REMOVE_LAST)
        {
          auto priorIt = it;
          if (it != mComponents.end()) ++it;
          while (it != mComponents.end() && it->sequence == rem.sequence)
          {
            priorIt = it;
            ++it;
          }

          // Call component's destructor (we need to do this at the end).
          if (priorIt != mComponents.end() && priorIt->sequence == rem.sequence)
          {
            maybe_component_destruct(priorIt->component, rem.sequence, 0);
            priorIt = mComponents.erase(priorIt);
            --mLastSortedSize;
          }
        }
        else if (rem.removeType == REMOVE_INDEX)
        {
          int index = 0;
          while (it != mComponents.end() && it->sequence == rem.sequence)
          {
            if (index == rem.removeIndex)
            {
              // Call component's destructor (we need to do this at the end).
              maybe_component_destruct(it->component, rem.sequence, 0);

              it = mComponents.erase(it);
              --mLastSortedSize;
              break;
            }
            else
            {
              ++it;
            }
            ++index;
          }
        }
        else // if (rem.removeType == REMOVE_FIRST)
        {
          if (it != mComponents.end() && it->sequence == rem.sequence)
          {
            maybe_component_destruct(it->component, rem.sequence, 0);

            it = mComponents.erase(it);
            --mLastSortedSize;
          }
        }
      }
      mRemovals.clear();
    }
  }

  /// Get the least sequence held by the component.
  uint64_t getLowerSequence() const override { return mLowerSequence; }

  /// Get the upper sequence held by the component.
  uint64_t getUpperSequence() const override { return mUpperSequence; }

  /// Retrieves the number of sorted components.
  uint64_t getNumComponents() const override { return mLastSortedSize; }

  /// Retrieves the sequence associated with a particular index.
  uint64_t getSequenceFromIndex(int index) const override
  {
    if (index < 0 || index >= mLastSortedSize)
      return 0;

    return mComponents[index].sequence;
  }

  /// Adds the component to the end of our components list. It will only become
  /// available upon renormalization (which usually occurs at the end of
  /// a frame).
  void addComponent(uint64_t sequence, const T& component)
  {
    // Add the component to the end of mComponents and wait for a renormalize.
    if (isStatic() == true)
    {
      std::cerr << "Attempting to add entityID component to a static component container!" << std::endl;
      throw std::runtime_error("Attempting to add entityID component to static component container!");
      return;
    }
    mComponents.emplace_back(sequence, component);
  }

  virtual std::string describe() const override
  {
    std::ostringstream ostr;
    ostr << "ComponentContainer(): " << "\n";
    ostr << "\tmLastSortedSize: " << mLastSortedSize << "\n";
    ostr << "\tmUpperSequence: " << mUpperSequence << "\n";
    ostr << "\tmLowerSequence: " << mLowerSequence << "\n";
    ostr << "\tmIsStatic: " << mIsStatic << "\n";
    ostr << "\tmComponents.size: " << mComponents.size() << "\n";
    ostr << "\tmRemovals.size: " << mRemovals.size() << "\n";
    ostr << "\tmModifications.size: " << mModifications.size() << "\n";
    return ostr.str();
  }

  /// Returns the index the static component was added at.
  size_t addStaticComponent(const T& component)
  {
   if (isStatic() == false)
   {
     if (mComponents.size() > 0)
     {
       std::cerr << "Cannot add static components to a container that already has";
       std::cerr << " non-static\ncomponents!" << std::endl;
       throw std::runtime_error("Cannot add static components to an entityID component container!");
       return -1;
     }
     else
     {
       setStatic(true);
     }
   }
   size_t newIndex = mComponents.size();
   mComponents.emplace_back(StaticEntID, component);
   return newIndex;
  }

  /// Returns the index the static component was added at.
  size_t addStaticComponent(T&& component)
  {
   if (isStatic() == false)
   {
     if (mComponents.size() > 0)
     {
       std::cerr << "Cannot add static components to a container that already has";
       std::cerr << " non-static\ncomponents!" << std::endl;
       throw std::runtime_error("Cannot add static components to an entityID component container!");
       return -1;
     }
     else
     {
       setStatic(true);
     }
   }
   size_t newIndex = mComponents.size();
   mComponents.emplace_back(StaticEntID, std::forward<T>(component));
   return newIndex;
  }

  void removeSequence(uint64_t sequence) override
  {
    /// \todo Check size of mRemovals. Ensure it is not greater than the size
    ///       mComponents is allowed to grow to.
    mRemovals.emplace_back(sequence, REMOVE_ALL);
  }

  void removeFirstSequence(uint64_t sequence) override
  {
    mRemovals.emplace_back(sequence, REMOVE_FIRST);
  }

  void removeLastSequence(uint64_t sequence) override
  {
    mRemovals.emplace_back(sequence, REMOVE_LAST);
  }

  void removeSequenceWithIndex(uint64_t sequence, int32_t componentID) override
  {
    mRemovals.emplace_back(sequence, REMOVE_INDEX, componentID);
  }

  void removeAll() override
  {
    // Iterate through all active components and mark every one for removal.
    uint64_t lastComponent = 0; // No component will ever be 0, including static components
                                // (and because of static components...).
    auto last = mComponents.begin() + mLastSortedSize;
    for (auto it = mComponents.begin(); it != last; ++it)
    {
      if (lastComponent != it->sequence)
      {
        removeSequence(it->sequence);
      }
      lastComponent = it->sequence;
    }
  }

  void removeAllImmediately() override
  {
    for (auto it = mComponents.begin(); it != mComponents.begin() + mLastSortedSize; ++it)
    {
      maybe_component_destruct(it->component, it->sequence, 0);
    }

    mComponents.clear();
    mRemovals.clear();
    mModifications.clear();

    // Clear state related to mComponents.
    mLastSortedSize = 0;
    mUpperSequence = 0;
    mLowerSequence = 0;
  }

  ComponentItem* getComponentArray()
  {
    if (mComponents.size() != 0)
      return &mComponents[0];
    else
      return nullptr;
  }

  void modifyIndex(const T& val, size_t index, int priority)
  {
    mModifications.emplace_back(val, index, priority);
  }

  /// Retrieves the active size of the vector backing this component container.
  /// Used only for debugging purposes (see addStaticComponent in ESCore).
  size_t getSizeOfBackingContainer()  {return mComponents.size();}

  bool isStatic() const override    {return mIsStatic;}
  void setStatic(bool truth)        {mIsStatic = truth;}

  int mLastSortedSize;                ///< Unsorted elements can be added to the end
                                      ///< of mComponents. This represents the last
                                      ///< sorted element inside mComponents.
  uint64_t mUpperSequence;                 ///< Largest sequence in the list.
  uint64_t mLowerSequence;                 ///< Smallest sequence in the list.

  bool mIsStatic;                     ///< True if this container contains static
                                      ///< component data.

  /// \todo Look into possibly optimizing binary search by having a separate
  ///       vector containing component sequences. We are at less of a risk
  ///       of cache hits that way.
  ///       This simple implementation is to just sort mComponents, and copy
  ///       over the sequences. But I don't know if this will help or hurt
  ///       overall performance, so we should look into that later.

  enum REMOVAL_TYPE
  {
    REMOVE_ALL,
    REMOVE_FIRST,
    REMOVE_LAST,
    REMOVE_INDEX
  };

  struct RemovalItem
  {
    RemovalItem(uint64_t sequenceIn, REMOVAL_TYPE removeTypeIn, int32_t index = -1) :
        sequence(sequenceIn),
        removeType(removeTypeIn),
        removeIndex(index)
    {}

    uint64_t      sequence;
    REMOVAL_TYPE  removeType;
    int32_t       removeIndex;
  };

  struct ModificationItem
  {
    ModificationItem(const T& val, size_t idx, int pri) :
        value(val),
        componentIndex(idx),
        priority(pri)
    {}

    //ModificationItem(T&& val, size_t idx, int pri) :
    //    value(std::move(val)),
    //    componentIndex(idx),
    //    priority(pri)
    //{}

    T value;
    size_t componentIndex;
    int priority;
  };

  static bool modificationCompare(const ModificationItem& a, const ModificationItem& b)
  {
    return a.componentIndex < b.componentIndex;
  }

protected:
  /// \todo Separate out additions to mComponents instead of having them
  ///       added into mComponents. We don't want the address of mComponents
  ///       to change during the frame if we want to execute tasks
  ///       asynchronously against the components. In this sense, we could
  ///       execute systems asynchronously as long as they don't generate
  ///       unintended mutations of global state.
  std::vector<ComponentItem>    mComponents;    ///< All components currently in the system.
  std::vector<RemovalItem>      mRemovals;      ///< An array of objects to remove during
                                                ///< renormalization.
  std::vector<ModificationItem> mModifications; ///< An array of objects whose values need
                                                ///< to be updated during renormalization.
};

} // namespace spire

#endif

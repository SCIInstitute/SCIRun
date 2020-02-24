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


#ifndef SPIRE_ENTITY_SYSTEM_GENERICSYSTEM_HPP
#define SPIRE_ENTITY_SYSTEM_GENERICSYSTEM_HPP

// GenericSystems are not associated with any one core. You can use a system
// with any number of cores. All important functions are static and don't
// use any stored state. You can create instances of this class and use
// the BaseSystem interface to iterate over all systems with the
// walkComponentsOver override.

#include <es-log/trace-log.h>
#include <iostream>
#include <array>
#include <set>          // Only used in a corner case of walkComponents where
                        // all components are optional.
#include <type_traits>
#include "ESCoreBase.hpp"
#include "src/ComponentContainer.hpp"
#include "src/TemplateID.hpp"
#include "src/ComponentGroup.hpp"
#include <spire/scishare.h>

namespace spire {


namespace gs_detail
{

/// See: http://stackoverflow.com/questions/10766112/c11-i-can-go-from-multiple-args-to-tuple-but-can-i-go-from-tuple-to-multiple
template <typename C, typename F, typename Tuple, bool Done, int Total, int... N>
struct call_impl
{
  static void call(ESCoreBase& core, uint64_t sequence, C c, F f, Tuple && t)
  {
    call_impl<C, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(core, sequence, c, f, std::forward<Tuple>(t));
  }
};

template <typename C, typename F, typename Tuple, int Total, int... N>
struct call_impl<C, F, Tuple, bool(true), Total, N...>
{
  static void call(ESCoreBase& core, uint64_t sequence, C c, F f, Tuple && t)
  {
    // Call the variadic member function (c.*f) with expanded template
    // parameter list (std::get<N>(std::forward<Tuple>(t))...).
    (c->*f)(core, sequence, std::get<N>(std::forward<Tuple>(t))...);
  }
};

/// C = class type, F = function type.
/// c = class instance, f = member function pointer.
template <typename C, typename F, typename Tuple>
void call(ESCoreBase& core, uint64_t sequence, C c, F f, Tuple && t)
{
  typedef typename std::decay<Tuple>::type ttype;
  call_impl<C, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(core, sequence, c, f, std::forward<Tuple>(t));
}

}

// Unfortunately, this part still does not compile in VS2013. Seems safe to comment out.
#ifndef WIN32
/// See: http://stackoverflow.com/questions/18986560/check-variadic-templates-parameters-for-uniqueness?lq=1
/// The mpl namespace contains everything to implement is_unique. It is used in
/// the static assertion at the beginning of GenericSystem below.
namespace mpl
{
template< class T > using invoke = typename T :: type ;
template< class C, class I, class E > using if_t     = invoke< std::conditional< C{}, I, E> >;
template< class T > struct id{};
struct empty{};
template< class A, class B > struct base : A, B {};
template< class B , class ... > struct is_unique_impl;
template< class B > struct is_unique_impl<B>: std::true_type{};
template< class B, class T, class ... U>
struct is_unique_impl<B, T, U...> : if_t< std::is_base_of< id<T>, B>, std::false_type, is_unique_impl< base<B,id<T>>, U...> >{};

template< class ...T >struct is_unique : is_unique_impl< empty, T ... > {};
} // mpl
#endif

/// Base class implementation of generic system. You do not need instances of
/// this class in order to use it. All important functions are static.
template <bool GroupComponents, typename... Ts>
class GenericSystem : public BaseSystem
{
public:
#ifndef WIN32
  static_assert(mpl::is_unique<Ts...>::value, "GenericSystem does not allow duplicate component types.");
#endif
  GenericSystem()           {}
  virtual ~GenericSystem()  {}

  bool walkEntity(ESCoreBase& core, uint64_t entityID) override
  {
    if (sizeof...(Ts) == 0)
      return false;

    std::array<BaseComponentContainer*, sizeof...(Ts)> baseComponents = { core.getComponentContainer(TemplateID<Ts>::getID())... };
    std::array<int, sizeof...(Ts)> indices;
    std::array<int, sizeof...(Ts)> nextIndices;
    std::array<bool, sizeof...(Ts)> isStatic;
    std::array<int, sizeof...(Ts)> numComponents;
    std::array<bool, sizeof...(Ts)> optionalComponents = { isComponentOptional(TemplateID<Ts>::getID())... }; // Detect optional components via overriden function call (simplest).

    // Arrays containing components arrays. BuildComponentArrays was made to be
    // called before we correct baseComponents with getEmptyContainer.
    std::tuple<typename ComponentContainer<Ts>::ComponentItem*...> componentArrays;
    BuildComponentArraysAndIndicesWithID<0, Ts...>::exec(baseComponents, componentArrays, indices, entityID);

    std::tuple<Ts*...> values;  ///< Values that will be passed into execute.
    std::tuple<ComponentGroup<Ts>...> groupValues;  ///< Grouped values that will be passed into group execute.

    AddComponentsToGroupInputs<0, Ts...>::exec(core, baseComponents, groupValues);

    bool execute = true;
    for (size_t i = 0; i < sizeof...(Ts); i++)
    {
      if (!baseComponents[i])
        baseComponents[i] = ESCoreBase::getEmptyContainer();

      isStatic[i] = baseComponents[i]->isStatic();
      numComponents[i] = static_cast<int>(baseComponents[i]->getNumComponents());

      // Note: For static components, getComponentItemIndexWithSequence always
      //       returns 0. Which is the correct index to start at.
      if (indices[i] == -1)
      {
        if (!optionalComponents[i])
        {
          execute = false;
          break;
        }
        else
        {
          // For optional components that we don't find, we set the index to the
          // size of the components array. When recursively executing, this
          // will continue the recursion with no extra branches as a result
          // of this component.
          indices[i] = numComponents[i];
        }
      }
    }

    if (execute)
    {
      if (!GroupComponents)
        RecurseExecute<0, Ts...>::exec(core, this, componentArrays, numComponents,
                                       indices, optionalComponents, isStatic,
                                       nextIndices, values, entityID);
      else
        GroupExecute<0, Ts...>::exec(core, this, componentArrays, numComponents,
                                     indices, optionalComponents, isStatic,
                                     nextIndices, groupValues, entityID);
      return true;
    }
    else
    {
      return false;
    }
  }

  void walkComponents(ESCoreBase& core) override
  {
    preWalkComponents(core);
    walkComponentsInternal(core);
    postWalkComponents(core);
  }

  //TODO: log the **** out of this function
  void walkComponentsInternal(ESCoreBase& core)
  {
    if (sizeof...(Ts) == 0)
      return;

    std::array<BaseComponentContainer*, sizeof...(Ts)> baseComponents = { core.getComponentContainer(TemplateID<Ts>::getID())... };
    std::array<int, sizeof...(Ts)> indices;
    std::array<int, sizeof...(Ts)> nextIndices;
    std::array<int, sizeof...(Ts)> numComponents;
    std::array<bool, sizeof...(Ts)> isStatic;
    std::array<bool, sizeof...(Ts)> optionalComponents = { isComponentOptional(TemplateID<Ts>::getID())... }; // Detect optional components via overriden function call (simplest).

    // Arrays containing components arrays. BuildComponentArrays was made to be
    // called before we correct baseComponents with getEmptyContainer.
    std::tuple<typename ComponentContainer<Ts>::ComponentItem*...> componentArrays;
    BuildComponentArrays<0, Ts...>::exec(baseComponents, componentArrays);

    // Start off with std::numeric_limits max on lowest upper sequence,
    // and check for optional components. No optional components are allowed
    // to be the leading sequence. If all we find are optional components,
    // we write out a warning. We could also iterate through all components
    // and combinations in existence. We may implement this later and warn
    // the user about possible performance problems when doing this.
    uint64_t lowestUpperSequence = std::numeric_limits<uint64_t>::max();
    int leadingComponent = -1;
    for (size_t i = 0; i < sizeof...(Ts); ++i)
    {
      if (!baseComponents[i])
        baseComponents[i] = ESCoreBase::getEmptyContainer();

      bool optional = optionalComponents[i];
      // An empty mandatory component results in an immediate termination
      // of the walk. Even if the component is static.
      if (baseComponents[i]->getNumComponents() == 0 && !optional)
        return;

      indices[i] = 0;
      nextIndices[i] = 0;
      numComponents[i] = static_cast<int>(baseComponents[i]->getNumComponents());
      isStatic[i] = baseComponents[i]->isStatic();

      // Optional and static components are not allowed to be leading components
      if (optional || isStatic[i])
        continue;

      if (baseComponents[i]->getUpperSequence() < lowestUpperSequence)
      {
        leadingComponent = static_cast<int>(i);
        lowestUpperSequence = baseComponents[i]->getUpperSequence();
      }
    }

    std::tuple<Ts*...> values;                      ///< Values that will be passed into execute.
    std::tuple<ComponentGroup<Ts>...> groupValues;  ///< Grouped values that will be passed into group execute.

    AddComponentsToGroupInputs<0, Ts...>::exec(core, baseComponents, groupValues);

    // leadingComponent == -1 if and only if the number of optional and static
    // components == sizeof...(Ts). Because the if statement following the
    // "if (optional || isStatic) contitue;" statement will always succeed
    // due to the numeric_limits setting, leading to a valid value of
    // loading component.
    if (leadingComponent != -1)
    {
      // This clause is called when there is at least 1 non-optional
      // (mandatory) non-static component in the system.

      // Start off at the first target sequence and march forward from there.
      uint64_t targetSequence = baseComponents[leadingComponent]->getSequenceFromIndex(indices[leadingComponent]);
      while (targetSequence != 0)
      {
        // Find the target sequence in the other components.
        bool failed = false;
        for (size_t i = 0; i < baseComponents.size(); ++i)
        {
          uint64_t curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
          bool optional = optionalComponents[i];
          bool compStatic = isStatic[i];

          // Static components don't have associated sequences.
          // They are always at index 0 and always valid. Existance of some
          // static components were guaranteed above.
          if (!compStatic)
          {
            // Note: when indices[i] = numComponents[i], we necessarily have an
            // optional component. This is because of the check for a zero number
            // of components above, in conjunction with the iteration logic
            // below. We always exit the function if we reach the end of a
            // mandatory component's array; but for an optional we break out and
            // continue.
            while (curSequence < targetSequence && indices[i] != numComponents[i])
            {
              ++indices[i];
              // Check to see if this is an optional component! If it is, then
              // we shouldn't return!
              if (indices[i] == numComponents[i])
              {
                if (!optional)
                  return;   // We are done -- this was a mandatory component and we reached the end of its list.
                else
                  break;    // Can't go any further.
              }
              curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
            }

            if (curSequence != targetSequence && !optional)
            {
              failed = true;
              break;
            }
          }
        }

        if (!failed)
        {
          // Execute with indices. This recursive execute will perform a cartesian
          // product.
          if (!GroupComponents)
          {
            if (!RecurseExecute<0, Ts...>::exec(core, this, componentArrays,
                                                numComponents, indices,
                                                optionalComponents, isStatic,
                                                nextIndices, values,
                                                targetSequence))
            {
              return; // We have reached the end of an array.
            }
          }
          else
          {
            if (!GroupExecute<0, Ts...>::exec(core, this, componentArrays,
                                              numComponents, indices,
                                              optionalComponents, isStatic,
                                              nextIndices, groupValues,
                                              targetSequence))
            {
              return;
            }
          }

          // Copy nextIndices into indices.
          indices = nextIndices;
        }

        // Find new target sequence.
        while (baseComponents[leadingComponent]->getSequenceFromIndex(indices[leadingComponent]) == targetSequence)
        {
          ++indices[leadingComponent];
          if (indices[leadingComponent] == numComponents[leadingComponent])
            return;   // We are done.
        }
        targetSequence = baseComponents[leadingComponent]->getSequenceFromIndex(indices[leadingComponent]);
      }
    }
    else
    {
      // This else clause is called when *all* components are optional or
      // static. Not a single one is mandatory other than the static components,
      // whose existence has already been tested above.

      // Now we walk in a different fashion than above.

      // Determine list of entity ids that need to be walked. We use these as
      // the list of target sequences to be used when walking all components.
      // We must keep the sequenced sorted. So we use a map to accomplish this.

      // Instead of a target sequence, we will use this list of sorted sequences
      // to walk the components.

      // Create the target sequence list by recursing through our parameters.
      /// Optimize? Set will cause dynamic memory allocation. *But*, this
      /// is only a corner case and very few, if any, systems need
      /// this case where all components are optional.
      std::set<uint64_t> sequenceSet;
      GenSequenceSet<0, Ts...>::exec(sequenceSet, componentArrays,
                                     isStatic, numComponents);

      if (sequenceSet.size() != 0)
      {
        // Use the set as a list of target sequences when iterating over the
        // sequences.
        for (auto it = sequenceSet.begin(); it != sequenceSet.end(); ++it)
        {
          uint64_t targetSequence = *it;

          // Find the target sequence in the components.
          for (size_t i = 0; i < baseComponents.size(); ++i)
          {
            if (!isStatic[i])
            {
              uint64_t curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
              while (curSequence < targetSequence && indices[i] != numComponents[i])
              {
                ++indices[i];
                if (indices[i] == numComponents[i])
                  break;   // We are done.
                curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
              }
            }

            // We are never going to fail finding this sequence in at least one
            // of the optional components.
          }

          // Execute with indices. This recursive execute will perform a cartesian
          // product.
          if (!GroupComponents)
          {
            if (!RecurseExecute<0, Ts...>::exec(core, this, componentArrays,
                                                numComponents, indices,
                                                optionalComponents, isStatic,
                                                nextIndices, values,
                                                targetSequence))
            {
              return; // We have reached the end of an array.
            }
          }
          else
          {
            if (!GroupExecute<0, Ts...>::exec(core, this, componentArrays,
                                              numComponents, indices,
                                              optionalComponents, isStatic,
                                              nextIndices, groupValues,
                                              targetSequence))
            {
              return; // We have reached the end of an arry
            }
          }

          // Copy nextIndices into indices.
          indices = nextIndices;
        }
      }
      else // if (sequenceSet.size() != 0)
      {
        // When the sequence set is zero, that means that we could have a corner
        // case where all of the components are static. If that is the case,
        // then we simply iterate over all static members, either as a group
        // or recursively. If there are any optional components, then we don't
        // execute at all.
        bool allStatic = true;
        for (size_t i = 0; i < sizeof...(Ts); ++i)
        {
          if (!isStatic[i])
          {
            allStatic = false;
            break;
          }
        }

        if (allStatic)
        {
          if (!GroupComponents)
          {
            if (!RecurseExecute<0, Ts...>::exec(core, this, componentArrays,
                                                numComponents, indices,
                                                optionalComponents, isStatic,
                                                nextIndices, values,
                                                BaseComponentContainer::StaticEntID))
            {
              return; // We have reached the end of an array.
            }
          }
          else
          {
            if (!GroupExecute<0, Ts...>::exec(core, this, componentArrays,
                                              numComponents, indices,
                                              optionalComponents, isStatic,
                                              nextIndices, groupValues,
                                              BaseComponentContainer::StaticEntID))
            {
              return; // We have reached the end of an arry
            }
          }
        }
      }
    }
  }

  std::vector<uint64_t> getComponents() const override
  {
    std::vector<uint64_t> components = { TemplateID<Ts>::getID()... };
    return components;
  }

  template <int TupleIndex, typename... RTs>
  struct GenSequenceSet;

  template <int TupleIndex, typename RT, typename... RTs>
  struct GenSequenceSet<TupleIndex, RT, RTs...>
  {
    static void exec(std::set<uint64_t>& sequenceSet,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     const std::array<int, sizeof...(Ts)>& componentSizes)
    {
      // array may be a nullptr!
      typename ComponentContainer<RT>::ComponentItem* array = std::get<TupleIndex>(componentArrays);
      bool isStatic     = componentStatic[TupleIndex];

      // Only generate set of sequences if we do not have a static component.
      // Static componets always have 1 sequence, and they are handled
      // internally by the RecurseExecute and GroupExecute template structures.
      if (!isStatic)
      {
        // Loop over components at this RT.
        for (int i = 0; i < componentSizes[TupleIndex]; ++i)
        {
          sequenceSet.insert(array[i].sequence);
        }
      }

      // Recurse into corresponding container.
      GenSequenceSet<TupleIndex + 1, RTs...>::exec(
          sequenceSet, componentArrays, componentStatic, componentSizes);
    }
  };

  template <int TupleIndex>
  struct GenSequenceSet<TupleIndex>
  {
    static void exec(std::set<uint64_t>& /*sequenceSet*/,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& /* componentArrays */,
                     const std::array<bool, sizeof...(Ts)>& /* componentStatic */,
                     const std::array<int, sizeof...(Ts)>& /* componentSizes */)
    {}
  };

  template <int TupleIndex, typename... RTs>
  struct BuildComponentArrays;

  template <int TupleIndex, typename RT, typename... RTs>
  struct BuildComponentArrays<TupleIndex, RT, RTs...>
  {
    static void exec(
        std::array<BaseComponentContainer*, sizeof...(Ts)>& baseComponents,
        std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays)
    {
      if (baseComponents[TupleIndex] != nullptr)
      {
        std::get<TupleIndex>(componentArrays) =
            dynamic_cast<ComponentContainer<RT>*>(baseComponents[TupleIndex])->getComponentArray();
      }
      else
      {
        std::get<TupleIndex>(componentArrays) = nullptr;
      }
      BuildComponentArrays<TupleIndex + 1, RTs...>::exec(baseComponents, componentArrays);
    }
  };

  template <int TupleIndex>
  struct BuildComponentArrays<TupleIndex>
  {
    static void exec(
        std::array<BaseComponentContainer*, sizeof...(Ts)>&,
        std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>&)
    {}
  };

  template <int TupleIndex, typename... RTs>
  struct BuildComponentArraysAndIndicesWithID;

  template <int TupleIndex, typename RT, typename... RTs>
  struct BuildComponentArraysAndIndicesWithID<TupleIndex, RT, RTs...>
  {
    static void exec(
        std::array<BaseComponentContainer*, sizeof...(Ts)>& baseComponents,
        std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
        std::array<int, sizeof...(Ts)>& indices, uint64_t entityID)
    {
      if (baseComponents[TupleIndex] != nullptr)
      {
        ComponentContainer<RT>* derived = dynamic_cast<ComponentContainer<RT>*>(baseComponents[TupleIndex]);

        std::get<TupleIndex>(componentArrays) = derived->getComponentArray();
        int foundIndex = derived->getComponentItemIndexWithSequence(entityID);
        indices[TupleIndex] = foundIndex;
      }
      else
      {
        std::get<TupleIndex>(componentArrays) = nullptr;
        indices[TupleIndex] = 0;
      }
      BuildComponentArraysAndIndicesWithID<TupleIndex + 1, RTs...>::exec(
          baseComponents, componentArrays, indices, entityID);
    }
  };

  template <int TupleIndex>
  struct BuildComponentArraysAndIndicesWithID<TupleIndex>
  {
    static void exec(
        std::array<BaseComponentContainer*, sizeof...(Ts)>&,
        std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>&,
        std::array<int, sizeof...(Ts)>&, uint64_t)
    {}
  };

  template <int TupleIndex, typename... RTs>
  struct AddComponentsToGroupInputs;

  template <int TupleIndex, typename RT, typename... RTs>
  struct AddComponentsToGroupInputs<TupleIndex, RT, RTs...>
  {
    static void exec(ESCoreBase& core, std::array<BaseComponentContainer*, sizeof...(Ts)>& baseComponents,
                     std::tuple<ComponentGroup<Ts>...>& input)
    {
      if (baseComponents[TupleIndex] != nullptr)
      {
        std::get<TupleIndex>(input).container
            = dynamic_cast<ComponentContainer<RT>*>(core.getComponentContainer(TemplateID<RT>::getID()));
      }
      else
      {
        std::get<TupleIndex>(input).container = nullptr;
      }
      AddComponentsToGroupInputs<TupleIndex + 1, RTs...>::exec(core, baseComponents, input);
    }
  };

  template <int TupleIndex>
  struct AddComponentsToGroupInputs<TupleIndex>
  {
    static void exec(ESCoreBase& /* core */, std::array<BaseComponentContainer*, sizeof...(Ts)>& /* baseComponents */,
                     std::tuple<ComponentGroup<Ts>...>& /* input */) {}
  };

  template <int TupleIndex, typename... RTs>
  struct RecurseExecute;

  template <int TupleIndex, typename RT, typename... RTs>
  struct RecurseExecute<TupleIndex, RT, RTs...>
  {
    static bool exec(ESCoreBase& core, GenericSystem<GroupComponents, Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<Ts*...>& input,
                     uint64_t targetSequence)
    {
      int arraySize     = componentSizes[TupleIndex];
      int currentIndex  = indices[TupleIndex];
      bool optional     = componentOptional[TupleIndex];
      bool isStatic     = componentStatic[TupleIndex];
      typename ComponentContainer<RT>::ComponentItem* array = std::get<TupleIndex>(componentArrays);

      // Check to see if we are at the end of the array. This will happen with
      // optional components.
      if (currentIndex == arraySize)
      {
        if (optional)
        {
          std::get<TupleIndex>(input) = nullptr;
        }
        else
        {
          // Terminate early if we are at the end of the array and are not an
          // optional component.
          return false;
        }
      }
      else
      {
        // Only pass target sequence. We won't get here if arraySize == 0.
        if (array[currentIndex].sequence != targetSequence || isStatic)
        {
          if (isStatic && arraySize > 0)
          {
            std::get<TupleIndex>(input) = &array[0].component;
          }
          else
          {
            if (!optional)
              std::cerr << "Generic System: invalid sequence on non-optional component!!" << std::endl;
            std::get<TupleIndex>(input) = nullptr;
          }
        }
        else
        {
          std::get<TupleIndex>(input) = &array[currentIndex].component;
        }
      }

      // Depth first this will perform one execution with the target sequence
      // we found.
      bool reachedEnd = false;
      if (!RecurseExecute<TupleIndex + 1, RTs...>::exec(core, ptr,
              componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence))
      {
        reachedEnd = true;
      }

      // Only increment current index if we are currently on the target
      // sequence. This *can* happen if we have an optional component that
      // passed nullptr in as its parameter.
      if (!isStatic)
      {
        if (array && array[currentIndex].sequence == targetSequence)
          ++currentIndex;

        if (currentIndex == arraySize)
        {
          if (nextIndices[TupleIndex] < currentIndex)
            nextIndices[TupleIndex] = currentIndex;

          // Terminate early in either case, but with different return values.
          // For optional components, reaching the end of the array does not
          // necessarily mean we should terminate the algorithm.
          return optional;
        }

        // Loop until we find a sequence that is not in our target.
        while (array[currentIndex].sequence == targetSequence)
        {
          std::get<TupleIndex>(input) = &array[currentIndex].component;
          // We don't need to check return value of RecurseExecute since any
          // lower component would have returned false on the first call. The
          // result of the first call is cached in reachedEnd.
          RecurseExecute<TupleIndex + 1, RTs...>::exec(core,
              ptr, componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence);

          ++currentIndex;
          if (currentIndex == arraySize)
          {
            if (optional)
              break;
            else
              return false;
          }
        }

        if (nextIndices[TupleIndex] < currentIndex)
          nextIndices[TupleIndex] = currentIndex;
      }
      else // if (!isStatic)
      {
        // Loop over static array (we've already executed the first element above).
        for (int i = 1; i < arraySize; ++i)
        {
          std::get<TupleIndex>(input) = &array[i].component;
          // We don't need to check return value of RecurseExecute since any
          // lower component would have returned false on the first call. The
          // result of the first call is cached in reachedEnd.
          RecurseExecute<TupleIndex + 1, RTs...>::exec(core,
              ptr, componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence);
        }
      }

      return !reachedEnd;
    }
  };

  template <int TupleIndex>
  struct RecurseExecute<TupleIndex>
  {
    static bool exec(ESCoreBase& core, GenericSystem<GroupComponents, Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<Ts*...>& input,
                     uint64_t targetSequence)
    {
      // Call our execute function with 'targetSequence' and 'input'.
      gs_detail::call(core, targetSequence, ptr, &GenericSystem<GroupComponents, Ts...>::execute, input);
      return true;
    }
  };

  template <int TupleIndex, typename... RTs>
  struct GroupExecute;

  template <int TupleIndex, typename RT, typename... RTs>
  struct GroupExecute<TupleIndex, RT, RTs...>
  {
    static bool exec(ESCoreBase& core, GenericSystem<GroupComponents, Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<ComponentGroup<Ts>...>& input,
                     uint64_t targetSequence)
    {
      int arraySize     = componentSizes[TupleIndex];
      int currentIndex  = indices[TupleIndex];
      bool optional     = componentOptional[TupleIndex];
      bool isStatic     = componentStatic[TupleIndex];
      typename ComponentContainer<RT>::ComponentItem* array = std::get<TupleIndex>(componentArrays);
      bool endOfArray   = false;

      std::get<TupleIndex>(input).containerIndex = currentIndex;

      // Check to see if we are at the end of the array. This will happen with
      // optional components. Static components will never enter this case.
      if (currentIndex == arraySize)
      {
        if (optional)
        {
          std::get<TupleIndex>(input).numComponents = 0;
          std::get<TupleIndex>(input).components = nullptr;
        }
        else
        {
          // Terminate early if we are at the end of the array and are not an
          // optional component (this works for isStatic as well).
          return false;
        }
      }
      else
      {
        // Only pass target sequence. array may be a nullptr, but we will never
        // reach this condition if that is the case. arraySize will be 0 as will
        // be currentIndex.
        if (array[currentIndex].sequence != targetSequence || isStatic)
        {
          if (isStatic && arraySize > 0)
          {
            std::get<TupleIndex>(input).numComponents = arraySize;
            std::get<TupleIndex>(input).components = &array[0];
          }
          else
          {
            if (!optional) std::cerr << "Generic System: invalid sequence on non-optional or static component!!" << std::endl;
            std::get<TupleIndex>(input).numComponents = 0;
            std::get<TupleIndex>(input).components = nullptr;
          }
        }
        else
        {
          /// This is the only case in which we need to determine the number
          /// of components.
          int numComponents = 0;

          // Set value before we reassign currentIndex.
          std::get<TupleIndex>(input).components = &array[currentIndex];

          // Loop until we find a sequence that is not in our target.
          while (array[currentIndex].sequence == targetSequence)
          {
            ++currentIndex;
            ++numComponents;
            if (currentIndex == arraySize)
            {
              endOfArray = true;
              break;
            }
          }

          // Now set how many components should exist.
          std::get<TupleIndex>(input).numComponents = numComponents;
        }
      }

      // Depth first this will perform one execution with the target sequence
      // we found.
      if (!GroupExecute<TupleIndex + 1, RTs...>::exec(core, ptr,
              componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence))
      {
        return false;
      }

      if (nextIndices[TupleIndex] < currentIndex)
        nextIndices[TupleIndex] = currentIndex;

      if (endOfArray && !optional)
        return false;
      else
        return true;
    }
  };

  template <int TupleIndex>
  struct GroupExecute<TupleIndex>
  {
    static bool exec(ESCoreBase& core, GenericSystem<GroupComponents, Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<ComponentGroup<Ts>...>& input,
                     uint64_t targetSequence)
    {
      // Call our execute function with 'targetSequence' and 'input'.
      gs_detail::call(core, targetSequence, ptr, &GenericSystem<GroupComponents, Ts...>::groupExecute, input);
      return true;
    }
  };

  // Non-grouped version of execute.
  virtual void execute(ESCoreBase&, uint64_t, const Ts*... vs)
  {
    std::cerr << "entity-system: Unimplmented system execute." << std::endl;
    throw std::runtime_error("entity-system: Unimplemented system execute.");
  }

  // Grouped version of execute.
  virtual void groupExecute(ESCoreBase&, uint64_t, const ComponentGroup<Ts>&... groups)
  {
    std::cerr << "entity-system: Unimplmented system group execute." << std::endl;
    throw std::runtime_error("entity-system: Unimplemented system group execute.");
  }

  /// This function should be overriden and return true for all components
  /// which may be optional.
  virtual bool isComponentOptional(uint64_t templateID) override {return false;}

  /// This function gets called before we start walking components from the
  /// walkComponents function.
  virtual void preWalkComponents(ESCoreBase& core)            {}

  /// This function gets called after we finish walking components from the
  /// walkComponents function.
  virtual void postWalkComponents(ESCoreBase& core)           {}
};

namespace optional_components_impl
{
template <typename... RTs>
struct OptionalCompImpl;

template <typename RT, typename... RTs>
struct OptionalCompImpl<RT, RTs...>
{
  static bool exec(uint64_t templateID)
  {
    if (TemplateID<RT>::getID() == templateID) return true;
    else return OptionalCompImpl<RTs...>::exec(templateID);
  }
};

template <>
struct OptionalCompImpl<>
{
  static bool exec(uint64_t templateID)  {return false;}
};
} // namespace optional_components_impl

template <typename... RTs>
bool OptionalComponents(uint64_t templateID)
{
  // Recursively determine if any of the given template parameters match the
  // given templateID.
  return optional_components_impl::OptionalCompImpl<RTs...>::exec(templateID);
}

} // namespace spire

#endif

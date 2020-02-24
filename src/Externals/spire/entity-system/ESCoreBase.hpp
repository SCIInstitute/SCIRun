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


#ifndef SPIRE_ENTITY_SYSTEM_ESCORE_BASE_HPP
#define SPIRE_ENTITY_SYSTEM_ESCORE_BASE_HPP

#include <es-log/trace-log.h>
#include <map>
#include <list>
#include <iostream>
#include <stdexcept>
#include "BaseSystem.hpp"
#include "src/ComponentContainer.hpp"
#include "src/EmptyComponentContainer.hpp"
#include <spire/scishare.h>

namespace spire {

template <typename T>
uint64_t getESTypeID()
{
  return TemplateID<typename std::decay<T>::type>::getID();
}

/// An example of creating a component container map using a std::map.
/// Feel free to use this class instead of rolling your own.
/// NOTE: This class is not meant to be used by itself! You should either use
/// the ESCore class or you should derive from this class and implement the
/// necessary addComponent and addStaticComponent template functions. See ESCore
/// for an example implementation. I cannot enforce the existence of template
/// functions in a derived class, but this class is fairly useless without
/// the ability to add components.
class SCISHARE ESCoreBase
{
public:
  ESCoreBase();
  virtual ~ESCoreBase() {deleteAllComponentContainers();}

  /// Returns false if the component doesn't exist.
  bool hasComponentContainer(uint64_t componentID) const;

  /// When called, ESCoreBase takes ownership of \p component.
  /// Adds a new component to the system. If a component of the same
  /// TypeID already exists, the request is ignored.
  void addComponentContainer(BaseComponentContainer* componentCont, uint64_t componentID);

  /// Retrieves a base component container. Component is the output from
  /// the TemplateID class.
  BaseComponentContainer* getComponentContainer(uint64_t component);

  /// Clears out all component containers (deletes all entities).
  void clearAllComponentContainers();

  /// Clears out all component containers *immediately*.
  void clearAllComponentContainersImmediately();

  /// Call this function at the beginning or end of every frame. It renormalizes
  /// all your components (adds / removes components). To call a system, execute
  /// the walkComponents function on BaseSystem. Most systems don't need a
  /// stable sort. But if you need to guarantee the relative order of multiple
  /// components with the same entity ID, use stable sort.
  virtual void renormalize(bool stableSort = false);

  /// Removes all components associated with entity.
  virtual void removeEntity(uint64_t entityID);

  /// Removes all components with their entityID equal to \p entityID, but only
  /// within the \p compTemplateID component (the compTemplateID identifier
  /// comes from the TemplateID<> class).
  void removeAllComponents(uint64_t entityID, uint64_t compTemplateID);

  template <typename T>
  void removeAllComponentsT(uint64_t entityID)
  {
    removeAllComponents(entityID, getESTypeID<T>());
  }

  /// Removes the component for \p entityID at the given index. Usually
  /// used with group systems that count the index until a particular
  /// component.
  void removeComponentAtIndex(uint64_t entityID, int32_t index, uint64_t templateID);

  template <typename T>
  void removeComponentAtIndexT(uint64_t entityID, int32_t index)
  {
    removeComponentAtIndex(entityID, index, getESTypeID<T>());
  }

  /// NOTE: If you use either of the following functions, it is highly advised
  ///       that you renormalize with stableSort = true! This ensures that
  ///       your logic regarding what component comes first / last, when
  ///       entityIDs are the same, is valid. Stable sort is slower than a
  ///       regular sort, so only use it when necessary.
  /// @{
  /// Removes the first component with the given entityID associated with the
  /// given \p compTemplateID (the unique identifier for the component type,
  /// from the TemplateID<> class).
  void removeFirstComponent(uint64_t entityID, uint64_t compTemplateID);

  template <typename T>
  void removeFirstComponentT(uint64_t entityID)
  {
    removeFirstComponent(entityID, getESTypeID<T>());
  }

  /// Removes the last component with the given \p compTemplateID.
  void removeLastComponent(uint64_t entityID, uint64_t compTemplateID);

  template <typename T>
  void removeLastComponentT(uint64_t entityID)
  {
    removeLastComponent(entityID, getESTypeID<T>());
  }
  /// @}

  /// Retrieves the list of static components. You can modify these values at
  /// will until renormalize is called. Use this to update the values of static
  /// components if you don't want to specifically write a system for that.
  /// Will return a nullptr if there are no static components.
  template <typename T>
  typename ComponentContainer<T>::ComponentItem* getStaticComponents()
  {
    auto componentContainer = getComponentContainer(getESTypeID<T>());
    if (componentContainer)
    {
      auto concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
      return concreteContainer->getComponentArray();
    }
    else
    {
      return nullptr;
    }
  }

  template <typename T>
  T* getStaticComponent(size_t index = 0)
  {
    auto componentContainer = getComponentContainer(getESTypeID<T>());
    if (componentContainer)
    {
      auto concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
      auto components = concreteContainer->getComponentArray();
      if (components && index < concreteContainer->getNumComponents())
        return &components[index].component;
      else
        return nullptr;
    }
    else
    {
      return nullptr;
    }
  }

  /// Retrieve static dummy empty BaseComponentContainer implementation.
  static BaseComponentContainer* getEmptyContainer() {return static_cast<BaseComponentContainer*>(&mEmptyContainer);}

  // Note: getNewEntityID is a deprecated function that will be removed in a
  // future version of the entity system.

  /// Returns a, new, valid entity ID. This is a trivial function and should
  /// not be used if you are controlling entity ids using external code.
  uint64_t getNewEntityID() {return ++mCurSequence;}

protected:

  /// Deletes all component containers.
  void deleteAllComponentContainers();

  /// Adds a component. If a component container already exists, then that is
  /// used. Otherwise, a new component container is created and used.
  template <typename T, class CompCont = ComponentContainer<T>>
  void coreAddComponent(uint64_t entityID, const T& component)
  {
    if (entityID == 0)
    {
      std::cerr << "entity-system: Attempting to add a component of entityID 0! Not allowed." << std::endl;
      throw std::runtime_error("Attempting to add a component of entityID 0.");
      return;
    }

    auto componentContainer = ensureComponentArrayExists<T, CompCont>();
    auto concreteContainer = dynamic_cast<CompCont*>(componentContainer);
    concreteContainer->addComponent(entityID, component);
  }

  /// Adds a static component. Static components work exactly like normal
  /// components, except that they are not associated with an entityID and are
  /// fed into each system in the same manner. Use this to make 'global'
  /// components (for lighting, view camera position, input, etc...).
  /// Static components and regular components cannot be mixed.
  /// Returns the index of the static component.

  // See: http://thbecker.net/articles/rvalue_references/section_08.html
  // Reference collapsing rules:
  // A& & becomes A&
  // A& && becomes A&
  // A&& & becomes A&
  // A&& && becomes A&&

  // Note: This std::decay is the reason why we don't get perfect forwarding
  // into the ComponentContainer<T> class. We remove its cv qualifiers so
  // we don't get a type match.
  template <typename T, class CompCont = ComponentContainer<typename std::decay<T>::type>>
  size_t coreAddStaticComponent(T&& component)
  {
   // If the container isn't already marked as static, mark it and ensure
   // that it is empty.
   auto componentContainer = ensureComponentArrayExists<T, CompCont>();
   auto concreteContainer = dynamic_cast<CompCont*>(componentContainer);
   return concreteContainer->addStaticComponent(std::forward<T>(component));
  }

  template <typename T, class CompCont>
  BaseComponentContainer* ensureComponentArrayExists()
  {
    auto componentContainer = getComponentContainer(getESTypeID<T>());
    if (!componentContainer)
    {
      componentContainer = new CompCont();
      addComponentContainer(componentContainer, getESTypeID<T>());
    }
    return componentContainer;
  }

  std::map<uint64_t, BaseComponentContainer*> mComponents;
  uint64_t                                    mCurSequence;

  static EmptyComponentContainer mEmptyContainer;
};



} // namespace spire

#endif

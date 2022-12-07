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


#include "ESCoreBase.hpp"

#include <es-log/trace-log.h>

namespace spire {

EmptyComponentContainer ESCoreBase::mEmptyContainer;

// Note: It is *very* important that mCurSequence starts one greater than
// the StaticEntityID! Otherwise we could accidentally delete static components!
ESCoreBase::ESCoreBase()
 : mCurSequence(BaseComponentContainer::StaticEntID)
{

}

bool ESCoreBase::hasComponentContainer(uint64_t componentID) const
{
  return mComponents.find(componentID) != mComponents.end();
}

/// When called, ESCoreBase takes ownership of \p component.
/// Adds a new component to the system. If a component of the same
/// TypeID already exists, the request is ignored.
void ESCoreBase::addComponentContainer(BaseComponentContainer* componentCont, uint64_t componentID)
{
  auto it = mComponents.find(componentID);
  if (it == mComponents.end())
  {
    mComponents.insert(std::make_pair(componentID, componentCont));
    componentCont->setId(componentID);
  }
  else
  {
    //logRendererError("entity-system - Warning: Attempting to add pre-existing component container!");
    delete componentCont;
  }
}

/// Retrieves a base component container. Component is the output from
/// the TemplateID class.
BaseComponentContainer* ESCoreBase::getComponentContainer(uint64_t component)
{
  auto it = mComponents.find(component);
  if (it != mComponents.end())
    return (it->second);
  else
    return nullptr;
}

/// Clears out all component containers (deletes all entities).
void ESCoreBase::clearAllComponentContainers()
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->removeAll();
}

void ESCoreBase::clearAllComponentContainersImmediately()
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->removeAllImmediately();
}

void ESCoreBase::deleteAllComponentContainers()
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    delete iter->second;

  mComponents.clear();
}

/// Call this function at the beginning or end of every frame. It renormalizes
/// all your components (adds / removes components). To call a system, execute
/// the walkComponents function on BaseSystem. Most systems don't need a
/// stable sort. But if you need to guarantee the relative order of multiple
/// components with the same entity ID, use stable sort.
void ESCoreBase::renormalize(bool stableSort)
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->renormalize(stableSort);
}

/// Removes all components associated with entity.
void ESCoreBase::removeEntity(uint64_t entityID)
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->removeSequence(entityID);
}

void ESCoreBase::removeFirstComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeFirstSequence(entityID);
}

void ESCoreBase::removeLastComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeLastSequence(entityID);
}

void ESCoreBase::removeComponentAtIndex(uint64_t entityID, int32_t index, uint64_t templateID)
{
  BaseComponentContainer* cont = getComponentContainer(templateID);
  cont->removeSequenceWithIndex(entityID, index);
}

void ESCoreBase::removeAllComponents(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeSequence(entityID);
}

} // namespace spire

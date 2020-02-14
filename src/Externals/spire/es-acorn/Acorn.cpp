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


#include "Acorn.hpp"
#include <string>

namespace es      = spire;
namespace systems = spire;

namespace spire {

Acorn::Acorn() :
    mSystems(new systems::SystemCore())
{
}

Acorn::~Acorn()
{
}

bool Acorn::isSystemActive(const std::string& name)
{
  return mSystems->isSystemActive(name);
}

void Acorn::runSystemOnce(const std::string& systemName)
{
  mSystems->createNewSystem(systemName)->walkComponents(*this);
}

bool Acorn::displayEntityVersusSystemInfo(uint64_t entityID, const std::string& sysName)
{
  /// \todo Fix this function so that it can detect when a component name
  ///       is *not* in the component map.
  // Create a temporary system.
  std::cout << "-------- Entity Vs System Comparison -------" << std::endl;
  std::cout << "System: " << sysName << " EntityID: " << entityID << std::endl;

  if (!mSystems->isSystemActive(sysName))
  {
    std::cout << "WARNING: System (" << sysName << ") is NOT active." << std::endl;
  }

  bool satisfied = true;
  std::shared_ptr<spire::BaseSystem> sys = mSystems->createNewSystem(sysName);
  if (sys)
  {
    std::vector<uint64_t> components = sys->getComponents();
    for (uint64_t c : components)
    {
      std::string optionalText = "";
      if (sys->isComponentOptional(c))
        optionalText = " - OPTIONAL";

      // Check to see if the entityID exists in the component container.
      spire::BaseComponentContainer* cont = getComponentContainer(c);
      if (cont != nullptr)
      {
        std::string staticText;
        if (cont->isStatic())
          staticText = " - STATIC";

        int numComponents = cont->getNumComponentsWithSequence(entityID);
        if (numComponents > 0)
        {
          std::cout << "== " << numComponents << " - (" << mComponentIDNameMap.find(c)->second << ")"
                    << staticText << optionalText << std::endl;
        }
        else
        {
          if (cont->isStatic())
          {
            numComponents = cont->getNumComponentsWithSequence(static_cast<uint64_t>(spire::BaseComponentContainer::StaticEntID));
            if (numComponents > 0)
            {
              std::cout << "== " << numComponents << " - (" << mComponentIDNameMap.find(c)->second << ")"
                        << staticText << optionalText << std::endl;
            }
            else
            {
              if (sys->isComponentOptional(c))
              {
                std::cout << "== 0 - (" << mComponentIDNameMap.find(c)->second << ")"
                          << staticText << optionalText << std::endl;
              }
              else
              {
                std::cout << "!! 0 - (" << mComponentIDNameMap.find(c)->second << ")"
                          << staticText << optionalText << " -- WARNING" << std::endl;
                satisfied = false;
              }
            }
          }
          else if(sys->isComponentOptional(c))
          {
            std::cout << "== 0 - (" << mComponentIDNameMap.find(c)->second << ")"
                      << staticText << optionalText << std::endl;
          }
          else
          {
            std::cout << "!! 0 - (" << mComponentIDNameMap.find(c)->second << ")"
                      << staticText << optionalText << " -- WARNING" << std::endl;
            satisfied = false;
          }
        }
      }
      else
      {
        if (mComponentIDNameMap.find(c) != mComponentIDNameMap.end())
        {
          std::cout << "!! 0 - (" << mComponentIDNameMap.find(c)->second << ") "
                    << " unable to find container for component." << std::endl;
        }
        else
        {
          std::cout << "!! 0 - () Unable to find component with ID: " << c
                    << " in mComponentIDNameMap. Component hasn't been registered yet." << std::endl;
        }
        satisfied = false;
      }
    }
  }
  else
  {
    std::cerr << "displayEntityAndSystemInfo: Unable to find system - " << sysName << std::endl;
  }

  if (satisfied)
  {
    std::cout << "++ GOOD - Entity satisfies system." << std::endl;
  }
  else
  {
    std::cout << "++ BAD - Entity does *NOT* satisfy system." << std::endl;
  }

  return satisfied;
}

void Acorn::addKernelSystem(const std::string& name, uint64_t ms,
                           uint64_t referenceTime, uint64_t stagger)
{
  if (warnIfSystemPresent(name)) return;

  mKernelSystems.insert(name);
  mSystems->addActiveSystem(name, ms, referenceTime, stagger);
}

void Acorn::addUserSystem(const std::string& name, uint64_t ms,
                         uint64_t referenceTime, uint64_t stagger)
{
  if (warnIfSystemPresent(name)) return;

  mUserSystems.insert(name);
  mSystems->addActiveSystem(name, ms, referenceTime, stagger);
}

void Acorn::addUserSystemForced(const std::string& name, uint64_t ms,
                               uint64_t referenceTime, uint64_t stagger)
{
  // Add the system only if it is not already present.
  if (mUserSystems.find(name) == mUserSystems.end())
  {
    mUserSystems.insert(name);
  }

  mUserSystems.insert(name);
  mSystems->addActiveSystem(name, ms, referenceTime, stagger);
}

void Acorn::removeUserSystem(const std::string& name)
{
  if (mUserSystems.find(name) != mUserSystems.end())
  {
    mSystems->removeActiveSystem(name);
    mUserSystems.erase(mUserSystems.find(name));
  }
  else
  {
    std::cerr << "Refusing to remove system (" << name << "). No such user space system." << std::endl;
    if (mKernelSystems.find(name) != mKernelSystems.end())
    {
      std::cerr << "** But system (" << name << ") exists in the kernel space." << std::endl;
    }
  }
}

void Acorn::removeAllUserSystems()
{
  for (const std::string& name : mUserSystems)
  {
    mSystems->removeActiveSystem(name);
  }
  mUserSystems.clear();
}

bool Acorn::warnIfSystemPresent(const std::string& name)
{
  // Check to make sure another system doesn't already exist with the same
  // name in either user space or kernel space. Refuse to add if either is
  // the case.
  if (mKernelSystems.find(name) != mKernelSystems.end())
  {
    std::cerr << "Refusing to add system (" << name << "). System already exists in kernel space." << std::endl;
    return true;
  }

  if (mUserSystems.find(name) != mUserSystems.end())
  {
    std::cerr << "Refusing to add system (" << name << "). System already exists in user space." << std::endl;
    return true;
  }

  return false;
}

void Acorn::clearAllNonExemptComponents(bool debug)
{
  for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
  {
    if (mExemptComponents.find(it->first) == mExemptComponents.end())
    {
      if (debug)
      {
        std::cout << "Removing component - " << mComponentIDNameMap.find(it->first)->second << std::endl;
      }
      // Remove all components.
      it->second->removeAll();
    }
  }
}

void Acorn::addGarbageCollectorSystem(const std::string& sys)
{
  // Ensure the garbage collector is not already present.
  for (const std::string& str : mGarbageCollectorSystems)
  {
    if (str == sys)
    {
      std::cerr << "GC system (" << sys << ") is already present in the GC array." << std::endl;
      return;
    }
  }

  mGarbageCollectorSystems.push_back(sys);
}

void Acorn::runCompleteGC()
{
  for (const std::string& str : mGarbageCollectorSystems)
  {
    std::shared_ptr<spire::BaseSystem> sys = mSystems->createNewSystem(str);
    sys->walkComponents(*this);
  }
}

} // namespace spire

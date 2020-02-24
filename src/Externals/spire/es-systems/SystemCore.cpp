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


#include <algorithm>
#include <string>

#include "SystemCore.hpp"

namespace spire {

std::string SystemCore::toString(std::string prefix) const
{
  std::string output = prefix + "Systems: " + std::to_string(mSystems.size()) + "\n";
  prefix += "  ";

  for(auto& sysItm: mSystems)
  {
    output += prefix + "Name: " + sysItm.systemName
      + "  NextExecutionTime: " + std::to_string(sysItm.nextExecutionTime) + "\n";
  }

  return output;
}


void SystemCore::runSystems(spire::ESCoreBase& core, uint64_t referenceTime)
{
  for (SystemItem& sys : mSystems)
  {
    //auto name = sys.systemName;
    //const auto p = name.find("GeomPromise") != std::string::npos;
    //if (p)
    //  std::cout << "runSystems: " << name << " " << &sys << std::endl;

    if (sys.shouldExecute(referenceTime))
    {
      sys.system->walkComponents(core);
    }
  }
}

void SystemCore::renormalize()
{
  // The sequence in the system core is a bit different than the sequence in the
  // entity system. In general, we want to remove first then add. In reality,
  // we need a system of telling when an add or remove was issued to alleviate
  // all problems related to addition / removal. Maybe timestamping each and
  // using a priority queue to march through both.

  for (const std::string& name : mSystemsToRemove)
  {
    auto it = std::lower_bound(mSystems.begin(), mSystems.end(),
                               SystemItem(name), systemCompare);

    if (it != mSystems.end() && it->systemName == name)
    {
      mSystems.erase(it);
    }
    else
    {
      std::cerr << "es-system: Unable to find system with name: " << name << " in active list." << std::endl;
    }
  }
  mSystemsToRemove.clear();

  // Additions.
  for (SystemItem& sys : mSystemsToAdd)
  {
    // Ensure the system is not already present in our list.
    // Brute force search.
    bool shouldAdd = true;
    for (const SystemItem& vecItem : mSystems)
    {
      if (vecItem.systemName == sys.systemName)
      {
        shouldAdd = false;
      }
    }

    if (shouldAdd)
    {
      mSystems.push_back(sys);
    }
    else
    {
      std::cerr << "Refusing to add system " << sys.systemName << ". Already present." << std::endl;
    }
  }

  // Re-sort the systems if the number of systems just added was greater than 0.
  if (mSystemsToAdd.size() > 0)
  {
    std::sort(mSystems.begin(), mSystems.end(), systemCompare);
  }

  mSystemsToAdd.clear();
}

bool SystemCore::systemCompare(const SystemCore::SystemItem& a, const SystemCore::SystemItem& b)
{
  return a.systemName < b.systemName;
}

bool SystemCore::isSystemActive(const std::string& name) const
{
  // See if there is a pre-existing system. Our list is alphabetically
  // sorted, so we can use a binary search on it.
  auto it = std::lower_bound(mSystems.cbegin(), mSystems.cend(),
                             SystemItem(name), systemCompare);

  bool foundSystem = false;
  if (it != mSystems.end())
  {
    if (it->systemName == name)
    {
      foundSystem = true;
    }
  }

  if (foundSystem == false)
  {
    for (const SystemItem& item : mSystemsToAdd)
    {
      if (item.systemName == name)
      {
        foundSystem = true;
        break;
      }
    }
  }

  return foundSystem;
}

void SystemCore::addActiveSystem(const std::string& name, uint64_t ms,
                                 uint64_t referenceTime, uint64_t stagger)
{
  std::shared_ptr<spire::BaseSystem> sys = mSystemFactory.newSystemFromName(name.c_str());
  if (sys != nullptr)
  {
    SystemItem item(name, sys, ms, referenceTime, stagger);
    mSystemsToAdd.push_back(item);
  }
  else
  {
    std::cerr << "es-system: Unable to find system with name: " << name << std::endl;
    std::cerr << "es-system: Was the system registered?" << std::endl;
  }
}

void SystemCore::removeActiveSystem(const std::string& name)
{
  mSystemsToRemove.push_back(name);
}

void SystemCore::deserializeActiveSystems(Tny* root, uint64_t referenceTime)
{
  if (root->type != TNY_DICT)
  {
    std::cerr << "es-system: Unexpected type during deserialization." << std::endl;
    throw std::runtime_error("Unexepected Tny type");
  }

  while (Tny_hasNext(root))
  {
    root = Tny_next(root);

    std::string name = root->key;
    if (root->type != TNY_OBJ)
    {
      std::cerr << "es-system: Unexpected type during deserialization." << std::endl;
      throw std::runtime_error("Unexpected Tny type");
    }

    Tny* comp = root->value.tny;

    if (comp->type != TNY_DICT)
    {
      std::cerr << "es-system: Unexpected type during deserialization." << std::endl;
      throw std::runtime_error("Unexpected Tny type");
    }
    Tny* val = Tny_get(comp, "interval");
    uint64_t interval = val->value.num;

    Tny_get(comp, "stagger");
    uint64_t stagger = val->value.num;

    // We ignore next exec and calculate it ourselves.
    //Tny_get(comp, "nextExec");
    //uint64_t nextExec = val->value.num;

    addActiveSystem(name, interval, referenceTime, stagger);
  }
}

Tny* SystemCore::serializeActiveSystems()
{
  // Iterate through all active systems and serialize them out, in order, to
  // a TNY dictionary.
  Tny* root = Tny_add(NULL, TNY_DICT, NULL, NULL, 0);

  for (SystemItem& item : mSystems)
  {
    Tny* obj = Tny_add(NULL, TNY_DICT, NULL, NULL, 0);
    obj = Tny_add(obj, TNY_INT64, const_cast<char*>("interval"), static_cast<void*>(&item.interval), 0);
    obj = Tny_add(obj, TNY_INT64, const_cast<char*>("stagger"), static_cast<void*>(&item.stagger), 0);
    obj = Tny_add(obj, TNY_INT64, const_cast<char*>("nextExec"), static_cast<void*>(&item.nextExecutionTime), 0);
    root = Tny_add(root, TNY_OBJ, const_cast<char*>(item.systemName.c_str()), obj->root, 0);
  }

  return root;
}

bool SystemCore::SystemItem::shouldExecute(uint64_t referenceTime)
{
  if (this->nextExecutionTime <= referenceTime)
  {
    // Only use prior next execution time as the base to build on next execution
    // interval only if its multiplier is not greater than one.
    if (this->interval != 0 && referenceTime - this->nextExecutionTime >= this->interval)
    {
      // Adding 1 to the reference time doesn't affect next execution time
      // other than ensuring that the next execution time isn't *now*.
      // This is since we use mod to calculate the execution time.
      this->nextExecutionTime = calcNextExecutionTime(referenceTime + 1);
    }
    else
    {
      this->nextExecutionTime = this->nextExecutionTime + this->interval;
    }

    return true;
  }
  else
  {
    return false;
  }
}

uint64_t SystemCore::SystemItem::calcNextExecutionTime(uint64_t referenceTime)
{
  if (this->interval != 0)
  {
    // Recalculate the execution time using mod and stagger.
    uint64_t modInterval = (referenceTime + this->stagger) % this->interval;

    // This will set the next execution time to the closest stagger point.
    if (modInterval == 0)
      return referenceTime;
    else
      return referenceTime + (this->interval - modInterval);
  }
  else
  {
    return referenceTime;
  }
}

} // namespace spire

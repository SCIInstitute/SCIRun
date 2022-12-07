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


#ifndef SPIRE_ENTITY_SYSTEM_DUPLICATECOMPONENT_HPP
#define SPIRE_ENTITY_SYSTEM_DUPLICATECOMPONENT_HPP

#include <es-log/trace-log.h>
#include "GenericSystem.hpp"
#include <spire/scishare.h>

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

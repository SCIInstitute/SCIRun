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


/// author James Hughes
/// date   February 2014

#ifndef SPIRE_ENTITY_SYSTEM_EMPTYSYSTEM_HPP
#define SPIRE_ENTITY_SYSTEM_EMPTYSYSTEM_HPP

#include <es-log/trace-log.h>
#include "BaseSystem.hpp"
#include "ESCoreBase.hpp"
#include <spire/scishare.h>

namespace spire {

/// A system that executes without any associated components.
class EmptySystem : public BaseSystem
{
public:
  /// Abstract functions.
  virtual void execute(ESCoreBase& core) = 0;

  void walkComponents(ESCoreBase& core) override
  {
    preWalkComponents(core);
    execute(core);
    postWalkComponents(core);
  }

  bool walkEntity(ESCoreBase& core, uint64_t entityID) override
  {
    preWalkComponents(core);
    execute(core);
    postWalkComponents(core);
    return true;
  }

  // Used in the two functions above.
  virtual void preWalkComponents(ESCoreBase& core)            {}
  virtual void postWalkComponents(ESCoreBase& core)           {}

  std::vector<uint64_t> getComponents() const override
  {
    return std::vector<uint64_t>();
  }

  virtual bool isComponentOptional(uint64_t /* component */) override
  {
    return false;
  }
};

} // namespace spire

#endif

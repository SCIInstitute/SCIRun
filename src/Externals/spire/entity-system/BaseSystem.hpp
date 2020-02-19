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


#ifndef SPIRE_ENTITY_SYSTEM_BASESYSTEM_HPP
#define SPIRE_ENTITY_SYSTEM_BASESYSTEM_HPP

#include <es-log/trace-log.h>
#include <vector>
#include <cstdint>
#include <spire/scishare.h>

namespace spire {

class ESCoreBase;

class BaseSystem
{
public:
  BaseSystem()          {}
  virtual ~BaseSystem() {}

  // The following two functions may be convertable to const functions.
  virtual void walkComponents(ESCoreBase& core) = 0;
  virtual bool walkEntity(ESCoreBase& core, uint64_t entityID) = 0;

  // Debug functions related to printing components that a particular
  // entity is missing in relation to the system executing.
  virtual std::vector<uint64_t> getComponents() const = 0;
  virtual bool isComponentOptional(uint64_t component) = 0;
};

} // namespace spire

#endif

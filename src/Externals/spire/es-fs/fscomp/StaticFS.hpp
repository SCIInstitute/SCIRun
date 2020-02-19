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


#ifndef SPIRE_ASYNC_STATICFS_HPP
#define SPIRE_ASYNC_STATICFS_HPP

#include <es-log/trace-log.h>
#include <stdexcept>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../Filesystem.hpp"
#include <spire/scishare.h>

namespace spire {

struct StaticFS
{
  // -- Data --

  /// \todo Should be unique ptr. But we need to be able to add move
  ///       constructible components.
  std::shared_ptr<Filesystem> instance;

  // -- Functions --

  StaticFS() : instance(nullptr) {}
  StaticFS(std::shared_ptr<Filesystem> man) : instance(man) {}

  static const char* getName() {return "es-fs:StaticFS";}

  bool serialize(spire::ComponentSerialize&, uint64_t /* entityID */)
  {
    //logRendererError("No serialization should be performed on StaticFS!");
    throw std::runtime_error("Cannot serialize StaticFS!");
  }
};

} // namespace spire

#endif

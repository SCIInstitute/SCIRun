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


#ifndef DATAFLOW_NETWORK_PORTNAMES_H
#define DATAFLOW_NETWORK_PORTNAMES_H

#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  template <class Type, size_t N>
  struct PortNameBase
  {
    explicit PortNameBase(const PortId& id) : id_(id) {}

    operator PortId() const
    {
      return toId();
    }

    PortId toId() const
    {
      if (id_.name.empty())
        BOOST_THROW_EXCEPTION(DataPortException() << SCIRun::Core::ErrorMessage("Port name not initialized!"));
      return id_;
    }
    operator std::string() const
    {
      return toId().name;
    }

    PortId id_;
  };

  template <class Type, size_t N>
  struct StaticPortName : PortNameBase<Type,N>
  {
    explicit StaticPortName(const PortId& id = PortId(0, "[not defined yet]")) : PortNameBase<Type,N>(id) {}
  };

  template <class Type, size_t N>
  struct DynamicPortName : PortNameBase<Type,N>
  {
    explicit DynamicPortName(const PortId& id = PortId(0, "[not defined yet]")) : PortNameBase<Type,N>(id) {}
  };

}}}

#endif

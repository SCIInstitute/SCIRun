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


#ifndef SPIRE_RENDER_COMPONENT_STATIC_TEXTURE_MAN_HPP
#define SPIRE_RENDER_COMPONENT_STATIC_TEXTURE_MAN_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../TextureMan.hpp"
#include <spire/scishare.h>

namespace ren {

struct StaticTextureMan
{
    // -- Data --
    std::shared_ptr<TextureMan> instance_;

    // -- Functions --
    StaticTextureMan() : instance_(new TextureMan) {}
    explicit StaticTextureMan(TextureMan* s) : instance_(s) {}

    // This assignment operator is only used during modification calls inside
    // of the entity system. We don't care about those calls as they won't
    // affect this static shader man.
    StaticTextureMan& operator=(const StaticTextureMan&)
    {
    // We don't care about the incoming object. We've already created oun own
    // shader man and will continue to use that.
      return *this;
    }

    static const char* getName() {return "ren:StaticTextureMan";}

private:
    friend class spire::CerealHeap<StaticTextureMan>;

    bool serialize(spire::ComponentSerialize&, uint64_t)
    {
    // No need to serialize. But we do want that we were in the component
    // system to be serialized out.
      return true;
    }
};

} // namespace ren

#endif

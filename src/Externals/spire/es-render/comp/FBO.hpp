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


#ifndef SPIRE_RENDER_COMPONENT_FBO_HPP
#define SPIRE_RENDER_COMPONENT_FBO_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-render/comp/Texture.hpp>
#include <spire/scishare.h>

namespace ren {

  struct FBO
  {
    // -- Data --
    GLuint glid;
    GLenum textureType;
    bool initialized;

    // -- Functions --
    FBO()
    {
      glid = 0;
      textureType = GL_TEXTURE_2D;
      initialized = false;
    }

    static const char* getName() { return "ren:FBO"; }

    struct TextureData
    {
      GLenum att;
      std::string texName;
    };

    std::vector<TextureData> textures;

    bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
    {
      /// Nothing needs to be serialized. This is context specific.
      return true;
    }
  };

} // namespace ren

#endif

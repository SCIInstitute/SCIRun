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


#ifndef SPIRE_COMPONENT_RENDER_TEXTURE_HPP
#define SPIRE_COMPONENT_RENDER_TEXTURE_HPP

#include <es-log/trace-log.h>
#include <es-cereal/ComponentSerialize.hpp>

#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/CerealCore.hpp>
#include "TexturePromise.hpp"
#include "StaticTextureMan.hpp"
#include <spire/scishare.h>

namespace ren {

struct Texture
{
  // -- Data --
  static const int MaxUniformName = 24;
  GLuint          glid;         ///< glid associated with the loaded texture.
  int32_t         textureUnit;  ///< The texture unit this texture is bound to.
  GLint           uniformLocation;
  char            uniformName[MaxUniformName];
  GLenum          textureType;
  GLsizei textureWidth;
  GLsizei textureHeight;
  GLsizei textureDepth;
  GLint internalFormat;
  GLenum format;
  GLenum type;
  GLint filter;

  // -- Functions --
  Texture()
  {
    textureUnit = 0;
    glid = 0;
    uniformLocation = -1;
    uniformName[0] = '\0';
    textureType = GL_TEXTURE_2D;
  }

  static const char* getName() {return "ren:Texture";}

  void checkUniform(GLuint shaderID);
  void applyUniform() const;
  bool isSetUp() const;

  void setUniformName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxUniformName - 1)
    {
      std::strncpy(uniformName, name, MaxUniformName);
    }
    else
    {
      std::cerr << "Texture: Unable to set uniform name: " << name << " Name must be "
                << (MaxUniformName - 1) << "characters or shorter." << std::endl;
    }
  }

  bool serialize(spire::ComponentSerialize& s, uint64_t entityID)
  {
    // The logic below ensures we deserialize with promises, not with actual
    // texture assets (which wouldn't make sense with OpenGL assets).
    if (s.isDeserializing())
    {
      std::string assetName;
      int32_t tu;
      s.serialize("name", assetName);
      s.serialize("unit", tu);

      // Build texture promise.
      TexturePromise newPromise;
      newPromise.requestInitiated = false;
      newPromise.textureUnit = tu;
      newPromise.setAssetName(assetName.c_str());

      spire::CerealCore& core
          = dynamic_cast<spire::CerealCore&>(s.getCore());
      core.addComponent(entityID, newPromise);

      return false; // We do not want to add this texture component back into the components.
                    // Instead we rely on the texture promise we created above.
    }
    else
    {
      spire::CerealCore& core
          = dynamic_cast<spire::CerealCore&>(s.getCore());
      StaticTextureMan* staticTX = core.getStaticComponent<StaticTextureMan>();
      std::weak_ptr<TextureMan> texMan = staticTX->instance_;

      // Find the asset name associated with our glid and serialize it out.
      if (std::shared_ptr<TextureMan> tm = texMan.lock()) {
        std::string assetName = tm->getAssetFromID(glid);
        s.serialize("name", assetName);
        s.serialize("unit", textureUnit);
        return true;
      }
      return false;
    }
  }
};

} // namespace ren

#endif

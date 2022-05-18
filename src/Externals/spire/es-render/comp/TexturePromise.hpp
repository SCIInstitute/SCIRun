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


#ifndef SPIRE_RENDER_TEXTUREPROMISE_HPP
#define SPIRE_RENDER_TEXTUREPROMISE_HPP

#include <es-log/trace-log.h>
#include <cstring>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct TexturePromise
{
  // -- Data --
  static const int MaxAssetName = 64;
  static const int MaxUniformName = 24;

  /// Name of the shader that was promised.
  char assetName[MaxAssetName];

  /// The texture unit we should bind to.
  int32_t textureUnit;

  /// Name of the uniform this texture will be applied to.
  char uniformName[MaxUniformName];

  // Indicator variable. Never serialized out and is used to indicate when
  // we need to re-issue a request to the filesystem.
  bool requestInitiated;

  // -- Functions --
  TexturePromise()
  {
    requestInitiated = false;
    assetName[0] = 0;
    uniformName[0] = 0;
  }

  static const char* getName()    {return "ren:TexturePromise";}

  void setAssetName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxAssetName - 1)
    {
      std::strncpy(assetName, name, MaxAssetName);
    }
    else
    {
      std::cerr << "TexturePromise: Unable to set name: " << name << " Name must be "
                << (MaxAssetName - 1) << "characters or shorter." << std::endl;
    }
  }

  void setUniformName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxUniformName - 1)
    {
      std::strncpy(uniformName, name, MaxUniformName);
    }
    else
    {
      std::cerr << "TexturePromise: Unable to set uniform name: " << name << " Name must be "
                << (MaxUniformName - 1) << "characters or shorter." << std::endl;
    }
  }

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    std::string asset = assetName;
    std::string uname = uniformName;
    s.serialize("name", asset);
    s.serialize("texUnit", textureUnit);
    s.serialize("uName", uname);

    // If we are deserializing, then we need to absolutely sure that
    // requestInitiated is false (although it should be from the constructor
    // and the fact that we did not serialize it).
    if (s.isDeserializing())
    {
      requestInitiated = false;
      setAssetName(asset.c_str());
      setUniformName(uname.c_str());
    }
    return true;
  }

};

} // namespace ren

#endif

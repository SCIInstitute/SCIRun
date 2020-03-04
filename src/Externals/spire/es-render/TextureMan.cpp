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


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <string>
#include <sstream>
#include <es-fs/Filesystem.hpp>
#include <es-fs/fscomp/StaticFS.hpp>
#include <lodepng/lodepng.h>
#include <entity-system/GenericSystem.hpp>
#include <es-acorn/Acorn.hpp>

#include "TextureMan.hpp"
#include "comp/Texture.hpp"
#include "comp/TexturePromise.hpp"
#include "comp/StaticTextureMan.hpp"

namespace es = spire;
namespace fs = spire;

namespace ren {

  TextureMan::TextureMan(int numRetries) :
    mNewUnfulfilledAssets(false),
    mNumRetries(numRetries)
  {
  }

  TextureMan::~TextureMan()
  {
    // Destroy all GLIDs.
    for (auto it = mGLToName.begin(); it != mGLToName.end(); ++it)
    {
      GL(glDeleteTextures(1, &it->first));
    }
  }

  void TextureMan::loadTexture(
    spire::CerealCore& core, uint64_t entityID,
    const std::string& assetName, int32_t textureUnit,
    const std::string& uniformName)
  {
    // We don't even attempt to check whether the asset is already loaded. We
    // let the asset fulfillment system do that.

    // Ensure there is a file extension on the asset name. The texture loader
    // does also accept requests for png.
    std::string::size_type dotIdx = assetName.rfind('.');
    std::string fullAssetName = assetName;
    if (dotIdx == std::string::npos)
    {
      fullAssetName = assetName + ".itx";
    }

    if (buildComponent(core, entityID, fullAssetName, textureUnit, uniformName) == false)
    {
      // We failed to build the component immediately, build a promise for the
      // asset which will initiate a request.
      TexturePromise newPromise;
      newPromise.requestInitiated = false;
      newPromise.textureUnit = textureUnit;
      newPromise.setAssetName(fullAssetName.c_str());
      newPromise.setUniformName(uniformName.c_str());

      core.addComponent(entityID, newPromise);
    }
  }

  ren::Texture TextureMan::createTexture(
    const std::string& assetName,
    GLsizei textureWidth, GLsizei textureHeight,
    GLint internalFormat, GLenum format,
    GLenum type, GLint filter)
  {
    GLuint texID;

    auto it = mNameToGL.find(assetName);
    if (it != mNameToGL.end())
      texID = it->second;
    else
    {
      GL(glGenTextures(1, &texID));
      GL(glBindTexture(GL_TEXTURE_2D, texID));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

      GL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
        textureWidth, textureHeight, 0,
        format, type, 0));

      // Simply update mGLToName and mNameToGL. The fulfillment system will
      // handle everything else.
      mGLToName.insert(std::make_pair(texID, assetName));
      mNameToGL.insert(std::make_pair(assetName, texID));

      // Set new unfulfilled assets flag so we know if a GC cycle will remove
      // valid assets. We can use this to disable a GC cycle if it will have
      // unintended side-effects.
      mNewUnfulfilledAssets = true;
      GL(glBindTexture(GL_TEXTURE_2D, 0));
    }

    ren::Texture tex;
    tex.glid = texID;
    tex.textureType = GL_TEXTURE_2D;
    tex.textureWidth = textureWidth;
    tex.textureHeight = textureHeight;
    tex.textureDepth = 1;
    tex.internalFormat = internalFormat;
    tex.format = format;
    tex.type = type;
    tex.filter = filter;
    return tex;
  }

  ren::Texture TextureMan::createTexture(
    const std::string& assetName,
    GLsizei textureWidth, GLsizei textureHeight,
    const std::vector<uint8_t>& bitmap)
  {
    GLuint texID;

    auto it = mNameToGL.find(assetName);
    if (it != mNameToGL.end())
      texID = it->second;
    else
    {
      GL(glGenTextures(1, &texID));
      GL(glBindTexture(GL_TEXTURE_2D, texID));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,
        textureWidth, textureHeight, 0,
        GL_LUMINANCE, GL_UNSIGNED_BYTE, (const GLvoid*)(&(bitmap[0]))));
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

      // Simply update mGLToName and mNameToGL. The fulfillment system will
      // handle everything else.
      mGLToName.insert(std::make_pair(texID, assetName));
      mNameToGL.insert(std::make_pair(assetName, texID));

      // Set new unfulfilled assets flag so we know if a GC cycle will remove
      // valid assets. We can use this to disable a GC cycle if it will have
      // unintended side-effects.
      mNewUnfulfilledAssets = true;
      GL(glBindTexture(GL_TEXTURE_2D, 0));
    }

    ren::Texture tex;
    tex.glid = texID;
    tex.textureType = GL_TEXTURE_2D;
    tex.textureWidth = textureWidth;
    tex.textureHeight = textureHeight;
    tex.textureDepth = 1;
    tex.internalFormat = GL_LUMINANCE;
    tex.format = GL_LUMINANCE;
    tex.type = GL_UNSIGNED_BYTE;
    tex.filter = GL_LINEAR;
    return tex;
  }

  ren::Texture TextureMan::createTexture(
    const std::string& assetName,
    GLint internalformat,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    const std::vector<uint8_t>& data)
  {
    GLuint texID;
    auto it = mNameToGL.find(assetName);
    if (it != mNameToGL.end())
    {
      texID = it->second;
    }
    else
    {
      GL(glGenTextures(1, &texID));
      GL(glBindTexture(GL_TEXTURE_2D, texID));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      GL(glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type,
        (const GLvoid*)(&data[0])));

      mGLToName.insert(std::make_pair(texID, assetName));
      mNameToGL.insert(std::make_pair(assetName, texID));

      GL(glBindTexture(GL_TEXTURE_2D, 0));
    }

    ren::Texture tex;
    tex.glid = texID;
    tex.textureType = GL_TEXTURE_2D;
    tex.textureWidth = width;
    tex.textureHeight = height;
    tex.textureDepth = 1;
    tex.internalFormat = internalformat;
    tex.format = format;
    tex.type = type;
    tex.filter = GL_LINEAR;
    return tex;
  }

  bool TextureMan::resizeTexture(
    ren::Texture &tex, GLsizei textureWidth,
    GLsizei textureHeight)
  {
    if (glIsTexture(tex.glid) != GL_TRUE)
      return false;
    if (tex.textureWidth == textureWidth &&
      tex.textureHeight == textureHeight)
      return true;
    GL(glBindTexture(GL_TEXTURE_2D, tex.glid));
    GL(glTexImage2D(GL_TEXTURE_2D, 0, tex.internalFormat,
      textureWidth, textureHeight, 0,
      tex.format, tex.type, 0));
    tex.textureWidth = textureWidth;
    tex.textureHeight = textureHeight;
    return true;
  }

  void TextureMan::requestTexture(spire::ESCoreBase& core, const std::string& assetName,
    int32_t numRetries)
  {
    fs::StaticFS* sfs = core.getStaticComponent<fs::StaticFS>();

    /// \todo Get rid of this code when we switch to the new emscripten backend.
    ///       std::bind is preferable to cooking up a lambda. See ShaderMan
    ///       for functional examples.
    spire::ESCoreBase* refPtr = &core;
    auto callbackLambda = [this, numRetries, refPtr](
      const std::string& asset, bool error, size_t bytesRead, uint8_t* buffer)
    {
      loadTextureCB(asset, error, bytesRead, buffer, numRetries, *refPtr);
    };

    sfs->instance->readFile(assetName, callbackLambda);
  }

  void TextureMan::loadTextureCB(const std::string& assetName, bool error,
    size_t bytesRead, uint8_t* buffer,
    int32_t numRetries, spire::ESCoreBase& core)
  {
    if (!error)
    {
      // Simple extension check.
      std::string::size_type dotIdx = assetName.rfind('.');
      if (dotIdx != std::string::npos && assetName.substr(dotIdx + 1) == "png")
      {
        loadRawPNG(assetName, buffer, bytesRead, numRetries, core);
      }
      else // extension != png
      {
        loadRawITX(assetName, buffer, bytesRead, numRetries, core);
      }
    }
    else
    {
      if (numRetries > 0)
      {
        // Reattempt the request
        --numRetries;
        requestTexture(core, assetName, numRetries);
      }
      else
      {
        logRendererError("TextureMan: Failed promise for {}", assetName);
      }
    }
  }

  void TextureMan::loadRawPNG(const std::string& assetName, uint8_t* buffer,
    size_t bytesRead, int /* numRetries */,
    spire::ESCoreBase& /* core */)
  {
    unsigned int width, height;
    unsigned char* image = nullptr;

    unsigned int pngError = lodepng_decode32(&image, &width, &height, buffer, bytesRead);

    if (!pngError)
    {
      // We have successfully loaded an image into 'image'.

      // Perform actual loading of the texture *here*.
      GLuint texID;
      GL(glGenTextures(1, &texID));
      GL(glBindTexture(GL_TEXTURE_2D, texID));

      // We need to figure out what filtering settings we want to use.
      // Default linear.
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

      GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
      GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));

      GL(glTexImage2D(GL_TEXTURE_2D, 0,
        GL_RGBA,
        static_cast<GLsizei>(width), static_cast<GLsizei>(height),
        0, GL_RGBA,
        GL_UNSIGNED_BYTE, image));

      // Simply update mGLToName and mNameToGL. The fulfillment system will
      // handle everything else.
      mGLToName.insert(std::make_pair(texID, assetName));
      mNameToGL.insert(std::make_pair(assetName, texID));

      // Set new unfulfilled assets flag so we know if a GC cycle will remove
      // valid assets. We can use this to disable a GC cycle if it will have
      // unintended side-effects.
      mNewUnfulfilledAssets = true;
    }
    else
    {
      std::cout << "TextureMan: decoder error " << pngError << ": "
        << lodepng_error_text(pngError) << std::endl;
    }

    free(image);
  }

  void TextureMan::loadRawITX(const std::string& assetName, uint8_t* buffer,
    size_t bytesRead, int /* numRetries */,
    spire::ESCoreBase& /* core */)
  {
    // We have a texture of our own format (possibly with mip-chain included).
    Tny* doc = Tny_loads(buffer, bytesRead);
    if (doc)
    {
      uint32_t width, height;
      spire::CerealSerializeType<uint32_t>::in(doc, "width", width);
      spire::CerealSerializeType<uint32_t>::in(doc, "height", height);

      bool rgba;
      spire::CerealSerializeType<bool>::in(doc, "rgba", rgba);

      bool mips;
      spire::CerealSerializeType<bool>::in(doc, "mips", mips);

      std::string filtering;
      spire::CerealSerializeType<std::string>::in(doc, "filter", filtering);

      /// \todo Handle texture wrapping from the JSON file.

      /// \todo Add exists functionality to CerealSerializeType.

      /// \todo Add functionality for all of the mipmap filters (4 total).

      // See: https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml

      // Base texture data that will be present in the mip mapped verison
      // as well.
      Tny* docBaseTexData = Tny_get(doc, "tex0");

      // Generate texture.
      GLuint texID;
      GL(glGenTextures(1, &texID));
      GL(glBindTexture(GL_TEXTURE_2D, texID));

      GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
      GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));


      unsigned int decodedWidth, decodedHeight;
      unsigned char* decodedImage = nullptr;

      uint8_t* encodedBuffer = static_cast<uint8_t*>(docBaseTexData->value.ptr);
      size_t encodedBufferSize = docBaseTexData->size;
      GLenum glColorType = GL_RGBA;
      if (rgba)
      {
        lodepng_decode32(&decodedImage, &decodedWidth, &decodedHeight,
          encodedBuffer, encodedBufferSize);
        glColorType = GL_RGBA;
      }
      else
      {
        lodepng_decode24(&decodedImage, &decodedWidth, &decodedHeight,
          encodedBuffer, encodedBufferSize);
        glColorType = GL_RGB;
      }

      /// \todo Need more appropriate calculation of the color type
      GL(glTexImage2D(GL_TEXTURE_2D, 0,
        static_cast<GLint>(glColorType),
        static_cast<GLsizei>(width), static_cast<GLsizei>(height),
        0, glColorType,
        GL_UNSIGNED_BYTE, decodedImage));

      free(decodedImage);

      // Simply update mGLToName and mNameToGL. The fulfillment system will
      // handle everything else.
      mGLToName.insert(std::make_pair(texID, assetName));
      mNameToGL.insert(std::make_pair(assetName, texID));

      // Set new unfulfilled assets flag so we know if a GC cycle will remove
      // valid assets. We can use this to disable a GC cycle if it will have
      // unintended side-effects.
      mNewUnfulfilledAssets = true;

      if (mips)
      {
        // Extract mipmaps from document.
        uint32_t numMipLevels;
        spire::CerealSerializeType<uint32_t>::in(doc, "num_mips", numMipLevels);

        int twoFactor = 2;
        for (uint32_t i = 0; i < numMipLevels; ++i)
        {
          std::stringstream ss;
          ss << "tex" << (i + 1);
          std::string texName = ss.str();

          //unsigned int mipWidth  = width / twoFactor;
          //unsigned int mipHeight = height / twoFactor;

          docBaseTexData = Tny_get(doc, texName.c_str());

          // glColorType has been set elsewhere.
          encodedBuffer = static_cast<uint8_t*>(docBaseTexData->value.ptr);
          encodedBufferSize = docBaseTexData->size;
          if (rgba)
          {
            lodepng_decode32(&decodedImage, &decodedWidth, &decodedHeight,
              encodedBuffer, encodedBufferSize);
          }
          else
          {
            lodepng_decode24(&decodedImage, &decodedWidth, &decodedHeight,
              encodedBuffer, encodedBufferSize);
          }

          glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(i + 1),
            static_cast<GLint>(glColorType),
            static_cast<GLint>(decodedWidth),
            static_cast<GLint>(decodedHeight),
            0, glColorType, GL_UNSIGNED_BYTE, decodedImage);

          free(decodedImage);

          twoFactor *= 2;
        }

        if (filtering == "linear-nearest")
        {
          // Setup mip-map filtering.
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }
        else if (filtering == "nearest-linear")
        {
          // Setup mip-map filtering.
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        }
        else if (filtering == "linear-linear")
        {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        }
      }
      else
      {
        // Set filtering options without mipmaps (linear). Although filtering
        // could be changed in the json file...
        if (filtering == "linear")
        {
          GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
          GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        }
        else
        {
          GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
          GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        }
      }

      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

      Tny_free(doc);
    }
  }

  GLuint TextureMan::getIDForAsset(const char* assetName) const
  {
    auto it = mNameToGL.find(std::string(assetName));
    if (it != mNameToGL.end())
    {
      return it->second;
    }
    else
    {
      return 0;
    }
  }

  std::string TextureMan::getAssetFromID(GLuint id) const
  {
    auto it = mGLToName.find(id);
    if (it != mGLToName.end())
    {
      return it->second;
    }
    else
    {
      return "";
    }
  }

  bool TextureMan::buildComponent(spire::CerealCore& core, uint64_t entityID,
    const std::string& assetName, int32_t textureUnit,
    const std::string& uniformName)
  {
    // Check to see if this promise has been fulfilled. If it has, then
    // remove it and create the appropriate component for the indicated
    // entity.
    GLuint id = getIDForAsset(assetName.c_str());
    if (id != 0)
    {
      // Go ahead and add a new component for the entityID. If this is the
      // last promise to fulfill, then systems should automatically start
      // rendering the data.
      Texture component;
      component.glid = id;
      component.textureUnit = textureUnit;
      component.setUniformName(uniformName.c_str());
      core.addComponent(entityID, component);
      return true;
    }
    else
    {
      return false;
    }
  }

  //------------------------------------------------------------------------------
  // PROMISE FULFILLMENT
  //------------------------------------------------------------------------------

  class TexturePromiseFulfillment :
    public spire::GenericSystem<true,
    TexturePromise,
    StaticTextureMan>
  {
  public:

    static const char* getName() { return "ren:TexturePromiseFulfillment"; }

    /// This is only ever touched if requestInitiated is false for any component.
    /// It is updated during traversal and checked at the end of traversal
    /// against requested assets to see if the asset is already being loaded.
    std::set<std::string> mAssetsAwaitingRequest;

    /// Names of the assets currently being processed for which an additional
    /// request should not be attempted.
    std::set<std::string> mAssetsAlreadyRequested;

    void preWalkComponents(spire::ESCoreBase&) override
    {
      mAssetsAwaitingRequest.clear();
      mAssetsAlreadyRequested.clear();
    }

    void postWalkComponents(spire::ESCoreBase& core) override
    {
      auto man = core.getStaticComponent<StaticTextureMan>();
      if (!man)
      {
        logRendererError("Unable to complete texture fulfillment. There is no StaticTextureMan.");
        return;
      }
      std::weak_ptr<TextureMan> tm = man->instance_;
      if (std::shared_ptr<TextureMan> textureMan = tm.lock())
      {
        textureMan->mNewUnfulfilledAssets = false;

        if (mAssetsAwaitingRequest.size() > 0)
        {
          std::set<std::string> assetsWithNoRequest;
          // Compute set difference and initiate requests for appropriate
          // components.
          std::set_difference(mAssetsAwaitingRequest.begin(), mAssetsAwaitingRequest.end(),
            mAssetsAlreadyRequested.begin(), mAssetsAlreadyRequested.end(),
            std::inserter(assetsWithNoRequest, assetsWithNoRequest.end()));

          for (const std::string& asset : assetsWithNoRequest)
          {
            textureMan->requestTexture(core, asset, textureMan->mNumRetries);
          }
        }
      }
    }

    void groupExecute(spire::ESCoreBase& core, uint64_t entityID,
      const spire::ComponentGroup<TexturePromise>& promisesGroup,
      const spire::ComponentGroup<StaticTextureMan>& textureManGroup) override
    {
      std::weak_ptr<TextureMan> tm = textureManGroup.front().instance_;
      if (std::shared_ptr<TextureMan> textureMan = tm.lock()) {

        auto ourCorePtr = dynamic_cast<spire::CerealCore*>(&core);
        if (!ourCorePtr)
        {
          logRendererError("Unable to execute texture promise fulfillment. Bad cast.");
          return;
        }
        spire::CerealCore& ourCore = *ourCorePtr;

        int index = 0;
        for (const TexturePromise& p : promisesGroup)
        {
          if (textureMan->buildComponent(ourCore, entityID,
            p.assetName, p.textureUnit, p.uniformName))
          {
            // Remove this promise, and add a texture component to this promises'
            // entityID. It is safe to remove components while we are using a
            // system - addition / removal / modification doesn't happen until
            // a renormalization step.
            ourCore.removeComponentAtIndexT<TexturePromise>(entityID, index);
          }
          else
          {
            // The asset has not be loaded. Check to see if a request has
            // been initiated for the assets; if not, then run the request.
            // (this can happen when we serialize the game while we are
            // still waiting for assets).
            if (p.requestInitiated == false)
            {
              // Modify pre-existing promise to indicate that we are following
              // up with the promise. But, we don't initiate the request yet
              // since another promise may have already done so. We wait until
              // postWalkComponents to make a decision.
              TexturePromise newPromise = p;
              newPromise.requestInitiated = true;
              promisesGroup.modify(newPromise, static_cast<size_t>(index));

              mAssetsAwaitingRequest.insert(std::string(newPromise.assetName));
            }
            else
            {
              mAssetsAlreadyRequested.insert(std::string(p.assetName));
            }
          }

          ++index;
        }
      }
    }
  };

  const char* TextureMan::getPromiseSystemName()
  {
    return TexturePromiseFulfillment::getName();
  }

  //------------------------------------------------------------------------------
  // GARBAGE COLLECTION
  //------------------------------------------------------------------------------

  void TextureMan::runGCAgainstVaidIDs(const std::set<GLuint>& validKeys)
  {
    if (mNewUnfulfilledAssets)
    {
      logRendererError("TextureMan: Terminating garbage collection. Orphan assets that"
        " have yet to be associated with entity ID's would be GC'd");
      return;
    }

    // Every GLuint in validKeys should be in our map. If there is not, then
    // there is an error in the system, and it should be reported.
    // The reverse is not expected to be true, and is what we are attempting to
    // correct with this function.
    auto it = mGLToName.begin();
    for (const GLuint& id : validKeys)
    {
      // Find the key in the map, eliminating any keys that do not match the
      // current id along the way. We iterate through both the map and the set
      // in an ordered fashion.
      while (it != mGLToName.end() && it->first < id)
      {
        // Find the asset name in mNameToGL and erase.
        mNameToGL.erase(mNameToGL.find(it->second));

        //std::cout << "Texture GC: " << it->second << std::endl;

        // Erase our iterator and move on. Ensure we delete the program.
        GLuint idToErase = it->first;
        mGLToName.erase(it++);
        GL(glDeleteTextures(1, &idToErase));
      }

      if (it == mGLToName.end())
      {
        logRendererError("runGCAgainstVaidIDs: terminating early, validKeys contains "
          "elements not in texture map.");
        break;
      }

      // Check to see if the valid ids contain a component that is not in
      // mGLToName. If an object manages its own texture, but still uses the
      // texture component, this is not an error.
      if (it->first > id)
      {
        logRendererError("runGCAgainstValidIDs: validKeys contains elements not in the texture map.");
      }

      ++it;
    }

    // Terminate any remaining assets.
    while (it != mGLToName.end())
    {
      // Find the asset name in mNameToGL and erase.
      mNameToGL.erase(mNameToGL.find(it->second));

      //std::cout << "Texture GC: " << it->second << std::endl;

      // Erase our iterator and move on. Ensure we delete the program.
      GLuint idToErase = it->first;
      mGLToName.erase(it++);
      GL(glDeleteTextures(1, &idToErase));
    }
  }

  class TextureGarbageCollector :
    public spire::GenericSystem<false, Texture>
  {
  public:

    static const char* getName() { return "ren:TextureGarbageCollector"; }

    std::set<GLuint> mValidKeys;

    void preWalkComponents(spire::ESCoreBase&) override
    {
      mValidKeys.clear();
    }

    void postWalkComponents(spire::ESCoreBase& core) override
    {
      auto man = core.getStaticComponent<StaticTextureMan>();
      if (!man)
      {
        logRendererError("Unable to complete texture garbage collection. There is no StaticTextureMan.");
        return;
      }
      std::weak_ptr<TextureMan> tm = man->instance_;
      if (std::shared_ptr<TextureMan> texMan = tm.lock()) {
        texMan->runGCAgainstVaidIDs(mValidKeys);
        mValidKeys.clear();
      }
    }

    void execute(spire::ESCoreBase&, uint64_t /* entityID */, const Texture* tex) override
    {
      mValidKeys.insert(tex->glid);
    }
  };

  const char* TextureMan::getGCName()
  {
    return TextureGarbageCollector::getName();
  }

  void TextureMan::registerSystems(spire::Acorn& core)
  {
    core.registerSystem<TexturePromiseFulfillment>();
    core.registerSystem<TextureGarbageCollector>();
  }

  void TextureMan::runGCCycle(spire::ESCoreBase& core)
  {
    TextureGarbageCollector gc;
    gc.walkComponents(core);
  }

} // namespace ren

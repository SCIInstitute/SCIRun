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

#include <set>
#include <stdexcept>
#include <string>

#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>

#include "FBOMan.hpp"
#include "comp/StaticFBOMan.hpp"
#include "comp/FBO.hpp"

namespace es = spire;
namespace shaders = spire;

namespace ren {

  FBOMan::FBOMan()
  {
  }

  FBOMan::~FBOMan()
  {
    for (auto it = mFBOData.begin(); it != mFBOData.end(); ++it)
    {
      GLuint idToErase = it->first;
      GL(glDeleteBuffers(1, &idToErase));
    }
    mFBOData.clear();
  }

  GLuint FBOMan::createFBO(spire::CerealCore& core, GLenum ttype,
    GLsizei npixelx, GLsizei npixely, GLsizei npixelz,
    const std::string& assetName)
  {
    ren::FBO fbo;
    GLuint glid;
    GL(glGenFramebuffers(1, &glid));
    bindFBO(glid);
    fbo.glid = glid;
    fbo.textureType = ttype;

    //texture
    std::weak_ptr<TextureMan> tm = core.getStaticComponent<StaticTextureMan>()->instance_;
    if (std::shared_ptr<TextureMan> textureMan = tm.lock())
    {
      //integer
      ren::FBO::TextureData texData;
      texData.att = GL_COLOR_ATTACHMENT0;
      texData.texName = "FBO:Texture:Integer";
      ren::Texture tex = textureMan->createTexture(
        texData.texName, npixelx, npixely, GL_RGBA,
        GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST);
      uint64_t entityID = getEntityIDForName(texData.texName);
      core.addComponent(entityID, tex);
      fbo.textures.push_back(texData);
      //depth
      texData.att = GL_DEPTH_ATTACHMENT;
      texData.texName = "FBO:Texture:Depth";
      tex = textureMan->createTexture(
        texData.texName, npixelx, npixely, GL_DEPTH_COMPONENT,
        GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST);
      entityID = getEntityIDForName(texData.texName);
      core.addComponent(entityID, tex);
      fbo.textures.push_back(texData);

      //attach
      GLuint texid;
      for (auto i = fbo.textures.begin();
        i != fbo.textures.end(); ++i)
      {
        texid = textureMan->getIDForAsset((i->texName).c_str());
        //just assume 2D texture here
        GL(glFramebufferTexture2D(GL_FRAMEBUFFER,
          i->att, fbo.textureType, texid, 0));
      }
    }

    uint64_t entityID = getEntityIDForName(assetName);
    fbo.initialized = true;
    core.addComponent(entityID, fbo);

    mFBOData.insert(std::make_pair(glid,
      FBOData(assetName, ttype, npixelx, npixely, npixelz)));
    mNameToGL.insert(std::make_pair(assetName, glid));

    unbindFBO();
    return glid;
  }

  GLuint FBOMan::resizeFBO(spire::CerealCore& core,
    const std::string& assetName,
    GLsizei npixelx, GLsizei npixely, GLsizei npixelz)
  {
    FBOData fboData = getFBOData(assetName);
    if (fboData.numPixelsX == npixelx &&
      fboData.numPixelsY == npixely)
      return 0;
    fboData.numPixelsX = npixelx;
    fboData.numPixelsY = npixely;
    modifyFBOData(assetName, fboData);

    //get fbo
    spire::CerealHeap<ren::FBO>* contFBO =
      core.getOrCreateComponentContainer<ren::FBO>();
    std::pair<const ren::FBO*, size_t> component =
      contFBO->getComponent(getEntityIDForName(assetName));
    if (component.first == nullptr)
      return 0;
    ren::FBO fbo = *component.first;
    //get tex container
    //containerID = spire::getESTypeID<ren::Texture>();
    //container = core.getComponentContainer(containerID);
    //spire::CerealHeap<ren::Texture>* contTex =
    //	dynamic_cast<spire::CerealHeap<ren::Texture>*>(container);
    spire::CerealHeap<ren::Texture>* contTex =
      core.getOrCreateComponentContainer<ren::Texture>();
    //get tex manager
    std::weak_ptr<TextureMan> tm = core.getStaticComponent<StaticTextureMan>()->instance_;
    std::shared_ptr<TextureMan> textureMan = tm.lock();
    if (!textureMan)
      return 0;
    GLuint texid;
    for (auto i = fbo.textures.begin();
      i != fbo.textures.end(); ++i)
    {
      texid = textureMan->getIDForAsset((i->texName).c_str());
      GL(glBindTexture(fbo.textureType, texid));
      //get texture
      std::pair<const ren::Texture*, size_t> compTex =
        contTex->getComponent(getEntityIDForName(i->texName));
      if (compTex.first == nullptr)
        continue;
      ren::Texture tex = *compTex.first;
      tex.textureWidth = npixelx;
      tex.textureHeight = npixely;
      GL(glTexImage2D(fbo.textureType, 0, tex.internalFormat,
        tex.textureWidth, tex.textureHeight, 0,
        tex.format, tex.type, 0));
      //modify
      contTex->modifyIndex(tex, compTex.second, 0);
    }
    GL(glBindTexture(fbo.textureType, 0));
    return fbo.glid;
  }

  GLuint FBOMan::getOrCreateFBO(spire::CerealCore& core, GLenum ttype,
    GLsizei npixelx, GLsizei npixely, GLsizei npixelz,
    const std::string& assetName)
  {
    //get fbo
    //uint64_t containerID = spire::getESTypeID<ren::FBO>();
    //spire::BaseComponentContainer* container =
    //	core.getComponentContainer(containerID);
    //spire::CerealHeap<ren::FBO>* contFBO =
    //	dynamic_cast<spire::CerealHeap<ren::FBO>*>(container);
    spire::CerealHeap<ren::FBO>* contFBO =
      core.getOrCreateComponentContainer<ren::FBO>();
    std::pair<const ren::FBO*, size_t> component =
      contFBO->getComponent(getEntityIDForName(assetName));
    if (component.first == nullptr)
      return createFBO(core, ttype, npixelx, npixely, npixelz, assetName);
    else
    {
      FBOData fboData = getFBOData(assetName);
      if (fboData.numPixelsX == npixelx &&
        fboData.numPixelsY == npixely)
        return component.first->glid;
      else
        return resizeFBO(core, assetName, npixelx, npixely, npixelz);
    }
  }

  void FBOMan::bindFBO(const std::string& assetName)
  {
    GLint saveglid;
    GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &saveglid));
    mFBOIds.push(saveglid);
    GLuint glid = getIDForAsset(assetName.c_str());
    GL(glBindFramebuffer(GL_FRAMEBUFFER, glid));
  }

  void FBOMan::bindFBO(GLuint glid)
  {
    GLint saveglid;
    GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &saveglid));
    mFBOIds.push(saveglid);
    GL(glBindFramebuffer(GL_FRAMEBUFFER, glid));
  }

  void FBOMan::unbindFBO()
  {
    GLint saveglid = mFBOIds.top();
    GL(glBindFramebuffer(GL_FRAMEBUFFER, saveglid));
    mFBOIds.pop();
  }

  bool FBOMan::readFBO(spire::CerealCore& core,
    const std::string& assetName,
    GLint posx, GLint posy, GLsizei width, GLsizei height,
    GLvoid* value, GLvoid* depth)
  {
    //get fbo
    spire::CerealHeap<ren::FBO>* contFBO =
      core.getOrCreateComponentContainer<ren::FBO>();
    std::pair<const ren::FBO*, size_t> component =
      contFBO->getComponent(getEntityIDForName(assetName));
    if (component.first == nullptr)
      return false;
    ren::FBO fbo = *component.first;

    //get tex container
    spire::CerealHeap<ren::Texture>* contTex =
      core.getOrCreateComponentContainer<ren::Texture>();
    //get tex manager
    std::weak_ptr<TextureMan> tm = core.getStaticComponent<StaticTextureMan>()->instance_;
    std::shared_ptr<TextureMan> textureMan = tm.lock();
    if (!textureMan)
      return false;

    auto texi = fbo.textures.begin();
    if (texi == fbo.textures.end())
      return false;
    //get texture
    std::pair<const ren::Texture*, size_t> compTex =
      contTex->getComponent(getEntityIDForName(texi->texName));
    if (compTex.first == nullptr)
      return false;

    ren::Texture tex = *compTex.first;
    GL(glReadPixels(posx, tex.textureHeight - posy, width, height,
      tex.format, tex.type, value));
    if (depth)
      GL(glReadPixels(posx, tex.textureHeight - posy, width, height,
        GL_DEPTH_COMPONENT, GL_FLOAT, depth));
    return true;
  }

  void FBOMan::removeInMemoryFBO(GLuint glid)
  {
    auto iter = mFBOData.find(glid);
    mFBOData.erase(iter);

    GL(glDeleteFramebuffers(1, &glid));
  }

  GLuint FBOMan::hasFBO(const std::string& assetName) const
  {
    for (auto it = mFBOData.begin(); it != mFBOData.end(); ++it)
    {
      if (it->second.assetName == assetName)
      {
        return it->first;
      }
    }
    return 0;
  }

  FBOMan::FBOData FBOMan::getFBOData(const std::string& assetName) const
  {
    for (auto it = mFBOData.begin(); it != mFBOData.end(); ++it)
    {
      if (it->second.assetName == assetName)
      {
        return it->second;
      }
    }

    throw std::runtime_error("FBOMan: Unable to find FBO data");
  }

  void FBOMan::modifyFBOData(const std::string& assetName, FBOData &fboData)
  {
    for (auto it = mFBOData.begin(); it != mFBOData.end(); ++it)
    {
      if (it->second.assetName == assetName)
      {
        it->second.numPixelsX = fboData.numPixelsX;
        it->second.numPixelsY = fboData.numPixelsY;
        it->second.numPixelsZ = fboData.numPixelsZ;
        it->second.textureType = fboData.textureType;
        return;
      }
    }

    throw std::runtime_error("FBOMan: Unable to find FBO data");
  }

  GLuint FBOMan::getIDForAsset(const char* assetName) const
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

  std::string FBOMan::getAssetFromID(GLuint id) const
  {
    auto it = mFBOData.find(id);
    if (it != mFBOData.end())
    {
      return it->second.assetName;
    }
    else
    {
      return "";
    }
  }

  uint64_t FBOMan::getEntityIDForName(const std::string& name)
  {
    return (static_cast<uint64_t>(std::hash<std::string>()(name)) >> 8);
  }

  //------------------------------------------------------------------------------
  // GARBAGE COLLECTION
  //------------------------------------------------------------------------------

  void FBOMan::runGCAgainstVaidIDs(const std::set<GLuint>& validKeys)
  {
    // Every GLuint in validKeys should be in our map. If there is not, then
    // there is an error in the system, and it should be reported.
    // The reverse is not expected to be true, and is what we are attempting to
    // correct with this function.
    auto it = mFBOData.begin();
    for (const GLuint& id : validKeys)
    {
      // Find the key in the map, eliminating any keys that do not match the
      // current id along the way.
      while (it != mFBOData.end() && it->first < id)
      {
        //\cb std::cout << "FBO GC: " << it->second.assetName << std::endl;

        GLuint idToErase = it->first;
        mFBOData.erase(it++);
        GL(glDeleteBuffers(1, &idToErase));
      }

      if (it == mFBOData.end())
      {
        std::cerr << "runGCAgainstVaidIDs: terminating early, validKeys contains "
          << "elements not in FBO map." << std::endl;
        break;
      }

      // Check to see if the valid ids contain a component that is not in
      // mFBOData. If an object manages its own FBO, but still uses the FBO
      // component, this is not an error.
      if (it->first > id)
      {
        std::cerr << "runGCAgainstVaidIDs: validKeys contains elements not in the FBO map." << std::endl;
      }

      // Increment passed current validKey id.
      ++it;
    }

    while (it != mFBOData.end())
    {
      //\cb std::cout << "FBO GC: " << it->second.assetName << std::endl;

      GLuint idToErase = it->first;
      mFBOData.erase(it++);
      GL(glDeleteBuffers(1, &idToErase));
    }
  }

  class FBOGarbageCollector :
    public spire::GenericSystem<false, FBO>
  {
  public:

    static const char* getName()    { return "ren:FBOGarbageCollector"; }

    std::set<GLuint> mValidKeys;

    void preWalkComponents(spire::ESCoreBase&) override { mValidKeys.clear(); }
    void postWalkComponents(spire::ESCoreBase& core) override
    {
      std::weak_ptr<FBOMan> im = core.getStaticComponent<StaticFBOMan>()->instance_;
      if (std::shared_ptr<FBOMan> man = im.lock()) {
        man->runGCAgainstVaidIDs(mValidKeys);
        mValidKeys.clear();
      }
      else
      {
        std::cerr << "FBOMan: StaticFBOMan not found is given core. Failed to run FBO GC." << std::endl;
      }
    }

    void execute(spire::ESCoreBase&, uint64_t /* entityID */, const FBO* fbo) override
    {
      mValidKeys.insert(fbo->glid);
    }
  };

  void FBOMan::registerSystems(spire::Acorn& core)
  {
    // Register the garbage collector.
    core.registerSystem<FBOGarbageCollector>();
  }

  void FBOMan::runGCCycle(spire::ESCoreBase& core)
  {
    FBOGarbageCollector gc;
    gc.walkComponents(core);
  }

  const char* FBOMan::getGCName()
  {
    return FBOGarbageCollector::getName();
  }

} // namespace ren

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


#include <es-log/trace-log.h>
#include <entity-system/GenericSystem.hpp>
#include <tny/tny.hpp>
#include <es-fs/Filesystem.hpp>
#include <es-fs/fscomp/StaticFS.hpp>

#include "GeomMan.hpp"
#include "comp/Geom.hpp"
#include "comp/GeomPromise.hpp"
#include "comp/StaticGeomMan.hpp"
#include "comp/StaticVBOMan.hpp"
#include "comp/StaticIBOMan.hpp"
#include "comp/StaticShaderMan.hpp"
#include "comp/StaticTextureMan.hpp"
#include "comp/IBO.hpp"
#include "comp/VBO.hpp"

namespace es = spire;
namespace fs = spire;
using namespace spire;

namespace ren {

GeomMan::GeomMan(int numRetries) :
    mNewUnfulfilledAssets(false),
    mNumRetries(numRetries)
{
  RENDERER_LOG_FUNCTION_SCOPE;
  RENDERER_LOG("GeomMan ctor (numRetries={})", numRetries);
}

void GeomMan::loadGeometry(spire::CerealCore& core,
                    uint64_t entityID, const std::string& assetName)
{
  RENDERER_LOG_FUNCTION_SCOPE;
  RENDERER_LOG("GeomMan::loadGeometry (entityID={}, assetName={})", entityID, assetName);

  // Ensure there is a file extension on the asset name. The texture loader
  // does also accept requests for png.
  std::string::size_type dotIdx = assetName.rfind('.');
  std::string fullAssetName = assetName;
  if (dotIdx == std::string::npos)
  {
    fullAssetName = assetName + ".geom";
  }

  if (!buildComponent(core, entityID, fullAssetName))
  {
    // We failed to build the component immediately. Initiate a promise for
    // the component.
    GeomPromise newPromise;
    newPromise.requestInitiated = false;
    newPromise.setAssetName(fullAssetName.c_str());

    core.addComponent(entityID, newPromise);
  }
}

void GeomMan::requestAsset(spire::ESCoreBase& core, const std::string& assetName,
                           int32_t numRetries)
{
  RENDERER_LOG_FUNCTION_SCOPE;
  RENDERER_LOG("{} (numRetries={}, assetName={})", LOG_FUNC, numRetries, assetName);

  // Begin by attempting to load the vertex shader.
  fs::StaticFS* sfs = core.getStaticComponent<fs::StaticFS>();

  spire::ESCoreBase* refPtr = &core;
  auto callbackLambda = [this, numRetries, refPtr](
      const std::string& name, bool error, size_t bytesRead, uint8_t* buffer)
  {
    loadAssetCB(name, error, bytesRead, buffer, numRetries, *refPtr);
  };
  sfs->instance->readFile(assetName, callbackLambda);
}

void GeomMan::loadAssetCB(const std::string& assetName, bool error,
                   size_t bytesRead, uint8_t* buffer, int32_t numRetries,
                   spire::ESCoreBase& core)
{
  if (!error)
  {
    // Check to see if the VBO / IBO man already have assets of the given name.
    // Load Tny from raw data.
    Tny* doc = Tny_loads(buffer, bytesRead);
    if (doc)
    {
      // Run through Tny document and extract all appropriate information
      // from geom document.
      bool littleEndian = true;
      spire::CerealSerializeType<bool>::in(doc, "little_endian", littleEndian);
      if (littleEndian == false)
      {
        logRendererError("Big endian is not a supported geometry type.");
        Tny_free(doc);
        return;
      }

      GeomItem geom;

      uint32_t numMeshes = 0;
      spire::CerealSerializeType<uint32_t>::in(doc, "num_meshes", numMeshes);
      if (numMeshes > 1)
      {
        logRendererError("There is no planned support for multiple meshes in a single geom file.");
      }

      std::string shaderName;
      if (Tny_get(doc, "shader") != NULL)
      {
        // Set shader name appropriately.
        spire::CerealSerializeType<std::string>::in(doc, "shader", shaderName);
      }
      geom.shaderName = shaderName;

      const int tmpNameSize = 255;
      char tmpName[tmpNameSize];
      Tny* textureRoot = Tny_get(doc, "textures")->value.tny;
      textureRoot = Tny_next(textureRoot);
      while (textureRoot && Tny_hasNext(textureRoot))
      {
        std::string texName;
        uint32_t texUnit;
        std::string samplerName;
        textureRoot = spire::CST_detail::inStringArray(textureRoot, tmpName, tmpNameSize);
        texName = tmpName;
        textureRoot = spire::CST_detail::inUInt32Array(textureRoot, texUnit);
        textureRoot = spire::CST_detail::inStringArray(textureRoot, tmpName, tmpNameSize);
        samplerName = tmpName;

        // Add item to texture list.
        geom.textures.emplace_back(texName, samplerName, texUnit);
      }

      std::weak_ptr<VBOMan>  vm = core.getStaticComponent<StaticVBOMan>()->instance_;
      std::weak_ptr<IBOMan>  im = core.getStaticComponent<StaticIBOMan>()->instance_;

      if (std::shared_ptr<VBOMan> vboMan = vm.lock()) {
          if (std::shared_ptr<IBOMan> iboMan = im.lock()) {
              Tny* meshRoot = Tny_get(doc, "meshes")->value.tny;
              // A number of meshes doesn't make any sense since we select on the asset
              // name in the IBO and VBO.
              for (uint32_t i = 0; i < numMeshes; i++)
              {
                meshRoot = Tny_next(meshRoot);
                Tny* meshDict = meshRoot->value.tny;

                // Retrieve the attributes stored in the mesh's vertex buffer.
                Tny* attributeRoot = Tny_get(meshDict, "attrib_list")->value.tny;
                attributeRoot = Tny_next(attributeRoot);

                // Retrieve the attributes in the geometry.
                int32_t numAttribs = 0;
                attributeRoot = spire::CST_detail::inInt32Array(
                            attributeRoot, numAttribs);

                std::vector<std::tuple<std::string, size_t, bool>> attribs;
                for (int32_t ii = 0; ii < numAttribs; ii++)
                {
                  std::string attribName;
                  int32_t attribSize;
                  bool normalize;
                  attributeRoot = spire::CST_detail::inStringArray(
                              attributeRoot, tmpName, tmpNameSize);
                  attribName = tmpName;
                  attributeRoot = spire::CST_detail::inInt32Array(
                              attributeRoot, attribSize);
                  attributeRoot = spire::CST_detail::inBoolArray(
                              attributeRoot, normalize);
                  attribs.push_back(std::make_tuple(attribName, attribSize, normalize));
                }

                // Retrieve the number of vertices. Even though we really don't use
                // the number of vertices.
                uint32_t numVertices;
                spire::CerealSerializeType<uint32_t>::in(
                            meshDict, "num_vertices", numVertices);

                // Retrieve and install binary data.
                Tny* vboData = Tny_get(meshDict, "vbo");
                GLuint vboID = vboMan->hasVBO(assetName);
                if (vboID == 0)
                  vboID = vboMan->addInMemoryVBO(vboData->value.ptr,
                                                 vboData->size, attribs, assetName);

                if (vboID == 0)
                {
                  logRendererError("GeomMan: Unable to generate appropriate VBO.");
                }

                // Retrieve and create ibo (if not already created).
                Tny* iboData = Tny_get(meshDict, "ibo");
                GLuint iboID = iboMan->hasIBO(assetName);
                if (iboID == 0)
                  iboID = iboMan->addInMemoryIBO(iboData->value.ptr, iboData->size,
                                                GL_TRIANGLES, GL_UNSIGNED_SHORT,
                                                iboData->size / sizeof(uint16_t), assetName);

                if (iboID == 0)
                {
                  logRendererError("GeomMan: Unable to generate appropriate IBO.");
                }

                // We are done. Now the promise fulfillment system will locate and add
                // the appropriate VBO, IBO, shader, and textures.
              }
              mNameMap.insert(std::make_pair(assetName, geom));

              Tny_free(doc);


          }
      }
    }
    else
    {
      logRendererError("GeomMan: Unable to generate Tny document from {}", assetName);
      if (numRetries > 0)
      {
        --numRetries;
        requestAsset(core, assetName, numRetries);
      }
      else
      {
        logRendererError("GeomMan: Failed promise for {}", assetName);
      }
    }
  }
  else
  {
    if (numRetries > 0)
    {
      --numRetries;
      requestAsset(core, assetName, numRetries);
    }
    else
    {
      logRendererError("GeomMan: Failed promise for {}", assetName);
    }
  }
}


bool GeomMan::hasGeomItemForAsset(const char* assetName)
{
  auto it = mNameMap.find(std::string(assetName));
  if (it != mNameMap.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool GeomMan::buildComponent(spire::CerealCore& core,
                             uint64_t entityID, const std::string& assetName)
{
  if (hasGeomItemForAsset(assetName.c_str()))
  {
    // Asset has been loaded. Forget about everything else and build
    // those components!

    // We will need to look

    // Go ahead and add a new component for the entityID. If this is the
    // last promise to fulfill, then systems should automatically start
    // rendering the data.
    Geom component;
    component.setAssetName(assetName.c_str());
    core.addComponent(entityID, component);

    std::weak_ptr<VBOMan>  vm = core.getStaticComponent<StaticVBOMan>()->instance_;
    std::weak_ptr<IBOMan>  im = core.getStaticComponent<StaticIBOMan>()->instance_;

    if (std::shared_ptr<VBOMan> vboMan = vm.lock()) {
        if (std::shared_ptr<IBOMan> iboMan = im.lock()) {

            // VBO and IBO
            ren::VBO vbo;
            vbo.glid = vboMan->hasVBO(assetName);
            if (vbo.glid == 0)
            {
              logRendererError("GeomMan: Failed VBO promise. No VBO present.");
            }

            ren::IBO ibo;
            ibo.glid = iboMan->hasIBO(assetName);
            if (ibo.glid == 0)
            {
              logRendererError("GeomMan: Failed VBO promise. No VBO present.");
            }

            if (vbo.glid != 0 && ibo.glid != 0)
            {
              core.addComponent(entityID, vbo);

              // Retrieve IBO data.
              const IBOMan::IBOData& data = iboMan->getIBOData(assetName);
              ibo.primMode = data.primMode;
              ibo.primType = data.primType;
              ibo.numPrims = data.numPrims;
              core.addComponent(entityID, ibo);
            }

            auto geomAsset = mNameMap.find(assetName);
            if (geomAsset != mNameMap.end())
            {
              const GeomMan::GeomItem& geomItem = geomAsset->second;
              // Shader (will be loaded if necessary).
              std::weak_ptr<ShaderMan> sm =
                      core.getStaticComponent<StaticShaderMan>()->instance_;
              if (std::shared_ptr<ShaderMan> shaderMan = sm.lock()) {
                    shaderMan->loadVertexAndFragmentShader(core, entityID, geomItem.shaderName);
              }
              // Textures (will be loaded if necessary).
              std::weak_ptr<TextureMan> tm =
                      core.getStaticComponent<StaticTextureMan>()->instance_;
              if (std::shared_ptr<TextureMan> texMan = tm.lock()) {
                  for (const GeomMan::GeomItem::TextureItem& texItem : geomItem.textures)
                  {
                    texMan->loadTexture(core, entityID, texItem.name,
                                       static_cast<int32_t>(texItem.textureUnit),
                                       texItem.samplerName);
                  }
              }
            }
            else
            {
              logRendererError("GeomMan: Failed asset promise. Couldn't find asset in map.");
            }

            return true;  // We want to clear the promise regardless of failures.
        }
    }
    return false;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
// PROMISE FULFILLMENT
//------------------------------------------------------------------------------

class GeomPromiseFulfillment :
    public spire::GenericSystem<true,
                             GeomPromise,
                             StaticGeomMan>
{
public:

  static const char* getName() {return "ren:GeomPromiseFulfillment";}

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
    std::weak_ptr<GeomMan> gm = core.getStaticComponent<StaticGeomMan>()->instance_;
    if (std::shared_ptr<GeomMan> man = gm.lock()) {
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
            man->requestAsset(core, asset, man->mNumRetries);
          }
        }
    } else {
        logRendererError("Unable to complete geom fulfillment. There is no StaticGeomMan.");
    }
  }

  void groupExecute(spire::ESCoreBase& core, uint64_t entityID,
               const spire::ComponentGroup<GeomPromise>& promisesGroup,
               const spire::ComponentGroup<StaticGeomMan>& geomManGroup) override
  {
    std::weak_ptr<GeomMan> gm = geomManGroup.front().instance_;
    if (std::shared_ptr<GeomMan> geomMan = gm.lock()) {

        spire::CerealCore* ourCorePtr =
                dynamic_cast<spire::CerealCore*>(&core);
        if (!ourCorePtr)
        {
          logRendererError("Unable to execute geom promise fulfillment. Bad cast.");
          return;
        }
        spire::CerealCore& ourCore = *ourCorePtr;

        int index = 0;
        for (const GeomPromise& p : promisesGroup)
        {
          // Check to see if this promise has been fulfilled. If it has, then
          // remove it and create the appropriate component for the indicated
          // entity.
          if (geomMan->buildComponent(ourCore, entityID, p.assetName))
          {
            // Remove this promise, and add a geom component to this promises'
            // entityID. It is safe to remove components while we are using a
            // system - addition / removal / modification doesn't happen until
            // a renormalization step.
            ourCore.removeComponentAtIndexT<GeomPromise>(entityID, index);
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
              GeomPromise newPromise = p;
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

const char* GeomMan::getPromiseSystemName()
{
  return GeomPromiseFulfillment::getName();
}

//------------------------------------------------------------------------------
// GARBAGE COLLECTION
//------------------------------------------------------------------------------

void GeomMan::runGCAgainstVaidNames(const std::set<std::string>& validKeys)
{
  if (mNewUnfulfilledAssets)
  {
    //RendererLog::get()->error("GeomMan: Terminating garbage collection. Orphan assets that have yet to be associated with entity ID's would be GC'd");
    return;
  }

  // Every GLuint in validKeys should be in our map. If there is not, then
  // there is an error in the system, and it should be reported.
  // The reverse is not expected to be true, and is what we are attempting to
  // correct with this function.
  auto it = mNameMap.begin();
  for (const std::string& asset : validKeys)
  {
    // Find the key in the map, eliminating any keys that do not match the
    // current id along the way. We iterate through both the map and the set
    // in an ordered fashion.
    while (it != mNameMap.end() && it->first < asset)
    {
      std::cout << "Geom GC: " << it->first << std::endl;
      mNameMap.erase(it++);
    }

    if (it == mNameMap.end())
    {
      //RendererLog::get()->error("runGCAgainstVaidNames: terminating early, validKeys contains elements not in Geom map.");
      break;
    }

    // Check to see if the valid ids contain a component that is not in
    // mGLToName. If an object manages its own geom, but still uses the geom
    // component, this is not an error.
    if (it->first > asset)
    {
      //RendererLog::get()->error("runGCAgainstVaidNames: validKeys contains elements not in the Geom map.");
    }

    ++it;
  }

  while (it != mNameMap.end())
  {
    //RendererLog::get()->info("Geom GC: {}", it->first);
    mNameMap.erase(it++);
  }
}

class GeomGarbageCollector :
    public spire::GenericSystem<false, Geom>
{
public:

  static const char* getName() {return "ren:GeomGarbageCollector";}

  std::set<std::string> mValidKeys;

  void preWalkComponents(spire::ESCoreBase&) override {mValidKeys.clear();}

  void postWalkComponents(spire::ESCoreBase& core) override
  {
    std::weak_ptr<GeomMan> gm = core.getStaticComponent<StaticGeomMan>()->instance_;
    if (std::shared_ptr<GeomMan> geomMan = gm.lock())
    {
        geomMan->runGCAgainstVaidNames(mValidKeys);
        mValidKeys.clear();
    }
    else
    {
        //RendererLog::get()->error("Unable to complete geom garbage collection. There is no StaticGeomMan.");
    }
  }

  void execute(spire::ESCoreBase&, uint64_t /* entityID */, const Geom* geom) override
  {
    mValidKeys.insert(geom->assetName);
  }
};

const char* GeomMan::getGCName()
{
  return GeomGarbageCollector::getName();
}

void GeomMan::registerSystems(spire::Acorn& core)
{
  core.registerSystem<GeomPromiseFulfillment>();
  core.registerSystem<GeomGarbageCollector>();
}

void GeomMan::runGCCycle(spire::ESCoreBase& core)
{
  GeomGarbageCollector gc;
  gc.walkComponents(core);
}

} // namespace ren

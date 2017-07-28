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

namespace es = CPM_ES_NS;
namespace fs = CPM_ES_FS_NS;

namespace ren {

GeomMan::GeomMan(int numRetries) :
    mNewUnfulfilledAssets(false),
    mNumRetries(numRetries)
{}

GeomMan::~GeomMan()
{
  // There is nothing special that we need to do to clean up geometry,
  // unlike shaders and textures.
  mNameMap.clear();
}

void GeomMan::loadGeometry(CPM_ES_CEREAL_NS::CerealCore& core,
                    uint64_t entityID, const std::string& assetName)
{
  // Ensure there is a file extension on the asset name. The texture loader
  // does also accept requests for png.
  std::string::size_type dotIdx = assetName.rfind('.');
  std::string fullAssetName = assetName;
  if (dotIdx == std::string::npos)
  {
    fullAssetName = assetName + ".geom";
  }

  if (buildComponent(core, entityID, fullAssetName) == false)
  {
    // We failed to build the component immediately. Initiate a promise for
    // the component.
    GeomPromise newPromise;
    newPromise.requestInitiated = false;
    newPromise.setAssetName(fullAssetName.c_str());

    core.addComponent(entityID, newPromise);
  }
}

void GeomMan::requestAsset(es::ESCoreBase& core, const std::string& assetName,
                           int32_t numRetries)
{
  // Begin by attempting to load the vertex shader.
  fs::StaticFS* sfs = core.getStaticComponent<fs::StaticFS>();

  /// \todo Remove IFDEF when we switch to emscripten's new compiler backend
  ///       which will support std::bind (which I prefer over using lambdas).
//#ifndef EMSCRIPTEN
//  sfs->instance->readFile(assetName,
//                         std::bind(&GeomMan::loadAssetCB, this, 
//                                   std::placeholders::_1, std::placeholders::_2,
//                                   std::placeholders::_3, std::placeholders::_4,
//                                   numRetries, std::ref(core)));
//#else
  es::ESCoreBase* refPtr = &core;
  auto callbackLambda = [this, numRetries, refPtr](
      const std::string& name, bool error, size_t bytesRead, uint8_t* buffer)
  {
    loadAssetCB(name, error, bytesRead, buffer, numRetries, *refPtr);
  };
  sfs->instance->readFile(assetName, callbackLambda);
//#endif
}

void GeomMan::loadAssetCB(const std::string& assetName, bool error,
                   size_t bytesRead, uint8_t* buffer, int32_t numRetries,
                   es::ESCoreBase& core)
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
      CPM_ES_CEREAL_NS::CerealSerializeType<bool>::in(doc, "little_endian", littleEndian);
      if (littleEndian == false)
      {
        std::cerr << "Big endian is not a supported geometry type." << std::endl;
        Tny_free(doc);
        return;
      }

      GeomItem geom;

      uint32_t numMeshes = 0;
      CPM_ES_CEREAL_NS::CerealSerializeType<uint32_t>::in(doc, "num_meshes", numMeshes);
      if (numMeshes > 1)
      {
        std::cerr << "There is no planned support for multiple meshes in a single geom file." << std::endl;
      }

      std::string shaderName;
      if (Tny_get(doc, "shader") != NULL)
      {
        // Set shader name appropriately.
        CPM_ES_CEREAL_NS::CerealSerializeType<std::string>::in(doc, "shader", shaderName);
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
        textureRoot = CPM_ES_CEREAL_NS::CST_detail::inStringArray(textureRoot, tmpName, tmpNameSize);
        texName = tmpName;
        textureRoot = CPM_ES_CEREAL_NS::CST_detail::inUInt32Array(textureRoot, texUnit);
        textureRoot = CPM_ES_CEREAL_NS::CST_detail::inStringArray(textureRoot, tmpName, tmpNameSize);
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
                attributeRoot = CPM_ES_CEREAL_NS::CST_detail::inInt32Array(
                            attributeRoot, numAttribs);

                std::vector<std::tuple<std::string, size_t, bool>> attribs;
                for (int32_t ii = 0; ii < numAttribs; ii++)
                {
                  std::string attribName;
                  int32_t attribSize;
                  bool normalize;
                  attributeRoot = CPM_ES_CEREAL_NS::CST_detail::inStringArray(
                              attributeRoot, tmpName, tmpNameSize);
                  attribName = tmpName;
                  attributeRoot = CPM_ES_CEREAL_NS::CST_detail::inInt32Array(
                              attributeRoot, attribSize);
                  attributeRoot = CPM_ES_CEREAL_NS::CST_detail::inBoolArray(
                              attributeRoot, normalize);
                  attribs.push_back(std::make_tuple(attribName, attribSize, normalize));
                }

                // Retrieve the number of vertices. Even though we really don't use
                // the number of vertices.
                uint32_t numVertices;
                CPM_ES_CEREAL_NS::CerealSerializeType<uint32_t>::in(
                            meshDict, "num_vertices", numVertices);

                // Retrieve and install binary data.
                Tny* vboData = Tny_get(meshDict, "vbo");
                GLuint vboID = vboMan->hasVBO(assetName);
                if (vboID == 0)
                  vboID = vboMan->addInMemoryVBO(vboData->value.ptr,
                                                 vboData->size, attribs, assetName);

                if (vboID == 0)
                {
                  std::cerr << "GeomMan: Unable to generate appropriate VBO." << std::endl;
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
                  std::cerr << "GeomMan: Unable to generate appropriate IBO." << std::endl;
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
      std::cerr << "GeomMan: Unable to generate Tny document from " << assetName << std::endl;
      if (numRetries > 0)
      {
        --numRetries;
        requestAsset(core, assetName, numRetries);
      }
      else
      {
        std::cerr << "GeomMan: Failed promise for " << assetName << std::endl;
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
      std::cerr << "GeomMan: Failed promise for " << assetName << std::endl;
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

bool GeomMan::buildComponent(CPM_ES_CEREAL_NS::CerealCore& core,
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
              std::cerr << "GeomMan: Failed VBO promise. No VBO present." << std::endl;
            }

            ren::IBO ibo;
            ibo.glid = iboMan->hasIBO(assetName);
            if (ibo.glid == 0)
            {
              std::cerr << "GeomMan: Failed VBO promise. No VBO present." << std::endl;
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
              std::cerr << "GeomMan: Failed asset promise. " <<
                           "Couldn't find asset in map." << std::endl;
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
    public es::GenericSystem<true,
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

  void preWalkComponents(es::ESCoreBase&)
  {
    mAssetsAwaitingRequest.clear();
    mAssetsAlreadyRequested.clear();
  }

  void postWalkComponents(es::ESCoreBase& core)
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
        std::cerr << "Unable to complete geom fulfillment. " <<
                     "There is no StaticGeomMan." << std::endl;
    }
  }

  void groupExecute(es::ESCoreBase& core, uint64_t entityID,
               const es::ComponentGroup<GeomPromise>& promisesGroup,
               const es::ComponentGroup<StaticGeomMan>& geomManGroup) override
  {
    std::weak_ptr<GeomMan> gm = geomManGroup.front().instance_;
    if (std::shared_ptr<GeomMan> geomMan = gm.lock()) {

        CPM_ES_CEREAL_NS::CerealCore* ourCorePtr =
                dynamic_cast<CPM_ES_CEREAL_NS::CerealCore*>(&core);
        if (ourCorePtr == nullptr)
        {
          std::cerr << "Unable to execute geom promise fulfillment. Bad cast." << std::endl;
          return;
        }
        CPM_ES_CEREAL_NS::CerealCore& ourCore = *ourCorePtr;

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
    std::cerr << "GeomMan: Terminating garbage collection. Orphan assets that"
              << " have yet to be associated with entity ID's would be GC'd" << std::endl;
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
      std::cerr << "runGCAgainstVaidNames: terminating early, validKeys contains "
                << "elements not in Geom map." << std::endl;
      break;
    }

    // Check to see if the valid ids contain a component that is not in
    // mGLToName. If an object manages its own geom, but still uses the geom
    // component, this is not an error.
    if (it->first > asset)
    {
      std::cerr << "runGCAgainstVaidNames: validKeys contains elements not in the Geom map." << std::endl;
    }

    ++it;
  }

  while (it != mNameMap.end())
  {
    std::cout << "Geom GC: " << it->first << std::endl;
    mNameMap.erase(it++);
  }
}

class GeomGarbageCollector :
    public es::GenericSystem<false, Geom>
{
public:

  static const char* getName() {return "ren:GeomGarbageCollector";}

  std::set<std::string> mValidKeys;

  void preWalkComponents(es::ESCoreBase&) {mValidKeys.clear();}

  void postWalkComponents(es::ESCoreBase& core)
  {
    std::weak_ptr<GeomMan> gm = core.getStaticComponent<StaticGeomMan>()->instance_;
    if (std::shared_ptr<GeomMan> geomMan = gm.lock()) {
        geomMan->runGCAgainstVaidNames(mValidKeys);
        mValidKeys.clear();
    } else {
          std::cerr << "Unable to complete geom garbage collection. "
                    << "There is no StaticGeomMan." << std::endl;
    }
  }

  void execute(es::ESCoreBase&, uint64_t /* entityID */, const Geom* geom) override
  {
    mValidKeys.insert(geom->assetName);
  }
};

const char* GeomMan::getGCName()
{
  return GeomGarbageCollector::getName();
}

void GeomMan::registerSystems(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<GeomPromiseFulfillment>();
  core.registerSystem<GeomGarbageCollector>();
}

void GeomMan::runGCCycle(CPM_ES_NS::ESCoreBase& core)
{
  GeomGarbageCollector gc;
  gc.walkComponents(core);
}

} // namespace ren


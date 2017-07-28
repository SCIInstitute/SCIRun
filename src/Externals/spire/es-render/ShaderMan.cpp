#include <entity-system/GenericSystem.hpp>
#include <gl-shaders/GLShader.hpp>
#include <es-fs/Filesystem.hpp>
#include <es-fs/fscomp/StaticFS.hpp>

#include "ShaderMan.hpp"
#include "comp/ShaderPromiseVF.hpp"
#include "comp/StaticShaderMan.hpp"
#include "comp/Shader.hpp"

namespace es = CPM_ES_NS;
namespace fs = CPM_ES_FS_NS;

namespace ren {

std::string ShaderMan::mShaderHeader;

ShaderMan::ShaderMan(int numRetries) :
    mNewUnfulfilledAssets(false),
    mNumRetries(numRetries)
{
}

ShaderMan::~ShaderMan()
{
  // Destroy all GLIDs.
  for (auto it = mGLToName.begin(); it != mGLToName.end(); ++it)
  {
    GL(glDeleteShader(it->first));
  }
}

void ShaderMan::setShaderHeaderCode(const std::string& header)
{
  mShaderHeader = header;
}

void ShaderMan::loadVertexAndFragmentShader(
    CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
    const std::string& assetName)
{
  // Attempt to build the component from what we have in-memory.
  if (buildComponent(core, entityID, assetName) == false)
  {
    // Simply add a new promise component. Promise fulfillment will initiate
    // requests and create components as needed.
    ShaderPromiseVF newPromise;
    newPromise.requestInitiated = false;
    newPromise.setAssetName(assetName.c_str());

    core.addComponent(entityID, newPromise);
  }
}

GLuint ShaderMan::addInMemoryVSFS(
    const std::string& vsSource, const std::string& fsSource,
    const std::string& assetName)
{
  GLuint program = 0;
  if (mShaderHeader.size() > 0)
  {
    program = CPM_GL_SHADERS_NS::loadShaderProgram(
        {
        CPM_GL_SHADERS_NS::ShaderSource({mShaderHeader.c_str(), vsSource.c_str()}, GL_VERTEX_SHADER),
        CPM_GL_SHADERS_NS::ShaderSource({mShaderHeader.c_str(), fsSource.c_str()}, GL_FRAGMENT_SHADER)
        });
  }
  else
  {
    program = CPM_GL_SHADERS_NS::loadShaderProgram(
        {
        CPM_GL_SHADERS_NS::ShaderSource({vsSource.c_str()}, GL_VERTEX_SHADER),
        CPM_GL_SHADERS_NS::ShaderSource({fsSource.c_str()}, GL_FRAGMENT_SHADER)
        });
  }

  // Simply update mGLToName and mNameToGL. The fulfillment system will
  // handle everything else.
  mGLToName.insert(std::make_pair(program, assetName));
  mNameToGL.insert(std::make_pair(assetName, program));

  return program;
}

void ShaderMan::requestVSandFS(CPM_ES_NS::ESCoreBase& core, const std::string& assetName,
                               int32_t numRetries)
{
  // Begin by attempting to load the vertex shader.
  fs::StaticFS* sfs = core.getStaticComponent<fs::StaticFS>();
  std::string vertexShader = assetName + ".vs";

  //sfs->instance->readFile(vertexShader,
  //                       std::bind(&ShaderMan::loadVertexShaderCB, this, 
  //                                 std::placeholders::_1, std::placeholders::_2,
  //                                 std::placeholders::_3, std::placeholders::_4,
  //                                 assetName, numRetries, std::ref(core)));

  /// \todo Get rid of this code when we switch to the new emscripten backend.
  ///       std::bind is preferable to cooking up a lambda. Working bind code
  ///       is given directly above.
  es::ESCoreBase* refPtr = &core;
  auto callbackLambda = [this, assetName, numRetries, refPtr](
      const std::string& vsName, bool error, size_t bytesRead, uint8_t* buffer)
  {
    loadVertexShaderCB(vsName, error, bytesRead, buffer, assetName, numRetries, *refPtr);
  };
  sfs->instance->readFile(vertexShader, callbackLambda);
}

void ShaderMan::loadVertexShaderCB(const std::string& /* vsName */, bool error,
                                   size_t bytesRead, uint8_t* buffer,
                                   std::string assetName, int32_t numRetries,
                                   CPM_ES_NS::ESCoreBase& core)
{
  if (!error)
  {
    // Build a buffer 1 byte longer and terminate it with a null.
    // Pass the resulting string into our system. In the future, we should
    // add enough buffer to the beginning of the buffer to store any header
    // information.
    char* vertexSourceRaw = static_cast<char*>(std::malloc(bytesRead + 1));
    std::memcpy(vertexSourceRaw , buffer, bytesRead);
    vertexSourceRaw[bytesRead] = '\0';

    std::string vertexSource = vertexSourceRaw;

    std::free(vertexSourceRaw);

    std::string fragmentShader = assetName + ".fs";
    fs::StaticFS* frag = core.getStaticComponent<fs::StaticFS>();

    /// \todo Get rid of this code when we switch to the new emscripten backend.
    ///       std::bind is preferable to cooking up a lambda. Working bind code
    ///       is given directly below.
    es::ESCoreBase* refPtr = &core;
    auto callbackLambda = [this, assetName, numRetries, refPtr, vertexSource](
        const std::string& fsName, bool lambdaError, size_t lambdaBytesRead, uint8_t* lambdaBuffer)
    {
      loadFragmentShaderCB(fsName, lambdaError, lambdaBytesRead, lambdaBuffer, 
                           vertexSource, assetName, numRetries, *refPtr);
    };
    frag->instance->readFile(fragmentShader, callbackLambda);
  }
  else
  {
    if (numRetries > 0)
    {
      // Reattempt the request
      --numRetries;
      requestVSandFS(core, assetName, numRetries);
    }
    else
    {
      std::cerr << "ShaderMan: Failed promise for " << assetName << std::endl;
      std::cerr << "Failed on vertex shader. Number of retries exceeded." << std::endl;
    }
  }
}

void ShaderMan::loadFragmentShaderCB(const std::string& /* fsName */, bool error,
                                     size_t bytesRead, uint8_t* buffer,
                                     std::string vertexSource, std::string assetName,
                                     int32_t numRetries, CPM_ES_NS::ESCoreBase& core)
{
  // We have both the vertex shader and fragment shader source. Proceed
  // to compile the shader and add the appropriate component to the indicated
  // entityID.
  if (!error)
  {
    char* fragmentSourceRaw = static_cast<char*>(std::malloc(bytesRead + 1));
    std::memcpy(fragmentSourceRaw , buffer, bytesRead);
    fragmentSourceRaw[bytesRead] = '\0';

    std::string fragmentSource = fragmentSourceRaw;

    std::free(fragmentSourceRaw);

    GLuint program = 0;
    if (mShaderHeader.size() > 0)
    {
      program = CPM_GL_SHADERS_NS::loadShaderProgram(
        {
          CPM_GL_SHADERS_NS::ShaderSource({mShaderHeader.c_str(), vertexSource.c_str()}, GL_VERTEX_SHADER),
          CPM_GL_SHADERS_NS::ShaderSource({mShaderHeader.c_str(), fragmentSource.c_str()}, GL_FRAGMENT_SHADER)
        });
    }
    else
    {
      program = CPM_GL_SHADERS_NS::loadShaderProgram(
        {
          CPM_GL_SHADERS_NS::ShaderSource({vertexSource.c_str()}, GL_VERTEX_SHADER),
          CPM_GL_SHADERS_NS::ShaderSource({fragmentSource.c_str()}, GL_FRAGMENT_SHADER)
        });
    }

    // Simply update mGLToName and mNameToGL. The fulfillment system will
    // handle everything else.
    mGLToName.insert(std::make_pair(program, assetName));
    mNameToGL.insert(std::make_pair(assetName, program));

    // Set new unfulfilled assets flag so we know if a GC cycle will remove
    // valid assets. We can use this to disable a GC cycle if it will have
    // unintended side-effects.
    mNewUnfulfilledAssets = true;
  }
  else
  {
    if (numRetries > 0)
    {
      // Reattempt the request
      --numRetries;
      requestVSandFS(core, assetName, numRetries);
    }
    else
    {
      std::cerr << "ShaderMan: Failed promise for " << assetName << std::endl;
      std::cerr << "Failed on fragment shader. Number of retries exceeded." << std::endl;
    }
  }
}

GLuint ShaderMan::getIDForAsset(const char* assetName) const
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

std::string ShaderMan::getAssetFromID(GLuint id) const
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

/// Returns false if we failed to generate the component because the asset
/// has not been loaded yet.
bool ShaderMan::buildComponent(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                               const std::string& assetName)
{
  GLuint id = getIDForAsset(assetName.c_str());
  if (id != 0)
  {
    // Go ahead and add a new component for the entityID. If this is the
    // last promise to fulfill, then systems should automatically start
    // rendering the data.
    Shader component;
    component.glid = id;
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

class ShaderPromiseVFFulfillment :
    public es::GenericSystem<true,
                             ShaderPromiseVF,
                             StaticShaderMan>
{
public:

  static const char* getName() {return "ren:ShaderPromiseVFFulfillment";}

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
    std::weak_ptr<ShaderMan> sm = core.getStaticComponent<StaticShaderMan>()->instance_;
    if (std::shared_ptr<ShaderMan> man = sm.lock()) {
        man->mNewUnfulfilledAssets = false;

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
            man->requestVSandFS(core, asset, man->mNumRetries);
          }
        }
    } else {
        std::cerr << "Unable to complete shader fulfillment. There is no ShaderMan." << std::endl;
    }
  }

  void groupExecute(es::ESCoreBase& core, uint64_t entityID,
               const es::ComponentGroup<ShaderPromiseVF>& promisesGroup,
               const es::ComponentGroup<StaticShaderMan>& shaderManGroup) override
  {
    std::weak_ptr<ShaderMan> sm = shaderManGroup.front().instance_;
    if (std::shared_ptr<ShaderMan> shaderMan = sm.lock()) {
        CPM_ES_CEREAL_NS::CerealCore* ourCorePtr = dynamic_cast<CPM_ES_CEREAL_NS::CerealCore*>(&core);
        if (ourCorePtr == nullptr)
        {
          std::cerr << "Unable to execute shader promise fulfillment. Bad cast." << std::endl;
          return;
        }
        CPM_ES_CEREAL_NS::CerealCore& ourCore = *ourCorePtr;

        int index = 0;
        for (const ShaderPromiseVF& p : promisesGroup)
        {
          // Check to see if this promise has been fulfilled. If it has, then
          // remove it and create the appropriate component for the indicated
          // entity.
          if (shaderMan->buildComponent(ourCore, entityID, p.assetName))
          {
            // Remove this promise, and add a shader component to this promises'
            // entityID. It is safe to remove components while we are using a
            // system - addition / removal / modification doesn't happen until
            // a renormalization step.
            ourCore.removeComponentAtIndexT<ShaderPromiseVF>(entityID, index);
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
              ShaderPromiseVF newPromise = p;
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

const char* ShaderMan::getPromiseVFFulfillmentName()
{
  return ShaderPromiseVFFulfillment::getName();
}

//------------------------------------------------------------------------------
// GARBAGE COLLECTION
//------------------------------------------------------------------------------

void ShaderMan::runGCAgainstVaidIDs(const std::set<GLuint>& validKeys)
{
  if (mNewUnfulfilledAssets)
  {
    std::cerr << "ShaderMan: Terminating garbage collection. Orphan assets that"
              << " have yet to be associated with entity ID's would be GC'd" << std::endl;
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

      std::cout << "Shader GC: " << it->second << std::endl;

      // Erase our iterator and move on. Ensure we delete the program.
      GLuint idToErase = it->first;
      mGLToName.erase(it++);
      GL(glDeleteShader(idToErase));
    }

    if (it == mGLToName.end())
    {
      std::cerr << "runGCAgainstVaidIDs: terminating early, validKeys contains "
                << "elements not in Shader map." << std::endl;
      break;
    }

    // Check to see if the valid ids contain a component that is not in
    // mGLToName. If an object manages its own shader, but still uses the shader
    // component, this is not an error.
    if (it->first > id)
    {
      std::cerr << "runGCAgainstVaidIDs: validKeys contains elements not in the Shader map." << std::endl;
    }

    ++it;
  }

  // Terminate any remaining assets.
  while (it != mGLToName.end())
  {
    // Find the asset name in mNameToGL and erase.
    mNameToGL.erase(mNameToGL.find(it->second));

    std::cout << "Shader GC: " << it->second << std::endl;

    // Erase our iterator and move on. Ensure we delete the program.
    GLuint idToErase = it->first;
    mGLToName.erase(it++);
    GL(glDeleteShader(idToErase));
  }
}

class ShaderGarbageCollector :
    public es::GenericSystem<false, Shader>
{
public:

  static const char* getName() {return "ren:ShaderGarbageCollector";}

  std::set<GLuint> mValidKeys;

  void preWalkComponents(es::ESCoreBase&) {mValidKeys.clear();}

  void postWalkComponents(es::ESCoreBase& core)
  {
    std::weak_ptr<ShaderMan> sm = core.getStaticComponent<StaticShaderMan>()->instance_;
    if (std::shared_ptr<ShaderMan> man = sm.lock()) {
        man->runGCAgainstVaidIDs(mValidKeys);
        mValidKeys.clear();
    } else {
        std::cerr << "Unable to complete shader garbage collection. " <<
                     "There is no ShaderMan." << std::endl;

    }
  }

  void execute(es::ESCoreBase&, uint64_t /* entityID */, const Shader* shader) override
  {
    mValidKeys.insert(shader->glid);
  }
};

const char* ShaderMan::getGCName()
{
  return ShaderGarbageCollector::getName();
}

void ShaderMan::registerSystems(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<ShaderPromiseVFFulfillment>();
  core.registerSystem<ShaderGarbageCollector>();
}

void ShaderMan::runGCCycle(CPM_ES_NS::ESCoreBase& core)
{
  ShaderGarbageCollector gc;
  gc.walkComponents(core);
}

} // namespace ren


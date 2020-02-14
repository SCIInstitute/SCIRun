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


#ifndef SPIRE_RENDER_SHADERMAN_HPP
#define SPIRE_RENDER_SHADERMAN_HPP

#include <es-log/trace-log.h>
#include <map>
#include <set>
#include <es-cereal/CerealCore.hpp>
#include <entity-system/BaseSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace ren {

class ShaderGarbageCollector;
class ShaderPromiseVFFulfillment;

/// Basic shader manager.
class SCISHARE ShaderMan
{
public:
  /// \param  numRetries  The number of retries we have to load the asset.
  ///                     Zombie promises will remain present in the system
  ///                     and a load will be re-attempted again when
  ///                     serialized and deserialized.
  explicit ShaderMan(int numRetries = 2);
  ~ShaderMan();

  static void setShaderHeaderCode(const std::string& header);

  /// Loads a vertex and fragment shader given an asset name. This load does not
  /// happen immediately and is performed asynchronously. ShaderPromiseFV is
  /// used to keep track of the promises and add the shader component when
  /// it is loaded. If the asset is already loaded, then the shader component
  /// will be applied the next time the promise fulfillment system is run.
  /// The vertex and fragment shader *must* conform to the following
  /// naming convention: assetName + ".vs" and assetName + ".fs"
  /// \param  core        Core base.
  /// \param  entityID    Entity ID which will receive the ren::Shader component.
  /// \param  assetName   Name of the asset which to load.
  void loadVertexAndFragmentShader(spire::CerealCore& core,
                                   uint64_t entityID, const std::string& assetName);

  /// Add in-memory vertex and fragment shader. Associates the assets with
  /// 'assetName' and returns the programID.
  GLuint addInMemoryVSFS(const std::string& vsSource, const std::string& fsSource,
                         const std::string& assetName);

  /// Runs a single garbage collection cycle on the current state of the core.
  void runGCCycle(spire::ESCoreBase& core);

  /// Returns the GLID for the given assetName, if one has been generated.
  /// Returns 0 if the shader is not found in the system.
  GLuint getIDForAsset(const char* assetName) const;

  /// Retrieve the asset name from the GLuint.
  std::string getAssetFromID(GLuint id) const;

  /// Registers ShaderMan's systems. Both the garbage collector and the promise
  /// fullfillment system are registered.
  static void registerSystems(spire::Acorn& core);

  /// Obtains the garbage collectors name so that you can setup intermitent
  /// garbage collection cycles.
  static const char* getGCName();

  /// Obtains the shader promise fullfilment system's name. This system should
  /// be installed in every core. You shouldn't need to run it every frame.
  /// Maybe about every 200 MS.
  static const char* getPromiseVFFulfillmentName();

private:
  friend class ShaderGarbageCollector;
  friend class ShaderPromiseVFFulfillment;

  bool buildComponent(spire::CerealCore& core, uint64_t entityID,
                      const std::string& assetName);

  void runGCAgainstVaidIDs(const std::set<GLuint>& validKeys);

  /// Issues a request for a vertex shader and fragment shader.
  void requestVSandFS(spire::ESCoreBase& core, const std::string& assetName,
                      int32_t numRetries);

  /// Callback issued to load vertex shader. Think of a better format to use
  /// for storing shaders.
  void loadVertexShaderCB(const std::string& vsName, bool error,
                          size_t bytesRead, uint8_t* buffer,
                          std::string assetName, int32_t numRetries,
                          spire::ESCoreBase& core);

  /// Callback issued to load fragment shader.
  void loadFragmentShaderCB(const std::string& fsName, bool error,
                            size_t bytesRead, uint8_t* buffer,
                            std::string vertexShaderSource,
                            std::string assetName, int32_t numRetries,
                            spire::ESCoreBase& core);

  /// Map from GL id to asset name.
  std::map<GLuint, std::string> mGLToName;

  /// Map from asset name to GL id.
  std::map<std::string, GLuint> mNameToGL;

  /// Indicates whether new assets have been loaded but a promise fulfillment
  /// run has not been made. A GC cycle when this is true will lead to
  /// incorrect results. Hence, if a GC cycle is requested, a warning will
  /// be issued and the GC cycle will be aborted.
  bool mNewUnfulfilledAssets;

  /// Number of retries to attempt before resulting in promise failure.
  int32_t mNumRetries;

  // Shader header.
  static std::string mShaderHeader;
};

} // namespace ren

#endif

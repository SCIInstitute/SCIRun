#ifndef SPIRE_RENDER_IBOMAN_HPP
#define SPIRE_RENDER_IBOMAN_HPP

#include <es-log/trace-log.h>
#include <set>
#include <map>
#include <gl-platform/GLPlatform.hpp>
#include <gl-shaders/GLShader.hpp>
#include <entity-system/BaseSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include "comp/AssetPromise.hpp"
#include <spire/scishare.h>

namespace ren {

class IBOGarbageCollector;

class IBOMan
{
public:
  IBOMan();
  virtual ~IBOMan();

  /// Adds a IBO whose data is already in memory. Returns the GLid of the
  /// generated IBO.
  GLuint addInMemoryIBO(void* iboData, size_t iboDataSize, GLenum primMode,
                        GLenum primType, GLsizei numPrims,
                        const std::string& assetName);

  void removeInMemoryIBO(GLuint glid);

  /// Returns glid if \p assetName is in the VBO man. Returns 0 otherwise.
  /// Use sparingly.
  GLuint hasIBO(const std::string& assetName) const;

  // Use this structure if you are storing IBO data for later retrieval.
  struct MinIBOData
  {
    GLenum primMode;
    GLenum primType;
    GLsizei numPrims;
  };

  struct IBOData
  {
    IBOData(const std::string& name, GLenum pmode, GLenum ptype, GLsizei nprims) :
        assetName(name),
        primMode(pmode),
        primType(ptype),
        numPrims(nprims)
    {}

    std::string assetName;
    GLenum primMode;
    GLenum primType;
    GLsizei numPrims;
  };

  /// Retrieves IBO Data structure. An exception is thrown if the structure
  /// does not exist.
  const IBOData& getIBOData(const std::string& assetName) const;

  /// Runs a single garbage collection cycle against the given ESCoreBase.
  /// This does not add an active system. It simply runs the garbage collection
  /// cycle and removes all shaders no longer in use. If this was a system
  /// that had promises, you would run promises before running a GC cycle
  /// since you don't want GC to remove useful shaders.
  void runGCCycle(spire::ESCoreBase& core);

  /// Retrieves the GC's name. You can use this in conjunction with
  /// SystemCore to setup an intermitent GC cycle.
  static const char* getGCName();

  /// Registers IBO managers systems. In this case, just the GC system.
  /// Other managers also have a promise system as well, which will fufill
  /// promises made to entities when assets are loaded from disk.
  static void registerSystems(spire::Acorn& core);

private:
  friend class IBOGarbageCollector;

  /// Runs garbage collection against a set of valid keys. All OpenGL ids not
  /// in validKeys will be removed from the system.
  void runGCAgainstVaidIDs(const std::set<GLuint>& validKeys);

  std::map<GLuint, IBOData>      mIBOData;
};

} // namespace ren

#endif

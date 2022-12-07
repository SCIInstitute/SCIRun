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

class SCISHARE IBOMan
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

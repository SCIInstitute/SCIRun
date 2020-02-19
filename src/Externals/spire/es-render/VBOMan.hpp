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


#ifndef SPIRE_RENDER_VBOMAN_HPP
#define SPIRE_RENDER_VBOMAN_HPP

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

class VBOGarbageCollector;

class SCISHARE VBOMan
{
public:
  VBOMan();
  virtual ~VBOMan();

  //struct MinimalAttribData
  //{
  //  MinimalAttribData(const std::tuple<std::string, size_t, bool>& data) :
  //      mName(std::get<0>(data)),
  //      mSizeBytes(std::get<1>(data)),
  //      mNormalize(std::get<2>(data))
  //  {}

  //  std::string mName;
  //  size_t      mSizeBytes;
  //  bool        mNormalize;
  //};

  /// Adds a new VBO. This class assumes responsibility for cleaning up the
  /// GL id that is passed in, and stores the list of VBO attributes in a
  /// sorted map. If you need more control over the VBO lifecycle, by all
  /// means handle the lifecycle outside of the influence of this class.
  /// This class is only meant to manage instancing of VBO attributes.
  /// Look into moving the std::vector if performance ever becomes an issue.
  /// Tuple corresponds to:
  ///   <0> - Attribute name
  ///   <1> - Attribute size in bytes (for 1 element).
  ///   <2> - True if data should be normalized.
  void addVBOAttributes(GLuint glid, const std::vector<std::tuple<std::string, size_t, bool>>& attribs,
                        const std::string& assetName = "");

  /// Adds a VBO whose data is already in memory. Returns the GLid of the
  /// generated VBO.
  GLuint addInMemoryVBO(void* vboData, size_t vboDataSize,
                        const std::vector<std::tuple<std::string, size_t, bool>>& attribs,
                        const std::string& assetName);

  /// Returns a list of sorted VBO attributes, based on glid.
  const std::vector<spire::ShaderAttribute>& getVBOAttributes(GLuint glid) const;

  /// Returns true if \p assetName is in the VBO man. Use sparingly.
  GLuint hasVBO(const std::string& assetName) const;

  /// Runs a single garbage collection cycle against the given ESCoreBase.
  /// This does not add an active system. It simply runs the garbage collection
  /// cycle and removes all shaders no longer in use. If this was a system
  /// that had promises, you would run promises before running a GC cycle
  /// since you don't want GC to remove useful shaders.
  void runGCCycle(spire::ESCoreBase& core);

  /// Retrieves the GC's name. You can use this in conjunction with
  /// SystemCore to setup an intermitent GC cycle.
  static const char* getGCName();

  /// Registers VBO managers systems. In this case, just the GC system.
  /// Other managers also have a promise system as well, which will fufill
  /// promises made to entities when assets are loaded from disk.
  static void registerSystems(spire::Acorn& core);

private:
  friend class VBOGarbageCollector;

  // Runs garbage collection against a set of valid keys. All OpenGL ids not
  // in validKeys will be removed from the system.
  void runGCAgainstVaidIDs(const std::set<GLuint>& validKeys);

  struct VBOData
  {
    VBOData(const std::vector<std::tuple<std::string, size_t, bool>>& attribs,
            const char* name = nullptr)
    {
      if (name != nullptr)
        assetName = name;

      spire::ShaderAttribute attrib;
      for (auto item : attribs)
      {
        attrib.nameInCode = std::get<0>(item);
        attrib.sizeBytes = std::get<1>(item);
        if (std::get<2>(item))
          attrib.normalize = GL_TRUE;
        else
          attrib.normalize = GL_FALSE;

        attributes.push_back(attrib);
      }
    }

    std::vector<spire::ShaderAttribute> attributes;
    std::string assetName;    ///< Asset name if VBO originated from an asset.
  };

  std::map<GLuint, VBOData>      mVBOData;
};

} // namespace ren

#endif

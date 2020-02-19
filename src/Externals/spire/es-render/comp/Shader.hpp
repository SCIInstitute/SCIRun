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


#ifndef SPIRE_COMPONENT_RENDER_SHADER_HPP
#define SPIRE_COMPONENT_RENDER_SHADER_HPP

#include <es-log/trace-log.h>
#include <gl-platform/GLPlatform.hpp>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-cereal/CerealCore.hpp>
#include "ShaderPromiseVF.hpp"
#include "StaticShaderMan.hpp"
#include <spire/scishare.h>

namespace ren {

struct Shader
{
  // -- Data --
  GLuint glid;     // glid associated with shader program.

  // -- Functions --
  static const char* getName() {return "ren:shader";}

  bool serialize(spire::ComponentSerialize& s, uint64_t entityID)
  {
    // The logic below ensures we deserialize with promises, not with actual
    // shader assets (which wouldn't make sense with OpenGL assets).
    if (s.isDeserializing())
    {
      std::string assetName;
      s.serialize("name", assetName);

      // Build shader promise.
      ShaderPromiseVF newPromise;
      newPromise.requestInitiated = false;
      newPromise.setAssetName(assetName.c_str());

      spire::CerealCore& core
          = dynamic_cast<spire::CerealCore&>(s.getCore());
      core.addComponent(entityID, newPromise);

      return false; // We do not want to add this shader component back into the components.
                    // Instead we rely on the shader promise we created above.
    } else {
      spire::CerealCore& core
          = dynamic_cast<spire::CerealCore&>(s.getCore());
      std::weak_ptr<ShaderMan> sm = core.getStaticComponent<StaticShaderMan>()->instance_;
      if (std::shared_ptr<ShaderMan> shaderMan = sm.lock()) {
          // Find the asset name associated with our glid and serialize it out.
          std::string assetName = shaderMan->getAssetFromID(glid);
          s.serialize("name", assetName);
          return true;
      }
      return false;
    }
  }
};

} // namespace ren

#endif

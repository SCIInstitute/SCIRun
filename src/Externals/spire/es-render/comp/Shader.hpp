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

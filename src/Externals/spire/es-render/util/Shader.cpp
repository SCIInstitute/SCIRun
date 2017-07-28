#include "Shader.hpp"
#include "es-render/comp/StaticShaderMan.hpp"

namespace ren {

void addShaderVSFS(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                   const std::string& shader)
{
  std::weak_ptr<ren::ShaderMan> sm = core.getStaticComponent<ren::StaticShaderMan>()->instance_;
  if (std::shared_ptr<ShaderMan> shaderMan = sm.lock())
    shaderMan->loadVertexAndFragmentShader(core, entityID, shader);
}

} // namespace ren


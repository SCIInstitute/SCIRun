#include "Registration.hpp"

// Components
#include "comp/CommonUniforms.hpp"
#include "comp/GLState.hpp"
#include "comp/IBO.hpp"
#include "comp/MatUniform.hpp"
#include "comp/RenderSequence.hpp"
#include "comp/Shader.hpp"
#include "comp/ShaderPromiseVF.hpp"
#include "comp/RenderSimpleGeom.hpp"
#include "comp/SkinnedGeom.hpp"
#include "comp/StaticArcBallCam.hpp"
#include "comp/StaticGLState.hpp"
#include "comp/StaticShaderMan.hpp"
#include "comp/StaticVBOMan.hpp"
#include "comp/StaticGeomMan.hpp"
#include "comp/StaticTextureMan.hpp"
#include "comp/StaticFontMan.hpp"
#include "comp/StaticIBOMan.hpp"
#include "comp/GeomPromise.hpp"
#include "comp/Geom.hpp"
#include "comp/Font.hpp"
#include "comp/FontPromise.hpp"
#include "comp/Texture.hpp"
#include "comp/TexturePromise.hpp"
#include "comp/UniformLocation.hpp"
#include "comp/VBO.hpp"
#include "comp/VecUniform.hpp"
#include "comp/RenderFont.hpp"

#include "comp/UtilViewPosAlign.hpp"

namespace ren {

  void register1(CPM_ES_ACORN_NS::Acorn& core)
  {
    core.registerComponent<CommonUniforms>();
    core.registerComponent<GLState>();
    core.registerComponent<IBO>();
    core.registerComponent<MatUniform>();
    core.registerComponent<RenderSequence>();
    core.registerComponent<Shader>();
    core.registerComponent<ShaderPromiseVF>();
    core.registerComponent<RenderSimpleGeom>();
    core.registerComponent<SkinnedGeom>();
  }

} // namespace ren


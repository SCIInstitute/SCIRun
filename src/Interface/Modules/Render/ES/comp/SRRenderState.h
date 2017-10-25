#ifndef INTERFACE_MODULES_RENDER_ES_COMP_SR_RENDER_STATE_H
#define INTERFACE_MODULES_RENDER_ES_COMP_SR_RENDER_STATE_H

#include <gl-shaders/GLShader.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-render/util/Shader.hpp>
#include <es-render/comp/StaticVBOMan.hpp>
#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Core/Datatypes/Geometry.h>

namespace SCIRun {
namespace Render {

/// \todo Transition this class to use the template ShaderVBOAttribs class
///       under utils (utils/Shader.hpp).
struct SRRenderState
{
  // -- Data --
  RenderState state;

  // -- Functions --
  SRRenderState() {}

  static const char* getName() {return "SRRenderState";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Shouldn't need to serialize these values. They are context specific.
    // Maybe? Will need to figure out as I go along.
    return true;
  }
};

} // namespace Render
} // namespace SCIRun

#endif 

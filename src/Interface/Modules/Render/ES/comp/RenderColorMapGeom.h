#ifndef INTERFACE_MODULES_RENDER_ES_COMP_RENDER_COLOR_MAP_GEOM_H
#define INTERFACE_MODULES_RENDER_ES_COMP_RENDER_COLOR_MAP_GEOM_H

#include <gl-shaders/GLShader.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-render/util/Shader.hpp>
#include <es-render/comp/StaticVBOMan.hpp>

namespace SCIRun {
namespace Render {

struct RenderColorMapGeom
{
  // -- Data --
  static const int MaxNumAttributes = 5;
  ren::ShaderVBOAttribs<MaxNumAttributes> attribs;

  // -- Functions --
  RenderColorMapGeom() {}

  static const char* getName() {return "RenderColorMapGeom";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Shouldn't need to serialize these values. They are context specific.
    // Maybe? Will need to figure out as I go along.
    return true;
  }
};

} // namespace Render
} // namespace SCIRun

#endif 

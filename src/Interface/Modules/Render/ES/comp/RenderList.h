#ifndef INTERFACE_MODULES_RENDER_ES_COMP_RENDER_LIST_H
#define INTERFACE_MODULES_RENDER_ES_COMP_RENDER_LIST_H

#include <gl-shaders/GLShader.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-render/util/Shader.hpp>
#include <es-render/comp/StaticVBOMan.hpp>
#include <Core/Datatypes/Geometry.h>

namespace SCIRun {
namespace Render {

struct RenderList
{
  // -- Data --
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> data;
  std::vector<Core::Datatypes::GeometryImpl::SpireVBO::AttributeData> attributes;
  Core::Datatypes::GeometryImpl::RenderType renderType;
  int64_t numElements;

  // -- Functions --
  RenderList() {}

  static const char* getName() {return "RenderList";}

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

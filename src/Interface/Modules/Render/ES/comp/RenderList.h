#ifndef INTERFACE_MODULES_RENDER_ES_COMP_RENDER_LIST_H
#define INTERFACE_MODULES_RENDER_ES_COMP_RENDER_LIST_H

#include <es-cereal/ComponentSerialize.hpp>
#include <Graphics/Datatypes/GeometryImpl.h>

namespace SCIRun {
namespace Render {

struct RenderList
{
  // -- Data --
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> data;
  std::vector<Graphics::Datatypes::SpireVBO::AttributeData> attributes;
  Graphics::Datatypes::RenderType renderType;
  int64_t numElements;

  // -- Functions --
  RenderList() : renderType(Graphics::Datatypes::RenderType::RENDER_VBO_IBO), numElements(0) {}

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

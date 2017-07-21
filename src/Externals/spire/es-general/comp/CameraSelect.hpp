#ifndef IAUNS_RENDER_COMPONENT_CAMERA_SELECT_HPP
#define IAUNS_RENDER_COMPONENT_CAMERA_SELECT_HPP

#include <es-cereal/ComponentSerialize.hpp>

namespace gen {

struct CameraSelect
{
  enum Selection
  {
    PERSPECTIVE_CAMERA,
    ORTHOGONAL_CAMERA
  };

  // -- Data --
  Selection cam;

  // -- Functions --
  CameraSelect()
  {
    cam = PERSPECTIVE_CAMERA;
  }

  static const char* getName() {return "gen:CameraSelect";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    /// Nothing needs to be serialized. This is context specific.
    int32_t camValue = static_cast<int32_t>(cam);
    if (s.isDeserializing())
    {
      s.serialize("cam", camValue);
      cam = static_cast<Selection>(camValue);
    }
    else
    {
      s.serialize("cam", camValue);
    }
    return true;
  }
};

} // namespace gen

#endif 

#ifndef SPIRE_RENDER_COMPONENT_CAMERA_SELECT_HPP
#define SPIRE_RENDER_COMPONENT_CAMERA_SELECT_HPP

#include <es-log/trace-log.h>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

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

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
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

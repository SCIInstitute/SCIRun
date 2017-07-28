#ifndef IAUNS_RENDER_COMPONENTS_STATIC_ARC_BALL_CAMERA_HPP
#define IAUNS_RENDER_COMPONENTS_STATIC_ARC_BALL_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <arc-look-at/ArcLookAt.hpp>

namespace ren {

struct StaticArcBallCam
{
  enum CamerasAffected
  {
    PERSPECTIVE,
    ORTHO,
    BOTH
  };

  // -- Data --
  CamerasAffected           cams;
  CPM_LOOK_AT_NS::ArcLookAt arcLookAt;

  // -- Functions --
  StaticArcBallCam()
  {
    cams = PERSPECTIVE;
  }

  StaticArcBallCam(const StaticArcBallCam& other)
  {
    cams = other.cams;
  }

  StaticArcBallCam& operator=(const StaticArcBallCam& other)
  {
    // We don't care about the incoming object. We've already created oun own
    // shader man and will continue to use that.
    cams = other.cams;
    return *this;
  }

  static const char* getName() {return "ren:StaticArcBallCam";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize camera settings.
    return true;
  }
};

} // namespace ren

#endif 

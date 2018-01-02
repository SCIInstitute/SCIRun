#ifndef SPIRE_RENDER_ARC_BALL_CAMERA_HPP
#define SPIRE_RENDER_ARC_BALL_CAMERA_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace ren {

// Mouse based arc ball camera.
void        registerSystem_ArcBallCameraMouse(spire::Acorn& core);
const char* getSystemName_ArcBallCameraMouse();

// Todo: Touch based arc ball camera.

} // namespace ren

#endif

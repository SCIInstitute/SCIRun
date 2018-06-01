#ifndef SPIRE_ES_RENDER_UTIL_CAMERA_HPP
#define SPIRE_ES_RENDER_UTIL_CAMERA_HPP

#include <es-log/trace-log.h>
#include <es-cereal/CerealCore.hpp>
#include <es-general/comp/CameraSelect.hpp>
#include <spire/scishare.h>

namespace ren {

/// Selects a camera for the object. Either perspective or orthogonal.
/// If newComponent is true, then a new component is created without checking
/// for the existence of a pre-existing component (more efficient if you
/// know there won't be a component of this kind already).
/// \todo Create a function that adds a new component if it doesn't already
///       exist? Is this needed for anything?
void selectCamera(spire::CerealCore& core, uint64_t entityID,
                  gen::CameraSelect::Selection sel, bool newComponent = false);

} // namespace ren

#endif

#ifndef IAUNS_ES_GENERAL_COMP_CLICKBOX2D_HPP
#define IAUNS_ES_GENERAL_COMP_CLICKBOX2D_HPP

#include <cstdint>
#include <entity-system/GenericSystem.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-cereal/CerealCore.hpp>
#include "../AABB2D.hpp"

namespace gen {

// Note: Transform DOES affect the click box.
// Note: This box will detect both touch and mouse input.
// Note: The selected camera is treated as an orthographic camera with Z
//       into the screen and x to the left and y up.
// Note: Rotation about the z axis is not taken into account at this time.
//       It could be done if necessary, however.
struct ClickBox2D
{
  // -- Data --

  // Data related to the hitbox.
  AABB2D hitBox;
  bool hit;

  // User ID
  int64_t userID;

  // -- Functions --
  ClickBox2D()
  {
    hit = false;
    userID = 0;
  }

  // Required functions (CPM_ES_CEREAL_NS::CerealHeap)
  static const char* getName() {return "gp:ClickBox2D";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    /// \todo Serialize ClickBox2D
    return true;
  }
};


} // namespace gen

#endif 

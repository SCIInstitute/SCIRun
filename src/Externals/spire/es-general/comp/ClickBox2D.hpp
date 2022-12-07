/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


#ifndef SPIRE_ES_GENERAL_COMP_CLICKBOX2D_HPP
#define SPIRE_ES_GENERAL_COMP_CLICKBOX2D_HPP

#include <es-log/trace-log.h>
#include <entity-system/GenericSystem.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-cereal/CerealCore.hpp>
#include "../AABB2D.hpp"
#include <spire/scishare.h>

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
  bool hit {false};

  // User ID
  int64_t userID {0};

  // Required functions (spire::CerealHeap)
  static const char* getName() {return "gp:ClickBox2D";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    /// \todo Serialize ClickBox2D
    return true;
  }
};


} // namespace gen

#endif

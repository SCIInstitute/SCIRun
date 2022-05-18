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

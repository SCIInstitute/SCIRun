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

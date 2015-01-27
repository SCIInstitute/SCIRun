/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <gl-platform/GLPlatform.hpp>
#include "Registration.h"
#include "CoreBootstrap.h"
#include "AssetBootstrap.h"
#include "comp/StaticSRInterface.h"
#include "comp/RenderBasicGeom.h"
#include "comp/RenderColorMapGeom.h"
#include "comp/SRRenderState.h"
#include "comp/RenderList.h"
#include "comp/StaticWorldLight.h"
#include "comp/LightingUniforms.h"
#include "systems/RenderBasicSys.h"
#include "systems/RenderColorMapSys.h"
#include "systems/RenderTransBasicSys.h"
#include "systems/RenderTransColorMapSys.h"
#include "Core/Datatypes/Geometry.h"

namespace SCIRun {
namespace Render {

void rendererRegisterAll(CPM_ES_ACORN_NS::Acorn& core)
{
  // Register systems
  registerSystem_CoreBootstrap(core);
  registerSystem_AssetBootstrap(core);
  registerSystem_RenderBasicGeom(core);
  registerSystem_RenderColorMap(core);
  registerSystem_RenderBasicTransGeom(core);
  registerSystem_RenderColorMapTrans(core);

  // Register components
  core.registerComponent<StaticSRInterface>();
  core.registerComponent<StaticWorldLight>();
  core.registerComponent<LightingUniforms>();
	core.registerComponent<RenderBasicGeom>();
  core.registerComponent<RenderColorMapGeom>();
  core.registerComponent<SRRenderState>();
  core.registerComponent<RenderList>();
  //core.registerComponent<SpireSubPass>();
}

} // namespace Render
} // namespace SCIRun


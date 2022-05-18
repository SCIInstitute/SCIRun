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


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include "Registration.hpp"

// Components
#include "comp/CommonUniforms.hpp"
#include "comp/GLState.hpp"
#include "comp/IBO.hpp"
#include "comp/MatUniform.hpp"
#include "comp/RenderSequence.hpp"
#include "comp/Shader.hpp"
#include "comp/ShaderPromiseVF.hpp"
#include "comp/RenderSimpleGeom.hpp"
#include "comp/SkinnedGeom.hpp"
#include "comp/StaticArcBallCam.hpp"
#include "comp/StaticGLState.hpp"
#include "comp/StaticShaderMan.hpp"
#include "comp/StaticVBOMan.hpp"
#include "comp/StaticGeomMan.hpp"
#include "comp/StaticTextureMan.hpp"
#include "comp/StaticFontMan.hpp"
#include "comp/StaticIBOMan.hpp"
#include "comp/StaticFBOMan.hpp"
#include "comp/GeomPromise.hpp"
#include "comp/Geom.hpp"
#include "comp/Font.hpp"
#include "comp/FontPromise.hpp"
#include "comp/Texture.hpp"
#include "comp/TexturePromise.hpp"
#include "comp/UniformLocation.hpp"
#include "comp/VBO.hpp"
#include "comp/VecUniform.hpp"
#include "comp/RenderFont.hpp"

#include "comp/UtilViewPosAlign.hpp"

namespace ren {

  void register2(spire::Acorn& core)
  {
    core.registerComponent<StaticArcBallCam>();
    core.registerComponent<StaticGLState>();
    core.registerComponent<StaticShaderMan>();
    core.registerComponent<StaticGeomMan>();
    core.registerComponent<StaticTextureMan>();
    core.registerComponent<StaticVBOMan>();
    core.registerComponent<StaticIBOMan>();
	core.registerComponent<StaticFBOMan>();
    core.registerComponent<StaticFontMan>();
  }

} // namespace ren

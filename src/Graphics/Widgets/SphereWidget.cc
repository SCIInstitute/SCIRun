/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Color.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

std::string AbstractGlyphFactory::sphere(SphereParameters params, WidgetBase& widget)
{
  if (params.common.scale < 0) params.common.scale = 1.;
  if (params.common.resolution < 0) params.common.resolution = 10;

  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  ss << params.common.scale << params.common.resolution << static_cast<int>(colorScheme);

  auto name = widget.uniqueID() + "widget" + ss.str();

  Graphics::GlyphGeom glyphs;
  ColorRGB node_color;
  glyphs.addSphere(params.point, params.common.scale, params.common.resolution, node_color);

  auto renState = getSphereRenderState(params.common.defaultColor);

  glyphs.buildObject(widget, name, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, params.common.bbox);

  return name;
}

SphereWidget::SphereWidget(const GeneralWidgetParameters& gen,
                           SphereParameters params)
  : WidgetBase({gen.base.idGenerator, "SphereWidget::" + gen.base.tag})//, point, origin)
{
  name_ = gen.glyphMaker->sphere(params, *this);
  setPosition(params.point);
}

RenderState AbstractGlyphFactory::getSphereRenderState(const std::string& defaultColor) const
{
  RenderState renState;

  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(defaultColor);
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);

  return renState;
}

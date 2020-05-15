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


#include <Modules/Visualization/ShowColorMapModule.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Color.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Graphics/Datatypes/GeometryImpl.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(ShowColorMap, Visualization, SCIRun)

ShowColorMap::ShowColorMap() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(GeometryOutput);
}

void ShowColorMap::setStateDefaults()
{
  auto state = get_state();
  state->setValue(DisplaySide, 0);
  state->setValue(DisplayLength, 0);
  state->setValue(TextSize, 8);
  state->setValue(TextRed, 1.);
  state->setValue(TextGreen, 1.);
  state->setValue(TextBlue, 1.);
  state->setValue(Labels, 10);
  state->setValue(Scale, 1.0);
  state->setValue(Units, std::string(""));
  state->setValue(SignificantDigits, 2);
  state->setValue(AddExtraSpace, false);
  state->setValue(XTranslation, 0);
  state->setValue(YTranslation, 0);
  state->setValue(ColorMapName, std::string(""));
}

void ShowColorMap::execute()
{
  auto colorMap = getRequiredInput(ColorMapObject);
  if (needToExecute())
  {
    std::ostringstream ostr;
    ostr << id() << "$" <<
      colorMap->getColorMapInvert() << colorMap->getColorMapName() << colorMap->getColorMapRescaleScale() <<
      colorMap->getColorMapRescaleShift() << colorMap->getColorMapResolution() << colorMap.get() <<
      colorMap->getColorMapShift();
    auto geom = buildGeometryObject(colorMap, get_state(), ostr.str());
    sendOutput(GeometryOutput, geom);
  }
}

GeometryBaseHandle ShowColorMap::buildGeometryObject(ColorMapHandle cm, ModuleStateHandle state, const std::string& geomId)
{
  std::vector<Vector> points;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;
  int32_t numVBOElements = 0;
  double resolution = 1. / static_cast<double>(cm->getColorMapResolution());
  //show colormap does not rescale colors, so reset them. we want to see the whole colormap on the scale.
  ColorMap new_map(cm->getColorData(), cm->getColorMapName(), cm->getColorMapResolution(),
    cm->getColorMapShift(), cm->getColorMapInvert(), 1., 0.);
  for (double i = 0.; std::abs(i - 1.) > 0.000001; i += resolution) {
    ColorRGB col = new_map.valueToColor(i);
    uint32_t offset = static_cast<uint32_t>(points.size());
    points.push_back(Vector(0., i, 0.001));
    colors.push_back(col);
    points.push_back(Vector(1., i, 0.001));
    colors.push_back(col);
    points.push_back(Vector(0., i + resolution, 0.001));
    colors.push_back(col);
    points.push_back(Vector(1., i + resolution, 0.001));
    colors.push_back(col);
    numVBOElements += 2;
    indices.push_back(offset + 0);
    indices.push_back(offset + 1);
    indices.push_back(offset + 3);
    indices.push_back(offset + 3);
    indices.push_back(offset + 2);
    indices.push_back(offset + 0);
  }

  // IBO/VBOs and sizes
  uint32_t iboSize = sizeof(uint32_t) * static_cast<uint32_t>(indices.size());
  uint32_t vboSize = sizeof(float) * 7 * static_cast<uint32_t>(points.size());

  std::shared_ptr<spire::VarBuffer> iboBufferSPtr(
    new spire::VarBuffer(iboSize));
  std::shared_ptr<spire::VarBuffer> vboBufferSPtr(
    new spire::VarBuffer(vboSize));

  spire::VarBuffer* iboBuffer = iboBufferSPtr.get();
  spire::VarBuffer* vboBuffer = vboBufferSPtr.get();

  for (auto a : indices) iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++) {
    vboBuffer->write(static_cast<float>(points[i].x()));
    vboBuffer->write(static_cast<float>(points[i].y()));
    vboBuffer->write(static_cast<float>(points[i].z()));
    vboBuffer->write(static_cast<float>(colors[i].r()));
    vboBuffer->write(static_cast<float>(colors[i].g()));
    vboBuffer->write(static_cast<float>(colors[i].b()));
    vboBuffer->write(static_cast<float>(1.f));
  }

  //add the actual points and colors

  int sigdig = state->getValue(SignificantDigits).toInt();
  int numlabel = state->getValue(Labels).toInt();
  int txtsize = state->getValue(TextSize).toInt();
  double scale = state->getValue(Scale).toDouble();
  int displaySide = state->getValue(DisplaySide).toInt();
  float red = static_cast<float>(state->getValue(TextRed).toDouble());
  float green = static_cast<float>(state->getValue(TextGreen).toDouble());
  float blue = static_cast<float>(state->getValue(TextBlue).toDouble());
  float xTrans = static_cast<float>(state->getValue(XTranslation).toInt());
  float yTrans = static_cast<float>(state->getValue(YTranslation).toInt());
  std::stringstream ss;
  ss << resolution << sigdig << txtsize << numlabel << state->getValue(Units).toString() <<
    scale << displaySide << red << green << blue << xTrans << yTrans;

  auto uniqueNodeID = geomId + "colorMapLegend" + ss.str();
  auto vboName = uniqueNodeID + "VBO";
  auto iboName = uniqueNodeID + "IBO";
  auto passName = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/ColorMapLegend";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
  std::vector<SpireSubPass::Uniform> uniforms;
  uniforms.push_back(SpireSubPass::Uniform("uXTranslate",static_cast<float>(xTrans)));
  uniforms.push_back(SpireSubPass::Uniform("uYTranslate",static_cast<float>(yTrans)));
  uniforms.push_back(SpireSubPass::Uniform("uDisplaySide", static_cast<float>(displaySide)));
  int displayLength = state->getValue(DisplayLength).toInt();
  uniforms.push_back(SpireSubPass::Uniform("uDisplayLength", static_cast<float>(displayLength)));
  auto geomVBO = SpireVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, BBox(Point{}, Point{}), true);

  // Construct IBO.

  SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::HAS_DATA, true);

  SpireText text;

  SpireSubPass pass(passName, vboName, iboName, shader,
                    ColorScheme::COLOR_MAP, renState, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, text);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  std::string idname = "ShowColorMap";
  if (!state->getValue(ColorMapName).toString().empty())
  {
    idname += GeometryObject::delimiter + state->getValue(ColorMapName).toString() + " (from " + id().id_ + ")";
  }

  auto geom(boost::make_shared<GeometryObjectSpire>(*this, idname, false));

  geom->setColorMap(cm->getColorMapName());
  geom->ibos().push_back(geomIBO);
  geom->vbos().push_back(geomVBO);
  geom->passes().push_back(pass);

  //text
  char str2[128];
  std::stringstream sd;
  sd << "%." << sigdig << "g";
  std::vector<Vector> txt_coords;
  double increment = 1.0 / (numlabel - 1);
  double textSize = 3.0 * txtsize;
  double dash_size = 18.;
  double pipe_size = 18.;
  size_t text_size = static_cast<size_t>(textSize);

  if (!textBuilder_.initialize(text_size))
    return geom;

  if (textBuilder_.getFaceSize() != text_size)
    textBuilder_.setFaceSize(text_size);
  textBuilder_.setColor(red, green, blue, 1.0);

  for (double i = 0.; i <= 1.000000001; i += increment)
  {
    std::stringstream line;
    sprintf(str2, sd.str().c_str(), (i / cm->getColorMapRescaleScale() - cm->getColorMapRescaleShift()) * scale);
    line << str2 << " " << state->getValue(Units).toString();
    Vector shift((displaySide == 0) ?
      (xTrans > 50 ? -(textSize*strlen(line.str().c_str())) : dash_size) : 0.,
      (displaySide == 0) ?
      0. : (yTrans > 50 ? (-textSize - pipe_size / 2.) : pipe_size), i);
    bool ds = displaySide == 0;
    bool full = displayLength == 1;
    bool half1 = displayLength == 0;
    double bar_buffer = 0.1;
    double tick_separation = shift.z() * (1. - bar_buffer) * (full ? 2. : 1.);
    double x_trans = (ds ? 0. : (tick_separation + (full ? bar_buffer : (half1 ? 0. : (bar_buffer + 1.)))))
      + xTrans / 50.;
    double y_trans = ((!ds) ? 0. : (tick_separation + (full ? bar_buffer : (half1 ? 0. : (bar_buffer + 1.)))))
      + yTrans / 50.;
    Vector trans(x_trans, y_trans, 0.0);

    textBuilder_.printString(line.str(), trans, shift, geomId, *geom);
  }

  return geom;
}

const AlgorithmParameterName ShowColorMap::DisplaySide("DisplaySide");
const AlgorithmParameterName ShowColorMap::DisplayLength("DisplayLength");
const AlgorithmParameterName ShowColorMap::TextSize("TextSize");
const AlgorithmParameterName ShowColorMap::TextColor("TextColor");
const AlgorithmParameterName ShowColorMap::Labels("Labels");
const AlgorithmParameterName ShowColorMap::Scale("Scale");
const AlgorithmParameterName ShowColorMap::Units("Units");
const AlgorithmParameterName ShowColorMap::SignificantDigits("SignificantDigits");
const AlgorithmParameterName ShowColorMap::AddExtraSpace("AddExtraSpace");
const AlgorithmParameterName ShowColorMap::TextRed("TextRed");
const AlgorithmParameterName ShowColorMap::TextGreen("TextGreen");
const AlgorithmParameterName ShowColorMap::TextBlue("TextBlue");
const AlgorithmParameterName ShowColorMap::XTranslation("XTranslation");
const AlgorithmParameterName ShowColorMap::YTranslation("YTranslation");
const AlgorithmParameterName ShowColorMap::ColorMapName("ColorMapName");

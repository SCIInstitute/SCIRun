/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

ShowColorMapModule::ShowColorMapModule() : GeometryGeneratingModule(ModuleLookupInfo("ShowColorMap", "Visualization", "SCIRun"))
{
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(GeometryOutput);
}

void ShowColorMapModule::setStateDefaults()
{
  auto state = get_state();
  state->setValue(DisplaySide, 0);
  state->setValue(DisplayLength, 0);
  state->setValue(TextSize, 2);
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
}

void ShowColorMapModule::execute()
{
  auto colorMap = getRequiredInput(ColorMapObject);
  if (needToExecute())
  {
    std::ostringstream ostr;
    ostr << get_id() << "_" <<
      colorMap->getColorMapInvert() << colorMap->getColorMapName() << colorMap->getColorMapRescaleScale() <<
      colorMap->getColorMapRescaleShift() << colorMap->getColorMapResolution() << colorMap.get() <<
      colorMap->getColorMapShift();
    auto geom = buildGeometryObject(colorMap, get_state(), ostr.str());
    sendOutput(GeometryOutput, geom);
  }
}

GeometryBaseHandle
ShowColorMapModule::buildGeometryObject(ColorMapHandle cm, ModuleStateHandle state, const std::string& id)
{
  std::vector<Vector> points;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;
  int32_t numVBOElements = 0;
  double resolution = 1. / static_cast<double>(cm->getColorMapResolution());
  //show colormap does not rescale colors, so reset them. we want to see the whole colormap on the scale.
  ColorMap new_map(cm->getColorMapName(), cm->getColorMapResolution(),
    cm->getColorMapShift(), cm->getColorMapInvert(), 1., 0.);
  for (double i = 0.; std::abs(i - 1.) > 0.000001; i += resolution) {
    ColorRGB col = new_map.valueToColor(i);
    uint32_t offset = static_cast<uint32_t>(points.size());
    points.push_back(Vector(0., i, +0.001));
    colors.push_back(col);
    points.push_back(Vector(1., i, +0.001));
    colors.push_back(col);
    points.push_back(Vector(0., i + resolution, +0.001));
    colors.push_back(col);
    points.push_back(Vector(1., i + resolution, +0.001));
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

  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));

  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

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

  auto st = get_state();
  int sigdig = st->getValue(SignificantDigits).toInt();
  int numlabel = st->getValue(Labels).toInt();
  int txtsize = st->getValue(TextSize).toInt();
  double scale = st->getValue(Scale).toDouble();
  int displaySide = state->getValue(DisplaySide).toInt();
  float red = static_cast<float>(st->getValue(TextRed).toDouble());
  float green = static_cast<float>(st->getValue(TextGreen).toDouble());
  float blue = static_cast<float>(st->getValue(TextBlue).toDouble());
  float xTrans = static_cast<float>(st->getValue(XTranslation).toInt());
  float yTrans = static_cast<float>(st->getValue(YTranslation).toInt());
  std::stringstream ss;
  ss << resolution << sigdig << txtsize << numlabel << st->getValue(Units).toString() <<
    scale << displaySide << red << green << blue << xTrans << yTrans;

  std::string uniqueNodeID = id + "colorMapLegend" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

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
  SpireVBO geomVBO = SpireVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, BBox(), true);

  // Construct IBO.

  SpireIBO geomIBO(iboName, SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::HAS_DATA, true);
  
  SpireSubPass pass(passName, vboName, iboName, shader,
    COLOR_MAP, renState, RENDER_VBO_IBO, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  GeometryHandle geom(new GeometryImpl(*this, "ShowColorMap"));

  geom->mColorMap = cm->getColorMapName();
  geom->mIBOs.push_back(geomIBO);
  geom->mVBOs.push_back(geomVBO);
  geom->mPasses.push_back(pass);
  {
    //########################################
    // Now render the numbers for the scale bar

    char str2[128];
    std::stringstream sd;
    sd << "%." << sigdig << "f";
    points.clear();
    indices.clear();
    std::vector<Vector> txt_coords;
    numVBOElements = 0;
    uint32_t count = 0;
    double increment = 1. / static_cast<double>(numlabel - 1);
    double textSize = 10. * static_cast<double>(txtsize + 3);
    const double dash_size = 20.;
    const double pipe_size = 40.;

    for (double i = 0.; i <= 1.000000001; i += increment) {
      std::stringstream ss;
      sprintf(str2, sd.str().c_str(), i / cm->getColorMapRescaleScale() - cm->getColorMapRescaleShift());
      ss << str2 << " " << st->getValue(Units).toString();
      //flip the text to the side with more space depending on xTrans/yTrans > 50%
      //text offsets vary depending on side of bar and left vs. bottom. Might need to vary per platform.
      text_.reset(ss.str(), textSize, Vector((displaySide == 0) ?
        (xTrans > 50 ? -(textSize*strlen(ss.str().c_str())) : 4.*dash_size) : 0.,
        (displaySide == 0) ?
        0. : (yTrans > 50 ? (-textSize - pipe_size / 2.) : pipe_size), i));
      std::vector<Vector> tmp;
      std::vector<Vector> coords;
      text_.getStringVerts(tmp, coords);
      if (displaySide != 0)
        //pipe texture is 18 pixels too far right, move dash down a bit if closer to the top
        text_.reset("|", pipe_size, Vector(-18., yTrans > 50 ? -15. : 0., i));
      else
        text_.reset("____", dash_size, Vector(xTrans > 50 ? -15. : 0., 0., i)); //move dashed over if bar on the right
      text_.getStringVerts(tmp, coords);

      for (auto a : tmp) {
        points.push_back(a);
        indices.push_back(count);
        count++;
      }
      for (auto a : coords)
        txt_coords.push_back(a);
    }
    numVBOElements = (uint32_t)points.size();

    // IBO/VBOs and sizes
    iboSize = sizeof(uint32_t) * (uint32_t)indices.size();
    vboSize = sizeof(float) * 5 * (uint32_t)points.size();

    std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr2(
      new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
    std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr2(
      new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));

    CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer2 = iboBufferSPtr2.get();
    CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer2 = vboBufferSPtr2.get();

    for (auto a : indices) iboBuffer2->write(a);
    for (size_t i = 0; i < points.size(); i++) {
      vboBuffer2->write(static_cast<float>(points[i].x()));
      vboBuffer2->write(static_cast<float>(points[i].y()));
      vboBuffer2->write(static_cast<float>(points[i].z()));
      vboBuffer2->write(static_cast<float>(txt_coords[i].x()));
      vboBuffer2->write(static_cast<float>(txt_coords[i].y()));
    }

    //add the actual points and colors

    uniqueNodeID = id + "colorMapLegendTextFont" + ss.str();
    vboName = uniqueNodeID + "VBO";
    iboName = uniqueNodeID + "IBO";
    passName = uniqueNodeID + "Pass2";

    // NOTE: Attributes will depend on the color scheme. We will want to
    // normalize the colors if the color scheme is COLOR_IN_SITU.

    // Construct VBO.
    shader = "Shaders/ColorMapLegendText";
    attribs.clear();
    attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
    attribs.push_back(SpireVBO::AttributeData("aTexCoord", 2 * sizeof(float)));
    uniforms.clear();
    uniforms.push_back(SpireSubPass::Uniform("uXTranslate", static_cast<float>(xTrans)));
    uniforms.push_back(SpireSubPass::Uniform("uYTranslate", static_cast<float>(yTrans)));
    uniforms.push_back(SpireSubPass::Uniform("uDisplaySide", static_cast<float>(displaySide)));
    uniforms.push_back(SpireSubPass::Uniform("uDisplayLength", static_cast<float>(displayLength)));
    uniforms.push_back(SpireSubPass::Uniform("uColor", glm::vec4(red, green, blue, 1.0f)));
    SpireVBO geomVBO2 = SpireVBO(vboName, attribs, vboBufferSPtr2,
      numVBOElements, BBox(), true);

    geom->mVBOs.push_back(geomVBO2);

    // Construct IBO.

    SpireIBO geomIBO2(iboName, SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr2);
    geom->mIBOs.push_back(geomIBO2);
    renState.set(RenderState::USE_COLORMAP, false);
    renState.set(RenderState::USE_TRANSPARENCY, true);
    SpireSubPass pass2(passName, vboName, iboName, shader,
      COLOR_MAP, renState, RENDER_VBO_IBO, geomVBO2, geomIBO2);

    // Add all uniforms generated above to the pass.
    for (const auto& uniform : uniforms) { pass2.addUniform(uniform); }
    //******************************************************************************************
    // TODO we're not adding this geometry (font) until we debug for it to work on Windows.
    geom->mPasses.push_back(pass2);
    //******************************************************************************************
  }
  return geom;
}

const AlgorithmParameterName ShowColorMapModule::DisplaySide("DisplaySide");
const AlgorithmParameterName ShowColorMapModule::DisplayLength("DisplayLength");
const AlgorithmParameterName ShowColorMapModule::TextSize("TextSize");
const AlgorithmParameterName ShowColorMapModule::TextColor("TextColor");
const AlgorithmParameterName ShowColorMapModule::Labels("Labels");
const AlgorithmParameterName ShowColorMapModule::Scale("Scale");
const AlgorithmParameterName ShowColorMapModule::Units("Units");
const AlgorithmParameterName ShowColorMapModule::SignificantDigits("SignificantDigits");
const AlgorithmParameterName ShowColorMapModule::AddExtraSpace("AddExtraSpace");
const AlgorithmParameterName ShowColorMapModule::TextRed("TextRed");
const AlgorithmParameterName ShowColorMapModule::TextGreen("TextGreen");
const AlgorithmParameterName ShowColorMapModule::TextBlue("TextBlue");
const AlgorithmParameterName ShowColorMapModule::XTranslation("XTranslation");
const AlgorithmParameterName ShowColorMapModule::YTranslation("YTranslation");

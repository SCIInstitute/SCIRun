/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Visualization/DataConversions.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Color.h>
#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

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
}

void ShowColorMapModule::execute()
{
  auto colorMap = getRequiredInput(ColorMapObject);
  if (needToExecute())
  {
    std::ostringstream ostr;
    ostr << get_id() << "_" << colorMap->getColorMapActualMin() << colorMap->getColorMapActualMax() <<
      colorMap->getColorMapInvert() << colorMap->getColorMapName() << colorMap->getColorMapRescaleScale() <<
      colorMap->getColorMapRescaleShift() << colorMap->getColorMapResolution() << colorMap.get() <<
      colorMap->getColorMapShift();
    GeometryHandle geom = buildGeometryObject(colorMap, get_state(), ostr.str());
    sendOutput(GeometryOutput, geom);
  }
}

GeometryHandle
ShowColorMapModule::buildGeometryObject(ColorMapHandle cm, ModuleStateHandle state, const std::string& id)
{
  std::vector<Vector> points;
  std::vector<double> colors;
  std::vector<uint32_t> indices;
  int32_t numVBOElements = 0;
  ColorMap * map = cm.get();
  double resolution = 1. / static_cast<double>(map->getColorMapResolution());
  for (double i = 0.; std::abs(i - 1.) > 0.000001; i += resolution) {
    uint32_t offset = (uint32_t)points.size();
    points.push_back(Vector(0., i, +0.001));
    colors.push_back(i);
    points.push_back(Vector(1., i, +0.001));
    colors.push_back(i);
    points.push_back(Vector(0., i + resolution, +0.001));
    colors.push_back(i);
    points.push_back(Vector(1., i + resolution, +0.001));
    colors.push_back(i);
    numVBOElements += 2;
    indices.push_back(offset + 0);
    indices.push_back(offset + 1);
    indices.push_back(offset + 3);
    indices.push_back(offset + 3);
    indices.push_back(offset + 2);
    indices.push_back(offset + 0);
  }

  // IBO/VBOs and sizes
  uint32_t iboSize = sizeof(uint32_t) * (uint32_t)indices.size();
  uint32_t vboSize = sizeof(float) * 4 * (uint32_t)points.size();

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
    vboBuffer->write(static_cast<float>(colors[i]));
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
  std::stringstream ss;
  ss << resolution << sigdig << txtsize << numlabel << st->getValue(Units).toString() <<
    scale << displaySide << red << green << blue;

  std::string uniqueNodeID = id + "colorMapLegend" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/ColorMapLegend";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));
  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  bool extraSpace = state->getValue(AddExtraSpace).toBool();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uExtraSpace", extraSpace ? 1.f : 0.f));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplaySide", static_cast<float>(displaySide)));
  int displayLength = state->getValue(DisplayLength).toInt();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplayLength", static_cast<float>(displayLength)));
  GeometryObject::SpireVBO geomVBO = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, Core::Geometry::BBox(), true);
  //push the color map parameters
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMInvert", map->getColorMapInvert() ? 1.f : 0.f));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMShift", static_cast<float>(map->getColorMapShift())));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMResolution", static_cast<float>(map->getColorMapResolution())));

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO(iboName, GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::USE_COLORMAP, true);

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::ColorScheme scheme = GeometryObject::COLOR_MAP;
  GeometryObject::SpireSubPass pass(passName, vboName, iboName, shader,
    scheme, renState, GeometryObject::RENDER_VBO_IBO, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  Core::Datatypes::GeometryHandle geom(new Core::Datatypes::GeometryObject(nullptr, *this, "ShowColorMap"));

  geom->mColorMap = cm->getColorMapName();
  geom->mIBOs.push_back(geomIBO);
  geom->mVBOs.push_back(geomVBO);
  geom->mPasses.push_back(pass);
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
  double textSize = 10. * static_cast<double>(txtsize + 1) + 30.;

  for (double i = 0.; i <= 1.000000001; i += increment) {
    std::stringstream ss;
    sprintf(str2, sd.str().c_str(), ((cm->getColorMapActualMax() -
      cm->getColorMapActualMin()) * i +
      cm->getColorMapActualMin()) * scale);
    ss << str2 << " " << st->getValue(Units).toString();
    text_.reset(ss.str(), textSize, Vector((displaySide == 0) ? 80. : 1., (displaySide == 0) ? 0. : 40., i));
    std::vector<Vector> tmp;
    std::vector<Vector> coords;
    text_.getStringVerts(tmp, coords);
    if (displaySide != 0)
      text_.reset("|", 40., Vector(1., 0., i));
    else
      text_.reset("____", 20., Vector(10., 0., i));
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
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aTexCoord", 2 * sizeof(float)));
  uniforms.clear();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uExtraSpace", extraSpace ? 1. : 0.));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplaySide", static_cast<float>(displaySide)));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplayLength", static_cast<float>(displayLength)));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uColor", glm::vec4(red,green,blue,1.0f)));
  GeometryObject::SpireVBO geomVBO2 = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr2,
    numVBOElements, Core::Geometry::BBox(), true);

  geom->mVBOs.push_back(geomVBO2);

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO2(iboName, GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr2);
  geom->mIBOs.push_back(geomIBO2);
  renState.set(RenderState::USE_COLORMAP, false);
  renState.set(RenderState::USE_TRANSPARENCY, true);

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  scheme = GeometryObject::COLOR_UNIFORM;
  GeometryObject::SpireSubPass pass2(passName, vboName, iboName, shader,
    scheme, renState, GeometryObject::RENDER_VBO_IBO, geomVBO2, geomIBO2);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass2.addUniform(uniform); }

  geom->mPasses.push_back(pass2);

  return geom;
}

AlgorithmParameterName ShowColorMapModule::DisplaySide("DisplaySide");
AlgorithmParameterName ShowColorMapModule::DisplayLength("DisplayLength");
AlgorithmParameterName ShowColorMapModule::TextSize("TextSize");
AlgorithmParameterName ShowColorMapModule::TextColor("TextColor");
AlgorithmParameterName ShowColorMapModule::Labels("Labels");
AlgorithmParameterName ShowColorMapModule::Scale("Scale");
AlgorithmParameterName ShowColorMapModule::Units("Units");
AlgorithmParameterName ShowColorMapModule::SignificantDigits("SignificantDigits");
AlgorithmParameterName ShowColorMapModule::AddExtraSpace("AddExtraSpace");
AlgorithmParameterName ShowColorMapModule::TextRed("TextRed");
AlgorithmParameterName ShowColorMapModule::TextGreen("TextGreen");
AlgorithmParameterName ShowColorMapModule::TextBlue("TextBlue");

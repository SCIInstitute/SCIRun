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
/// @todo Documentation Modules/Visualization/ShowString.cc

#include <Modules/Visualization/ShowString.h>
#include <Modules/Visualization/TextBuilder.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Geometry.h>
#include <Graphics/Datatypes/GeometryImpl.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Visualization;
using namespace Core::Geometry;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, TextRed);
ALGORITHM_PARAMETER_DEF(Visualization, TextGreen);
ALGORITHM_PARAMETER_DEF(Visualization, TextBlue);
ALGORITHM_PARAMETER_DEF(Visualization, TextAlpha);
ALGORITHM_PARAMETER_DEF(Visualization, FontName);
ALGORITHM_PARAMETER_DEF(Visualization, FontSize);
ALGORITHM_PARAMETER_DEF(Visualization, PositionType);

MODULE_INFO_DEF(ShowString, Visualization, SCIRun)

ShowString::ShowString() : GeometryGeneratingModule(staticInfo_), textBuilder_(boost::make_shared<TextBuilder>())
{
  INITIALIZE_PORT(String);
  INITIALIZE_PORT(RenderedString);
}

void ShowString::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::TextRed, 1.0);
  state->setValue(Parameters::TextGreen, 1.0);
  state->setValue(Parameters::TextBlue, 1.0);
  state->setValue(Parameters::FontSize, 16);
}

void ShowString::execute()
{
  auto str = getRequiredInput(String);

  if (needToExecute())
  {
    auto geom = buildGeometryObject(str->value());
    sendOutput(RenderedString, geom);
  }
}

// TODO: clean up duplication here and in ShowColorMap
GeometryBaseHandle ShowString::buildGeometryObject(const std::string& text)
{
  std::vector<Vector> points;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;
  int32_t numVBOElements = 0;

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

  auto uniqueNodeID = get_id().id_ + "_showString_" + text;
  auto vboName = uniqueNodeID + "VBO";
  auto iboName = uniqueNodeID + "IBO";
  auto passName = uniqueNodeID + "Pass";

  // Construct VBO.
  std::string shader = "Shaders/ColorMapLegend";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
  std::vector<SpireSubPass::Uniform> uniforms;
  
  auto state = get_state();

  int xTrans = 1;   // USER PARAM
  int yTrans = 1;   // USER PARAM

  uniforms.push_back(SpireSubPass::Uniform("uXTranslate", static_cast<float>(xTrans)));
  uniforms.push_back(SpireSubPass::Uniform("uYTranslate", static_cast<float>(yTrans)));
  
  SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, numVBOElements, BBox(), true);
  SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::HAS_DATA, true);

  SpireText spiretext;

  SpireSubPass pass(passName, vboName, iboName, shader,
    ColorScheme::COLOR_MAP, renState, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, spiretext);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  GeometryHandle geom(new GeometryObjectSpire(*this, "ShowString", false));

  geom->mIBOs.push_back(geomIBO);
  geom->mVBOs.push_back(geomVBO);
  geom->mPasses.push_back(pass);

  int fontSize = state->getValue(Parameters::FontSize).toInt();

  if (!textBuilder_->initialize(fontSize))
    return geom;

  if (textBuilder_->getFaceSize() != fontSize)
    textBuilder_->setFaceSize(fontSize);

  {
    auto r = state->getValue(Parameters::TextRed).toDouble();
    auto g = state->getValue(Parameters::TextGreen).toDouble();
    auto b = state->getValue(Parameters::TextBlue).toDouble();
    auto a = state->getValue(Parameters::TextBlue).toDouble();

    textBuilder_->setColor(r, g, b, 0.5);
  }

  Vector trans(xTrans, yTrans, 0.0); 
  Vector shift(20, 0, 0.5);  // USER PARAM
  textBuilder_->printString(text, trans, shift, text, *geom);

  return geom;
}

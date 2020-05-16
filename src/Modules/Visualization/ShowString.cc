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


/// @todo Documentation Modules/Visualization/ShowString.cc

#include <Modules/Visualization/ShowString.h>
#include <Modules/Visualization/TextBuilder.h>
#include <Core/Datatypes/String.h>
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
ALGORITHM_PARAMETER_DEF(Visualization, FixedHorizontal);
ALGORITHM_PARAMETER_DEF(Visualization, FixedVertical);
ALGORITHM_PARAMETER_DEF(Visualization, CoordinateHorizontal);
ALGORITHM_PARAMETER_DEF(Visualization, CoordinateVertical);

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
  state->setValue(Parameters::TextAlpha, 1.0);
  state->setValue(Parameters::FontSize, 16);

  state->setValue(Parameters::FontName, std::string("FreeSansBold"));
  state->setValue(Parameters::PositionType, std::string("Preset"));
  state->setValue(Parameters::FixedHorizontal, std::string("Left"));
  state->setValue(Parameters::FixedVertical, std::string("Top"));
  state->setValue(Parameters::CoordinateHorizontal, 0.5);
  state->setValue(Parameters::CoordinateVertical, 0.5);

  getOutputPort(RenderedString)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWindowResizeFeedback(var); });
}

void ShowString::processWindowResizeFeedback(const ModuleFeedback& var)
{
  if (!executedOnce_)
    return;

  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (lastWindowSize_ != vsf.windowSize)
    {
      lastWindowSize_ = vsf.windowSize;
      needReexecute_ = true;
      enqueueExecuteAgain(false);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void ShowString::execute()
{
  auto str = getRequiredInput(String);

  if (needToExecute() || needReexecute_)
  {
    auto geom = buildGeometryObject(str->value());
    sendOutput(RenderedString, geom);
    needReexecute_ = false;
    executedOnce_ = true;
  }
}

std::tuple<double, double> ShowString::getTextPosition()
{
  auto state = get_state();
  auto positionChoice = state->getValue(Parameters::PositionType).toString();
  if ("Preset" == positionChoice)
  {
    double x, y;
    auto horizontal = state->getValue(Parameters::FixedHorizontal).toString();
    if ("Left" == horizontal)
      x = 0.3;
    else if ("Center" == horizontal)
      x = 1.0;
    else // "Right"
      x = 1.7;

    auto vertical = state->getValue(Parameters::FixedVertical).toString();
    if ("Top" == vertical)
      y = 1.7;
    else if ("Middle" == vertical)
      y = 1.0;
    else // "Bottom"
      y = 0.3;

    state->setValue(Parameters::CoordinateHorizontal, x / 2.0);
    state->setValue(Parameters::CoordinateVertical, y / 2.0);
    return std::make_tuple(x, y);
  }
  else if ("Coordinates" == positionChoice)
  {
    return std::make_tuple(2 * state->getValue(Parameters::CoordinateHorizontal).toDouble(),
      2 * state->getValue(Parameters::CoordinateVertical).toDouble());
  }
  else
  {
    throw "logical error";
  }
}

// TODO: clean up duplication here and in ShowColorMap
GeometryBaseHandle ShowString::buildGeometryObject(const std::string& text)
{
  std::shared_ptr<spire::VarBuffer> iboBufferSPtr(new spire::VarBuffer(0));
  std::shared_ptr<spire::VarBuffer> vboBufferSPtr(new spire::VarBuffer(0));

  auto uniqueNodeID = id().id_ + "_showString_" + text;
  auto vboName = uniqueNodeID + "VBO";
  auto iboName = uniqueNodeID + "IBO";
  auto passName = uniqueNodeID + "Pass";

  // Construct VBO.
  std::string shader = "Shaders/ColorMapLegend";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
  std::vector<SpireSubPass::Uniform> uniforms;

  double xTrans, yTrans;
  std::tie(xTrans, yTrans) = getTextPosition();

  uniforms.push_back(SpireSubPass::Uniform("uXTranslate", xTrans));
  uniforms.push_back(SpireSubPass::Uniform("uYTranslate", yTrans));

  SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, 0, BBox(Point{}, Point{}), true);
  SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::HAS_DATA, true);

  SpireText spiretext;

  SpireSubPass pass(passName, vboName, iboName, shader,
    ColorScheme::COLOR_MAP, renState, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, spiretext);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  auto geom(boost::make_shared<GeometryObjectSpire>(*this, "ShowString", false));

  geom->ibos().push_back(geomIBO);
  geom->vbos().push_back(geomVBO);
  geom->passes().push_back(pass);

  auto state = get_state();
  auto fontSize = state->getValue(Parameters::FontSize).toInt();
  auto fontName = state->getValue(Parameters::FontName).toString() + ".ttf";

  if (textBuilder_ && textBuilder_->isReady() && textBuilder_->getFontName() != fontName)
  {
    textBuilder_.reset(new TextBuilder);
  }

  if (!textBuilder_->initialize(fontSize, fontName))
    return geom;

  if (textBuilder_->getFaceSize() != fontSize)
    textBuilder_->setFaceSize(fontSize);

  {
    auto r = state->getValue(Parameters::TextRed).toDouble();
    auto g = state->getValue(Parameters::TextGreen).toDouble();
    auto b = state->getValue(Parameters::TextBlue).toDouble();
    auto a = state->getValue(Parameters::TextAlpha).toDouble();

    textBuilder_->setColor(r, g, b, a);
  }

  auto dims = textBuilder_->getStringDims(text);
  auto length = std::get<0>(dims) + 20;
  auto width = std::get<1>(dims) + 20;

  xTrans *= 1 - length / std::get<0>(lastWindowSize_);
  yTrans *= 1 - width / std::get<1>(lastWindowSize_);

  if (containsDescenderLetter(text))
  {
    yTrans += 0.02;
  }

  Vector trans(xTrans, yTrans, 0.0);
  textBuilder_->printString(text, trans, Vector(), text, *geom);

  return geom;
}

bool ShowString::containsDescenderLetter(const std::string& text)
{
  static const std::string descenders("qpygj");
  return std::any_of(descenders.begin(), descenders.end(),
    [&text](char c) { return text.find(c) != std::string::npos; });
}

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

/// @todo Documentation Modules/Visualization/CreateBasicColorMap.cc

#include <Modules/Visualization/ShowColorMapModule.h>
#include <Modules/Visualization/TextBuilder.h>
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

ShowColorMapModule::ShowColorMapModule() : Module(ModuleLookupInfo("ShowColorMap", "Visualization", "SCIRun"))
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
  boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap> colorMap = getRequiredInput(ColorMapObject);
  if (needToExecute())
  {
    std::ostringstream ostr;
    ColorMap *cm = colorMap.get();
    ostr << get_id() << "_" << cm->getColorMapActualMin() << cm->getColorMapActualMax() <<
    cm->getColorMapInvert() << cm->getColorMapName()<< cm->getColorMapRescaleScale() <<
    cm->getColorMapRescaleShift() << cm->getColorMapResolution() << colorMap.get() << (rand() / RAND_MAX) <<
    cm->getColorMapShift();
    GeometryHandle geom = buildGeometryObject(colorMap, get_state(), ostr.str());
    sendOutput(GeometryOutput, geom);
  }
}

SCIRun::Core::Datatypes::GeometryHandle
ShowColorMapModule::buildGeometryObject(boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap> cm,
                                        Dataflow::Networks::ModuleStateHandle state,
                                        const std::string& id) 
{
  std::vector<Vector> points;
  std::vector<double> colors;
  std::vector<uint32_t> indices;
  int32_t numVBOElements = 0;
  ColorMap * map = cm.get();
  double resolution = 1. / static_cast<double>(map->getColorMapResolution());
  
  for (double i = 0.; std::abs(i - 1.) > 0.000001; i+=resolution) {
    uint32_t offset = (uint32_t)points.size();
    points.push_back(Vector(0.,i,0.));
    colors.push_back(i);
    points.push_back(Vector(1.,i,0.));
    colors.push_back(i);
    points.push_back(Vector(0.,i+resolution,0.));
    colors.push_back(i);
    points.push_back(Vector(1.,i+resolution,0.));
    colors.push_back(i);
    numVBOElements+=2;
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
  
  for (size_t i = 0; i < points.size(); i ++) {
    vboBuffer->write(static_cast<float>(points[i].x()));
    vboBuffer->write(static_cast<float>(points[i].y()));
    vboBuffer->write(static_cast<float>(points[i].z()));
    vboBuffer->write(static_cast<float>(colors[i]));
  }

  //add the actual points and colors

  std::string uniqueNodeID = id + "colorMapLegend";
  std::string vboName      = uniqueNodeID + "VBO";
  std::string iboName      = uniqueNodeID + "IBO";
  std::string passName     = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/ColorMapLegend";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));
  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  bool extraSpace = state->getValue(AddExtraSpace).toBool();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uExtraSpace",extraSpace?1.f:0.f));
  int displaySide = state->getValue(DisplaySide).toInt();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplaySide",static_cast<float>(displaySide)));
  int displayLength = state->getValue(DisplayLength).toInt();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplayLength",static_cast<float>(displayLength)));
  GeometryObject::SpireVBO geomVBO = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
		numVBOElements,Core::Geometry::BBox(),true);
  //push the color map parameters
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMInvert",map->getColorMapInvert()?1.f:0.f));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMShift",static_cast<float>(map->getColorMapShift())));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMResolution",static_cast<float>(map->getColorMapResolution())));

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
  
  Core::Datatypes::GeometryHandle geom(new Core::Datatypes::GeometryObject(nullptr));
  std::ostringstream ostr;
  ostr << get_id() << "ShowColorMap_" << geom.get();
  geom->objectName = ostr.str();

  geom->mColorMap = cm->getColorMapName();
  geom->mIBOs.push_back(geomIBO);
  geom->mVBOs.push_back(geomVBO);
  geom->mPasses.push_back(pass);
  //########################################
  // Now render the numbers for the scale bar
  
  auto st = get_state();
  char str2[128];
  std::stringstream sd;
  sd << "%." << st->getValue(SignificantDigits).toInt() << "f";
  points.clear();
  indices.clear();
  std::vector<Vector> txt_colors;
  numVBOElements = 0;
  TextBuilder txt, txt2;
  uint32_t count = 0;
  double scale = st->getValue(Scale).toDouble();
  double increment = 1./ static_cast<double>(st->getValue(Labels).toInt() - 1);
  double textSize = 5. * static_cast<double>(st->getValue(TextSize).toInt()+2);
  
  for (double i = 0.; i <= 1.000000001; i+=increment) {
    std::stringstream ss;
    sprintf(str2,sd.str().c_str(),((cm->getColorMapActualMax() -
                                    cm->getColorMapActualMin()) * i +
                                    cm->getColorMapActualMin()) * scale);
    if (displaySide==0)
        ss << "__";
    ss << str2 << " " << st->getValue(Units).toString();
    txt.reset(ss.str().c_str(), textSize, Vector((displaySide==0)?10.:1.,(displaySide==0)?0.:20.,i));
    if (displaySide!=0)
        txt2.reset("|", textSize, Vector(1.,0.,i));
    std::vector<Vector> tmp;
    std::vector<Vector> cols;
    txt.getStringVerts(tmp,cols);
    for (auto a : tmp) {
        points.push_back(a);
        indices.push_back(count);
        count++;
    }
    for (auto a : cols) 
        txt_colors.push_back(a);
    if (displaySide!=0) {
        std::vector<Vector> tmp2;
        std::vector<Vector> cols2;
        txt2.getStringVerts(tmp2,cols2);
        for (auto a : tmp2) {
            points.push_back(a);
            indices.push_back(count);
            count++;
        }
        for (auto a : cols2)
            txt_colors.push_back(a);
    }
  }
  numVBOElements = (uint32_t)points.size();

  // IBO/VBOs and sizes
  iboSize = sizeof(uint32_t) * (uint32_t)indices.size();
  vboSize = sizeof(float) * 7 * (uint32_t)points.size();
  
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr2(
      new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr2(
      new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));

  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer2 = iboBufferSPtr2.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer2 = vboBufferSPtr2.get();
  
  for (auto a : indices) iboBuffer2->write(a);
  float red = static_cast<float>(st->getValue(TextRed).toDouble());
  float green = static_cast<float>(st->getValue(TextRed).toDouble());
  float blue = static_cast<float>(st->getValue(TextRed).toDouble());
  for (size_t i = 0; i < points.size(); i ++) {
    vboBuffer2->write(static_cast<float>(points[i].x()));
    vboBuffer2->write(static_cast<float>(points[i].y()));
    vboBuffer2->write(static_cast<float>(points[i].z()));
    vboBuffer2->write(static_cast<float>(txt_colors[i].x()*red));
    vboBuffer2->write(static_cast<float>(txt_colors[i].x()*green));
    vboBuffer2->write(static_cast<float>(txt_colors[i].x()*blue));
    vboBuffer2->write(static_cast<float>(txt_colors[i].y()));
  }

  //add the actual points and colors

  uniqueNodeID = id + "colorMapLegendText";
  vboName      = uniqueNodeID + "VBO";
  iboName      = uniqueNodeID + "IBO";
  passName     = uniqueNodeID + "Pass2";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  shader = "Shaders/Text";
  attribs.clear();
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
  uniforms.clear();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uExtraSpace",extraSpace?1.:0.));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplaySide",static_cast<float>(displaySide)));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplayLength",static_cast<float>(displayLength)));
  GeometryObject::SpireVBO geomVBO2 = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr2,
		numVBOElements,Core::Geometry::BBox(),true);

  geom->mVBOs.push_back(geomVBO2);

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO2 = GeometryObject::SpireIBO(iboName,
                                        GeometryObject::SpireIBO::POINTS,
                                        sizeof(uint32_t), iboBufferSPtr2);
  geom->mIBOs.push_back(geomIBO2);
  renState.set(RenderState::USE_COLORMAP, false);
  renState.set(RenderState::USE_TRANSPARENT_NODES, true);
    
  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  scheme = GeometryObject::COLOR_UNIFORM;
  GeometryObject::SpireSubPass pass2 =
  GeometryObject::SpireSubPass(passName, vboName, iboName, shader,
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

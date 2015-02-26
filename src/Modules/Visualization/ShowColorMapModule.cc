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
	state->setValue(TextSize, 0);
	state->setValue(TextColor, ColorRGB(255, 255, 255).toString());//this is a guess
  //TODO change to doubles
	state->setValue(Labels, std::string("5"));
  state->setValue(Scale, std::string("1.0"));
  state->setValue(Units, std::string(""));
  state->setValue(SignificantDigits, std::string("2"));
	state->setValue(AddExtraSpace, false);
}

void ShowColorMapModule::execute()
{
  boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap> colorMap = getRequiredInput(ColorMapObject);
  if (needToExecute())
  {
    std::ostringstream ostr;
    ostr << get_id() << "_" << colorMap.get();
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
  
  for (double i = 0.; i < 1.0; i+=resolution) {
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
  
  points.clear();
  indices.clear();
  std::vector<float> shifts;
  numVBOElements = 0;
  TextBuilder txt, txt2;
  uint32_t count = 0;
  
  for (double i = 0.; i < 1.; i+=0.1) { //TODO increment/start/end will be passed in by the dialog
    std::stringstream ss;
    char cstr[128];
    sprintf(cstr,"%.2f",(cm->getColorMapActualMax()-cm->getColorMapActualMin())*i+cm->getColorMapActualMin());
                          //TODO decimal places will come from dialog, as well as start/end #'s
    if (displaySide==0)
        ss << "__ ";
    ss << cstr;
    txt.reset(ss.str().c_str(), 15., Vector((displaySide==0)?10.:1.,(displaySide==0)?0.:20.,0.));
    if (displaySide!=0)
        txt2.reset("|", 15., Vector(1.,0.,0.));
    std::vector<Vector> tmp;
    txt.getStringVerts(tmp);
    for (auto a : tmp) {
        points.push_back(a);
        indices.push_back(count);
        shifts.push_back(i);
        count++;
    }
    if (displaySide!=0) {
        std::vector<Vector> tmp2;
        txt2.getStringVerts(tmp2);
        for (auto a : tmp2) {
            points.push_back(a);
            indices.push_back(count);
            shifts.push_back(i);
            count++;
        }
    }
  }
  numVBOElements = (uint32_t)points.size() / 2;

  // IBO/VBOs and sizes
  iboSize = sizeof(uint32_t) * (uint32_t)indices.size();
  vboSize = sizeof(float) * 4 * (uint32_t)points.size();
  
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr2(
      new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr2(
      new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));

  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer2 = iboBufferSPtr2.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer2 = vboBufferSPtr2.get();
  
  for (auto a : indices) iboBuffer2->write(a);
  
  for (size_t i = 0; i < points.size(); i ++) {
    vboBuffer2->write(static_cast<float>(points[i].x()));
    vboBuffer2->write(static_cast<float>(points[i].y()));
    vboBuffer2->write(static_cast<float>(points[i].z()));
    vboBuffer2->write(static_cast<float>(shifts[i]));
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
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aShift", 1 * sizeof(float)));
  uniforms.clear();
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uExtraSpace",extraSpace?1.:0.));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplaySide",static_cast<float>(displaySide)));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDisplayLength",static_cast<float>(displayLength)));
  GeometryObject::SpireVBO geomVBO2 = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr2,
		numVBOElements,Core::Geometry::BBox(),true);

  geom->mVBOs.push_back(geomVBO2);

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO2 = GeometryObject::SpireIBO(iboName,
                                        GeometryObject::SpireIBO::LINES,
                                        sizeof(uint32_t), iboBufferSPtr2);
  geom->mIBOs.push_back(geomIBO2);
  renState.set(RenderState::USE_COLORMAP, false);
    
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

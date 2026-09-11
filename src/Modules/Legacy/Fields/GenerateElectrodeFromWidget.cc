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


///@brief This module makes a mesh that looks like a wire

#include <Modules/Legacy/Fields/GenerateElectrodeFromWidget.h>
#include <Modules/Legacy/Fields/GenerateElectrode.h>
#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Graphics/Datatypes/RenderFieldState.h>
#include <Core/Algorithms/Base/VariableHelper.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/Feedback.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/Logging/Log.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Graphics/Widgets/WidgetBuilders.h>
#include <Graphics/Widgets/ArrowWidget.h>



using namespace SCIRun;
using namespace Core;
using namespace Logging;
using namespace Modules::Fields;
using namespace Algorithms;
using namespace Fields;
using namespace Dataflow::Networks;
using namespace Datatypes;
//using namespace SCIRun::Core::Geometry;
using namespace Geometry;
using namespace Graphics;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(GenerateElectrodeFromWidget, NewField, SCIRun)

//ALGORITHM_PARAMETER_DEF(Fields, ElectrodeLength);
//ALGORITHM_PARAMETER_DEF(Fields, ElectrodeThickness);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeWidth);
//ALGORITHM_PARAMETER_DEF(Fields, NumberOfControlPoints);
//ALGORITHM_PARAMETER_DEF(Fields, ElectrodeType);
//ALGORITHM_PARAMETER_DEF(Fields, ElectrodeResolution);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeProjection);
ALGORITHM_PARAMETER_DEF(Fields, MoveAll);
ALGORITHM_PARAMETER_DEF(Fields, UseFieldNodes);
ALGORITHM_PARAMETER_DEF(Fields, Reset);


//ALGORITHM_PARAMETER_DEF(Fields, ProbeColor);
//ALGORITHM_PARAMETER_DEF(Fields, ProbeLabel);
//ALGORITHM_PARAMETER_DEF(Fields, ProbeSize);

//ALGORITHM_PARAMETER_DEF(Fields, TranslationPoint);
ALGORITHM_PARAMETER_DEF(Fields, PointPositions);
ALGORITHM_PARAMETER_DEF(Fields, DipoleDirection);

//const AlgorithmParameterName GenerateElectrodeFromWidget::MoveAll("MoveAll");
//const AlgorithmParameterName GenerateElectrodeFromWidget::PointPositions("PointPositions");
//const AlgorithmParameterName
//GenerateElectrodeFromWidget::DipoleDirection("DipoleDirection");
//const AlgorithmParameterName GenerateElectrodeFromWidget::Reset("Reset");
//const AlgorithmParameterName
//GenerateElectrodeFromWidget::TranslationPoint("TranslationPoint");

namespace SCIRun
{
namespace Modules
{
namespace Fields
{

class GenerateElectrodeFromWidgetImpl
{
public:
  GenerateElectrodeFromWidgetImpl(std::function<ModuleStateHandle()> s,
    GeometryGeneratingModule* module) : state_(s), module_(module) {}

  bool runImpl(FieldHandle&, FieldHandle&, FieldHandle&, GeometryHandle& );
  
  
private:
  std::function<ModuleStateHandle()> state_;
  GeometryGeneratingModule* module_;
  std::vector<WidgetHandle> widget_;
  
//  FieldHandle fieldInput_;
  
  // I don't think I'll need this.  Composite widgets are different in SR5
//  WidgetHandle arrow_widget_;
  std::vector<GeometryHandle> geoms_;
  
  std::vector<Point> Previous_points_;
  Transform previousTransform_;

  
};
}}}





bool GenerateElectrodeFromWidgetImpl::runImpl(FieldHandle& input, FieldHandle& outputField, FieldHandle& outputPoints, GeometryHandle& outWidget)
{

  
}


GenerateElectrodeFromWidget::GenerateElectrodeFromWidget() : GeometryGeneratingModule(staticInfo_), impl_(new GenerateElectrodeFromWidgetImpl([this]() { return get_state(); }, this))
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(ElectrodeMesh);
  INITIALIZE_PORT(ElectrodeWidget);
  INITIALIZE_PORT(ControlPoints);
}

void GenerateElectrodeFromWidget::setStateDefaults()
{
  auto state = get_state();
  using namespace Parameters;
  
  state->setValue(Reset, false);
  state->setValue(ElectrodeLength, 0.1);
  state->setValue(ElectrodeThickness, 0.003);
  state->setValue(ElectrodeWidth, 0.02);
  state->setValue(NumberOfControlPoints, 5);
  state->setValue(ElectrodeResolution, 10);
  state->setValue(UseFieldNodes, true);
  state->setValue(MoveAll, false);
  state->setValue(ElectrodeType, "wire");
  state->setValue(ElectrodeProjection, "midway");
  state->setValue(ProbeColor, "Color(1.0, 1.0, 1.0)");
  state->setValue(ProbeLabel, "GenerateElectrodeWidget");
  state->setValue(ProbeSize, 1.0);
  state->setValue(PointPositions, VariableList());
  state->setValue(DipoleDirection, "dip_dir(10.0, 10.0, 10.0)");
  
  getOutputPort(ElectrodeWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void GenerateElectrodeFromWidget::processWidgetFeedback(const ModuleFeedback& var)
{
  
}

void GenerateElectrodeFromWidget::execute()
{
  
//  auto state = get_state();
  
  auto source = getOptionalInput(InputField);
  

  FieldHandle outputField;
  FieldHandle outputPoints;
  GeometryHandle geomWidget;
  
  if (!impl_ -> runImpl(*source, outputField, outputPoints, geomWidget))
    error("False returned on legacy run call.");
  
  
  sendOutput(ElectrodeWidget, geomWidget);
  sendOutput(ElectrodeMesh, outputField);
  sendOutput(ControlPoints, outputPoints);
  
}



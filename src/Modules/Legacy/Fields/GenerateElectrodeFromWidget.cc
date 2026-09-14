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
#include <Modules/Legacy/Fields/GeneratePointSamplesFromField.h>

//#include <Core/Algorithms/Legacy/Fields/GenerateElectrodeFromPointsAlgo.h>

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
//ALGORITHM_PARAMETER_DEF(Fields, PointPositions);
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

//  bool runImpl(FieldHandle&, FieldHandle&, FieldHandle&, GeometryHandle& );
  FieldHandle makePointCloud();
  void setPointsToState();
  void createWidgets(std::vector<Point>& points);
  WidgetHandle createPointWidget(Point& point, size_t id);
  
  std::vector<WidgetHandle> widget_;
  BBox bbox_;
  
  
private:
  std::function<ModuleStateHandle()> state_;
  GeometryGeneratingModule* module_;
  
  
//  FieldHandle fieldInput_;
  
  // I don't think I'll need this.  Composite widgets are different in SR5
//  WidgetHandle arrow_widget_;
//  std::vector<GeometryHandle> geoms_;
  
//  std::vector<Point>& Previous_points_;
  Transform previousTransform_;

  
};
}}}


FieldHandle GenerateElectrodeFromWidgetImpl::makePointCloud()
{
  FieldInformation fi("PointCloudMesh", 1, "double");
  auto ofield = CreateField(fi);
  auto mesh = ofield->vmesh();
  auto field = ofield->vfield();

  for (int i = 0; i < widget_.size(); i++)
  {
    const Point location = widget_[i]->position();

    VMesh::Node::index_type pcindex = mesh->add_point(location);
    field->resize_fdata();
    field->set_value(static_cast<double>(i), pcindex);
  }
  return ofield;
}


void GenerateElectrodeFromWidgetImpl::createWidgets(std::vector<Point>& points)
{
  widget_.resize(0);
  
  bbox_=BBox(points);
  
  for (size_t i = 0; i < points.size() - 1; i++)
  {
    widget_.push_back(createPointWidget(points[i], i));
  }
  
  setPointsToState();
}

void GenerateElectrodeFromWidgetImpl::setPointsToState()
{

  VariableList positions;
  for (const auto& widget : widget_)
  {
    positions.push_back(makeVariable("widget_i", widget->position().get_string()));
  }

  state_()->setValue(Parameters::PointPositions, positions);
  state_()->setValue(Parameters::NumberOfControlPoints, static_cast<int>(widget_.size()));
}

WidgetHandle GenerateElectrodeFromWidgetImpl::createPointWidget(Point& point, size_t id)
{
  std::string probename = state_()->getValue(Parameters::ProbeLabel).toString();
  std::string widgetName = probename + "(" + std::to_string(id) + ")";
  
  auto sphere = SphereWidgetBuilder(*module_)
    .tag(widgetName)
    .transformMapping({{WidgetInteraction::CLICK, singleMovementWidget(WidgetMovement::TRANSLATE)}})
    .scale(state_()->getValue(Parameters::ProbeSize).toDouble())
    .defaultColor(state_()->getValue(Parameters::ProbeColor).toString())
    .origin(point)
    .boundingBox(bbox_)
    .resolution(10)
    .centerPoint(point)
    .build();
  return sphere;
}


//bool GenerateElectrodeFromWidgetImpl::runImpl(FieldHandle& input, FieldHandle& outputField, FieldHandle& outputPoints, GeometryHandle& outWidget)
//{
//  
//}


GenerateElectrodeFromWidget::GenerateElectrodeFromWidget() : GeometryGeneratingModule(staticInfo_), impl_(new GenerateElectrodeFromWidgetImpl([this]() { return get_state(); }, this))
{
  INITIALIZE_PORT(InputField);
//  INITIALIZE_PORT(ElectrodeMesh);
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
  state->setValue(ElectrodeType, std::string("wire"));
  state->setValue(ElectrodeProjection, std::string("midway"));
  state->setValue(ProbeColor, std::string("Color(1.0, 1.0, 1.0)"));
  state->setValue(ProbeLabel, std::string("GenerateElectrodeWidget"));
  state->setValue(ProbeSize, 1.0);
  state->setValue(PointPositions, VariableList());
  state->setValue(DipoleDirection, std::string("dip_dir(10.0, 10.0, 10.0)"));
  
  getOutputPort(ElectrodeWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void GenerateElectrodeFromWidget::processWidgetFeedback(const ModuleFeedback& var)
{
  std::string probename = get_state()->getValue(Parameters::ProbeLabel).toString();
//  std::string widgetName = probename + "(" + std::to_string(id) + ")";

  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()))
    {
      int widgetIndex = -1;
      try
      {
        static boost::regex r("SphereWidget::" + probename + "\\((.+)\\).+");
        boost::smatch what;
        regex_match(vsf.selectionName, what, r);
        widgetIndex = boost::lexical_cast<int>(what[1]);
      }
      catch (...)
      {
        logWarning("Failure parsing widget id");
        return;
      }
      
      adjustPositionFromTransform(vsf.transform, widgetIndex);
      enqueueExecuteAgain(false);

    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
  
}

void GenerateElectrodeFromWidget::adjustPositionFromTransform(const Transform& transformMatrix, int index)
{
  DenseMatrix center(4, 1);

  auto currLoc = impl_->widget_[index]->position();
  center << currLoc.x(), currLoc.y(), currLoc.z(), 1.0;
  DenseMatrix newTransform(DenseMatrix(transformMatrix) * center);

  Point newLocation(newTransform(0, 0) / newTransform(3, 0),
    newTransform(1, 0) / newTransform(3, 0),
    newTransform(2, 0) / newTransform(3, 0));

  impl_->widget_[index]->setPosition(newLocation);
  
}

std::vector<Point> GenerateElectrodeFromWidget::defaultPoints()
{
  std::vector<Point> orig_points;
  double l, lx;
  l = get_state()->getValue(Parameters::ElectrodeLength).toDouble();

  lx = l * .5774;
  
//  std::cout<<"length = "<<l <<" ("<<lx<<") "<<std::endl;

  orig_points.resize(5);

  orig_points[0] = (Point(0, 0, 0));
  orig_points[1] = (Point(lx*.25, lx*.25, lx*.25));
  orig_points[2] = (Point(lx*.5, lx*.5, lx*.5));
  orig_points[3] = (Point(lx*.75, lx*.75, lx*.75));
  orig_points[4] = (Point(lx, lx, lx));
  
  return orig_points;
}


void GenerateElectrodeFromWidget::execute()
{
  
//  auto state = get_state();
  
  auto source = getOptionalInput(InputField);
  

//  FieldHandle outputField;
//  FieldHandle outputPoints;
//  GeometryHandle geomWidget;
  
//  if (!impl_ -> runImpl(*source, outputField, outputPoints, geomWidget))
//    error("False returned on legacy run call.");
  
//
  std::vector<Point> orig_points = defaultPoints();
  
  std::cout<<"points ("<<orig_points.size()<<") = "<<std::endl;
  std::cout<<"   "<<orig_points<<std::endl;
//  for(auto p : orig_points)
//    std::cout<<"   "<<p<<std::endl;
//    std::cout<<"   "<<p.x()<<", "<<p.y()<<", "<<p.z()<<std::endl;
  
  
  impl_->createWidgets(orig_points);
  

  
  
  std::vector<GeometryHandle> geom_list;
  for(auto w : impl_->widget_)
    geom_list.push_back(w);

  GeometryHandle geomWidget = createGeomComposite(*this, "multiple_spheres", geom_list.begin(), geom_list.end());
  
  FieldHandle outputPoints=impl_->makePointCloud();
  
  
//  auto output = algo().run(withInputData((InputField, source)));
//  sendOutputFromAlgorithm(ElectrodeMesh, output);
  
  sendOutput(ElectrodeWidget, geomWidget);
//  sendOutput(ElectrodeMesh, outputField);
  sendOutput(ControlPoints, outputPoints);
  
}




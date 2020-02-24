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


///
///@file  GeneratePointSamplesFromField.cc
///
///@author
///   Robert Van Uitert
///   Diagnostic Radiology Department
///   National Institutes of Health
///@date  November 2004
///

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Logging/Log.h>
#include <Graphics/Widgets/WidgetBuilders.h>
#include <Modules/Legacy/Fields/GeneratePointSamplesFromField.h>
#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

using namespace SCIRun;
using namespace Core;
using namespace Logging;
using namespace Datatypes;
using namespace Dataflow::Networks;
using namespace Modules::Fields;
using namespace Geometry;
using namespace Algorithms;
using namespace Algorithms::Fields;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Fields, NumSeeds);
ALGORITHM_PARAMETER_DEF(Fields, ProbeScale);
ALGORITHM_PARAMETER_DEF(Fields, PointPositions);

MODULE_INFO_DEF(GeneratePointSamplesFromField, NewField, SCIRun)

namespace SCIRun
{
  namespace Modules
  {
    namespace Fields
    {
      class GeneratePointSamplesFromFieldImpl
      {
      public:
        BBox last_bounds_;
        std::vector<WidgetHandle> pointWidgets_;
        std::vector<Transform> previousTransforms_;
        double l2norm_;

        FieldHandle makePointCloud()
        {
          FieldInformation fi("PointCloudMesh", 1, "double");
          auto ofield = CreateField(fi);
          auto mesh = ofield->vmesh();
          auto field = ofield->vfield();

          for (int i = 0; i < pointWidgets_.size(); i++)
          {
            const Point location = pointWidgets_[i]->position();

            VMesh::Node::index_type pcindex = mesh->add_point(location);
            field->resize_fdata();
            field->set_value(static_cast<double>(i), pcindex);
          }
          return ofield;
        }
      };
    }
  }
}


GeneratePointSamplesFromField::GeneratePointSamplesFromField()
  : GeometryGeneratingModule(staticInfo_), impl_(new GeneratePointSamplesFromFieldImpl)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(GeneratedWidget);
  INITIALIZE_PORT(GeneratedPoints);
}

void GeneratePointSamplesFromField::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::NumSeeds, 1);
  state->setValue(Parameters::ProbeScale, 0.23);
  state->setValue(Parameters::PointPositions, VariableList());
  getOutputPort(GeneratedWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void GeneratePointSamplesFromField::execute()
{
  sendOutput(GeneratedPoints, GenerateOutputField());

  std::vector<GeometryHandle> geom_list;
  for(auto w : impl_->pointWidgets_)
    geom_list.push_back(w);

  auto geom = createGeomComposite(*this, "multiple_spheres", geom_list.begin(), geom_list.end());
  sendOutput(GeneratedWidget, geom);
}

void GeneratePointSamplesFromField::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()))
    {
      int widgetIndex = -1;
      try
      {
        static boost::regex r("SphereWidget::GPSFF\\((.+)\\).+");
        boost::smatch what;
        regex_match(vsf.selectionName, what, r);
        widgetIndex = boost::lexical_cast<int>(what[1]);
      }
      catch (...)
      {
        logWarning("Failure parsing widget id");
        return;
      }
      if (impl_->previousTransforms_[widgetIndex] != vsf.transform)
      {
        adjustPositionFromTransform(vsf.transform, widgetIndex);
        enqueueExecuteAgain(false);
      }
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void GeneratePointSamplesFromField::adjustPositionFromTransform(const Transform& transformMatrix, int index)
{
  DenseMatrix center(4, 1);

  auto currLoc = impl_->pointWidgets_[index]->position();
  center << currLoc.x(), currLoc.y(), currLoc.z(), 1.0;
  DenseMatrix newTransform(DenseMatrix(transformMatrix) * center);

  Point newLocation(newTransform(0, 0) / newTransform(3, 0),
    newTransform(1, 0) / newTransform(3, 0),
    newTransform(2, 0) / newTransform(3, 0));

  impl_->pointWidgets_[index]->setPosition(newLocation);
  impl_->previousTransforms_[index] = transformMatrix;
}

FieldHandle GeneratePointSamplesFromField::GenerateOutputField()
{
  auto ifieldhandle = getRequiredInput(InputField);

  auto bbox = ifieldhandle->vmesh()->get_bounding_box();

  Point center;
  Point bmin = bbox.get_min();
  Point bmax = bbox.get_max();

  if (!bbox.is_similar_to(impl_->last_bounds_))
  {
    // Fix degenerate boxes.
    const double size_estimate = std::max((bmax - bmin).length() * 0.01, 1.0e-5);
    if (fabs(bmax.x() - bmin.x()) < 1.0e-6)
    {
      bmin.x(bmin.x() - size_estimate);
      bmax.x(bmax.x() + size_estimate);
    }
    if (fabs(bmax.y() - bmin.y()) < 1.0e-6)
    {
      bmin.y(bmin.y() - size_estimate);
      bmax.y(bmax.y() + size_estimate);
    }
    if (fabs(bmax.z() - bmin.z()) < 1.0e-6)
    {
      bmin.z(bmin.z() - size_estimate);
      bmax.z(bmax.z() + size_estimate);
    }

    center = bmin + Vector(bmax - bmin) * 0.5;
    impl_->l2norm_ = (bmax - bmin).length();
    impl_->last_bounds_ = bbox;
  }

  auto state = get_state();
  auto numSeeds = state->getValue(Parameters::NumSeeds).toInt();
  auto scale = state->getValue(Parameters::ProbeScale).toDouble();
  auto widgetName = [](int i) { return "GPSFF(" + std::to_string(i) + ")"; };
  if (impl_->pointWidgets_.size() != numSeeds)
  {
    if (numSeeds < impl_->pointWidgets_.size())
    {
      impl_->pointWidgets_.resize(numSeeds);
    }
    else
    {
      auto positions = state->getValue(Parameters::PointPositions).toVector();
      for (size_t i = impl_->pointWidgets_.size(); i < numSeeds; i++)
      {
        auto location = center;
        if (i < positions.size())
          location = pointFromString(positions[i].toString());

        auto seed = SphereWidgetBuilder(*this)
          .tag(widgetName(i))
          .scale(scale)
          .defaultColor("Color(0.5,0.5,0.5)")
          .origin(location)
          .boundingBox(bbox)
          .resolution(10)
          .centerPoint(location)
          .build();
        impl_->pointWidgets_.push_back(seed);
      }
    }
    impl_->previousTransforms_.resize(impl_->pointWidgets_.size());
  }
  else
  {
    std::vector<WidgetHandle> newWidgets;
    int counter = 0;
    moveCount_++;
    for (const auto& oldWidget : impl_->pointWidgets_)
    {
      auto seed = SphereWidgetBuilder(*this)
        .tag(widgetName(counter++) + std::string(moveCount_, ' '))
        .scale(scale)
        .defaultColor("Color(0.5,0.5,0.5)")
        .origin(oldWidget->position())
        .boundingBox(bbox)
        .resolution(10)
        .centerPoint(oldWidget->position())
        .build();
      newWidgets.push_back(seed);
    }
    impl_->pointWidgets_ = newWidgets;
  }

  VariableList positions;
  for (const auto& widget : impl_->pointWidgets_)
  {
    positions.push_back(makeVariable("widget_i", widget->position().get_string()));
  }
  state->setValue(Parameters::PointPositions, positions);

  return impl_->makePointCloud();
}

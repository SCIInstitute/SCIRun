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

#include <Modules/Fields/EditMeshBoundingBox.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/Datatypes/Color.h>
#include <boost/format.hpp>

using namespace SCIRun;
using namespace Modules::Fields;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Graphics;
using namespace Datatypes;

MODULE_INFO_DEF(EditMeshBoundingBox, ChangeMesh, SCIRun)

class BoxWidgetNull : public BoxWidgetInterface
{
public:
  void setPosition(const Point& center, const Point& right, const Point& down, const Point& in) override
  {
    center_ = center;
    right_ = right;
    down_ = down;
    in_ = in;
  }
  void getPosition(Point& center, Point& right, Point& down, Point& in) const override
  {
    center = center_;
    right = right_;
    down = down_;
    in = in_;
  }

private:
  Point center_, right_, down_, in_;
};

class WidgetFactory
{
public:
  static BoxWidgetPtr createBox();
};

namespace SCIRun
{
  namespace Modules
  {
    namespace Fields
    {
      class EditMeshBoundingBoxImpl
      {
      public:
        Transform userWidgetTransform_;
        Transform field_initial_transform_;
        BBox box_initial_bounds_;
      };
    }
  }
}

EditMeshBoundingBox::EditMeshBoundingBox()
: GeometryGeneratingModule(staticInfo_),
impl_(new EditMeshBoundingBoxImpl), widgetMoved_(false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Transformation_Widget);
  INITIALIZE_PORT(Transformation_Matrix);
}

void EditMeshBoundingBox::processWidgetFeedback(const ModuleFeedback& var)
{
  auto vsf = static_cast<const ViewSceneFeedback&>(var);
  if (vsf.selectionName.find(get_id()) != std::string::npos &&
    impl_->userWidgetTransform_ != vsf.transform)
  {
    widgetMoved_ = true;
    adjustGeometryFromTransform(vsf.transform);
    enqueueExecuteAgain(false);
  }
}

void EditMeshBoundingBox::adjustGeometryFromTransform(const Transform& transformMatrix)
{
  Point currCenter, right, down, in;
  box_->getPosition(currCenter, right, down, in);

  DenseMatrix center(4, 1);
  center << currCenter.x(), currCenter.y(), currCenter.z(), 1.0;
  DenseMatrix newTransform(DenseMatrix(transformMatrix) * center);

  Point newLocation(newTransform(0, 0) / newTransform(3, 0),
                    newTransform(1, 0) / newTransform(3, 0),
                    newTransform(2, 0) / newTransform(3, 0));

  auto state = get_state();
  state->setValue(OutputCenterX, newLocation.x());
  state->setValue(OutputCenterY, newLocation.y());
  state->setValue(OutputCenterZ, newLocation.z());

  impl_->userWidgetTransform_ = transformMatrix;
}

void EditMeshBoundingBox::createBoxWidget()
{
  box_ = WidgetFactory::createBox();
}

void EditMeshBoundingBox::setStateDefaults()
{
  clear_vals();
  auto state = get_state();
  state->setValue(RestrictX, false);
  state->setValue(RestrictY, false);
  state->setValue(RestrictZ, false);
  state->setValue(RestrictR, false);
  state->setValue(RestrictD, false);
  state->setValue(RestrictI, false);
  state->setValue(OutputCenterX, 0.0);
  state->setValue(OutputCenterY, 0.0);
  state->setValue(OutputCenterZ, 0.0);
  state->setValue(OutputSizeX, 1.0);
  state->setValue(OutputSizeY, 1.0);
  state->setValue(OutputSizeZ, 1.0);
  state->setValue(Scale, 0.1);
  state->setValue(NoTranslation, true);
  state->setValue(XYZTranslation, false);
  state->setValue(RDITranslation, false);
  state->setValue(BoxRealScale, 0.0);
  state->setValue(BoxMode, 0);

  createBoxWidget();

  getOutputPort(Transformation_Widget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void EditMeshBoundingBox::execute()
{
  auto field = getRequiredInput(InputField);

  if (needToExecute())
  {
    clear_vals();
    update_input_attributes(field);
    executeImpl(field);
  }
}

void EditMeshBoundingBox::clear_vals()
{
  auto state = get_state();
  const std::string cleared("---");
  state->setValue(InputCenterX, cleared);
  state->setValue(InputCenterY, cleared);
  state->setValue(InputCenterZ, cleared);
  state->setValue(InputSizeX, cleared);
  state->setValue(InputSizeY, cleared);
  state->setValue(InputSizeZ, cleared);
}

namespace
{
  std::string convertForLabel(double coord)
  {
    return str(boost::format("%8.4f") % coord);
  }
}

void EditMeshBoundingBox::update_input_attributes(FieldHandle f)
{
  bbox_ = f->vmesh()->get_bounding_box();

  if (!bbox_.valid())
  {
    warning("Input field is empty -- using unit cube.");
    bbox_.extend(Point(0, 0, 0));
    bbox_.extend(Point(1, 1, 1));
  }
  auto size = bbox_.diagonal();
  auto center = bbox_.center();

  auto state = get_state();
  state->setValue(InputCenterX, convertForLabel(center.x()));
  state->setValue(InputCenterY, convertForLabel(center.y()));
  state->setValue(InputCenterZ, convertForLabel(center.z()));
  state->setValue(InputSizeX, convertForLabel(size.x()));
  state->setValue(InputSizeY, convertForLabel(size.y()));
  state->setValue(InputSizeZ, convertForLabel(size.z()));
}

GeometryBaseHandle EditMeshBoundingBox::buildGeometryObject()
{
  auto colorScheme(ColorScheme::COLOR_UNIFORM);
  //get all the bbox edges
  Point c,r,d,b;
  box_->getPosition(c,r,d,b);
  auto x = r - c, y = d - c, z = b - c;
  std::vector<Point> points;
  points.resize(8);
  points.at(0) = c + x + y + z;
  points.at(1) = c + x + y - z;
  points.at(2) = c + x - y + z;
  points.at(3) = c + x - y - z;
  points.at(4) = c - x + y + z;
  points.at(5) = c - x + y - z;
  points.at(6) = c - x - y + z;
  points.at(7) = c - x - y - z;
  uint32_t point_indicies[] = {
    0, 1, 0, 2, 0, 4,
    7, 6, 7, 5, 3, 7,
    4, 5, 4, 6, 1, 5,
    3, 2, 3, 1, 2, 6
  };
  auto state = get_state();
  auto scale = state->getValue(Scale).toDouble();
  auto num_strips = 50;
  std::vector<Vector> tri_points;
  std::vector<Vector> tri_normals;
  std::vector<uint32_t> tri_indices;
  std::vector<ColorRGB> colors;
  GlyphGeom glyphs;
  //generate triangles for the cylinders.
  for (auto edge = 0; edge < 24; edge += 2)
  {
    glyphs.addCylinder(points[point_indicies[edge]], points[point_indicies[edge + 1]], scale, num_strips, ColorRGB(), ColorRGB());
  }
  //generate triangles for the spheres
  for (const auto& a : points)
  {
    glyphs.addSphere(a, scale, num_strips, ColorRGB(1, 0, 0));
  }

  std::stringstream ss;
  ss << scale;
  for (const auto& a : points) ss << a.x() << a.y() << a.z();

  auto uniqueNodeID = "bounding_box_cylinders" + ss.str();

  RenderState renState;

  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(1, 1, 1);
  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);

  GeometryHandle geom(new GeometryObjectSpire(*this, "BoundingBox", true));

  glyphs.buildObject(geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, bbox_);

  return geom;
}

void EditMeshBoundingBox::computeWidgetBox(const BBox& box) const
{
  auto bbox(box);
  if (!bbox.valid())
  {
    warning("Input field is empty -- using unit cube.");
    bbox.extend(Point(0, 0, 0));
    bbox.extend(Point(1, 1, 1));
  }

  // build a widget identical to the BBox
  auto size = Vector(bbox.get_max() - bbox.get_min());
  const auto SMALL = 1e-4;
  if (fabs(size.x())<SMALL)
  {
    size.x(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(SMALL, 0.0, 0.0));
    bbox.extend(bbox.get_max() + Vector(SMALL, 0.0, 0.0));
  }
  if (fabs(size.y())<SMALL)
  {
    size.y(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(0.0, SMALL, 0.0));
    bbox.extend(bbox.get_max() + Vector(0.0, SMALL, 0.0));
  }
  if (fabs(size.z())<SMALL)
  {
    size.z(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(0.0, 0.0, SMALL));
    bbox.extend(bbox.get_max() + Vector(0.0, 0.0, SMALL));
  }
  auto center = Point(bbox.get_min() + size / 2.);

  Vector sizex(size.x(), 0, 0);
  Vector sizey(0, size.y(), 0);
  Vector sizez(0, 0, size.z());

  auto right(center + sizex / 2.);
  auto down(center + sizey / 2.);
  auto in(center + sizez / 2.);

  box_->setPosition(center, right, down, in);
}

namespace
{
  void checkForVerySmall(double& size)
  {
    const auto VERY_SMALL = 1e-12;
    if (size < VERY_SMALL) size = 1.0;
  }
}

void EditMeshBoundingBox::executeImpl(FieldHandle inputField)
{
  auto state = get_state();

  if (!transient_value_cast<bool>(state->getTransientValue(ScaleChanged)))
  {
    computeWidgetBox(inputField->vmesh()->get_bounding_box());

    Point initialWidgetCenter, initialWidgetRight, initialWidgetDown, initialWidgetIn;
    box_->getPosition(initialWidgetCenter, initialWidgetRight, initialWidgetDown, initialWidgetIn);

    auto initialXSize = (initialWidgetRight - initialWidgetCenter).length();
    auto initialYSize = (initialWidgetDown - initialWidgetCenter).length();
    auto initialZSize = (initialWidgetIn - initialWidgetCenter).length();

    checkForVerySmall(initialXSize);
    checkForVerySmall(initialYSize);
    checkForVerySmall(initialZSize);

    Transform r_transformThatIsAppliedSomewhere;
    {
      impl_->field_initial_transform_.load_identity();
      impl_->field_initial_transform_.pre_scale(Vector(initialXSize, initialYSize, initialZSize));
      r_transformThatIsAppliedSomewhere.load_frame((initialWidgetRight - initialWidgetCenter).safe_normal(),
        (initialWidgetDown - initialWidgetCenter).safe_normal(),
        (initialWidgetIn - initialWidgetCenter).safe_normal());

      impl_->field_initial_transform_.pre_trans(r_transformThatIsAppliedSomewhere);
      impl_->field_initial_transform_.pre_translate(Vector(initialWidgetCenter));
    }

    Vector outputFieldSizeX, outputFieldSizeY, outputFieldSizeZ;
    if (transient_value_cast<bool>(state->getTransientValue(ResetSize)))
    {
      outputFieldSizeX = Vector(initialXSize * 2, 0, 0);
      outputFieldSizeY = Vector(0, initialYSize * 2, 0);
      outputFieldSizeZ = Vector(0, 0, initialZSize * 2);
    }
    else
    {
      state->setValue(OutputSizeX, std::fabs(state->getValue(OutputSizeX).toDouble()));
      state->setValue(OutputSizeY, std::fabs(state->getValue(OutputSizeY).toDouble()));
      state->setValue(OutputSizeZ, std::fabs(state->getValue(OutputSizeZ).toDouble()));

      outputFieldSizeX = Vector(state->getValue(OutputSizeX).toDouble(), 0, 0);
      outputFieldSizeY = Vector(0, state->getValue(OutputSizeY).toDouble(), 0);
      outputFieldSizeZ = Vector(0, 0, state->getValue(OutputSizeZ).toDouble());
    }

    auto newWidgetCenter = initialWidgetCenter;
    if (!transient_value_cast<bool>(state->getTransientValue(ResetCenter)))
    {
      newWidgetCenter = Point(state->getValue(OutputCenterX).toDouble(),
                              state->getValue(OutputCenterY).toDouble(),
                              state->getValue(OutputCenterZ).toDouble());
    }

    auto newWidgetRight(newWidgetCenter + outputFieldSizeX / 2.);
    auto newWidgetDown(newWidgetCenter + outputFieldSizeY / 2.);
    auto newWidgetIn(newWidgetCenter + outputFieldSizeZ / 2.);

    box_->setPosition(newWidgetCenter, newWidgetRight, newWidgetDown, newWidgetIn);

    r_transformThatIsAppliedSomewhere.load_frame((newWidgetRight - newWidgetCenter).safe_normal(),
      (newWidgetDown - newWidgetCenter).safe_normal(),
      (newWidgetIn - newWidgetCenter).safe_normal());

    // Change the input field handle here.
    FieldHandle output(inputField->deep_clone());

    Transform transformAppliedToOutputMesh;
    transformAppliedToOutputMesh.load_identity();
    Vector sizeHalf((newWidgetRight - newWidgetCenter).length(),
      (newWidgetDown - newWidgetCenter).length(),
      (newWidgetIn - newWidgetCenter).length());
    transformAppliedToOutputMesh.pre_scale(sizeHalf);

    transformAppliedToOutputMesh.pre_trans(r_transformThatIsAppliedSomewhere);
    transformAppliedToOutputMesh.pre_translate(Vector(newWidgetCenter));

    auto inv(impl_->field_initial_transform_);
    inv.invert();
    transformAppliedToOutputMesh.post_trans(inv);

    // Change the input field handle here.
    output->vmesh()->transform(transformAppliedToOutputMesh);

    state->setValue(OutputSizeX, sizeHalf.x() * 2);
    state->setValue(OutputSizeY, sizeHalf.y() * 2);
    state->setValue(OutputSizeZ, sizeHalf.z() * 2);
    state->setValue(OutputCenterX, newWidgetCenter.x());
    state->setValue(OutputCenterY, newWidgetCenter.y());
    state->setValue(OutputCenterZ, newWidgetCenter.z());

    // Convert the transform into a matrix and send it out.
    MatrixHandle mh(new DenseMatrix(transformAppliedToOutputMesh));
    sendOutput(Transformation_Matrix, mh);

    //state->setValue(Resetting, false);
    widgetMoved_ = false;

    sendOutput(OutputField, output);
  }
  state->setTransientValue(ScaleChanged, false);
  state->setTransientValue(SetOutputCenter, false);
  state->setTransientValue(ResetCenter, false);
  state->setTransientValue(SetOutputSize, false);
  state->setTransientValue(ResetSize, false);

  sendOutput(Transformation_Widget, buildGeometryObject());
}

BoxWidgetPtr WidgetFactory::createBox()
{
  return boost::make_shared<BoxWidgetNull>();
}

const AlgorithmParameterName EditMeshBoundingBox::ResetCenter("ResetCenter");
const AlgorithmParameterName EditMeshBoundingBox::ResetSize("ResetSize");

const AlgorithmParameterName EditMeshBoundingBox::InputCenterX("InputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterY("InputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterZ("InputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeX("InputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeY("InputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeZ("InputSizeZ");
 //Output Field Atributes
const AlgorithmParameterName EditMeshBoundingBox::SetOutputCenter("SetOutputCenter");
const AlgorithmParameterName EditMeshBoundingBox::SetOutputSize("SetOutputSize");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterX("OutputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterY("OutputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterZ("OutputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeX("OutputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeY("OutputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeZ("OutputSizeZ");
//Widget Scale/Mode
const AlgorithmParameterName EditMeshBoundingBox::Scale("Scale");
const AlgorithmParameterName EditMeshBoundingBox::ScaleChanged("ScaleChanged");
const AlgorithmParameterName EditMeshBoundingBox::NoTranslation("NoTranslation");
const AlgorithmParameterName EditMeshBoundingBox::XYZTranslation("XYZTranslation");
const AlgorithmParameterName EditMeshBoundingBox::RDITranslation("RDITranslation");
const AlgorithmParameterName EditMeshBoundingBox::RestrictX("RestrictX");
const AlgorithmParameterName EditMeshBoundingBox::RestrictY("RestrictY");
const AlgorithmParameterName EditMeshBoundingBox::RestrictZ("RestrictZ");
const AlgorithmParameterName EditMeshBoundingBox::RestrictR("RestrictR");
const AlgorithmParameterName EditMeshBoundingBox::RestrictD("RestrictD");
const AlgorithmParameterName EditMeshBoundingBox::RestrictI("RestrictI");

const AlgorithmParameterName EditMeshBoundingBox::BoxMode("BoxMode");
const AlgorithmParameterName EditMeshBoundingBox::BoxRealScale("BoxRealScale");

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
  BoxWidgetNull(bool print = false) : print_(print) {}
  virtual void connect(OutputPortHandle port) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::connect called" << std::endl;
  }
  virtual void setRestrictX(bool restrict) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setRestrictX called with " << restrict << std::endl;
  }
  virtual void setRestrictY(bool restrict) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setRestrictY called with " << restrict << std::endl;
  }
  virtual void setRestrictZ(bool restrict) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setRestrictZ called with " << restrict << std::endl;
  }
  virtual void setRestrictR(bool restrict) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setRestrictR called with " << restrict << std::endl;
  }
  virtual void setRestrictD(bool restrict) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setRestrictD called with " << restrict << std::endl;
  }
  virtual void setRestrictI(bool restrict) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setRestrictI called with " << restrict << std::endl;
  }
  virtual void unrestrictTranslation() override
  {
    if (print_)
      std::cout << "BoxWidgetNull::unrestrictTranslation called" << std::endl;
  }
  virtual void restrictTranslationXYZ() override
  {
    if (print_)
      std::cout << "BoxWidgetNull::restrictTranslationXYZ called" << std::endl;
  }
  virtual void restrictTranslationRDI() override
  {
    if (print_)
      std::cout << "BoxWidgetNull::restrictTranslationRDI called" << std::endl;
  }
  virtual void setPosition(const Point& center, const Point& right, const Point& down, const Point& in) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setPosition called: " << center << " " << right << " " << down << " " << in << std::endl;
    center_ = center;
    right_ = right;
    down_ = down;
    in_ = in;
  }
  virtual void getPosition(Point& center, Point& right, Point& down, Point& in) const override
  {
    if (print_)
      std::cout << "BoxWidgetNull::getPosition called: " << std::endl;
    center = center_;
    right = right_;
    down = down_;
    in = in_;
  }
  virtual void setScale(double scale) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setScale called with " << scale << std::endl;
  }
  virtual void setCurrentMode(int mode) override
  {
    if (print_)
      std::cout << "BoxWidgetNull::setCurrentMode called with " << mode << std::endl;
  }

private:
  Point center_, right_, down_, in_;
  bool print_;
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
        Transform box_initial_transform_;
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
    enqueueExecuteAgain();
  }
}

void EditMeshBoundingBox::adjustGeometryFromTransform(const Core::Geometry::Transform& transformMatrix)
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
  box_->connect(getOutputPort(Transformation_Widget));
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
  state->setValue(UseOutputCenter, false);
  state->setValue(UseOutputSize, false);
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
  state->setValue(Resetting, false);
  state->setValue(BoxRealScale, 0.0);
  state->setValue(BoxMode, 0);

  createBoxWidget();
  setBoxRestrictions();

  getOutputPort(Transformation_Widget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void EditMeshBoundingBox::execute()
{
  //TODO: need version to pass a func for ifNull case--fancy but useful. For now just reset each time.
  setBoxRestrictions();
  auto field = getRequiredInput(InputField);

  if (needToExecute())
  {
    clear_vals();
    update_state(Executing);
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

void EditMeshBoundingBox::updateOutputAttributes(const BBox& box)
{
  auto size = box.diagonal();
  auto center = box.center();
  auto state = get_state();
  const bool useOutputSize = state->getValue(UseOutputSize).toBool();
  const bool useOutputCenter = state->getValue(UseOutputCenter).toBool();

  state->setValue(OutputCenterX, convertForLabel(center.x()));
  if (!useOutputCenter) state->setValue(OutputCenterX, center.x());

  state->setValue(OutputCenterY, convertForLabel(center.y()));
  if (!useOutputCenter) state->setValue(OutputCenterY, center.y());

  state->setValue(OutputCenterZ, convertForLabel(center.z()));
  if (!useOutputCenter) state->setValue(OutputCenterZ, center.z());

  state->setValue(OutputSizeX, convertForLabel(size.x()));
  if (!useOutputSize) state->setValue(OutputSizeX, size.x());

  state->setValue(OutputSizeY, convertForLabel(size.y()));
  if (!useOutputSize) state->setValue(OutputSizeY, size.y());

  state->setValue(OutputSizeZ, convertForLabel(size.z()));
  if (!useOutputSize) state->setValue(OutputSizeZ, size.z());
}

bool EditMeshBoundingBox::isBoxEmpty() const
{
  Point c,r,d,b;
  box_->getPosition(c,r,d,b);
  return (c == r) || (c == d) || (c == b);
}

GeometryBaseHandle EditMeshBoundingBox::buildGeometryObject()
{
  ColorScheme colorScheme(ColorScheme::COLOR_UNIFORM);
  std::vector<std::pair<Point,Point>> bounding_edges;
  //get all the bbox edges
  Point c,r,d,b;
  box_->getPosition(c,r,d,b);
  Vector x = r - c, y = d - c, z = b - c;
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
  double scale = state->getValue(Scale).toDouble();
  int num_strips = 50;
  std::vector<Vector> tri_points;
  std::vector<Vector> tri_normals;
  std::vector<uint32_t> tri_indices;
  std::vector<ColorRGB> colors;
  GlyphGeom glyphs;
  //generate triangles for the cylinders.
  for (int edge = 0; edge < 24; edge += 2)
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
  for (auto a : points) ss << a.x() << a.y() << a.z();

  std::string uniqueNodeID = "bounding_box_cylinders" + std::string(ss.str().c_str());

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

void
EditMeshBoundingBox::build_widget(FieldHandle f, bool reset)
{
  if (isBoxEmpty())
  {
    Point center;
    Vector size;
    BBox bbox = f->vmesh()->get_bounding_box();
    if (!bbox.valid())
    {
      warning("Input field is empty -- using unit cube.");
      bbox.extend(Point(0, 0, 0));
      bbox.extend(Point(1, 1, 1));
    }

    // build a widget identical to the BBox
    size = Vector(bbox.get_max() - bbox.get_min());
    const double SMALL = 1e-4;
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
    center = Point(bbox.get_min() + size / 2.);

    Vector sizex(size.x(), 0, 0);
    Vector sizey(0, size.y(), 0);
    Vector sizez(0, 0, size.z());

    Point right(center + sizex / 2.);
    Point down(center + sizey / 2.);
    Point in(center + sizez / 2.);

    // Translate * Rotate * Scale.
    Transform r;
    impl_->box_initial_transform_.load_identity();
    impl_->box_initial_transform_.pre_scale(Vector((right - center).length(),
      (down - center).length(),
      (in - center).length()));
    r.load_frame((right - center).safe_normal(),
      (down - center).safe_normal(),
      (in - center).safe_normal());
    impl_->box_initial_transform_.pre_trans(r);
    impl_->box_initial_transform_.pre_translate(Vector(center));

    auto state = get_state();
    const double newscale = size.length() * 0.015;
    double bscale = state->getValue(BoxRealScale).toDouble();
    if (bscale < newscale * 1e-2 || bscale > newscale * 1e2)
    {
      bscale = newscale;
    }
    box_->setScale(bscale); // callback sets box_scale for us.
    box_->setPosition(center, right, down, in);
    box_->setCurrentMode(state->getValue(BoxMode).toInt());
  }
}

void EditMeshBoundingBox::setBoxRestrictions()
{
  auto state = get_state();
  box_->setRestrictX(state->getValue(RestrictX).toBool());
  box_->setRestrictY(state->getValue(RestrictY).toBool());
  box_->setRestrictZ(state->getValue(RestrictZ).toBool());
  box_->setRestrictR(state->getValue(RestrictR).toBool());
  box_->setRestrictD(state->getValue(RestrictD).toBool());
  box_->setRestrictI(state->getValue(RestrictI).toBool());
  if (state->getValue(NoTranslation).toBool())
    box_->unrestrictTranslation();
  else if (state->getValue(XYZTranslation).toBool())
    box_->restrictTranslationXYZ();
  else if (state->getValue(RDITranslation).toBool())
    box_->restrictTranslationRDI();
}

void EditMeshBoundingBox::executeImpl(FieldHandle fh)
{
  auto state = get_state();
  {
    build_widget(fh, state->getValue(Resetting).toBool());
    BBox bbox = fh->vmesh()->get_bounding_box();
    if (!bbox.valid())
    {
      warning("Input field is empty -- using unit cube.");
      bbox.extend(Point(0, 0, 0));
      bbox.extend(Point(1, 1, 1));
    }
    Vector size(bbox.get_max() - bbox.get_min());
    const double SMALL = 1.e-4;
    if (fabs(size.x()) < SMALL)
    {
      size.x(2 * SMALL);
      bbox.extend(bbox.get_min() - Vector(SMALL, 0, 0));
    }
    if (fabs(size.y()) < SMALL)
    {
      size.y(2 * SMALL);
      bbox.extend(bbox.get_min() - Vector(0, SMALL, 0));
    }
    if (fabs(size.z()) < SMALL)
    {
      size.z(2 * SMALL);
      bbox.extend(bbox.get_min() - Vector(0, 0, SMALL));
    }
    Point center(bbox.get_min() + size / 2.);
    Vector sizex(size.x(), 0, 0);
    Vector sizey(0, size.y(), 0);
    Vector sizez(0, 0, size.z());

    Point right(center + sizex / 2.);
    Point down(center + sizey / 2.);
    Point in(center + sizez / 2.);

    Transform r;
    impl_->field_initial_transform_.load_identity();

    double sx = (right - center).length();
    double sy = (down - center).length();
    double sz = (in - center).length();

    const double VERY_SMALL = 1e-12;
    if (sx < VERY_SMALL) sx = 1.0;
    if (sy < VERY_SMALL) sy = 1.0;
    if (sz < VERY_SMALL) sz = 1.0;

    impl_->field_initial_transform_.pre_scale(Vector(sx, sy, sz));
    r.load_frame((right - center).safe_normal(),
      (down - center).safe_normal(),
      (in - center).safe_normal());

    impl_->field_initial_transform_.pre_trans(r);
    impl_->field_initial_transform_.pre_translate(Vector(center));

    state->setValue(Resetting, false);
  }

  const bool useOutputSize = state->getValue(UseOutputSize).toBool();
  const bool useOutputCenter = state->getValue(UseOutputCenter).toBool();

  if (useOutputSize || useOutputCenter || widgetMoved_)
  {
    Point center, right, down, in;

    state->setValue(OutputSizeX, std::fabs(state->getValue(OutputSizeX).toDouble()));
    state->setValue(OutputSizeY, std::fabs(state->getValue(OutputSizeY).toDouble()));
    state->setValue(OutputSizeZ, std::fabs(state->getValue(OutputSizeZ).toDouble()));

    Vector sizex, sizey, sizez;
    box_->getPosition(center, right, down, in);
    if (useOutputSize)
    {
      sizex = Vector(state->getValue(OutputSizeX).toDouble(), 0, 0);
      sizey = Vector(0, state->getValue(OutputSizeY).toDouble(), 0);
      sizez = Vector(0, 0, state->getValue(OutputSizeZ).toDouble());
    }
    else
    {
      sizex = (right - center) * 2;
      sizey = (down - center) * 2;
      sizez = (in - center) * 2;
    }
    if (useOutputCenter || widgetMoved_)
    {
      center = Point(state->getValue(OutputCenterX).toDouble(),
        state->getValue(OutputCenterY).toDouble(),
        state->getValue(OutputCenterZ).toDouble());
    }
    right = Point(center + sizex / 2.);
    down = Point(center + sizey / 2.);
    in = Point(center + sizez / 2.);

    box_->setPosition(center, right, down, in);

    widgetMoved_ = false;
  }

  // Transform the mesh if necessary.
  // Translate * Rotate * Scale.
  Point center, right, down, in;
  box_->getPosition(center, right, down, in);

  Transform t, r;
  t.load_identity();
  t.pre_scale(Vector((right - center).length(),
    (down - center).length(),
    (in - center).length()));
  r.load_frame((right - center).safe_normal(),
    (down - center).safe_normal(),
    (in - center).safe_normal());
  t.pre_trans(r);
  t.pre_translate(Vector(center));

  auto inv(impl_->field_initial_transform_);
  inv.invert();
  t.post_trans(inv);

  // Change the input field handle here.
  FieldHandle output(fh->deep_clone());
  output->vmesh()->transform(t);

  sendOutput(OutputField, output);

  // Convert the transform into a matrix and send it out.
  MatrixHandle mh(new DenseMatrix(t));
  sendOutput(Transformation_Matrix, mh);
  sendOutput(Transformation_Widget, buildGeometryObject());
}

void EditMeshBoundingBox::widget_moved(bool last)
{
  if (last)
  {
    Point center, right, down, in;
    box_->getPosition(center, right, down, in);
    auto state = get_state();
    state->setValue(OutputCenterX, center.x());
    state->setValue(OutputCenterY, center.y());
    state->setValue(OutputCenterZ, center.z());
    state->setValue(OutputSizeX, (right.x() - center.x())*2.);
    state->setValue(OutputSizeY, (down.y() - center.y())*2.);
    state->setValue(OutputSizeZ, (in.z() - center.z())*2.);
  }
}

BoxWidgetPtr WidgetFactory::createBox()
{
  return boost::make_shared<BoxWidgetNull>();
}

const AlgorithmParameterName EditMeshBoundingBox::Resetting("Resetting");

const AlgorithmParameterName EditMeshBoundingBox::InputCenterX("InputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterY("InputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterZ("InputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeX("InputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeY("InputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeZ("InputSizeZ");
 //Output Field Atributes
const AlgorithmParameterName EditMeshBoundingBox::UseOutputCenter("UseOutputCenter");
const AlgorithmParameterName EditMeshBoundingBox::UseOutputSize("UseOutputSize");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterX("OutputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterY("OutputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterZ("OutputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeX("OutputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeY("OutputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeZ("OutputSizeZ");
//Widget Scale/Mode
const AlgorithmParameterName EditMeshBoundingBox::Scale("Scale");
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

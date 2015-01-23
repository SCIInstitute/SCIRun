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

#include <Modules/Fields/EditMeshBoundingBox.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Modules/Fields/BoxWidgetTypes.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;

const ModuleLookupInfo EditMeshBoundingBox::staticInfo_("EditMeshBoundingBox", "ChangeMesh", "SCIRun");

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
        Transform box_initial_transform_;
        Transform field_initial_transform_;
        BBox box_initial_bounds_;
      };
    }
  }
}


EditMeshBoundingBox::EditMeshBoundingBox() : Module(staticInfo_),
  impl_(new EditMeshBoundingBoxImpl)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Transformation_Widget);
  INITIALIZE_PORT(Transformation_Matrix);
}

void EditMeshBoundingBox::createBoxWidget()
{
  box_ = WidgetFactory::createBox();  //new BoxWidget(this, &widget_lock_, 1.0, false, false);
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
  state->setValue(OutputSizeX, 0.0);
  state->setValue(OutputSizeY, 0.0);
  state->setValue(OutputSizeZ, 0.0);

  //TODO

  createBoxWidget();
  setBoxRestrictions();
}

void EditMeshBoundingBox::execute()
{
  //TODO: need version to pass a func for ifNull case--fancy but useful. For now just reset each time.
  clear_vals();
  setBoxRestrictions();
  auto field = getRequiredInput(InputField);

  if (needToExecute())
  {
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

void EditMeshBoundingBox::update_input_attributes(FieldHandle f)
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
  size = bbox.diagonal();
  center = bbox.center();

  auto state = get_state();
  state->setValue(InputCenterX, boost::lexical_cast<std::string>(center.x()));
  state->setValue(InputCenterY, boost::lexical_cast<std::string>(center.y()));
  state->setValue(InputCenterZ, boost::lexical_cast<std::string>(center.z()));
  state->setValue(InputSizeX, boost::lexical_cast<std::string>(size.x()));
  state->setValue(InputSizeY, boost::lexical_cast<std::string>(size.y()));
  state->setValue(InputSizeZ, boost::lexical_cast<std::string>(size.z()));
}

bool EditMeshBoundingBox::isBoxEmpty() const
{
  //TODO
  return true;
  /*
  reset || box_scale_.get() <= 0 ||
    (box_center_.get() == Point(0.0, 0.0, 0.0) &&
    box_right_.get() == Point(0.0, 0.0, 0.0) &&
    box_down_.get() == Point(0.0, 0.0, 0.0) &&
    box_in_.get() == Point(0.0, 0.0, 0.0)))
    */
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
    impl_->box_initial_bounds_ = bbox;

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
#ifdef WORKING_ON_EDITMESH
    box_center_.set(center);
    box_right_.set(right);
    box_down_.set(down);
    box_in_.set(in);
    box_scale_.set(-1.0);
#endif
  }
  else
  {
#ifdef WORKING_ON_EDITMESH
    const double l2norm = (box_right_.get().vector() +
      box_down_.get().vector() +
      box_in_.get().vector()).length();
    const double newscale = l2norm * 0.015;
    double bscale = box_real_scale_.get();
    if (bscale < newscale * 1e-2 || bscale > newscale * 1e2)
    {
      bscale = newscale;
    }
    box_->setScale(bscale); // callback sets box_scale for us.
    box_->setPosition(box_center_.get(), box_right_.get(),
      box_down_.get(), box_in_.get());
    box_->setCurrentMode(box_mode_.get());
#endif
  }
#ifdef WORKING_ON_EDITMESH
  GeomGroup *widget_group = new GeomGroup;
  widget_group->add(box_->GetWidget());

  GeometryOPortHandle ogport;
  get_oport_handle("Transformation Widget", ogport);
  widgetid_ = ogport->addObj(widget_group, "EditMeshBoundingBox Transform widget", &widget_lock_);
  ogport->flushViews();
#endif
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
    Point unused;
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
  if (useOutputSize || useOutputCenter)
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
    if (useOutputCenter)
    {
      center = Point(state->getValue(OutputCenterX).toDouble(),
        state->getValue(OutputCenterY).toDouble(),
        state->getValue(OutputCenterZ).toDouble());
    }
    right = Point(center + sizex / 2.);
    down = Point(center + sizey / 2.);
    in = Point(center + sizez / 2.);

    box_->setPosition(center, right, down, in);
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

  Transform inv(impl_->field_initial_transform_);
  inv.invert();
  t.post_trans(inv);

  // Change the input field handle here.
  FieldHandle output(fh->deep_clone());
  output->vmesh()->transform(t);

  sendOutput(OutputField, output);

  // Convert the transform into a matrix and send it out.
  MatrixHandle mh(new DenseMatrix(t));
  sendOutput(Transformation_Matrix, mh);
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
#ifdef WORKING_ON_EDITMESH
    state->setValue(BoxMode, box_mode_.set(box_->GetMode());
    box_center_.set(center);
    box_right_.set(right);
    box_down_.set(down);
    box_in_.set(in);
    box_scale_.set(1.0);
    want_to_execute();
#endif
  }
#ifdef WORKING_ON_EDITMESH
  box_real_scale_.set(box_->GetScale());
#endif
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
const AlgorithmParameterName EditMeshBoundingBox::DoubleScaleUp("DoubleScaleUp");
const AlgorithmParameterName EditMeshBoundingBox::ScaleUp("ScaleUp");
const AlgorithmParameterName EditMeshBoundingBox::ScaleDown("ScaleDown");
const AlgorithmParameterName EditMeshBoundingBox::DoubleScaleDown("DoubleScaleDown");
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


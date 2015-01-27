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

EditMeshBoundingBox::EditMeshBoundingBox()
: Module(staticInfo_),
  cylinder_scale_(1.0),
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
  state->setValue(Scale, 1.0);

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

  bbox_ = f->vmesh()->get_bounding_box();

  if (!bbox_.valid())
  {
    warning("Input field is empty -- using unit cube.");
    bbox_.extend(Point(0, 0, 0));
    bbox_.extend(Point(1, 1, 1));
  }
  size = bbox_.diagonal();
  center = bbox_.center();
  box_->setPosition(center,
                    center + Vector(size.x() / 2., 0, 0),
                    center + Vector(0, size.y() / 2., 0),
                    center + Vector(0, 0, size.z() / 2.));

    auto state = get_state();
    char s[32];
    sprintf(s, "%8.4f",center.x());
    state->setValue(InputCenterX, boost::lexical_cast<std::string>(s));
    sprintf(s, "%8.4f",center.y());
    state->setValue(InputCenterY, boost::lexical_cast<std::string>(s));
    sprintf(s, "%8.4f",center.z());
    state->setValue(InputCenterZ, boost::lexical_cast<std::string>(s));
    sprintf(s, "%8.4f",size.x());
    state->setValue(InputSizeX, boost::lexical_cast<std::string>(s));
    sprintf(s, "%8.4f",size.y());
    state->setValue(InputSizeY, boost::lexical_cast<std::string>(s));
    sprintf(s, "%8.4f",size.z());
    state->setValue(InputSizeZ, boost::lexical_cast<std::string>(s));
}

bool EditMeshBoundingBox::isBoxEmpty() const
{
  Point c,r,d,b;
  box_->getPosition(c,r,d,b);
  return (c == r) || (c == d) || (c == b);
}

Core::Datatypes::GeometryHandle EditMeshBoundingBox::buildGeometryObject() {
    
    Core::Datatypes::GeometryHandle geom(new Core::Datatypes::GeometryObject(NULL));
    
    GeometryObject::ColorScheme colorScheme(GeometryObject::COLOR_UNIFORM);
    int64_t numVBOElements = 0;
    std::vector<std::pair<Point,Point>> bounding_edges;
    //get all the bbox edges
    Point c,r,d,b;
    box_->getPosition(c,r,d,b);
    Vector x = r - c, y = d - c, z = b - c;
    std::vector<Point> points;
    points.resize(8);
    points.at(0) = c+x+y+z;
    points.at(1) = c+x+y-z;
    points.at(2) = c+x-y+z;
    points.at(3) = c+x-y-z;
    points.at(4) = c-x+y+z;
    points.at(5) = c-x+y-z;
    points.at(6) = c-x-y+z;
    points.at(7) = c-x-y-z;
    uint32_t point_indicies[] = {
        0,1,0,2,0,4,
        7,6,7,5,3,7,
        4,5,4,6,1,5,
        3,2,3,1,2,6
    };
    auto state = get_state();
    double scale = state->getValue(Scale).toDouble();
    if (scale < 0) scale *= -1.;
    int num_strips = int(30. * scale);
    std::vector<Vector> tri_points;
    std::vector<Vector> tri_normals;
    std::vector<uint32_t> tri_indices;
    //generate triangles for the cylinders.
    for (int edge = 0; edge < 24; edge +=2) {
        Vector c1,c2;
        c1 = Vector(points[point_indicies[edge]]);
        c2 = Vector(points[point_indicies[edge+1]]);
        Vector n(c1 - c2), u = Vector(1,0,0);
        n.normalize();
        if (n == u)
            u = Vector(0,1,0);
        if (n == u)
            u = Vector(0,0,1);
        Vector crx = Cross(u,n);
        Vector p;
        for(int strips = 0; strips <= num_strips; strips++) {
            uint32_t offset = (uint32_t)numVBOElements;
            p = std::cos(2. * M_PI * (double)strips / (double)num_strips) * u +
            std::sin(2. * M_PI * (double)strips / (double)num_strips) * crx;
            tri_points.push_back(scale * p + c1);
            numVBOElements++;
            tri_points.push_back(scale * p + c2);
            numVBOElements++;
            tri_normals.push_back(p);
            tri_normals.push_back(p);
            if (strips < num_strips) {
                tri_indices.push_back( 0 + offset);
                tri_indices.push_back( 1 + offset);
                tri_indices.push_back( 2 + offset);
                tri_indices.push_back( 2 + offset);
                tri_indices.push_back( 1 + offset);
                tri_indices.push_back( 3 + offset);
            }
        }
    }
    //generate triangles for the spheres
    Vector p1,p2;
    double theta_inc = 2. * M_PI / double(num_strips), phi_inc = M_PI / double(num_strips);
    for (auto a : points) {
        for (double phi = 0.; phi <= M_PI; phi += phi_inc ) {
            for (double theta = 0.; theta <= 2. * M_PI; theta += theta_inc) {
                uint32_t offset = (uint32_t)numVBOElements;
                p1 = Vector(sin(theta) * cos(phi),sin(theta) * sin(phi),cos(theta));
                p2 = Vector(sin(theta) * cos(phi+phi_inc),sin(theta) * sin(phi+phi_inc),cos(theta));
                tri_points.push_back(scale * p1 + Vector(a));
                numVBOElements++;
                tri_points.push_back(scale * p2 + Vector(a));
                numVBOElements++;
                tri_normals.push_back(p1);
                tri_normals.push_back(p2);
                if (theta+theta_inc < 2. * M_PI) {
                    tri_indices.push_back( 0 + offset);
                    tri_indices.push_back( 1 + offset);
                    tri_indices.push_back( 2 + offset);
                    tri_indices.push_back( 2 + offset);
                    tri_indices.push_back( 1 + offset);
                    tri_indices.push_back( 3 + offset);
                }
            }
        }
    }
    
    
    // Attempt some form of precalculation of iboBuffer and vboBuffer size.
    uint32_t iboSize = (uint32_t)(tri_indices.size() * sizeof(uint32_t));
    uint32_t vboSize = (uint32_t)(tri_points.size() * 2 * 3 * sizeof(float));
    
    
    /// \todo To reduce memory requirements, we can use a 16bit index buffer.
    
    /// \todo To further reduce a large amount of memory, get rid of the index
    ///       buffer and use glDrawArrays to render without an IBO. An IBO is
    ///       a waste of space.
    ///       http://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml
    
    /// \todo Switch to unique_ptrs and move semantics.
    std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(
                                                                new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));
    std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(
                                                                new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
    
    // Accessing the pointers like this is contrived. We only do this for
    // speed since we will be using the pointers in a tight inner loop.
    CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
    CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();
    
    
    //write to the IBO/VBOs
    for(size_t i = 0; i < tri_indices.size(); i++)
        iboBuffer->write(tri_indices[i]);
    
    for (size_t i = 0; i < tri_points.size(); i++)
    {
        // Write first point on line
        vboBuffer->write(static_cast<float>(tri_points.at(i).x()));
        vboBuffer->write(static_cast<float>(tri_points.at(i).y()));
        vboBuffer->write(static_cast<float>(tri_points.at(i).z()));
        // Write normal
        vboBuffer->write(static_cast<float>(tri_normals.at(i).x()));
        vboBuffer->write(static_cast<float>(tri_normals.at(i).y()));
        vboBuffer->write(static_cast<float>(tri_normals.at(i).z()));
    }
    
    std::string uniqueNodeID = "bounding_edge_face";
    std::string vboName      = uniqueNodeID + "VBO";
    std::string iboName      = uniqueNodeID + "IBO";
    std::string passName     = uniqueNodeID + "Pass";
    
    // Construct VBO.
    std::string shader = "Shaders/DirPhong";
    std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
    GeometryObject::RenderType renderType = GeometryObject::RENDER_VBO_IBO;
    
    // If true, then the VBO will be placed on the GPU. We don't want to place
    // VBOs on the GPU when we are generating rendering lists.
    GeometryObject::SpireVBO geomVBO = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
                                                                numVBOElements, bbox_, true);
    geom->mVBOs.push_back(geomVBO);
    
    // Construct IBO.
    GeometryObject::SpireIBO geomIBO = GeometryObject::SpireIBO(iboName,
                                                                GeometryObject::SpireIBO::TRIANGLES,
                                                                sizeof(uint32_t), iboBufferSPtr);
    geom->mIBOs.push_back(geomIBO);
    
    RenderState renState;
    
    renState.set(RenderState::IS_ON, true);
    renState.set(RenderState::USE_TRANSPARENCY, false);
    
    renState.defaultColor = ColorRGB(1,1,1);
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
    renState.set(RenderState::USE_NORMALS, true);
    
    // Construct Pass.
    GeometryObject::SpireSubPass pass =
    GeometryObject::SpireSubPass(passName, vboName, iboName, shader,
                                 colorScheme, renState, renderType, geomVBO, geomIBO);
    // Add all uniforms generated above to the pass.
    std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
                                                             glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDiffuseColor",
                                                             glm::vec4(1.f, 1.f, 1.f, 1.f)));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
                                                             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    for (const auto& uniform : uniforms) { pass.addUniform(uniform); }
    
    geom->mPasses.push_back(pass);
    
    
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


/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Fields/GeneratePointSamplesFromField.h>
#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Graphics/Glyphs/GlyphGeom.h>

using namespace SCIRun;
using namespace Core;
using namespace Datatypes;
using namespace Dataflow::Networks;
using namespace Modules::Fields;
using namespace Geometry;
using namespace Algorithms::Fields;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Fields, NumSeeds);
ALGORITHM_PARAMETER_DEF(Fields, ProbeScale);

const ModuleLookupInfo GeneratePointSamplesFromField::staticInfo_("GeneratePointSamplesFromField", "NewField", "SCIRun");

#if SCIRUN4_CODE_TO_BE_ENABLED_LATER //@cbrightsci: include real PointWidget header here
#include <Core/Thread/CrowdMonitor.h>
#include <Dataflow/Widgets/PointWidget.h>
#endif


namespace SCIRun
{
  namespace Modules
  {
    namespace Fields
    {

      class GeneratePointSamplesFromFieldImpl
      {
      public:
        //CrowdMonitor widget_lock_;
        BBox last_bounds_;

        std::vector<size_t>              widget_id_;
        //std::vector<GeomHandle>       widget_switch_;
        std::vector<PointWidgetPtr>     pointWidgets_;
        double l2norm_;

        GeometryHandle buildWidgetObject(FieldHandle field, double radius, const GeometryIDGenerator& idGenerator);
        RenderState getWidgetRenderState() const;
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
}

void GeneratePointSamplesFromField::execute()
{
  FieldHandle field = GenerateOutputField();
  sendOutput(GeneratedPoints, field);

  auto geom = impl_->buildWidgetObject(field, get_state()->getValue(Parameters::ProbeScale).toDouble(), *this);
  sendOutput(GeneratedWidget, geom);
}

FieldHandle GeneratePointSamplesFromField::GenerateOutputField()
{
  auto ifieldhandle = getRequiredInput(InputField);

  update_state(Executing);

  bool input_field_p = true;
  /// @todo: It looks like the input field is meant to be optional even
  // though it is not.
  //  if (!(ifp->get(ifieldhandle) && ifieldhandle.get_rep()))
  //  {
  //    input_field_p = false;
  //    return;
  //  }

  BBox bbox;
  if (input_field_p)
  {
    bbox = ifieldhandle->vmesh()->get_bounding_box();
  }
  else
  {
    bbox.extend(Point(-1.0, -1.0, -1.0));
    bbox.extend(Point(1.0, 1.0, 1.0));
  }

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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER //@cbrightsci: don't worry about this yet
    GeometryOPortHandle ogport;
    if (!(get_oport_handle("GeneratePointSamplesFromField Widget",ogport)))
    {
      error("Unable to initialize " + module_name_ + "'s oport.");
      return;
    }
    ogport->flushViews();
#endif

    impl_->last_bounds_ = bbox;
  }

  auto state = get_state();
  size_t numSeeds = state->getValue(Parameters::NumSeeds).toInt();
  auto scale = state->getValue(Parameters::ProbeScale).toDouble();

  if (impl_->widget_id_.size() != numSeeds)
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER //@cbrightsci: don't worry about this yet
    GeometryOPortHandle ogport;
    if (!(get_oport_handle("GeneratePointSamplesFromField Widget",ogport)))
    {
      error("Unable to initialize " + module_name_ + "'s oport.");
      return;
    }
#endif

    if (numSeeds < impl_->widget_id_.size())
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER //@cbrightsci: remove point widgets from viewscene
      for (int i = numSeeds; i < (int)widget_id_.size(); i++)
      {
        ((GeomSwitch *)(widget_switch_[i].get_rep()))->set_state(0);
        ogport->delObj(widget_id_[i]);
        ogport->flushViews();

        //delete in tcl side
        ostringstream str;
        str << i;
        TCLInterface::execute(get_id().c_str() + std::string(" clear_seed " + str.str()));
      }
#endif
      impl_->widget_id_.resize(numSeeds);
      impl_->pointWidgets_.resize(numSeeds);
    }
    else
    {
      for (size_t i = impl_->widget_id_.size(); i < numSeeds; i++)
      {
        PointWidgetPtr seed(new PointWidgetStub());  //@cbrightsci: replace with real PointWidget instance

        impl_->pointWidgets_.push_back(seed);
        impl_->widget_id_.push_back(i);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER //@cbrightsci: place new point widget on viewscene
        // input saved out positions
        std::ostringstream strX, strY, strZ;

        strX << "seedX" << i;
        GuiDouble* gui_locx = new GuiDouble(get_ctx()->subVar(strX.str()));
        seeds_.push_back(gui_locx);

        strY << "seedY" << i;
        GuiDouble* gui_locy = new GuiDouble(get_ctx()->subVar(strY.str()));
        seeds_.push_back(gui_locy);

        strZ << "seedZ" << i;
        GuiDouble* gui_locz = new GuiDouble(get_ctx()->subVar(strZ.str()));
        seeds_.push_back(gui_locz);

        Point curloc(gui_locx->get(),gui_locy->get(),gui_locz->get());

        if (curloc.x() >= bmin.x() && curloc.x() <= bmax.x() &&
          curloc.y() >= bmin.y() && curloc.y() <= bmax.y() &&
          curloc.z() >= bmin.z() && curloc.z() <= bmax.z() ||
          !input_field_p)
        {
          center = curloc;
        }
        else
        {
          TCLInterface::execute(get_id().c_str() + std::string(" make_seed " + to_string((int)i)));
        }

        TCLInterface::execute(get_id().c_str() + std::string(" set_seed " + to_string((int)i) + " " + to_string((double)center.x()) + " " + to_string((double)center.y()) + " " + to_string((double)center.z())));
#endif
        seed->setPosition(center);
        seed->setScale(scale * impl_->l2norm_ * 0.003);
      }
    }
  }

  const BBox ibox = ifieldhandle->vmesh()->get_bounding_box();
  Vector mag = ibox.get_max() - ibox.get_min();
  double max = 0.0;
  if (mag.x() > max) max = mag.x();
  if (mag.y() > max) max = mag.y();
  if (mag.z() > max) max = mag.z();

  for (int i = 0; i < numSeeds; i++)
  {
    impl_->pointWidgets_[i]->setScale(scale * impl_->l2norm_ * 0.003);
    const Point location = impl_->pointWidgets_[i]->position();

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER  //@cbrightsci: i think this adjusts the point/sphere scale
    TCLInterface::execute(get_id().c_str() + std::string(" set_seed " +
      to_string((int)i) + " " + to_string((double)location.x()) + " " +
      to_string((double)location.y()) + " " + to_string((double)location.z())));
#endif
  }

  //when push send button--TODO: for now, always update seed mesh
  FieldInformation fi("PointCloudMesh", 1, "double");
  FieldHandle ofield = CreateField(fi);
  VMesh* mesh = ofield->vmesh();
  VField* field = ofield->vfield();

  for (int i = 0; i < numSeeds; i++)
  {
    const Point location = impl_->pointWidgets_[i]->position();

    VMesh::Node::index_type pcindex = mesh->add_point(location);
    field->resize_fdata();
    field->set_value(static_cast<double>(i), pcindex);
  }

  return ofield;
}

GeometryHandle GeneratePointSamplesFromFieldImpl::buildWidgetObject(FieldHandle field, double radius, const Core::GeometryIDGenerator& idGenerator)
{
  GeometryHandle geom(new GeometryObjectSpire(idGenerator, "EntireSinglePointProbeFromField", true));

  VMesh*  mesh = field->vmesh();

  ColorScheme colorScheme = COLOR_UNIFORM;
  ColorRGB node_color;  

  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  double num_strips = 10;
  if (radius < 0) radius = 1.;
  if (num_strips < 0) num_strips = 10.;
  std::stringstream ss;
  ss << radius << num_strips << colorScheme;

  std::string uniqueNodeID = geom->uniqueID() + "widget" + ss.str();

  SpireIBO::PRIMITIVE primIn = SpireIBO::TRIANGLES;

  Graphics::GlyphGeom glyphs;
  while (eiter != eiter_end)
  {
    //checkForInterruption();

    Point p;
    mesh->get_point(p, *eiter);

    glyphs.addSphere(p, radius, num_strips, node_color);

    ++eiter;
  }

  RenderState renState = getWidgetRenderState();

  glyphs.buildObject(geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, primIn, mesh->get_bounding_box());

  return geom;
}

RenderState GeneratePointSamplesFromFieldImpl::getWidgetRenderState() const
{
  RenderState renState;

  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(0.5, 0.5, 0.5);
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);

  return renState;
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER //@cbrightsci: this will be part 3, with interactive widgets
void
GeneratePointSamplesFromField::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    if (gui_auto_execute_.get() == 1)
    {
      want_to_execute();
    }
  }
}

} // End namespace SCIRun


#endif

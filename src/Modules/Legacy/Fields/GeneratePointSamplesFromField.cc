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
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBox.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

ALGORITHM_PARAMETER_DEF(Fields, NumSeeds);
ALGORITHM_PARAMETER_DEF(Fields, ProbeScale);

const ModuleLookupInfo GeneratePointSamplesFromField::staticInfo_("GeneratePointSamplesFromField", "NewField", "SCIRun");

#if 0

#include <Core/Thread/CrowdMonitor.h>

#include <Dataflow/Widgets/PointWidget.h>
#include <Dataflow/Widgets/ArrowWidget.h>
#include <Dataflow/Widgets/RingWidget.h>
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
  #if 0

    GeneratePointSamplesFromField(GuiContext* ctx);
    virtual ~GeneratePointSamplesFromField() {}

    virtual void execute();
    virtual void widget_moved(bool, BaseWidget*);

    std::vector<GuiDouble*> seeds_;

    GuiInt    gui_num_seeds_;
    GuiDouble gui_probe_scale_;
    GuiInt    gui_send_;
    GuiInt    gui_widget_;
    GuiDouble red_;
    GuiDouble green_;
    GuiDouble blue_;
    GuiInt    gui_auto_execute_;

#endif
    //CrowdMonitor widget_lock_;
    BBox last_bounds_;

    std::vector<size_t>              widget_id_;
    //std::vector<GeomHandle>       widget_switch_;
    std::vector<PointWidgetPtr>     pointWidgets_;
    //std::vector<ArrowWidget*>     widget_vec_;
    //std::vector<RingWidget*>      widget_ring_;
    double l2norm_;

};
}}}


GeneratePointSamplesFromField::GeneratePointSamplesFromField()
  : Module(staticInfo_), impl_(new GeneratePointSamplesFromFieldImpl)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(GeneratedWidget);
  INITIALIZE_PORT(GeneratedPoints);
}

void GeneratePointSamplesFromField::setStateDefaults()
{
  //TODO
  auto state = get_state();
  state->setValue(Parameters::NumSeeds, 1);
  state->setValue(Parameters::ProbeScale, 5.0);
}

#if 0
GeneratePointSamplesFromField::GeneratePointSamplesFromField(GuiContext* ctx)
  : Module("GeneratePointSamplesFromField", ctx, Filter, "NewField", "SCIRun"),
    gui_num_seeds_(get_ctx()->subVar("num_seeds"), 1),
    gui_probe_scale_(get_ctx()->subVar("probe_scale"), 5.0),
    gui_send_(get_ctx()->subVar("send"), 0),
    gui_widget_(get_ctx()->subVar("widget"), 0),
    red_(get_ctx()->subVar("red"), 0.5),
    green_(get_ctx()->subVar("green"), 0.5),
    blue_(get_ctx()->subVar("blue"), 0.5),
    gui_auto_execute_(get_ctx()->subVar("auto_execute"), 1),
    widget_lock_("GeneratePointSamplesFromField widget lock")
{
}
#endif

void GeneratePointSamplesFromField::execute()
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

  // Maybe update the widget.
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
    const double size_estimate = std::max((bmax-bmin).length() * 0.01, 1.0e-5);
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

#if 0
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
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    GeometryOPortHandle ogport;
    if (!(get_oport_handle("GeneratePointSamplesFromField Widget",ogport)))
    {
      error("Unable to initialize " + module_name_ + "'s oport.");
      return;
    }
#endif

    if (numSeeds < impl_->widget_id_.size())
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
      //widget_switch_.resize(numSeeds);
      impl_->widget_id_.resize(numSeeds);
      if (true) //gui_widget_.get() == 0)
        impl_->pointWidgets_.resize(numSeeds);
      else
      {
        //widget_ring_.resize(numSeeds);
      }

    }
    else
    {
      for (size_t i = impl_->widget_id_.size(); i < numSeeds; i++)
      {
        if (true) //gui_widget_.get() == 0)
        {
          PointWidgetPtr seed(new PointWidgetStub()); //this, &widget_lock_, 1.0);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          MaterialHandle redMatl = new Material(Color(red_.get(), green_.get(), blue_.get()));
          seed->SetMaterial(0,redMatl);
          seed->Connect(ogport.get_rep());
#endif
          impl_->pointWidgets_.push_back(seed);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          widget_switch_.push_back(widget_[i]->GetWidget());
          ((GeomSwitch *)(widget_switch_[i].get_rep()))->set_state(1);
#endif
          impl_->widget_id_.push_back(i);  //ogport->addObj(widget_switch_[i], "SeedPoint" + to_string((int)i), &widget_lock_));

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
        else
        {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          RingWidget *seed = new RingWidget(this, &widget_lock_, gui_probe_scale_.get(), false);

          MaterialHandle redMatl = new Material(Color(red_.get(), green_.get(), blue_.get()));
          redMatl->specular.r(0.2);
          redMatl->specular.g(0.2);
          redMatl->specular.b(0.2);

          MaterialHandle greyMatl = new Material(Color(0.5, 0.5, 0.5));
          seed->SetMaterial(1,redMatl);
          seed->SetDefaultMaterial(5,greyMatl);
          seed->Connect(ogport.get_rep());
          widget_ring_.push_back(seed);

          widget_switch_.push_back(widget_ring_[i]->GetWidget());
          ((GeomSwitch *)(widget_switch_[i].get_rep()))->set_state(1);

          widget_id_.push_back(ogport->addObj(widget_switch_[i], "SeedPoint" + to_string((int)i),
                      &widget_lock_));

          // input saved out positions
          ostringstream strX, strY, strZ;

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

          TCLInterface::execute(get_id().c_str() + std::string(" set_seed "
                  + to_string((int)i) + " " + to_string(135) + " "
                  + to_string(293) + " " + to_string(0.1)));

          double r = gui_probe_scale_.get();
          Vector normal(0.0, 0.0, 1.0);
          center.x(0.5);
          center.y(0.5);
          center.z(0.1);
          seed->SetPosition(center, normal, r);
          seed->SetScale(gui_probe_scale_.get() * l2norm_ * 0.003);
          seed->SetRadius(r);
          seed->SetCurrentMode(3);
#endif
        }
      }
    }
  }

  // Find magnitude and base ring scale on that
  const BBox ibox = ifieldhandle->vmesh()->get_bounding_box();
  Vector mag = ibox.get_max() - ibox.get_min();
  double max = 0.0;
  if (mag.x() > max) max = mag.x();
  if (mag.y() > max) max = mag.y();
  if (mag.z() > max) max = mag.z();

  for (int i=0; i <numSeeds; i++)
  {
    if (true) //gui_widget_.get() == 0)
    {
      impl_->pointWidgets_[i]->setScale(scale * impl_->l2norm_ * 0.003);
      const Point location = impl_->pointWidgets_[i]->position();

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER  //@cbrightsci
      TCLInterface::execute(get_id().c_str() + std::string(" set_seed " +
        to_string((int)i) + " " + to_string((double)location.x()) + " " +
        to_string((double)location.y()) + " " + to_string((double)location.z())));
#endif
    }
    else
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      double r;
      Vector normal;
      widget_ring_[i]->GetPosition(center, normal, r);
      widget_ring_[i]->SetScale(max*0.02);

      // place rings at slight distance as seeds are added
      TCLInterface::execute(get_id().c_str() + std::string(" set_seed " +
        to_string((int)i) + " " + to_string(135) + " " +
        to_string(293) + " " + to_string(0.1)));
#endif
    }
  }

  //when push send button--TODO: for now, always update seed mesh
  if (true) // gui_send_.get())
  {
    FieldInformation fi("PointCloudMesh",1,"double");
    FieldHandle ofield = CreateField(fi);
    VMesh* mesh = ofield->vmesh();
    VField* field = ofield->vfield();

    for (int i=0; i < numSeeds; i++)
    {
      if (true) //gui_widget_.get() == 0)
      {
        const Point location = impl_->pointWidgets_[i]->position();

        VMesh::Node::index_type pcindex = mesh->add_point(location);
        field->resize_fdata();
        field->set_value(static_cast<double>(i), pcindex);
      }
      else
      {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
        Point location;
        double r;
        Vector normal;
        widget_ring_[i]->GetPosition(location, normal, r);

        VMesh::Node::index_type pcindex = mesh->add_point(location);
        field->resize_fdata();
        field->set_value(r, pcindex); // FIX ME: get radius
#endif
      }
    }

    if (true) //gui_widget_.get() == 0)
    {
      std::vector<double> values(numSeeds);
      for (size_t i = 0; i < numSeeds; i++)
      {
        const Point location = impl_->pointWidgets_[i]->position();
        mesh->add_point(location);
        values[i] = static_cast<double>(i);
      }

      field->resize_values();
      field->set_values(values);
    }
    else
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      std::vector<double> values(numSeeds);
      for (VField::index_type i=0; i <numSeeds; i++)
      {
        Point location;
        double r = 0.0;
        Vector normal;
        widget_ring_[i]->GetPosition(location, normal, r);
        mesh->add_point(location);
        values[i] = r;
      }

      field->resize_values();
      field->set_values(values);
#endif
    }

    sendOutput(GeneratedPoints, ofield);
  }
}

#if 0
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

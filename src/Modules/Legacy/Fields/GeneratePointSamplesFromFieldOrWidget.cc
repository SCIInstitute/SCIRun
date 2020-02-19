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


///@file  GeneratePointSamplesFromFieldOrWidget.cc
///
///@author
///   David Weinstein
///   University of Utah
///@date  October 2000

#include <Modules/Legacy/Fields/GeneratePointSamplesFromFieldOrWidget.h>
#include <Core/Algorithms/Legacy/Fields/SampleField/GeneratePointSamplesFromField.h>

#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Dataflow/Widgets/GaugeWidget.h>
#include <Dataflow/Widgets/RingWidget.h>
#include <Dataflow/Widgets/FrameWidget.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

MODULE_INFO_DEF(GeneratePointSamplesFromFieldOrWidget, NewField, SCIRun)

/// @class GeneratePointSamplesFromFieldOrWidget
/// @brief This module generates samples from any type of input field and
/// outputs the samples as a PointCloudField field.

GeneratePointSamplesFromFieldOrWidget::GeneratePointSamplesFromFieldOrWidget()
  : Module(staticInfo_)//, impl_(new GeneratePointSamplesFromFieldImpl)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Sampling_Widget);
  INITIALIZE_PORT(Samples);
}

void GeneratePointSamplesFromFieldOrWidget::setStateDefaults()
{
  auto state = get_state();
  setStateIntFromAlgo(Parameters::NumSamples);
  setStateStringFromAlgoOption(Parameters::DistributionType);
  setStateBoolFromAlgo(Parameters::ClampToNodes);
  state->setValue(Parameters::IncrementRNGSeed, true);
  setStateIntFromAlgo(Parameters::RNGSeed);
}

#if 0
namespace SCIRun {
  namespace Modules {
    namespace Fields {

class GeneratePointSamplesFromFieldOrWidgetImpl
{
  public:
    //explicit GeneratePointSamplesFromFieldOrWidgetImpl(GeneratePointSamplesFromFieldOrWidget* module);
    #if 0
    virtual ~GeneratePointSamplesFromFieldOrWidget();
    virtual void execute();
    virtual void widget_moved(bool last, BaseWidget*);

  virtual void post_read();

  private:

    GuiString gui_wtype_;
    GuiInt    gui_endpoints_;
    GuiDouble gui_endpoint0x_;
    GuiDouble gui_endpoint0y_;
    GuiDouble gui_endpoint0z_;
    GuiDouble gui_endpoint1x_;
    GuiDouble gui_endpoint1y_;
    GuiDouble gui_endpoint1z_;
    GuiDouble gui_widgetscale_;
    GuiString gui_ringstate_;
    GuiString gui_framestate_;

    GuiInt gui_widget_seeds_;
    GuiInt gui_random_seeds_;
    GuiInt gui_rngSeed_;
    GuiInt gui_rngInc_;
    GuiInt gui_clamp_;
    GuiInt gui_autoexec_;
    GuiString gui_randdist_;
    GuiString gui_whichTab_;
    GuiInt gui_force_rake_reset_;

    CrowdMonitor gui_widget_lock_;

    GaugeWidget *rake_;
    RingWidget  *ring_;
    FrameWidget *frame_;

    int widgetid_;
    int wtype_;      // 0 random (none), 1 rake, 2 ring, 3 frame

    bool widget_change_;

    BBox  rake_bbox_;
    BBox  ring_bbox_;
    BBox  frame_bbox_;
    Point endpoint0_;
    Point endpoint1_;

    void initialize_rake(FieldHandle ifield);
    FieldHandle execute_rake(FieldHandle ifield);
    FieldHandle execute_ring(FieldHandle ifield);
    FieldHandle execute_frame(FieldHandle ifield);
    #endif

    FieldHandle execute_random(FieldHandle ifield);
};
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
GeneratePointSamplesFromFieldOrWidget::GeneratePointSamplesFromFieldOrWidget(GuiContext* ctx)
  : Module("GeneratePointSamplesFromFieldOrWidget", ctx, Filter, "NewField", "SCIRun"),
    gui_wtype_(get_ctx()->subVar("wtype"), "rake"),
    gui_endpoints_ (get_ctx()->subVar("endpoints")),
    gui_endpoint0x_(get_ctx()->subVar("endpoint0x")),
    gui_endpoint0y_(get_ctx()->subVar("endpoint0y")),
    gui_endpoint0z_(get_ctx()->subVar("endpoint0z")),
    gui_endpoint1x_(get_ctx()->subVar("endpoint1x")),
    gui_endpoint1y_(get_ctx()->subVar("endpoint1y")),
    gui_endpoint1z_(get_ctx()->subVar("endpoint1z")),
    gui_widgetscale_(get_ctx()->subVar("widgetscale")),
    gui_ringstate_(get_ctx()->subVar("ringstate")),
    gui_framestate_(get_ctx()->subVar("framestate")),
    gui_widget_seeds_(get_ctx()->subVar("widget_seeds"), 15),
    gui_random_seeds_(get_ctx()->subVar("random_seeds"), 10),
    gui_rngSeed_(get_ctx()->subVar("rngseed"), 1),
    gui_rngInc_(get_ctx()->subVar("rnginc"), 1),
    gui_clamp_(get_ctx()->subVar("clamp"), 0),
    gui_autoexec_(get_ctx()->subVar("autoexecute"), 1),
    gui_randdist_(get_ctx()->subVar("dist"), "uniuni"),
    gui_whichTab_(get_ctx()->subVar("whichtab"), "Widget"),
    gui_force_rake_reset_(get_ctx()->subVar("force-rake-reset", false), 0),

    gui_widget_lock_("GeneratePointSamplesFromFieldOrWidget widget lock"),

    rake_(0),
    ring_(0),
    frame_(0),

    widgetid_(0),

    widget_change_(0)
{
  gui_endpoints_.set( 0 );
}


GeneratePointSamplesFromFieldOrWidget::~GeneratePointSamplesFromFieldOrWidget()
{
  if (rake_) delete rake_;
  if (ring_) delete ring_;
  if (frame_) delete frame_;
}


void
GeneratePointSamplesFromFieldOrWidget::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    if (rake_)
    {
      rake_->GetEndpoints(endpoint0_, endpoint1_);
      double ratio = rake_->GetRatio();
      const double smax = 1.0 / (200 - 1);  // Max the slider at 200 samples.
      if (ratio < smax) ratio = smax;
      int num_seeds = static_cast<int>(Max(0.0, 1.0/ratio+1.0));
      gui_widget_seeds_.set(num_seeds);

      gui_endpoint0x_.set( endpoint0_.x() );
      gui_endpoint0y_.set( endpoint0_.y() );
      gui_endpoint0z_.set( endpoint0_.z() );
      gui_endpoint1x_.set( endpoint1_.x() );
      gui_endpoint1y_.set( endpoint1_.y() );
      gui_endpoint1z_.set( endpoint1_.z() );
    }

    widget_change_ = true;

    gui_autoexec_.reset();
    if (gui_autoexec_.get())
      want_to_execute();

  }
  else
  { // rescaling the widget forces a "last=false" widget_moved event
    if (rake_)
      gui_widgetscale_.set(rake_->GetScale());

    widget_change_ = true;
  }
}


void
GeneratePointSamplesFromFieldOrWidget::initialize_rake(FieldHandle ifield)
{
  const BBox ibox = ifield->vmesh()->get_bounding_box();
  const Point &min = ibox.min();
  const Point &max = ibox.max();

  const Point center(((max.asVector() + min.asVector()) * 0.5).asPoint());
  const double length = (max.asVector() - min.asVector()).length();

  // This size seems empirically good.
  const double quarterl2norm = length / 4.0;
  gui_widgetscale_.set( quarterl2norm * .06 );

  gui_endpoint0x_.set( center.x() - quarterl2norm     );
  gui_endpoint0y_.set( center.y() - quarterl2norm / 3 );
  gui_endpoint0z_.set( center.z() - quarterl2norm / 4 );
  gui_endpoint1x_.set( center.x() + quarterl2norm     );
  gui_endpoint1y_.set( center.y() + quarterl2norm / 2 );
  gui_endpoint1z_.set( center.z() + quarterl2norm / 3 );

  gui_endpoints_.set( 1 );
}


FieldHandle
GeneratePointSamplesFromFieldOrWidget::execute_rake(FieldHandle ifield)
{
  GeometryOPortHandle ogport;
  get_oport_handle("Sampling Widget",ogport);

  const BBox ibox = ifield->vmesh()->get_bounding_box();
  bool reset = gui_force_rake_reset_.get();
  gui_force_rake_reset_.set(0);
  bool resize = rake_bbox_.valid() && !ibox.is_similar_to(rake_bbox_);

  if (!rake_)
  {
    if(!gui_endpoints_.get()) initialize_rake(ifield);

    endpoint0_ = Point(gui_endpoint0x_.get(),
                       gui_endpoint0y_.get(),
                       gui_endpoint0z_.get());

    endpoint1_ = Point(gui_endpoint1x_.get(),
                       gui_endpoint1y_.get(),
                       gui_endpoint1z_.get());

    rake_ = new GaugeWidget(this, &gui_widget_lock_,
                            gui_widgetscale_.get(), true);

    rake_->Connect(ogport.get_rep());
    rake_->SetScale(gui_widgetscale_.get()); // do first, widget_moved resets
    rake_->SetEndpoints(endpoint0_,endpoint1_);
    rake_->SetRatio(1/16.0);

    // The rake sets will cause the widget_moved method to be called
    // but this should only be done when the user moves the widget.
    widget_change_ = false;
  }

  if (reset || resize)
  {
    if( reset ) initialize_rake(ifield);

    endpoint0_ = Point(gui_endpoint0x_.get(),
                       gui_endpoint0y_.get(),
                       gui_endpoint0z_.get());

    endpoint1_ = Point(gui_endpoint1x_.get(),
                       gui_endpoint1y_.get(),
                       gui_endpoint1z_.get());

    rake_->SetScale(gui_widgetscale_.get()); // do first, widget_moved resets
    rake_->SetEndpoints(endpoint0_, endpoint1_);
    rake_->SetRatio(1/16.0);

    // The rake sets will cause the widget_moved method to be called
    // but this should only be done when the user moves the widget.
    widget_change_ = false;

    rake_bbox_ = ibox;
  }


  if (wtype_ != 1)
  {
    if (widgetid_)
    {
      ogport->delObj(widgetid_);
    }

    GeomHandle widget = rake_->GetWidget();
    widgetid_ = ogport->addObj(widget,
				"GeneratePointSamplesFromFieldOrWidget Rake",
				&gui_widget_lock_);
    ogport->flushViews();
    wtype_ = 1;
  }

  Point min, max;
  rake_->GetEndpoints(min, max);

  Vector dir(max-min);

  unsigned int num_seeds = Max(0, gui_widget_seeds_.get());
  remark("num_seeds = " + to_string(num_seeds));

  if (num_seeds > 1)
  {
    const double ratio = 1.0 / (static_cast<double>(num_seeds) - 1.0);

    rake_->SetRatio(ratio);
    dir *= ratio;

    // The rake sets will cause the widget_moved method to be called
    // but this should only be done when the user moves the widget.
    widget_change_ = false;
  }

  FieldInformation fi("PointCloudMesh",0,"double");
  FieldHandle seeds = CreateField(fi);
  VMesh* mesh = seeds->vmesh();
  VField* field = seeds->vfield();

  std::vector<double> values(num_seeds);

  for (unsigned int i=0; i<num_seeds; i++)
  {
    mesh->add_point(min+dir*(double)i);
    values[i] = i;
  }

  field->resize_values();
  field->set_values(values);

  return seeds;
}


FieldHandle
GeneratePointSamplesFromFieldOrWidget::execute_ring(FieldHandle ifield)
{
  GeometryOPortHandle ogport;
  get_oport_handle("Sampling Widget",ogport);

  const BBox ibox = ifield->vmesh()->get_bounding_box();
  bool reset = gui_force_rake_reset_.get();
  gui_force_rake_reset_.set(0);
  bool resize = ring_bbox_.valid() && !ibox.is_similar_to(ring_bbox_);

  if (!ring_)
  {
    ring_ = new RingWidget(this, &gui_widget_lock_,
                           gui_widgetscale_.get(),false);
    ring_->Connect(ogport.get_rep());

    if (gui_ringstate_.get() != "")
    {
      ring_->SetStateString(gui_ringstate_.get());

      // Check for state validity here.  If not valid then // reset = true;
    }
    else
    {
      reset = true;
    }
  }

  if (reset || resize)
  {
    Point c, nc;
    Vector n, nn;
    double r, nr;
    double s, ns;

    if (reset)
    {
      const Vector xaxis(0.0, 0.0, 0.2);
      const Vector yaxis(0.2, 0.0, 0.0);
      c = Point (0.5, 0.0, 0.0);
      n = Cross(xaxis, yaxis);
      r = 0.2;
      s = sqrt(3.0) * 0.03;

      ring_bbox_.reset();
      ring_bbox_.extend(Point(-1.0, -1.0, -1.0));
      ring_bbox_.extend(Point(1.0, 1.0, 1.0));
    }
    else
    {
      // Get the old coordinates.
      ring_->GetPosition(c, n, r);
      s = ring_->GetScale();
    }

    // Build a transform.
    Transform trans;
    trans.load_identity();
    const Vector scale =
      (ibox.max() - ibox.min()) / (ring_bbox_.max() - ring_bbox_.min());
    trans.pre_translate(-ring_bbox_.min().asVector());
    trans.pre_scale(scale);
    trans.pre_translate(ibox.min().asVector());

    // Do the transform.
    trans.project(c, nc);
    nn = n; //trans.project_normal(n, nn);
    nr = (r * scale).length() / sqrt(3.0);
    ns = (s * scale).length() / sqrt(3.0);

    // Apply the new coordinates.
    ring_->SetScale(ns); // do first, widget_moved resets
    ring_->SetPosition(nc, nn, nr);
    ring_->SetRadius(nr);
    gui_widgetscale_.set(ns);

    widget_change_ = false;

    ring_bbox_ = ibox;
  }

  if (wtype_ != 2)
  {
    if (widgetid_)
    {
      ogport->delObj(widgetid_);
    }

    GeomHandle widget = ring_->GetWidget();
    widgetid_ = ogport->addObj(widget,
                                "GeneratePointSamplesFromFieldOrWidget Ring",
                                &gui_widget_lock_);
    ogport->flushViews();
    wtype_ = 2;
  }

  unsigned int num_seeds = Max(0, gui_widget_seeds_.get());
  remark("num_seeds = " + to_string(num_seeds));

  FieldInformation fi("PointCloudMesh",0,"double");
  FieldHandle seeds = CreateField(fi);

  VMesh* mesh = seeds->vmesh();
  VField* field = seeds->vfield();

  Point center;
  double r;

  Vector normal, xaxis, yaxis;
  ring_->GetPosition(center, normal, r);
  ring_->GetPlane(xaxis, yaxis);

  std::vector<double> values(num_seeds);

  for (unsigned int i=0; i<num_seeds; ++i)
  {
    const double frac = 2.0 * M_PI * i / num_seeds;
    mesh->add_point(center + xaxis * r * cos(frac) + yaxis * r * sin(frac));

    values[i] = i;
  }

  field->resize_values();
  field->set_values(values);

  return seeds;
}


FieldHandle
GeneratePointSamplesFromFieldOrWidget::execute_frame(FieldHandle ifield)
{
  GeometryOPortHandle ogport;
  get_oport_handle("Sampling Widget",ogport);

  const BBox ibox = ifield->vmesh()->get_bounding_box();
  bool reset = gui_force_rake_reset_.get();
  gui_force_rake_reset_.set(0);
  bool resize = frame_bbox_.valid() && !ibox.is_similar_to(frame_bbox_);

  if (!frame_)
  {
    frame_ = new FrameWidget(this,&gui_widget_lock_,gui_widgetscale_.get());
    frame_->Connect(ogport.get_rep());

    if (gui_framestate_.get() != "")
    {
      frame_->SetStateString(gui_framestate_.get());
    }
    else
    {
      reset = true;
    }
  }

  if (reset || resize)
  {
    Point c, nc, r, nr, d, nd;
    double s, ns;

    if (reset)
    {
      c = Point(0.5, 0.0, 0.0);
      r = c + Vector(0.0, 0.0, 0.2);
      d = c + Vector(0.2, 0.0, 0.0);
      s = sqrt(3.0) * 0.03;

      frame_bbox_.reset();
      frame_bbox_.extend(Point(-1.0, -1.0, -1.0));
      frame_bbox_.extend(Point(1.0, 1.0, 1.0));
    }
    else
    {
      // Get the old coordinates.
      frame_->GetPosition(c, r, d);
      s = frame_->GetScale();
    }

    // Build a transform.
    Transform trans;
    trans.load_identity();
    const Vector scale =
      (ibox.max() - ibox.min()) / (frame_bbox_.max() - frame_bbox_.min());
    trans.pre_translate(-frame_bbox_.min().asVector());
    trans.pre_scale(scale);
    trans.pre_translate(ibox.min().asVector());

    // Do the transform.
    trans.project(c, nc);
    trans.project(r, nr);
    trans.project(d, nd);
    ns = (s * scale).length() / sqrt(3.0);

    // Apply the new coordinates.
    frame_->SetScale(ns); // do first, widget_moved resets
    frame_->SetPosition(nc, nr, nd);
    gui_widgetscale_.set(ns);

    frame_bbox_ = ibox;

    widget_change_ = false;
  }

  if (wtype_ != 3)
  {
    if (widgetid_)
    {
      ogport->delObj(widgetid_);
    }
    GeomHandle widget = frame_->GetWidget();
    widgetid_ = ogport->addObj(widget,
                                "GeneratePointSamplesFromFieldOrWidget Frame",
                                &gui_widget_lock_);
    ogport->flushViews();
    wtype_ = 3;
  }

  unsigned int num_seeds = Max(0, gui_widget_seeds_.get());
  remark("num_seeds = " + to_string(num_seeds));

  FieldInformation fi("PointCloudMesh",0,"double");
  FieldHandle seeds = CreateField(fi);

  VMesh* mesh = seeds->vmesh();
  VField* field = seeds->vfield();

  Point center, xloc, yloc;
  Point corner[4];
  Vector edge[4];
  frame_->GetPosition(center, xloc, yloc);
  const Vector xaxis = xloc - center;
  const Vector yaxis = yloc - center;
  corner[0] = center + xaxis + yaxis;
  corner[1] = center + xaxis - yaxis;
  corner[2] = center - xaxis - yaxis;
  corner[3] = center - xaxis + yaxis;
  edge[0] = corner[1] - corner[0];
  edge[1] = corner[2] - corner[1];
  edge[2] = corner[3] - corner[2];
  edge[3] = corner[0] - corner[3];

  std::vector<double> values(num_seeds);

  for (unsigned int i=0; i<num_seeds; ++i)
  {
    const double frac =  4.0 * static_cast<double>( i )/ static_cast<double>( num_seeds );
    const int ei = (int) frac;
    const double eo = frac - ei;
    mesh->add_point(corner[ei] + edge[ei] * eo);

    values[i] = (double) i;
  }

  field->resize_values();
  field->set_values(values);

  return seeds;
}

} // End namespace SCIRun
#endif


void GeneratePointSamplesFromFieldOrWidget::execute_random(FieldHandle ifield)
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  GeometryOPortHandle ogport;
  get_oport_handle("Sampling Widget",ogport);
#endif

//  SCIRunAlgo::GeneratePointSamplesFromFieldAlgo algo;
//  algo.set_progress_reporter(this);
  setAlgoIntFromState(Parameters::NumSamples);
  setAlgoIntFromState(Parameters::RNGSeed);
  setAlgoOptionFromState(Parameters::DistributionType);
  setAlgoBoolFromState(Parameters::ClampToNodes);
//  algo.set_int("num_seed_points",gui_random_seeds_.get());
  //algo.set_int("rng_seed",gui_rngSeed_.get());
//  algo.setOption("seed_method",gui_randdist_.get());
//  algo.set_bool("clamp",gui_clamp_.get());

  auto output = algo().run(withInputData((InputField, ifield)));

  auto state = get_state();
  if (state->getValue(Parameters::IncrementRNGSeed).toBool())
  {
    state->setValue(Parameters::RNGSeed, state->getValue(Parameters::RNGSeed).toInt() + 1);
  }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (widgetid_)
  {
    ogport->delObj(widgetid_);
    ogport->flushViews();
    widgetid_ = 0;
    wtype_ = 0;
  }
#endif

  sendOutputFromAlgorithm(Samples, output);
}

void
GeneratePointSamplesFromFieldOrWidget::execute()
{
  auto field_in_handle = getRequiredInput(InputField);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  // See if the tab has changed if so execute.
  if( gui_whichTab_.changed( true ) ) inputs_changed_ = true;

  if (ring_ ) { gui_ringstate_.set(ring_->GetStateString()); }
  if (frame_) { gui_framestate_.set(frame_->GetStateString()); }

  if (gui_whichTab_.get() == "Widget")
  {
    if( inputs_changed_ || !oport_cached("Samples") || widget_change_ ||
        gui_force_rake_reset_.changed( true ) ||
        gui_wtype_.changed( true ) ||
        gui_widget_seeds_.changed( true ) )
    {
      update_state(Executing);
      widget_change_ = false;
      FieldHandle field_out_handle;

      if (gui_wtype_.get() == "rake")
      {
        field_out_handle = execute_rake(field_in_handle);
      }
      else if (gui_wtype_.get() == "ring")
      {
        field_out_handle = execute_ring(field_in_handle);
      }
      else if (gui_wtype_.get() == "frame")
      {
        field_out_handle = execute_frame(field_in_handle);
      }

      send_output_handle("Samples", field_out_handle);
    }
  }
  else
  if (gui_whichTab_.get() == "Random")
#endif
  //only Random method for now
  {
    if (needToExecute())
    {
      execute_random(field_in_handle);
    }
  }
}

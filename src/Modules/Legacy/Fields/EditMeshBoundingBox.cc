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
///   @file    EditMeshBoundingBox.cc
///   @author  McKay Davis
///   @date    July 2002
///

#include <Dataflow/Network/Module.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Geometry/Transform.h>
#include <Core/Thread/CrowdMonitor.h>
#include <Dataflow/Widgets/BoxWidget.h>
#include <Dataflow/Network/NetworkEditor.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Util/StringUtil.h>
#include <map>
#include <iostream>

namespace SCIRun {

using std::endl;
using std::pair;

/// @class EditMeshBoundingBox
/// @brief EditMeshBoundingBox is used to transform the field geometry.

class EditMeshBoundingBox : public Module {
  public:
    EditMeshBoundingBox(GuiContext* ctx);
    virtual ~EditMeshBoundingBox();

    GuiDouble		outputcenterx_;	// the out geometry (center point and size)
    GuiDouble		outputcentery_;
    GuiDouble		outputcenterz_;
    GuiDouble		outputsizex_;
    GuiDouble		outputsizey_;
    GuiDouble		outputsizez_;
    GuiInt      useoutputcenter_;   // center checkbox
    GuiInt      useoutputsize_;   // size checkbox

    GuiString		inputcenterx_;
    GuiString		inputcentery_;
    GuiString		inputcenterz_;
    GuiString		inputsizex_;
    GuiString		inputsizey_;
    GuiString		inputsizez_;

    // NOTE: box_scale_ isn't actually the scale of the box.  It keeps
    // track of whether or not the box has been edited by hand or was
    // automatically generated.  It will have a negative value if the
    // box was automatically generated and a positive value if it was
    // edited by hand (widget_moved called).  The name has been
    // preserved for backwards compatability.
    GuiDouble             box_scale_;

    GuiInt                box_mode_;
    GuiDouble             box_real_scale_;
    GuiPoint              box_center_;
    GuiPoint              box_right_;
    GuiPoint              box_down_;
    GuiPoint              box_in_;

    GuiInt                resetting_;

    GuiInt restrict_translation_;
    GuiInt restrict_x_;
    GuiInt restrict_y_;
    GuiInt restrict_z_;
    GuiInt restrict_r_;
    GuiInt restrict_d_;
    GuiInt restrict_i_;

    CrowdMonitor		widget_lock_;
    BoxWidget *     box_;
    Transform       box_initial_transform_;
    Transform       field_initial_transform_;
    BBox            box_initial_bounds_;
    int             generation_;
    int             widgetid_;

    void clear_vals();
    void update_input_attributes(FieldHandle);
    void build_widget(FieldHandle, bool reset);

    virtual void execute();
    virtual void widget_moved(bool, BaseWidget*);

    virtual void tcl_command(GuiArgs&, void*);

    GuiDouble widget_scale_;
    GuiInt    widget_mode_;

    virtual void presave();
    virtual void post_read();
};

DECLARE_MAKER(EditMeshBoundingBox)

EditMeshBoundingBox::EditMeshBoundingBox(GuiContext* ctx)
  : Module("EditMeshBoundingBox", ctx, Filter, "ChangeMesh", "SCIRun"),
    outputcenterx_(get_ctx()->subVar("outputcenterx"), 0.0),
    outputcentery_(get_ctx()->subVar("outputcentery"), 0.0),
    outputcenterz_(get_ctx()->subVar("outputcenterz"), 0.0),
    outputsizex_(get_ctx()->subVar("outputsizex"), 0.0),
    outputsizey_(get_ctx()->subVar("outputsizey"), 0.0),
    outputsizez_(get_ctx()->subVar("outputsizez"), 0.0),
    useoutputcenter_(get_ctx()->subVar("useoutputcenter"), 0),
    useoutputsize_(get_ctx()->subVar("useoutputsize"), 0),
    inputcenterx_(get_ctx()->subVar("inputcenterx", false), "---"),
    inputcentery_(get_ctx()->subVar("inputcentery", false), "---"),
    inputcenterz_(get_ctx()->subVar("inputcenterz", false), "---"),
    inputsizex_(get_ctx()->subVar("inputsizex", false), "---"),
    inputsizey_(get_ctx()->subVar("inputsizey", false), "---"),
    inputsizez_(get_ctx()->subVar("inputsizez", false), "---"),
    box_scale_(get_ctx()->subVar("box-scale"), -1.0),
    box_mode_(get_ctx()->subVar("box-mode"), 0),
    box_real_scale_(get_ctx()->subVar("box-real_scale"), -1.0),
    box_center_(get_ctx()->subVar("box-center"), Point(0.0, 0.0, 0.0)),
    box_right_(get_ctx()->subVar("box-right"), Point(0.0, 0.0, 0.0)),
    box_down_(get_ctx()->subVar("box-down"), Point(0.0, 0.0, 0.0)),
    box_in_(get_ctx()->subVar("box-in"), Point(0.0, 0.0, 0.0)),
    resetting_(get_ctx()->subVar("resetting", false), 0),
    restrict_translation_(get_ctx()->subVar("restrict-translation"),0),
    restrict_x_(get_ctx()->subVar("restrict-x"),0),
    restrict_y_(get_ctx()->subVar("restrict-y"),0),
    restrict_z_(get_ctx()->subVar("restrict-z"),0),
    restrict_r_(get_ctx()->subVar("restrict-r"),0),
    restrict_d_(get_ctx()->subVar("restrict-d"),0),
    restrict_i_(get_ctx()->subVar("restrict-i"),0),
    widget_lock_("EditMeshBoundingBox widget lock"),
    generation_(-1),
    widgetid_(0),
    widget_scale_(get_ctx()->subVar("widget-scale"),-1.0),
    widget_mode_(get_ctx()->subVar("widget-mode"),0)
{
  GeometryOPortHandle ogport;
  get_oport_handle("Transformation Widget",ogport);

  box_ = new BoxWidget(this, &widget_lock_, 1.0, false, false);
  box_->Connect(ogport.get_rep());
  box_->SetRestrictX(restrict_x_.get());
  box_->SetRestrictY(restrict_y_.get());
  box_->SetRestrictZ(restrict_z_.get());
  box_->SetRestrictR(restrict_r_.get());
  box_->SetRestrictD(restrict_d_.get());
  box_->SetRestrictI(restrict_i_.get());
  int val = restrict_translation_.get() ;
  if (val== 0) box_->UnRestrictTranslation();
  else if (val== 1) box_->RestrictTranslationXYZ();
  else if (val== 2) box_->RestrictTranslationRDI();
  inputcenterx_.set("---");
  inputcentery_.set("---");
  inputcenterz_.set("---");
  inputsizex_.set("---");
  inputsizey_.set("---");
  inputsizez_.set("---");
}

void
EditMeshBoundingBox::post_read()
{
  if (widget_scale_.get() > 0.0)
  {
    if (box_)
    {
      box_->SetScale(widget_scale_.get());
      box_->SetCurrentMode(widget_mode_.get());
    }
  }
}

void
EditMeshBoundingBox::presave()
{
  if (box_)
  {
    widget_scale_.set(box_->GetScale());
    widget_mode_.set(box_->GetMode());
  }
}

void
EditMeshBoundingBox::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("EditMeshBoundingBox needs a minor command");
    return;
  }
	if(args[1] == "scale")
  {
    if (args.count() != 3)
    {
      args.error("widget needs user scale");
      return;
    }
    double us;
    if (!string_to_double(args[2], us))
    {
      args.error("widget can't parse user scale `"+args[2]+"'");
      return;
    }
    if (box_) box_->SetScale(box_->GetScale()*us);
  }
  else if (args[1] == "nextmode")
  {
    if (args.count() != 2)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    box_->NextMode();
  }
  else if (args[1] == "restricttranslation")
  {
    if (args.count() != 3)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    int val;
    string_to_int(args[2],val);
    if (val == 0) box_->UnRestrictTranslation();
    if (val == 1) box_->RestrictTranslationXYZ();
    if (val == 2) box_->RestrictTranslationRDI();
  }
  else if (args[1] == "restrictx")
  {
    if (args.count() != 2)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    restrict_x_.reset();
    box_->SetRestrictX(restrict_x_.get());
  }
  else if (args[1] == "restricty")
  {
    if (args.count() != 2)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    restrict_y_.reset();
    box_->SetRestrictY(restrict_y_.get());
  }
  else if (args[1] == "restrictz")
  {
    if (args.count() != 2)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    restrict_z_.reset();
    box_->SetRestrictZ(restrict_z_.get());
  }
  else if (args[1] == "restrictr")
  {
    if (args.count() != 2)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    restrict_r_.reset();
    box_->SetRestrictR(restrict_r_.get());

  }
  else if (args[1] == "restrictd")
  {
    if (args.count() != 2)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    restrict_d_.reset();
    box_->SetRestrictD(restrict_d_.get());
  }
  else if (args[1] == "restricti")
  {
    if (args.count() != 2)
    {
      args.error("widget doesn't need a minor command");
      return;
    }
    restrict_i_.reset();
    box_->SetRestrictI(restrict_i_.get());
  }
  else
  {
    // Relay data to the Module class
    Module::tcl_command(args, userdata);
  }

}


EditMeshBoundingBox::~EditMeshBoundingBox()
{
  delete box_;
}


void
EditMeshBoundingBox::clear_vals()
{
  inputcenterx_.set("---");
  inputcentery_.set("---");
  inputcenterz_.set("---");
  inputsizex_.set("---");
  inputsizey_.set("---");
  inputsizez_.set("---");
}


void
EditMeshBoundingBox::update_input_attributes(FieldHandle f)
{
  Point center;
  Vector size;

  BBox bbox = f->vmesh()->get_bounding_box();

  if (!bbox.valid()) {
    warning("Input field is empty -- using unit cube.");
    bbox.extend(Point(0,0,0));
    bbox.extend(Point(1,1,1));
  }
  size = bbox.diagonal();
  center = bbox.center();

  inputcenterx_.set(to_string(center.x()));
  inputcentery_.set(to_string(center.y()));
  inputcenterz_.set(to_string(center.z()));
  inputsizex_.set(to_string(size.x()));
  inputsizey_.set(to_string(size.y()));
  inputsizez_.set(to_string(size.z()));
}


void
EditMeshBoundingBox::build_widget(FieldHandle f, bool reset)
{
  if (reset || box_scale_.get() <= 0 ||
      (box_center_.get() == Point(0.0, 0.0, 0.0) &&
       box_right_.get() == Point(0.0, 0.0, 0.0) &&
       box_down_.get() == Point(0.0, 0.0, 0.0) &&
       box_in_.get() == Point(0.0, 0.0, 0.0)))
  {
    Point center;
    Vector size;
    BBox bbox = f->vmesh()->get_bounding_box();
    if (!bbox.valid()) {
      warning("Input field is empty -- using unit cube.");
      bbox.extend(Point(0,0,0));
      bbox.extend(Point(1,1,1));
    }
    box_initial_bounds_ = bbox;

    // build a widget identical to the BBox
    size = Vector(bbox.max()-bbox.min());
    if (fabs(size.x())<1.e-4)
    {
      size.x(2.e-4);
      bbox.extend(bbox.min()-Vector(1.0e-4, 0.0, 0.0));
      bbox.extend(bbox.max()+Vector(1.0e-4, 0.0, 0.0));
    }
    if (fabs(size.y())<1.e-4)
    {
      size.y(2.e-4);
      bbox.extend(bbox.min()-Vector(0.0, 1.0e-4, 0.0));
      bbox.extend(bbox.max()+Vector(0.0, 1.0e-4, 0.0));
    }
    if (fabs(size.z())<1.e-4)
    {
      size.z(2.e-4);
      bbox.extend(bbox.min()-Vector(0.0, 0.0, 1.0e-4));
      bbox.extend(bbox.max()+Vector(0.0, 0.0, 1.0e-4));
    }
    center = Point(bbox.min() + size/2.);

    Vector sizex(size.x(),0,0);
    Vector sizey(0,size.y(),0);
    Vector sizez(0,0,size.z());

    Point right(center + sizex/2.);
    Point down(center + sizey/2.);
    Point in(center +sizez/2.);

    // Translate * Rotate * Scale.
    Transform r;
    box_initial_transform_.load_identity();
    box_initial_transform_.pre_scale(Vector((right-center).length(),
					    (down-center).length(),
					    (in-center).length()));
    r.load_frame((right-center).safe_normal(),
		 (down-center).safe_normal(),
		 (in-center).safe_normal());
    box_initial_transform_.pre_trans(r);
    box_initial_transform_.pre_translate(center.asVector());

    const double newscale = size.length() * 0.015;
    double bscale = box_real_scale_.get();
    if (bscale < newscale * 1e-2 || bscale > newscale * 1e2)
    {
      bscale = newscale;
    }
    box_->SetScale(bscale); // callback sets box_scale for us.
    box_->SetPosition(center, right, down, in);
    box_->SetCurrentMode(box_mode_.get());
    box_center_.set(center);
    box_right_.set(right);
    box_down_.set(down);
    box_in_.set(in);
    box_scale_.set(-1.0);
  }
  else
  {

    const double l2norm = (box_right_.get().vector() +
			   box_down_.get().vector() +
			   box_in_.get().vector()).length();
    const double newscale = l2norm * 0.015;
    double bscale = box_real_scale_.get();
    if (bscale < newscale * 1e-2 || bscale > newscale * 1e2)
    {
      bscale = newscale;
    }
    box_->SetScale(bscale); // callback sets box_scale for us.
    box_->SetPosition(box_center_.get(), box_right_.get(),
		      box_down_.get(), box_in_.get());
    box_->SetCurrentMode(box_mode_.get());
  }

  GeomGroup *widget_group = new GeomGroup;
  widget_group->add(box_->GetWidget());

  GeometryOPortHandle ogport;
  get_oport_handle("Transformation Widget",ogport);
  widgetid_ = ogport->addObj(widget_group,"EditMeshBoundingBox Transform widget", &widget_lock_);
  ogport->flushViews();
}


void
EditMeshBoundingBox::execute()
{
  FieldHandle fh;
  if (!get_input_handle("Input Field", fh,false))
  {
    clear_vals();
    return;
  }

  box_->SetRestrictX(restrict_x_.get());
  box_->SetRestrictY(restrict_y_.get());
  box_->SetRestrictZ(restrict_z_.get());
  box_->SetRestrictR(restrict_r_.get());
  box_->SetRestrictD(restrict_d_.get());
  box_->SetRestrictI(restrict_i_.get());
  int val = restrict_translation_.get() ;
  if (val== 0) box_->UnRestrictTranslation();
  else if (val== 1) box_->RestrictTranslationXYZ();
  else if (val== 2) box_->RestrictTranslationRDI();

  // The output port is required.
  update_state(Executing);

  // build the transform widget and set the the initial
  // field transform.

  if (generation_ != fh.get_rep()->generation || resetting_.get())
  {
    generation_ = fh.get_rep()->generation;
    // get and display the attributes of the input field
    update_input_attributes(fh);
    build_widget(fh, resetting_.get());
    BBox bbox = fh->vmesh()->get_bounding_box();
    if (!bbox.valid()) {
      warning("Input field is empty -- using unit cube.");
      bbox.extend(Point(0,0,0));
      bbox.extend(Point(1,1,1));
    }
    Vector size(bbox.max()-bbox.min());
    if (fabs(size.x())<1.e-4)
    {
      size.x(2.e-4);
      bbox.extend(bbox.min()-Vector(1.e-4,0,0));
    }
    if (fabs(size.y())<1.e-4)
    {
      size.y(2.e-4);
      bbox.extend(bbox.min()-Vector(0,1.e-4,0));
    }
    if (fabs(size.z())<1.e-4)
    {
      size.z(2.e-4);
      bbox.extend(bbox.min()-Vector(0,0,1.e-4));
    }
    Point center(bbox.min() + size/2.);
    Vector sizex(size.x(),0,0);
    Vector sizey(0,size.y(),0);
    Vector sizez(0,0,size.z());

    Point right(center + sizex/2.);
    Point down(center + sizey/2.);
    Point in(center +sizez/2.);

    Transform r;
    Point unused;
    field_initial_transform_.load_identity();

    double sx = (right-center).length();
    double sy = (down-center).length();
    double sz = (in-center).length();

    if (sx < 1e-12) sx = 1.0;
    if (sy < 1e-12) sy = 1.0;
    if (sz < 1e-12) sz = 1.0;

    field_initial_transform_.pre_scale(Vector(sx,sy,sz));
    r.load_frame((right-center).safe_normal(),
		 (down-center).safe_normal(),
		 (in-center).safe_normal());

    field_initial_transform_.pre_trans(r);
    field_initial_transform_.pre_translate(center.asVector());

    resetting_.set(0);
  }

  if (useoutputsize_.get() || useoutputcenter_.get())
  {
    Point center, right, down, in;
    outputcenterx_.reset(); outputcentery_.reset(); outputcenterz_.reset();
    outputsizex_.reset(); outputsizey_.reset(); outputsizez_.reset();
    if (outputsizex_.get() < 0 ||
      outputsizey_.get() < 0 ||
      outputsizez_.get() < 0)
    {
      error("Degenerate BBox requested.");
      return;                    // degenerate
    }
    Vector sizex, sizey, sizez;
    box_->GetPosition(center,right,down,in);
    if (useoutputsize_.get())
    {
      sizex=Vector(outputsizex_.get(),0,0);
      sizey=Vector(0,outputsizey_.get(),0);
      sizez=Vector(0,0,outputsizez_.get());
    }
    else
    {
      sizex=(right-center)*2;
      sizey=(down-center)*2;
      sizez=(in-center)*2;
    }
    if (useoutputcenter_.get())
    {
      center = Point(outputcenterx_.get(),
		     outputcentery_.get(),
		     outputcenterz_.get());
    }
    right = Point(center + sizex/2.);
    down = Point(center + sizey/2.);
    in = Point(center + sizez/2.);

    box_->SetPosition(center,right,down,in);
  }

  // Transform the mesh if necessary.
  // Translate * Rotate * Scale.

  Point center, right, down, in;
  box_->GetPosition(center, right, down, in);

  Transform t, r;
  Point unused;
  t.load_identity();
  t.pre_scale(Vector((right-center).length(),
       (down-center).length(),
       (in-center).length()));
  r.load_frame((right-center).safe_normal(),
               (down-center).safe_normal(),
               (in-center).safe_normal());
  t.pre_trans(r);
  t.pre_translate(center.asVector());


  Transform inv(field_initial_transform_);
  inv.invert();
  t.post_trans(inv);

  // Change the input field handle here.
  fh.detach();

  fh->mesh_detach();
  fh->vmesh()->transform(t);

  send_output_handle("Output Field", fh);

  // Convert the transform into a matrix and send it out.
  MatrixHandle mh = new DenseMatrix(t);
  send_output_handle("Transformation Matrix", mh);

}


void
EditMeshBoundingBox::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    Point center, right, down, in;
    outputcenterx_.reset(); outputcentery_.reset(); outputcenterz_.reset();
    outputsizex_.reset(); outputsizey_.reset(); outputsizez_.reset();
    box_->GetPosition(center,right,down,in);
    outputcenterx_.set(center.x());
    outputcentery_.set(center.y());
    outputcenterz_.set(center.z());
    outputsizex_.set((right.x()-center.x())*2.);
    outputsizey_.set((down.y()-center.y())*2.);
    outputsizez_.set((in.z()-center.z())*2.);
    box_mode_.set(box_->GetMode());
    box_center_.set(center);
    box_right_.set(right);
    box_down_.set(down);
    box_in_.set(in);
    box_scale_.set(1.0);
    want_to_execute();
  }
  box_real_scale_.set(box_->GetScale());
}


} // End namespace SCIRun

/*/*
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


/// @file    SelectFieldROIWithBoxWidget.h
/// @author  Michael Callahan
/// @date    August 2001


#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Core/Thread/CrowdMonitor.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Widgets/BoxWidget.h>
#include <Core/Datatypes/Clipper.h>
#include <Dataflow/GuiInterface/GuiVar.h>

#include <iostream>
#include <stdio.h>

namespace SCIRun {

/// @class SelectFieldROIWithBoxWidget
/// @brief Select data from a field.

class SelectFieldROIWithBoxWidget : public Module
{
  public:
    SelectFieldROIWithBoxWidget(GuiContext* ctx);
    virtual ~SelectFieldROIWithBoxWidget();

    virtual void execute();

    virtual void presave();
    virtual void post_read();

  private:
    FieldHandle output_field_;
    CrowdMonitor widget_lock_;
    BoxWidget *box_;

    GuiInt value_;
    GuiInt mode_;  // 0 nothing 1 accumulate 2 replace

    int  last_generation_;
    BBox last_bounds_;
    int  widgetid_;
    GuiDouble widget_scale_;
    GuiInt    widget_mode_;

};


DECLARE_MAKER(SelectFieldROIWithBoxWidget)
SelectFieldROIWithBoxWidget::SelectFieldROIWithBoxWidget(GuiContext* ctx)
  : Module("SelectFieldROIWithBoxWidget", ctx, Filter, "MiscField", "SCIRun"),
    widget_lock_("SelectFieldROIWithBoxWidget widget lock"),
    value_(get_ctx()->subVar("stampvalue"), 100),
    mode_(get_ctx()->subVar("runmode"), 0),
    last_generation_(0),
    widgetid_(0),
    widget_scale_(get_ctx()->subVar("widget-scale"),0.1),
    widget_mode_(get_ctx()->subVar("widget-mode"),0)
{
  box_ = new BoxWidget(this, &widget_lock_, 1.0, false, false);
  GeometryOPortHandle ogport;
  get_oport_handle("Selection Widget",ogport);
  box_->Connect(ogport.get_rep());
}

void
SelectFieldROIWithBoxWidget::post_read()
{
  const std::string modName = get_ctx()->getfullname() + "-";
  std::string val;

  if( TCLInterface::get(modName+"widget-scale", val, get_ctx()) )
  {
    if (box_)
    {
      box_->SetScale(widget_scale_.get());
      box_->SetCurrentMode(widget_mode_.get());
    }
  }
}

void
SelectFieldROIWithBoxWidget::presave()
{
  if (box_)
  {
    widget_scale_.set(box_->GetScale());
    widget_mode_.set(box_->GetMode());
  }
}


SelectFieldROIWithBoxWidget::~SelectFieldROIWithBoxWidget()
{
  delete box_;
}



void
SelectFieldROIWithBoxWidget::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
  get_input_handle("Input Field", ifieldhandle, true);

  if (!(ifieldhandle->vfield()->is_scalar()))
  {
    error("This module only works on scalar fields.");
    return;
  }

  bool forward_p = false;

  if (output_field_.get_rep() == 0 ||
      last_generation_ != ifieldhandle->generation)
  {
    update_state(Executing);

    FieldInformation fi(ifieldhandle);
    fi.make_int();
    output_field_ = CreateField(fi,ifieldhandle->mesh());

    // Copy the properties.
    output_field_->copy_properties(ifieldhandle.get_rep());

    last_generation_ = ifieldhandle->generation;

    BBox obox = output_field_->vmesh()->get_bounding_box();
    if (!(last_bounds_.valid() && obox.valid() &&
	  obox.min() == last_bounds_.min() &&
	  obox.max() == last_bounds_.max()))
    {
      // Update the widget.
      const BBox bbox = output_field_->vmesh()->get_bounding_box();
      const Point &bmin = bbox.min();
      const Point &bmax = bbox.max();

      const Point center = bmin + Vector(bmax - bmin) * 0.25;
      const Point right = center + Vector((bmax.x()-bmin.x())/4.0, 0, 0);
      const Point down = center + Vector(0, (bmax.y()-bmin.y())/4.0, 0);
      const Point in = center + Vector(0, 0, (bmax.z()-bmin.z())/4.0);

      const double l2norm = (bmax - bmin).length();

      box_->SetScale(l2norm * 0.015);
      box_->SetPosition(center, right, down, in);

      GeomGroup *widget_group = new GeomGroup;
      widget_group->add(box_->GetWidget());

      GeometryOPortHandle ogport;
      get_oport_handle("Selection Widget",ogport);
      widgetid_ = ogport->addObj(widget_group, "SelectFieldROIWithBoxWidget Selection Widget",
				 &widget_lock_);
      ogport->flushViews();

      last_bounds_ = obox;
    }
    forward_p = true;
  }

  if (mode_.get() == 1 || mode_.get() == 2)
  {

    ClipperHandle clipper = box_->get_clipper();
    double value = value_.get();

    bool replace_p = false;
    if (mode_.get() == 2) { replace_p = true; }

    output_field_.detach();
    VField* ofield = output_field_->vfield();

    VField::size_type num_values = ofield->num_values();
    for (VField::index_type idx=0; idx<num_values;idx++)
    {
      Point p;
      ofield->get_center(p,idx);
      if (clipper->inside_p(p))
      {
        ofield->set_value(value,idx);
      }
      else if (replace_p)
      {
        ofield->set_value(0.0,idx);
      }
    }

    forward_p = true;
  }

  if (forward_p)
  {
    send_output_handle("Output Field", output_field_, true);
  }
}


} // End namespace SCIRun

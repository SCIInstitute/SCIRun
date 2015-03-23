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

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Core/Thread/CrowdMonitor.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/Widgets/PointWidget.h>

#include <iostream>
#include <stack>

namespace SCIRun {

using std::stack;

/// @class GenerateSinglePointProbeFromField
/// @brief This module generates a single probe. 

class GenerateSinglePointProbeFromField : public Module
{
  public:
    GenerateSinglePointProbeFromField(GuiContext* ctx);
    virtual ~GenerateSinglePointProbeFromField();

    virtual void execute();
    virtual void widget_moved(bool, BaseWidget*);
    
    virtual void tcl_command(GuiArgs& args, void* userdata);
  private:
    PointWidget *widget_;
    CrowdMonitor widget_lock_;
    int  last_input_generation_;
    BBox last_bounds_;

    GuiString gui_frame_;
    GuiDouble gui_locx_;
    GuiDouble gui_locy_;
    GuiDouble gui_locz_;
    
    GuiString gui_value_;
    GuiString gui_node_;
    GuiString gui_elem_;
    
    GuiInt gui_show_value_;
    GuiInt gui_show_node_;
    GuiInt gui_show_elem_;
    GuiString gui_moveto_;
    GuiDouble gui_probe_scale_;

    GuiString gui_label_;
    GuiDouble gui_color_r_;
    GuiDouble gui_color_g_;
    GuiDouble gui_color_b_;

    int widgetid_;

    double l2norm_;

    bool color_changed_;
};


DECLARE_MAKER(GenerateSinglePointProbeFromField)

GenerateSinglePointProbeFromField::GenerateSinglePointProbeFromField(GuiContext* ctx)
  : Module("GenerateSinglePointProbeFromField", ctx, Filter, "NewField", "SCIRun"),
    widget_lock_("GenerateSinglePointProbeFromField widget lock"),
    last_input_generation_(0),
    gui_frame_(get_ctx()->subVar("main_frame"), ""),
    gui_locx_(get_ctx()->subVar("locx"), 0.0),
    gui_locy_(get_ctx()->subVar("locy"), 0.0),
    gui_locz_(get_ctx()->subVar("locz"), 0.0),
    gui_value_(get_ctx()->subVar("value"), ""),
    gui_node_(get_ctx()->subVar("node"), ""),
    gui_elem_(get_ctx()->subVar("elem"), ""),
    gui_show_value_(get_ctx()->subVar("show-value"), 1),
    gui_show_node_(get_ctx()->subVar("show-node"), 1),
    gui_show_elem_(get_ctx()->subVar("show-elem"), 1),
    gui_moveto_(get_ctx()->subVar("moveto", false), ""),
    gui_probe_scale_(get_ctx()->subVar("probe_scale"), 0.5),
    gui_label_(get_ctx()->subVar("label"),""),
    gui_color_r_(get_ctx()->subVar("color-r"), 1.0),
    gui_color_g_(get_ctx()->subVar("color-g"), 1.0),
    gui_color_b_(get_ctx()->subVar("color-b"), 1.0),    
    widgetid_(0),
    color_changed_(false)
{
  widget_ = new PointWidget(this, &widget_lock_, 1.0);
  GeometryOPortHandle ogport;
  get_oport_handle("GenerateSinglePointProbeFromField Widget",ogport);
  widget_->Connect(ogport.get_rep());
}


GenerateSinglePointProbeFromField::~GenerateSinglePointProbeFromField()
{
  delete widget_;
}


void
GenerateSinglePointProbeFromField::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
  const bool input_field_p =
    get_input_handle("Input Field", ifieldhandle, false);

  update_state(Executing);

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

  if (!bbox.is_similar_to(last_bounds_))
  {
    Point bmin = bbox.min();
    Point bmax = bbox.max();

    // Fix degenerate boxes.
    const double size_estimate = Max((bmax-bmin).length() * 0.01, 1.0e-5);
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

    Point center = bmin + Vector(bmax - bmin) * 0.5;
    l2norm_ = (bmax - bmin).length();

    // If the current location looks reasonable, use that instead
    // of the center.
    Point curloc(gui_locx_.get(), gui_locy_.get(), gui_locz_.get());

    // Invalidate current position if it's outside of our field.
    // Leave it alone if there was no field, as our bbox is arbitrary anyway.
    if (curloc.x() >= bmin.x() && curloc.x() <= bmax.x() && 
        curloc.y() >= bmin.y() && curloc.y() <= bmax.y() && 
        curloc.z() >= bmin.z() && curloc.z() <= bmax.z() ||
        !input_field_p)
    {
      center = curloc;
    }
    
    widget_->SetPosition(center);
    
    GeomGroup *widget_group = new GeomGroup;
    widget_group->add(widget_->GetWidget());

    GeometryOPortHandle ogport;
    get_oport_handle("GenerateSinglePointProbeFromField Widget",ogport);
    widgetid_ = ogport->addObj(widget_group, "GenerateSinglePointProbeFromField Selection Widget",
			       &widget_lock_);
    ogport->flushViews();

    last_bounds_ = bbox;
  }

  widget_->SetScale(gui_probe_scale_.get() * l2norm_ * 0.003);
  widget_->SetColor(Color(gui_color_r_.get(),gui_color_g_.get(),gui_color_b_.get()));
  widget_->SetLabel(gui_label_.get());

  const std::string &moveto = gui_moveto_.get();
  bool moved_p = false;
  if (moveto == "location")
  {
    const Point newloc(gui_locx_.get(), gui_locy_.get(), gui_locz_.get());
    widget_->SetPosition(newloc);

    moved_p = true;
  }
  else if (moveto == "center")
  {
    Point bmin = bbox.min();
    Point bmax = bbox.max();

    // Fix degenerate boxes.
    const double size_estimate = Max((bmax-bmin).length() * 0.01, 1.0e-5);
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

    Point center = bmin + Vector(bmax - bmin) * 0.5;
    widget_->SetColor(Color(gui_color_r_.get(),gui_color_g_.get(),gui_color_b_.get()));
    widget_->SetLabel(gui_label_.get());
    widget_->SetPosition(center);
    moved_p = true;
  }
  else if (moveto != "" && input_field_p)
  {
    if (moveto == "node")
    {
      VMesh::index_type idx;
      std::string sval = gui_node_.get();
      from_string(sval,idx);
      if (idx >=0 && idx < ifieldhandle->vmesh()->num_nodes())
      {
        Point p;
        ifieldhandle->vmesh()->get_center(p,VMesh::Node::index_type(idx));
        widget_->SetPosition(p);
        moved_p = true;        
      }
    }
    else if (moveto == "elem")
    {
      VMesh::index_type idx;
      std::string sval = gui_elem_.get();
      from_string(sval,idx);
      if (idx >=0 && idx < ifieldhandle->vmesh()->num_elems())
      {
        Point p;
        ifieldhandle->vmesh()->get_center(p,VMesh::Elem::index_type(idx));
        widget_->SetPosition(p);
        moved_p = true;        
      }
    }
  }
  if (moved_p)
  {
    GeometryOPortHandle ogport;
    get_oport_handle("GenerateSinglePointProbeFromField Widget",ogport);
    ogport->flushViews();
    gui_moveto_.set("");
  }

  const Point location = widget_->GetPosition();
  
  FieldInformation fi("PointCloudMesh",0,"double");
  MeshHandle mesh = CreateMesh(fi);
  mesh->vmesh()->add_point(location);
  
  FieldHandle ofield;

  std::string nodestr, elemstr;
  if (input_field_p)
  {
    if (gui_show_node_.get())
    {
      ifieldhandle->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      Point r;
      VMesh::Node::index_type idx;
      ifieldhandle->vmesh()->find_closest_node(r,idx,location);
      nodestr = to_string(idx);
      gui_node_.set(nodestr);
    }

    if (gui_show_node_.get())
    {
      ifieldhandle->vmesh()->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
      Point r;
      VMesh::Elem::index_type idx;
      ifieldhandle->vmesh()->find_closest_elem(r,idx,location);
      elemstr = to_string(idx);
      gui_elem_.set(elemstr);
    }
  }

  std::ostringstream valstr;
  VField* vfield = 0;
  VMesh* vmesh = 0;
  if (ifieldhandle.get_rep())
  {
    vfield = ifieldhandle->vfield();
    vmesh = ifieldhandle->vmesh();    
  }
  
  if (!input_field_p ||
      ifieldhandle->basis_order() == -1 ||
      !gui_show_value_.get())
  {
    fi.make_double();
    ofield = CreateField(fi,mesh);
    ofield->vfield()->resize_values();
    valstr << 0;
    ofield->vfield()->set_value(0.0, VMesh::index_type(0));
  }
  else if (vfield->is_scalar())
  {
    double result = 0.0;
    if (!vfield->interpolate(result, location))
    {
      Point closest;
      VMesh::Node::index_type node_idx;
      if(vmesh->find_closest_node(closest,node_idx,location))
        vfield->get_value(result,node_idx);
    }
    valstr << result;

    fi.make_double();
    ofield = CreateField(fi,mesh);
    ofield->vfield()->set_value(result, VMesh::index_type(0));
  }
  else if (vfield->is_vector())
  {
    Vector result(0.0,0.0,0.0);
    if (!vfield->interpolate(result, location))
    {
      Point closest;
      VMesh::Node::index_type node_idx;
      if (vmesh->find_closest_node(closest,node_idx,location))
        vfield->get_value(result,node_idx);
    }
    valstr << result;

    fi.make_vector();
    ofield = CreateField(fi,mesh);
    ofield->vfield()->set_value(result, VMesh::index_type(0));
  }
  else if (vfield->is_tensor())
  {
    Tensor result(0.0);
    if (!vfield->interpolate(result, location))
    {
      Point closest;
      VMesh::Node::index_type node_idx;
      if(vmesh->find_closest_node(closest,node_idx,location))
        vfield->get_value(result,node_idx);
    }
    valstr << result;

    fi.make_tensor();
    ofield = CreateField(fi,mesh);
    ofield->vfield()->set_value(result, VMesh::index_type(0));
  }
  
  gui_locx_.set(location.x());
  gui_locy_.set(location.y());
  gui_locz_.set(location.z());
  if (gui_show_value_.get()) { gui_value_.set(valstr.str()); }

  send_output_handle("GenerateSinglePointProbeFromField Point", ofield);

  if (input_field_p)
  {
    index_type index = 0;

    if (gui_show_value_.get())
    {
      from_string(valstr.str(),index);
    }
    else if (gui_show_node_.get())
    {
      from_string(nodestr,index);  
    }
    else if (gui_show_elem_.get())
    {
      from_string(elemstr,index);    
    }
    else
    {
      from_string(valstr.str(),index);    
    }    
    
    MatrixHandle cm = new DenseMatrix(static_cast<double>(index));
    send_output_handle("Element Index", cm);
  }
}


void
GenerateSinglePointProbeFromField::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    want_to_execute();
  }
}



void 
GenerateSinglePointProbeFromField::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2) 
  {
    args.error("ShowString needs a minor command");
    return;
  }

  if (args[1] == "color_change") 
  {
    color_changed_ = true;
  } 
  else 
  {
    Module::tcl_command(args, userdata);
  }
}


} // End namespace SCIRun



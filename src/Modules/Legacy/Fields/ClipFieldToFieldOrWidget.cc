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
///@file  ClipFieldToFieldOrWidget.cc
///
///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001
///

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Clipper.h>
#include <Core/Util/StringUtil.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Core/Thread/CrowdMonitor.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Widgets/BoxWidget.h>

#include <iostream>
#include <stack>

namespace SCIRun {

using std::stack;

/// @class ClipFieldToFieldOrWidget
/// @brief This module selects a subset of a field using a field or a widget.

class ClipFieldToFieldOrWidget : public Module
{
  private:
    BoxWidget *box_;
    CrowdMonitor widget_lock_;
    BBox last_bounds_;
    GuiString clip_location_;
    GuiString clip_mode_;
    GuiInt    autoexec_;
    GuiInt    autoinvert_;
    GuiString exec_mode_;
    GuiDouble center_x_;
    GuiDouble center_y_;
    GuiDouble center_z_;
    GuiDouble right_x_;
    GuiDouble right_y_;
    GuiDouble right_z_;
    GuiDouble down_x_;
    GuiDouble down_y_;
    GuiDouble down_z_;
    GuiDouble in_x_;
    GuiDouble in_y_;
    GuiDouble in_z_;
    GuiDouble scale_;
    bool      first_pass_;
    int  last_input_generation_;
    int  last_clip_generation_;
    ClipperHandle clipper_;
    stack<ClipperHandle> undo_stack_;
    int widgetid_;
    FieldHandle ofield_;

    // check to see if any of the Points are not the default of -1
    bool points_differ();

  public:
    ClipFieldToFieldOrWidget(GuiContext* ctx);
    virtual ~ClipFieldToFieldOrWidget();

    virtual void execute();
    virtual void widget_moved(bool, BaseWidget*);

    GuiDouble widget_scale_;
    GuiInt    widget_mode_;

    virtual void presave();
    virtual void post_read();
};


DECLARE_MAKER(ClipFieldToFieldOrWidget)

ClipFieldToFieldOrWidget::ClipFieldToFieldOrWidget(GuiContext* ctx)
  : Module("ClipFieldToFieldOrWidget", ctx, Filter, "NewField", "SCIRun"),
    widget_lock_("ClipFieldToFieldOrWidget widget lock"),
    clip_location_(get_ctx()->subVar("clip-location"), "cell"),
    clip_mode_(get_ctx()->subVar("clipmode"), "replace"),
    autoexec_(get_ctx()->subVar("autoexecute"), 0),
    autoinvert_(get_ctx()->subVar("autoinvert"), 0),
    exec_mode_(get_ctx()->subVar("execmode"), "0"),
    center_x_(get_ctx()->subVar("center_x"), -1.0),
    center_y_(get_ctx()->subVar("center_y"), -1.0),
    center_z_(get_ctx()->subVar("center_z"), -1.0),
    right_x_(get_ctx()->subVar("right_x"), -1.0),
    right_y_(get_ctx()->subVar("right_y"), -1.0),
    right_z_(get_ctx()->subVar("right_z"), -1.0),
    down_x_(get_ctx()->subVar("down_x"), -1.0),
    down_y_(get_ctx()->subVar("down_y"), -1.0),
    down_z_(get_ctx()->subVar("down_z"), -1.0),
    in_x_(get_ctx()->subVar("in_x"), -1.0),
    in_y_(get_ctx()->subVar("in_y"), -1.0),
    in_z_(get_ctx()->subVar("in_z"), -1.0),
    scale_(get_ctx()->subVar("scale"), -1.0),
    first_pass_(true),
    last_input_generation_(0),
    last_clip_generation_(0),
    widgetid_(0),
    ofield_(0),
    widget_scale_(get_ctx()->subVar("widget-scale"),0.1),
    widget_mode_(get_ctx()->subVar("widget-mode"),0)
{
  box_ = new BoxWidget(this, &widget_lock_, 1.0, false, false);
  GeometryOPortHandle ogport;
  get_oport_handle("Selection Widget",ogport);
  box_->Connect(ogport.get_rep());
}


void
ClipFieldToFieldOrWidget::post_read()
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
ClipFieldToFieldOrWidget::presave()
{
  if (box_)
  {
    widget_scale_.set(box_->GetScale());
    widget_mode_.set(box_->GetMode());
  }
}

ClipFieldToFieldOrWidget::~ClipFieldToFieldOrWidget()
{
  delete box_;
}

bool
ClipFieldToFieldOrWidget::points_differ()
{
  reset_vars();
  if (center_x_.get() != -1 || center_y_.get() != -1 || center_z_.get() != -1 ||
      right_x_.get() != -1 || right_y_.get() != -1 || right_z_.get() != -1 ||
      down_x_.get() != -1 || down_y_.get() != -1 || down_z_.get() != -1 ||
      in_x_.get() != -1 || in_y_.get() != -1 || in_z_.get() != -1) {
    return true;
  }
  return false;
}


void
ClipFieldToFieldOrWidget::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
  get_input_handle("Input Field", ifieldhandle,true);

  if (!ifieldhandle->vmesh()->is_editable())
  {
    error("Not an editable mesh type (try passing Field through an ConvertMeshToUnstructuredMesh module first).");
    return;
  }

  bool do_clip_p = false;

  FieldHandle cfieldhandle;
  if (get_input_handle("Clip Field", cfieldhandle, false) &&
      cfieldhandle->generation != last_clip_generation_)
  {
    last_clip_generation_ = cfieldhandle->generation;
    clipper_ = new MeshClipper(cfieldhandle->vmesh());
    do_clip_p = true;
  }

  update_state(Executing);

  // Update the widget.
  const BBox bbox = ifieldhandle->vmesh()->get_bounding_box();
  if (first_pass_ && points_differ())
  {
    Point center(center_x_.get(), center_y_.get(), center_z_.get());
    Point right(right_x_.get(), right_y_.get(), right_z_.get());
    Point down(down_x_.get(), down_y_.get(), down_z_.get());
    Point in(in_x_.get(), in_y_.get(), in_z_.get());

    box_->SetPosition(center, right, down, in);

    GeomGroup *widget_group = new GeomGroup;
    widget_group->add(box_->GetWidget());

    GeometryOPortHandle ogport;
    get_oport_handle("Selection Widget",ogport);
    widgetid_ = ogport->addObj(widget_group, "ClipFieldToFieldOrWidget Selection Widget",
			       &widget_lock_);
    ogport->flushViews();

    last_bounds_ = bbox;
    // Force clipper to sync with new widget.
    if (clipper_.get_rep() && !clipper_->mesh_p()) { clipper_ = 0; }
    first_pass_ = false;
  }
  else if (!bbox.is_similar_to(last_bounds_) || exec_mode_.get() == "reset")
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
    widgetid_ = ogport->addObj(widget_group, "ClipFieldToFieldOrWidget Selection Widget",
			       &widget_lock_);
    ogport->flushViews();

    last_bounds_ = bbox;
    // Force clipper to sync with new widget.
    if (clipper_.get_rep() && !clipper_->mesh_p()) { clipper_ = 0; }
    first_pass_ = false;
  }

  if (!clipper_.get_rep())
  {
    clipper_ = box_->get_clipper();
    do_clip_p = true;
  }
  else if (exec_mode_.get() == "execute" && !clipper_->mesh_p())
  {
    undo_stack_.push(clipper_);
    ClipperHandle ctmp = box_->get_clipper();
    if (clip_mode_.get() == "intersect")
    {
      clipper_ = new IntersectionClipper(ctmp, clipper_);
    }
    else if (clip_mode_.get() == "union")
    {
      clipper_ = new UnionClipper(ctmp, clipper_);
    }
    else if (clip_mode_.get() == "remove")
    {
      ctmp = new InvertClipper(ctmp);
      clipper_ = new IntersectionClipper(ctmp, clipper_);
    }
    else
    {
      clipper_ = ctmp;
    }
    do_clip_p = true;
  }
  else if (exec_mode_.get() == "invert")
  {
    undo_stack_.push(clipper_);
    clipper_ = new InvertClipper(clipper_);
    do_clip_p = true;
  }
  else if (exec_mode_.get() == "undo")
  {
    if (!undo_stack_.empty())
    {
      clipper_ = undo_stack_.top();
      undo_stack_.pop();
      do_clip_p = true;
    }
  }
  else if (exec_mode_.get() == "location")
  {
    do_clip_p = true;
  }

  if (do_clip_p || ifieldhandle->generation != last_input_generation_ ||
      !ofield_.get_rep())
  {
    last_input_generation_ = ifieldhandle->generation;
    exec_mode_.set("");

    // Maybe invert the clipper again.
    ClipperHandle clipper(clipper_);
    if (autoinvert_.get())
    {
      clipper = new InvertClipper(clipper_);
    }

    // Do the clip, dispatch based on which clip location test we are using.
    FieldInformation fi(ifieldhandle);
    ofield_ = CreateField(fi);

    VMesh* mesh =    ifieldhandle->vmesh();
    VMesh* clipped = ofield_->vmesh();
    VField* ofield = ofield_->vfield();
    VField* ifield = ifieldhandle->vfield();
    ofield->copy_properties(ifield);
     #ifdef HAVE_HASH_MAP
    # if defined(__ECC) || defined(_MSC_VER)
      typedef hash_map<VMesh::index_type, VMesh::index_type> hash_type;
    # else
      typedef hash_map<VMesh::index_type,VMesh::index_type,
        hash<unsigned int>,
        std::equal_to<unsigned int> > hash_type;
    # endif
    #else
      typedef std::map<VMesh::index_type,VMesh::Node::index_type,
        std::less<unsigned int> > hash_type;
    #endif

    hash_type nodemap;
    std::vector<VMesh::Elem::index_type> elemmap;
    VMesh::Elem::size_type num_elems = mesh->num_elems();

    if ((clip_location_.get() == "nodeone")||(clip_location_.get() == "nodeall"))
    {
      bool any_inside_p = (clip_location_.get() == "nodeone");

      int cnt = 0;
      for(VMesh::Elem::index_type idx=0; idx< num_elems; idx++)
      {
        cnt++; if (cnt == 100) {cnt = 0; update_progress(idx,num_elems); }

        VMesh::Node::array_type onodes;
        mesh->get_nodes(onodes, idx);

        bool inside_p;
        size_t i;
        if (any_inside_p)
        {
          inside_p = false;
          for (i = 0; i < onodes.size(); i++)
          {
            Point p;
            mesh->get_center(p, onodes[i]);
            if (clipper->inside_p(p)) { inside_p = true; break; }
          }
        }
        else
        {
          inside_p = true;
          for (i = 0; i < onodes.size(); i++)
          {
            Point p;
            mesh->get_center(p, onodes[i]);
            if (!clipper->inside_p(p)) { inside_p = false; break; }
          }
        }

        if (inside_p)
        {
          // Add this element to the new mesh.
          VMesh::Node::array_type nnodes(onodes.size());

          for (size_t i = 0; i<onodes.size(); i++)
          {
            if (nodemap.find(onodes[i]) == nodemap.end())
            {
              Point np;
              mesh->get_center(np, onodes[i]);
              VMesh::Node::index_type nodeindex = clipped->add_point(np);
              nodemap[onodes[i]] = nodeindex;
              nnodes[i] = nodeindex;
            }
            else
            {
              nnodes[i] = nodemap[onodes[i]];
            }
          }

          clipped->add_elem(nnodes);
          elemmap.push_back(idx); // Assumes elements always added to end.
        }
      }

      ofield->resize_values();

      if (ifield->basis_order() == 1)
      {
        hash_type::iterator hitr = nodemap.begin();

        while (hitr != nodemap.end())
        {
          ofield->copy_value(ifield,(*hitr).first,(*hitr).second);
          ++hitr;
        }
      }
      else if ((ifield->basis_order() == ofield->basis_order())&&(ifield->basis_order() >= 0))
      {
        for (size_t i=0; i < elemmap.size(); i++)
        {
          ofield->copy_value(ifield,elemmap[i],i);
        }
      }
    }
    else // 'cell' and default
    {
      int cnt = 0;
      for(VMesh::Elem::index_type idx=0; idx< num_elems; idx++)
      {
        cnt++; if (cnt == 100) {cnt = 0; update_progress(idx,num_elems); }

        Point p;
        mesh->get_center(p, idx);
        if (clipper->inside_p(p))
        {
          // Add this element to the new mesh.
          VMesh::Node::array_type onodes;
          mesh->get_nodes(onodes, idx);
          VMesh::Node::array_type nnodes(onodes.size());

          for (size_t i=0; i<onodes.size(); i++)
          {
            if (nodemap.find(onodes[i]) == nodemap.end())
            {
              Point np;
              mesh->get_center(np, onodes[i]);
              const VMesh::Node::index_type nodeindex =
                clipped->add_point(np);
              nodemap[onodes[i]] = nodeindex;
              nnodes[i] = nodeindex;
            }
            else
            {
              nnodes[i] = nodemap[onodes[i]];
            }
          }

          clipped->add_elem(nnodes);
          elemmap.push_back(idx); // Assumes elements always added to end.
        }
      }

      ofield->resize_values();

      if (ifield->basis_order() == 1)
      {
        hash_type::iterator hitr = nodemap.begin();

        while (hitr != nodemap.end())
        {
          ofield->copy_value(ifield,(*hitr).first,(*hitr).second);
          ++hitr;
        }
      }
      else if ((ifield->basis_order() == ofield->basis_order())&&(ifield->basis_order() >= 0))
      {
        for (size_t i=0; i < elemmap.size(); i++)
        {
          ofield->copy_value(ifield,elemmap[i],i);
        }
      }
    }
  }

  send_output_handle("Output Field", ofield_, true);
}


void
ClipFieldToFieldOrWidget::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    if (!first_pass_)
    {
      Point center, right, down, in;
      box_->GetPosition(center, right, down, in);
      center_x_.set(center.x());
      center_y_.set(center.y());
      center_z_.set(center.z());
      right_x_.set(right.x());
      right_y_.set(right.y());
      right_z_.set(right.z());
      down_x_.set(down.x());
      down_y_.set(down.y());
      down_z_.set(down.z());
      in_x_.set(in.x());
      in_y_.set(in.y());
      in_z_.set(in.z());
      scale_.set(box_->GetScale());
    }
    autoexec_.reset();
    if (autoexec_.get())
    {
      exec_mode_.set("execute");
      want_to_execute();
    }
  }
}

} // End namespace SCIRun

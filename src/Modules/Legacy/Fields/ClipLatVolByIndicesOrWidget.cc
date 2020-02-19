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
///@file  ClipLatVolByIndicesOrWidget.cc
///
///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001
///

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Core/Thread/CrowdMonitor.h>
#include <Dataflow/Widgets/BoxWidget.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <iostream>

namespace SCIRun {

/// @class ClipLatVolByIndicesOrWidget
/// @brief Rotate and flip field to get it into "standard" view

class ClipLatVolByIndicesOrWidget : public Module
{
  private:
    BoxWidget *widget_;
    CrowdMonitor widget_lock_;
    BBox last_bounds_;
    bool gui_exec_p_;
    int  last_input_generation_;
    GuiInt use_text_bbox_;
    GuiDouble text_min_x_;
    GuiDouble text_min_y_;
    GuiDouble text_min_z_;
    GuiDouble text_max_x_;
    GuiDouble text_max_y_;
    GuiDouble text_max_z_;
    bool init_;
    int widgetid_;

  public:
    ClipLatVolByIndicesOrWidget(GuiContext* ctx);
    virtual ~ClipLatVolByIndicesOrWidget();

    virtual void execute();
    virtual void tcl_command(GuiArgs&, void*);
    virtual void widget_moved(bool last, BaseWidget*);

    GuiDouble widget_scale_;
    GuiInt    widget_mode_;

    virtual void presave();
    virtual void post_read();
};


DECLARE_MAKER(ClipLatVolByIndicesOrWidget)

ClipLatVolByIndicesOrWidget::ClipLatVolByIndicesOrWidget(GuiContext* ctx)
  : Module("ClipLatVolByIndicesOrWidget", ctx, Filter, "NewField", "SCIRun"),
    widget_lock_("ClipLatVolByIndicesOrWidget widget lock"),
    gui_exec_p_(true),
    last_input_generation_(0),
    use_text_bbox_(get_ctx()->subVar("use-text-bbox"), 0),
    text_min_x_(get_ctx()->subVar("text-min-x"), 0.0),
    text_min_y_(get_ctx()->subVar("text-min-y"), 0.0),
    text_min_z_(get_ctx()->subVar("text-min-z"), 0.0),
    text_max_x_(get_ctx()->subVar("text-max-x"), 0.0),
    text_max_y_(get_ctx()->subVar("text-max-y"), 0.0),
    text_max_z_(get_ctx()->subVar("text-max-z"), 0.0),
    init_(false),
    widgetid_(0),
    widget_scale_(get_ctx()->subVar("widget-scale"),0.1),
    widget_mode_(get_ctx()->subVar("widget-mode"),0)
{
  widget_ = new BoxWidget(this, &widget_lock_, 1.0, true, false);
  GeometryOPortHandle ogport;
  get_oport_handle("Selection Widget",ogport);
  widget_->Connect(ogport.get_rep());
}


ClipLatVolByIndicesOrWidget::~ClipLatVolByIndicesOrWidget()
{
  delete widget_;
}

void
ClipLatVolByIndicesOrWidget::post_read()
{
  const std::string modName = get_ctx()->getfullname() + "-";
  std::string val;

  if( TCLInterface::get(modName+"widget-scale", val, get_ctx()) )
  {
    if (widget_)
    {
      widget_->SetScale(widget_scale_.get());
      widget_->SetCurrentMode(widget_mode_.get());
    }
  }
}

void
ClipLatVolByIndicesOrWidget::presave()
{
  if (widget_)
  {
    widget_scale_.set(widget_->GetScale());
    widget_mode_.set(widget_->GetMode());
  }
}


void
ClipLatVolByIndicesOrWidget::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
  if (!get_input_handle("Input Field", ifieldhandle)) return;

	FieldInformation fi(ifieldhandle);
	if (!fi.is_latvol())
  {
    error("Input Field is not a LatVolMesh");
    return;
  }

  update_state(Executing);

  // Update the widget.
  const BBox bbox = ifieldhandle->vmesh()->get_bounding_box();
  if (!bbox.is_similar_to(last_bounds_) ||
      use_text_bbox_.get() || !init_)
  {
    Point center, right, down, in, bmin, bmax;
    bmin = Point(text_min_x_.get(), text_min_y_.get(), text_min_z_.get());
    bmax = Point(text_max_x_.get(), text_max_y_.get(), text_max_z_.get());
    if (use_text_bbox_.get() || (!init_ && bmin!=bmax))
    {
      center = bmin + Vector(bmax-bmin) * 0.5;
      right = center + Vector(bmax.x()-bmin.x()/2.0, 0, 0);
      down = center + Vector(0, bmax.x()-bmin.x()/2.0, 0);
      in = center + Vector(0, 0, bmax.x()-bmin.x()/2.0);
    }
    else
    {
      bmin = bbox.min();
      bmax = bbox.max();
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
      center = bmin + Vector(bmax - bmin) * 0.5;
      right = center + Vector((bmax.x()-bmin.x())/2.0, 0, 0);
      down = center + Vector(0, (bmax.y()-bmin.y())/2.0, 0);
      in = center + Vector(0, 0, (bmax.z()-bmin.z())/2.0);
      text_min_x_.set(right.x());
      text_min_y_.set(down.y());
      text_min_z_.set(in.z());
      text_max_x_.set(2.*center.x()-right.x());
      text_max_y_.set(2.*center.y()-down.y());
      text_max_z_.set(2.*center.z()-in.z());
    }

    const double l2norm = (bmax - bmin).length();

    widget_->SetScale(l2norm * 0.015);
    widget_->SetPosition(center, right, down, in);

    GeomGroup *widget_group = new GeomGroup;
    widget_group->add(widget_->GetWidget());

    GeometryOPortHandle ogport;
    get_oport_handle("Selection Widget",ogport);
    widgetid_ = ogport->addObj(widget_group, "ClipLatVolByIndicesOrWidget Selection Widget",
			       &widget_lock_);
    ogport->flushViews();

    last_bounds_ = bbox;
  }

  if (gui_exec_p_ || ifieldhandle->generation != last_input_generation_)
  {
    last_input_generation_ = ifieldhandle->generation;
    gui_exec_p_ = false;

    // Get widget bounds.
    Point center, r, d, i, top, bottom;
    if (use_text_bbox_.get() || !init_)
    {
      init_=true;
      top = Point(text_max_x_.get(), text_max_y_.get(), text_max_z_.get());
      bottom = Point(text_min_x_.get(), text_min_y_.get(), text_min_z_.get());
      center = bottom + Vector(top-bottom)/2.;
      r=d=i=center;
      r.x(bottom.x());
      d.y(bottom.y());
      i.z(bottom.z());
      widget_->SetPosition(center, r, d, i);
    }
    else
    {
      widget_->GetPosition(center, r, d, i);
      const Vector dx = r - center;
      const Vector dy = d - center;
      const Vector dz = i - center;
      text_min_x_.set(r.x());
      text_min_y_.set(d.y());
      text_min_z_.set(i.z());
      text_max_x_.set(2.*center.x()-r.x());
      text_max_y_.set(2.*center.y()-d.y());
      text_max_z_.set(2.*center.z()-i.z());
      top = center + dx + dy + dz;
      bottom = center - dx - dy - dz;
    }

    // Execute the clip.
    NrrdDataHandle nrrdh = new NrrdData();

    VField* ifield = ifieldhandle->vfield();
    VMesh*  imesh = ifieldhandle->vmesh();

    VMesh::Node::index_type ns, ne;
    imesh->locate(ns, top);
    imesh->locate(ne, bottom);

    const size_type onx = imesh->get_ni();
    const size_type ony = imesh->get_nj();
    const size_type onz = imesh->get_nk();

    index_type si,sj,sk;
    index_type ei,ej,ek;

    imesh->from_index(si,sj,sk,ns);
    imesh->from_index(ei,ej,ek,ne);

    if (si < 0) { si = 0; }
    if (sj < 0) { sj = 0; }
    if (sk < 0) { sk = 0; }
    if (ei < 0) { ei = 0; }
    if (ej < 0) { ej = 0; }
    if (ek < 0) { ek = 0; }

    if (si >= onx) { si = onx - 1; }
    if (sj >= ony) { sj = ony - 1; }
    if (sk >= onz) { sk = onz - 1; }
    if (ei >= onx) { ei = onx - 1; }
    if (ej >= ony) { ej = ony - 1; }
    if (ek >= onz) { ek = onz - 1; }

    index_type tmp;
    if (si > ei) { tmp = ei; ei = si; si = tmp; }
    if (sj > ej) { tmp = ej; ej = sj; sj = tmp; }
    if (sk > ek) { tmp = ek; ek = sk; sk = tmp; }

    const index_type nx = ei - si + 1;
    const index_type ny = ej - sj + 1;
    const index_type nz = ek - sk + 1;

    if (nx < 2 || ny < 2 || nz < 2) return;

    Point bmin(0.0, 0.0, 0.0);
    Point bmax(1.0, 1.0, 1.0);
    FieldInformation fi(ifieldhandle);

    MeshHandle mesh = CreateMesh(fi,nx,ny,nz,bmin,bmax);
    FieldHandle output_field = CreateField(fi,mesh);
    VMesh*  omesh = output_field->vmesh();
    VField* ofield = output_field->vfield();

    Transform trans = imesh->get_transform();
    trans.post_translate(Vector(si, sj, sk));

    omesh->set_transform(trans);
    ofield->copy_properties(ifield);

    if (ifield->basis_order() == 1)
    {
      VMesh::Node::iterator bi, ei;
      omesh->begin(bi);
      omesh->end(ei);
      VMesh::Node::size_type ns;
      imesh->size(ns);
      size_t dim[NRRD_DIM_MAX];
      dim[0] = ns;
      nrrdAlloc_nva(nrrdh->nrrd_, nrrdTypeUChar, 1, dim);
      unsigned char *mask = (unsigned char *)nrrdh->nrrd_->data;
      memset(mask, 0, dim[0]*sizeof(unsigned char));
      index_type ii,jj,kk;
      VMesh::Node::index_type idx;
      while (bi != ei)
      {
        omesh->from_index(ii,jj,kk,*bi);
        imesh->to_index(idx,ii+si,jj+sj,kk+sk);
        mask[(unsigned int)idx] = 1;
        ofield->copy_value(ifield, idx, *bi);
        ++bi;
      }
    }
    else if (ifield->basis_order() == 0)
    {
      VMesh::Elem::iterator bi, ei;
      omesh->begin(bi); omesh->end(ei);
      VMesh::Elem::size_type ns;
      imesh->size(ns);
      size_t dim[NRRD_DIM_MAX];
      dim[0] = ns;
      nrrdAlloc_nva(nrrdh->nrrd_, nrrdTypeUChar, 1, dim);
      unsigned char *mask = (unsigned char *)nrrdh->nrrd_->data;
      memset(mask, 0, dim[0]*sizeof(unsigned char));
      index_type ii,jj,kk;
      VMesh::Elem::index_type idx;
      while (bi != ei)
      {
        omesh->from_index(ii,jj,kk,*bi);
        imesh->to_index(idx,ii+si,jj+sj,kk+sk);
        mask[(unsigned int)idx] = 1;
        ofield->copy_value(ifield,idx,*bi);
        ++bi;
      }
    }

    send_output_handle("Output Field", output_field);
    send_output_handle("MaskVector", nrrdh);
  }
}


void
ClipLatVolByIndicesOrWidget::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    gui_exec_p_ = true;
    want_to_execute();
  }
}

void
ClipLatVolByIndicesOrWidget::tcl_command(GuiArgs& args, void* userdata) {
  if (args.count() < 2) {
    args.error("ClipLatVolByIndicesOrWidget needs a minor command");
    return;
  }
  if (args[1] == "execute") {
    gui_exec_p_ = true;
    want_to_execute();
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}


} // End namespace SCIRun

/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

//    File   : ReportNrrdInfo.cc
//    Author : Martin Cole
//    Date   : Tue Feb  4 08:55:34 2003

#include <Modules/Legacy/Teem/Misc/ReportNrrdInfo.h>
//#include <Core/Util/StringUtil.h>
#include <iostream>


#if 0
namespace SCITeem {

using namespace SCIRun;


class ReportNrrdInfo : public Module {
public:
  ReportNrrdInfo(GuiContext* ctx);
  virtual ~ReportNrrdInfo();

  void clear_vals();
  void update_input_attributes(NrrdDataHandle);
  void update_axis_var(const char *name, int axis, const std::string &val,
                       const char *pname);

  virtual void execute();

private:
  int                 generation_;
  GuiString           gui_name_;
  GuiString           gui_type_;
  GuiString           gui_dimension_;
  GuiString           gui_origin_;
};

DECLARE_MAKER(ReportNrrdInfo)

ReportNrrdInfo::ReportNrrdInfo(GuiContext* ctx)
  : Module("ReportNrrdInfo", ctx, Source, "Misc", "Teem"),
    generation_(-1),
    gui_name_(get_ctx()->subVar("name"), "---"),
    gui_type_(get_ctx()->subVar("type"), "---"),
    gui_dimension_(get_ctx()->subVar("dimension"), "0"),
    gui_origin_(get_ctx()->subVar("origin"), "0")
{
}


ReportNrrdInfo::~ReportNrrdInfo()
{
}


void
ReportNrrdInfo::clear_vals() 
{
  gui_name_.set("---");
  gui_type_.set("---");

  if (get_ctx()->is_active()) TCLInterface::execute(get_id() + " delete_tabs");
}


void
ReportNrrdInfo::update_axis_var(const char *name, int axis, const std::string &val,
                          const char *pname)
{
  std::ostringstream ostr;
  ostr << "set " << get_id() << "-" << name << axis << " {" << val << "}";
  TCLInterface::execute(ostr.str());
  if (sci_getenv_p("SCI_REGRESSION_TESTING"))
  {
    remark("Axis " + to_string(axis) + " " + pname + ": " + val);
  }
}

const char *nrrd_kind_strings[] = { // nrrdKinds, Matches teem 1.9!
  "nrrdKindUnknown",
  "nrrdKindDomain",            //  1: any image domain
  "nrrdKindSpace",             //  2: a spatial domain
  "nrrdKindTime",              //  3: a temporal domain
  // -------------------------- end domain kinds
  // -------------------------- begin range kinds
  "nrrdKindList",              //  4: any list of values, non-resample-able
  "nrrdKindPoint",             //  5: coords of a point
  "nrrdKindVector",            //  6: coeffs of (contravariant) vector
  "nrrdKindCovariantVector",   //  7: coeffs of covariant vector (eg gradient)
  "nrrdKindNormal",            //  8: coeffs of unit-length covariant vector
  // -------------------------- end arbitrary size kinds
  // -------------------------- begin size-specific kinds
  "nrrdKindStub",              //  9: axis with one sample (a placeholder)
  "nrrdKindScalar",            // 10: effectively, same as a stub
  "nrrdKindComplex",           // 11: real and imaginary components
  "nrrdKind2Vector",           // 12: 2 component vector
  "nrrdKind3Color",            // 13: ANY 3-component color value
  "nrrdKindRGBColor",          // 14: RGB, no colorimetry
  "nrrdKindHSVColor",          // 15: HSV, no colorimetry
  "nrrdKindXYZColor",          // 16: perceptual primary colors
  "nrrdKind4Color",            // 17: ANY 4-component color value
  "nrrdKindRGBAColor",         // 18: RGBA, no colorimetry
  "nrrdKind3Vector",           // 19: 3-component vector
  "nrrdKind3Gradient",         // 20: 3-component covariant vector
  "nrrdKind3Normal",           // 21: 3-component covector, assumed normalized
  "nrrdKind4Vector",           // 22: 4-component vector
  "nrrdKindQuaternion",        // 23: (w,x,y,z), not necessarily normalized
  "nrrdKind2DSymMatrix",       // 24: Mxx Mxy Myy
  "nrrdKind2DMaskedSymMatrix", // 25: mask Mxx Mxy Myy
  "nrrdKind2DMatrix",          // 26: Mxx Mxy Myx Myy
  "nrrdKind2DMaskedMatrix",    // 27: mask Mxx Mxy Myx Myy
  "nrrdKind3DSymMatrix",       // 28: Mxx Mxy Mxz Myy Myz Mzz
  "nrrdKind3DMaskedSymMatrix", // 29: mask Mxx Mxy Mxz Myy Myz Mzz
  "nrrdKind3DMatrix",          // 30: Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz
  "nrrdKind3DMaskedMatrix",    // 31: mask Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz
  "nrrdKindLast"
};


void
ReportNrrdInfo::update_input_attributes(NrrdDataHandle nh) 
{
  const bool regressing = sci_getenv_p("SCI_REGRESSION_TESTING");

  std::string name;
  if (!nh->get_property( "Name", name)) { 
    name = "Unknown";
  }
  gui_name_.set(name);
  if (regressing) { remark("Name: " + name); }

  std::string nrrdtype, stmp;
  get_nrrd_compile_type(nh->nrrd_->type, nrrdtype, stmp);
  gui_type_.set(nrrdtype);
  if (regressing) { remark("Data Type: " + nrrdtype); }

  gui_dimension_.set(to_string(nh->nrrd_->dim));
  if (regressing) { remark("Dimension: " + to_string(nh->nrrd_->dim)); }

  // space origin
  std::ostringstream spaceor;
  spaceor << "[ ";
  unsigned int last_dim = nh->nrrd_->spaceDim - 1;
  for (unsigned int p = 0; p < nh->nrrd_->spaceDim; ++p)
  {
    spaceor << nh->nrrd_->spaceOrigin[p];
    if (p < last_dim)
      spaceor << ", ";
  }
  spaceor << " ]";
  gui_origin_.set(spaceor.str());

  // TODO: Set Origin here.

  bool haveSpaceInfo=false;
  for (unsigned int i = 0; i < nh->nrrd_->dim; i++) 
    if (airExists(nh->nrrd_->axis[i].spaceDirection[0])) haveSpaceInfo=true;

  // Go through all axes...
  for (unsigned int i = 0; i < nh->nrrd_->dim; i++)
  {
    std::string labelstr;
    if (nh->nrrd_->axis[i].label == 0 ||
        std::string(nh->nrrd_->axis[i].label).length() == 0)
    {
      labelstr = "---";
    }
    else
    {
      labelstr = nh->nrrd_->axis[i].label;
    }
    update_axis_var("label", i, labelstr, "Label");

    int k = nh->nrrd_->axis[i].kind;
    if (k < 0 || k >= nrrdKindLast) k = 0;
    const char *kindstr = nrrd_kind_strings[k];
    update_axis_var("kind", i, kindstr, "Kind");

    update_axis_var("size", i, to_string(nh->nrrd_->axis[i].size), "Size");

    update_axis_var("min", i, to_string(nh->nrrd_->axis[i].min), "Min");
    update_axis_var("max", i, to_string(nh->nrrd_->axis[i].max), "Max");

    std::string locstr;
    switch (nh->nrrd_->axis[i].center) {
    case nrrdCenterUnknown :
      locstr = "Unknown";
      break;
    case nrrdCenterNode :
      locstr = "Node";
      break;
    case nrrdCenterCell :
      locstr = "Cell";
      break;
    }
    update_axis_var("center", i, locstr, "Center");

    if (!haveSpaceInfo) { // no "spaceDirection" info, just "spacing"
      update_axis_var("spacing", i, to_string(nh->nrrd_->axis[i].spacing), "Spacing");
    } else {
      std::ostringstream spacedir;
      spacedir << "[ ";
      double l2 = 0;
      for (unsigned int p = 0; p < nh->nrrd_->spaceDim; ++p)
      {
        double tmp = nh->nrrd_->axis[i].spaceDirection[p];
        spacedir << tmp;
        l2 += tmp*tmp;
        if (p < last_dim)
          spacedir << ", ";
      }
      spacedir << " ]";
      update_axis_var("spaceDir", i, spacedir.str(), "Spacing Direction");
      update_axis_var("spacing", i, to_string(sqrt(l2)), "Spacing");
    }
  }
  if (get_ctx()->is_active()) TCLInterface::execute(get_id() + " add_tabs");
}


void
ReportNrrdInfo::execute()
{
  NrrdDataHandle nh;
  if (!get_input_handle("Query Nrrd", nh, false))
  {
    clear_vals();
    generation_ = -1;
    return;
  }

  if (generation_ != nh.get_rep()->generation) 
  {
    generation_ = nh.get_rep()->generation;
    clear_vals();
    update_input_attributes(nh);
  }
}

} // end SCITeem namespace
#endif
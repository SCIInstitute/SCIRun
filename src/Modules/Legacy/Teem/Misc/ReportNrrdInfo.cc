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


///  File   : ReportNrrdInfo.cc
///  Author : Martin Cole
///  Date   : Tue Feb  4 08:55:34 2003

#include <Modules/Legacy/Teem/Misc/ReportNrrdInfo.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Teem;
using namespace SCIRun::Dataflow::Networks;
using namespace Core::Algorithms;

MODULE_INFO_DEF(ReportNrrdInfo, Misc, Teem)

ReportNrrdInfo::ReportNrrdInfo() : Module(staticInfo_)
{
  INITIALIZE_PORT(Query_Nrrd);
}

void ReportNrrdInfo::setStateDefaults()
{
}

template <typename T>
void
ReportNrrdInfo::update_axis_var(std::ostringstream& info, const char *name, int axis, const T& val,
                          const char *pname)
{
  info << '\t' << pname << ": " << val << "\n";
}

static const char *nrrd_kind_strings[] = { // nrrdKinds, Matches teem 1.9!
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
  std::ostringstream info;

  std::string name;
  if (!nh->properties().get_property( "Name", name))
    name = "Unknown";
  info << "Name: " << name << "\n";

  std::string nrrdtype, stmp;
  get_nrrd_compile_type(nh->getNrrd()->type, nrrdtype, stmp);
  info << "Type: " << nrrdtype << "\n";
  info << "Dimension: " << nh->getNrrd()->dim << "\n";

  // space origin
  std::ostringstream spaceor;
  spaceor << "[ ";
  unsigned int last_dim = nh->getNrrd()->spaceDim - 1;
  for (unsigned int p = 0; p < nh->getNrrd()->spaceDim; ++p)
  {
    spaceor << nh->getNrrd()->spaceOrigin[p];
    if (p < last_dim)
      spaceor << ", ";
  }
  spaceor << " ]";
  info << "Origin: " << spaceor.str() << "\n";

  // TODO: Set Origin here.

  bool haveSpaceInfo=false;
  for (unsigned int i = 0; i < nh->getNrrd()->dim; i++)
    if (airExists(nh->getNrrd()->axis[i].spaceDirection[0])) haveSpaceInfo=true;

  // Go through all axes...
  for (unsigned int i = 0; i < nh->getNrrd()->dim; i++)
  {
    info << "Axis " << i << ":\n";
    std::string labelstr;
    if (nh->getNrrd()->axis[i].label == 0 ||
        std::string(nh->getNrrd()->axis[i].label).length() == 0)
    {
      labelstr = "---";
    }
    else
    {
      labelstr = nh->getNrrd()->axis[i].label;
    }
    update_axis_var(info, "label", i, labelstr, "Label");

    int k = nh->getNrrd()->axis[i].kind;
    if (k < 0 || k >= nrrdKindLast) k = 0;
    const char *kindstr = nrrd_kind_strings[k];
    update_axis_var(info, "kind", i, kindstr, "Kind");

    update_axis_var(info, "size", i, nh->getNrrd()->axis[i].size, "Size");

    update_axis_var(info, "min", i, nh->getNrrd()->axis[i].min, "Min");
    update_axis_var(info, "max", i, nh->getNrrd()->axis[i].max, "Max");

    std::string locstr;
    switch (nh->getNrrd()->axis[i].center)
    {
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
    update_axis_var(info, "center", i, locstr, "Center");

    if (!haveSpaceInfo) { // no "spaceDirection" info, just "spacing"
      update_axis_var(info, "spacing", i, (nh->getNrrd()->axis[i].spacing), "Spacing");
    } else {
      std::ostringstream spacedir;
      spacedir << "[ ";
      double l2 = 0;
      for (unsigned int p = 0; p < nh->getNrrd()->spaceDim; ++p)
      {
        double tmp = nh->getNrrd()->axis[i].spaceDirection[p];
        spacedir << tmp;
        l2 += tmp*tmp;
        if (p < last_dim)
          spacedir << ", ";
      }
      spacedir << " ]";
      update_axis_var(info, "spaceDir", i, spacedir.str(), "Spacing Direction");
      update_axis_var(info, "spacing", i, (sqrt(l2)), "Spacing");
    }
  }
  get_state()->setTransientValue(Variables::ObjectInfo, info.str());
}

void
ReportNrrdInfo::execute()
{
  auto nh = getRequiredInput(Query_Nrrd);
  update_input_attributes(nh);
}

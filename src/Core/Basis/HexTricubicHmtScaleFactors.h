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

   Author:          Martin Cole, Frank B. Sachse
   Date:            March 1 2005
*/


#ifndef CORE_BASIS_HEXTRICUBICHMTSCALEFACTORS_H
#define CORE_BASIS_HEXTRICUBICHMTSCALEFACTORS_H 1

#include <Core/Persistent/PersistentSTL.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Basis/HexTricubicHmtScaleFactorsCommon.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of HexTricubicHmtScaleFactors
class HexTricubicHmtScaleFactorsUnitElement :
    public HexTrilinearLgnUnitElement {
public:
  HexTricubicHmtScaleFactorsUnitElement() {}
  virtual ~HexTricubicHmtScaleFactorsUnitElement() {}

  static int dofs() { return 64; } ///< return degrees of freedom
};


/// Class for handling of element of type hexahedron with
/// tricubic hermitian interpolation with scale factors
template <class T>
class HexTricubicHmtScaleFactors : public BasisAddDerivativesScaleFactors<T>,
           public HexApprox,
				   public HexGaussian3<double>,
           public HexSamplingSchemes,
				   public HexTricubicHmtScaleFactorsUnitElement,
           public HexElementWeights
{
public:
  typedef T value_type;

  HexTricubicHmtScaleFactors() {}
  virtual ~HexTricubicHmtScaleFactors() {}

  static int polynomial_order() { return 3; }

  template<class VECTOR>
  inline void get_weights(const VECTOR& coords, double *w) const
    { get_cubic_weights(coords,w); }

  template<class VECTOR>
  inline void get_derivate_weights(const VECTOR& coords, double *w) const
    { get_cubic_derivate_weights(coords,w); }

  /// get value at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR &coords, const ElemData &cd) const
  {
    double w[64];
    get_cubic_weights(coords, w);

    const double sx[8] = {
      this->scalefactors_[cd.node0_index()][0],
      this->scalefactors_[cd.node1_index()][0],
      this->scalefactors_[cd.node2_index()][0],
      this->scalefactors_[cd.node3_index()][0],
      this->scalefactors_[cd.node4_index()][0],
      this->scalefactors_[cd.node5_index()][0],
      this->scalefactors_[cd.node6_index()][0],
      this->scalefactors_[cd.node7_index()][0]
    };
    const double sy[8] = {
      this->scalefactors_[cd.node0_index()][1],
      this->scalefactors_[cd.node1_index()][1],
      this->scalefactors_[cd.node2_index()][1],
      this->scalefactors_[cd.node3_index()][1],
      this->scalefactors_[cd.node4_index()][1],
      this->scalefactors_[cd.node5_index()][1],
      this->scalefactors_[cd.node6_index()][1],
      this->scalefactors_[cd.node7_index()][1]
    };
    const double sz[8] = {
      this->scalefactors_[cd.node0_index()][2],
      this->scalefactors_[cd.node1_index()][2],
      this->scalefactors_[cd.node2_index()][2],
      this->scalefactors_[cd.node3_index()][2],
      this->scalefactors_[cd.node4_index()][2],
      this->scalefactors_[cd.node5_index()][2],
      this->scalefactors_[cd.node6_index()][2],
      this->scalefactors_[cd.node7_index()][2]
    };
    return hexScaledBasisInterpolate<T>(w, cd, this->derivs_, sx, sy, sz);
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    const double x=coords[0], y=coords[1], z=coords[2];
    const double x2=x*x;
    const double y2=y*y;
    const double z2=z*z;
    const double x12=(x-1)*(x-1);
    const double y12=(y-1)*(y-1);
    const double z12=(z-1)*(z-1);

    derivs.resize(3);

    const double sx[8] = {
      this->scalefactors_[cd.node0_index()][0],
      this->scalefactors_[cd.node1_index()][0],
      this->scalefactors_[cd.node2_index()][0],
      this->scalefactors_[cd.node3_index()][0],
      this->scalefactors_[cd.node4_index()][0],
      this->scalefactors_[cd.node5_index()][0],
      this->scalefactors_[cd.node6_index()][0],
      this->scalefactors_[cd.node7_index()][0]
    };
    const double sy[8] = {
      this->scalefactors_[cd.node0_index()][1],
      this->scalefactors_[cd.node1_index()][1],
      this->scalefactors_[cd.node2_index()][1],
      this->scalefactors_[cd.node3_index()][1],
      this->scalefactors_[cd.node4_index()][1],
      this->scalefactors_[cd.node5_index()][1],
      this->scalefactors_[cd.node6_index()][1],
      this->scalefactors_[cd.node7_index()][1]
    };
    const double sz[8] = {
      this->scalefactors_[cd.node0_index()][2],
      this->scalefactors_[cd.node1_index()][2],
      this->scalefactors_[cd.node2_index()][2],
      this->scalefactors_[cd.node3_index()][2],
      this->scalefactors_[cd.node4_index()][2],
      this->scalefactors_[cd.node5_index()][2],
      this->scalefactors_[cd.node6_index()][2],
      this->scalefactors_[cd.node7_index()][2]
    };
    hexScaledBasisDerivate<T>(x, y, z, x2, y2, z2, x12, y12, z12,
                               cd, this->derivs_, sx, sy, sz, derivs);
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    HexLocate< HexTricubicHmtScaleFactors<T> > CL;
    return CL.get_coords(this, coords, value, cd);
  }

  /// get arc length for edge
  template <class ElemData>
  double get_arc_length(const unsigned edge, const ElemData &cd) const
  {
    return get_arc3d_length<CrvGaussian2<double> >(this, edge, cd);
  }

  /// get area
  template <class ElemData>
    double get_area(const unsigned face, const ElemData &cd) const
  {
    return get_area3<QuadGaussian3<double> >(this, face, cd);
  }

  /// get volume
  template <class ElemData>
    double get_volume(const ElemData & cd) const
  {
    return get_volume3(this, cd);
  }
  static  const std::string type_name(int n = -1);

  virtual void io (Piostream& str);

};


template <class T>
const std::string
HexTricubicHmtScaleFactors<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("HexTricubicHmtScaleFactors");
    return nm;
  } else {
    return find_type_name((T *)nullptr);
  }
}




}}
template <class T>
const TypeDescription*
  get_type_description(Core::Basis::HexTricubicHmtScaleFactors<T> *)
{
  static TypeDescription* td = nullptr;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)nullptr);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("HexTricubicHmtScaleFactors",
      subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int HEXTRICUBICHMTSCALEFACTORS_VERSION = 1;
template <class T>
void
  Core::Basis::HexTricubicHmtScaleFactors<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    HEXTRICUBICHMTSCALEFACTORS_VERSION);
  Pio(stream, this->derivs_);
  Pio(stream, this->scalefactors_);
  stream.end_class();
}
}


#endif

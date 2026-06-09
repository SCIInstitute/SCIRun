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
   Date:            December 3 2004
*/


#ifndef CORE_BASIS_HEXTRICUBICHMT_H
#define CORE_BASIS_HEXTRICUBICHMT_H 1

#include <Core/Basis/HexTrilinearLgn.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of HexTricubicHmt
class HexTricubicHmtUnitElement : public HexTrilinearLgnUnitElement {
public:
  HexTricubicHmtUnitElement() {};
  virtual ~HexTricubicHmtUnitElement() {};

  static int dofs() { return 64; } ///< return degrees of freedom
};


/// Class for handling of element of type hexahedron with
/// tricubic hermitian interpolation
template <class T>
class HexTricubicHmt : public BasisAddDerivatives<T>,
                       public HexApprox,
		       public HexGaussian3<double>,
           public HexSamplingSchemes,
		       public HexTricubicHmtUnitElement,
           public HexElementWeights
{
public:
  typedef T value_type;

  HexTricubicHmt() {}
  virtual ~HexTricubicHmt() {}

  static int polynomial_order() { return 3; }

  template<class VECTOR>
  inline void get_weights(const VECTOR& coords, double *w) const
    { get_cubic_weights(coords,w); }

  template<class VECTOR>
  inline void get_derivate_weights(const VECTOR& coords, double *w) const
    { get_cubic_derivate_weights(coords,w); }

  /// Evaluate 64-term tricubic Hermite weighted sum shared by interpolate and derivate.
  template <class ElemData>
  double evaluateHexCubicBasis(const double* w, const ElemData& cd) const
  {
    return (w[0]  * cd.node0() +
	    w[1]  * this->derivs_[cd.node0_index()][0] +
	    w[2]  * this->derivs_[cd.node0_index()][1] +
	    w[3]  * this->derivs_[cd.node0_index()][2] +
	    w[4]  * this->derivs_[cd.node0_index()][3] +
	    w[5]  * this->derivs_[cd.node0_index()][4] +
	    w[6]  * this->derivs_[cd.node0_index()][5] +
	    w[7]  * this->derivs_[cd.node0_index()][6] +
	    w[8]  * cd.node1() +
	    w[9]  * this->derivs_[cd.node1_index()][0] +
	    w[10] * this->derivs_[cd.node1_index()][1] +
	    w[11] * this->derivs_[cd.node1_index()][2] +
	    w[12] * this->derivs_[cd.node1_index()][3] +
	    w[13] * this->derivs_[cd.node1_index()][4] +
	    w[14] * this->derivs_[cd.node1_index()][5] +
	    w[15] * this->derivs_[cd.node1_index()][6] +
	    w[16] * cd.node2() +
	    w[17] * this->derivs_[cd.node2_index()][0] +
	    w[18] * this->derivs_[cd.node2_index()][1] +
	    w[19] * this->derivs_[cd.node2_index()][2] +
	    w[20] * this->derivs_[cd.node2_index()][3] +
	    w[21] * this->derivs_[cd.node2_index()][4] +
	    w[22] * this->derivs_[cd.node2_index()][5] +
	    w[23] * this->derivs_[cd.node2_index()][6] +
	    w[24] * cd.node3() +
	    w[25] * this->derivs_[cd.node3_index()][0] +
	    w[26] * this->derivs_[cd.node3_index()][1] +
	    w[27] * this->derivs_[cd.node3_index()][2] +
	    w[28] * this->derivs_[cd.node3_index()][3] +
	    w[29] * this->derivs_[cd.node3_index()][4] +
	    w[30] * this->derivs_[cd.node3_index()][5] +
	    w[31] * this->derivs_[cd.node3_index()][6] +
	    w[32] * cd.node4() +
	    w[33] * this->derivs_[cd.node4_index()][0] +
	    w[34] * this->derivs_[cd.node4_index()][1] +
	    w[35] * this->derivs_[cd.node4_index()][2] +
	    w[36] * this->derivs_[cd.node4_index()][3] +
	    w[37] * this->derivs_[cd.node4_index()][4] +
	    w[38] * this->derivs_[cd.node4_index()][5] +
	    w[39] * this->derivs_[cd.node4_index()][6] +
	    w[40] * cd.node5() +
	    w[41] * this->derivs_[cd.node5_index()][0] +
	    w[42] * this->derivs_[cd.node5_index()][1] +
	    w[43] * this->derivs_[cd.node5_index()][2] +
	    w[44] * this->derivs_[cd.node5_index()][3] +
	    w[45] * this->derivs_[cd.node5_index()][4] +
	    w[46] * this->derivs_[cd.node5_index()][5] +
	    w[47] * this->derivs_[cd.node5_index()][6] +
	    w[48] * cd.node6() +
	    w[49] * this->derivs_[cd.node6_index()][0] +
	    w[50] * this->derivs_[cd.node6_index()][1] +
	    w[51] * this->derivs_[cd.node6_index()][2] +
	    w[52] * this->derivs_[cd.node6_index()][3] +
	    w[53] * this->derivs_[cd.node6_index()][4] +
	    w[54] * this->derivs_[cd.node6_index()][5] +
	    w[55] * this->derivs_[cd.node6_index()][6] +
	    w[56] * cd.node7() +
	    w[57] * this->derivs_[cd.node7_index()][0] +
	    w[58] * this->derivs_[cd.node7_index()][1] +
	    w[59] * this->derivs_[cd.node7_index()][2] +
	    w[60] * this->derivs_[cd.node7_index()][3] +
	    w[61] * this->derivs_[cd.node7_index()][4] +
	    w[62] * this->derivs_[cd.node7_index()][5] +
	    w[63] * this->derivs_[cd.node7_index()][6]);
  }

  /// get value at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR &coords, const ElemData &cd) const
  {
    double w[64];
    get_cubic_weights(coords, w);

    return (T)(evaluateHexCubicBasis(w, cd));
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[192];
    get_cubic_derivate_weights(coords, w);

    derivs.resize(3);

    derivs[0]= static_cast<typename VECTOR2::value_type>(evaluateHexCubicBasis(w, cd));
    derivs[1]= static_cast<typename VECTOR2::value_type>(evaluateHexCubicBasis(w + 64, cd));
    derivs[2]= static_cast<typename VECTOR2::value_type>(evaluateHexCubicBasis(w + 128, cd));

  }


  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    HexLocate< HexTricubicHmt<T> > CL;
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
HexTricubicHmt<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("HexTricubicHmt");
    return nm;
  } else {
    return find_type_name((T *)nullptr);
  }
}






}}
template <class T>
const SCIRun::TypeDescription*
  get_type_description(Core::Basis::HexTricubicHmt<T> *)
{
  static SCIRun::TypeDescription* td = nullptr;
  if(!td){
    const SCIRun::TypeDescription *sub = get_type_description((T*)nullptr);
    SCIRun::TypeDescription::td_vec *subs = new SCIRun::TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new SCIRun::TypeDescription("HexTricubicHmt", subs,
      std::string(__FILE__),
      "SCIRun",
      SCIRun::TypeDescription::BASIS_E);
  }
  return td;
}

const int HEXTRICUBICHMT_VERSION = 1;
template <class T>
void
  Core::Basis::HexTricubicHmt<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    HEXTRICUBICHMT_VERSION);
  Pio(stream, this->derivs_);
  stream.end_class();
}
}

#endif

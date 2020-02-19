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
   Date:            December 1 2004
*/


#ifndef CORE_BASIS_PRISMCUBICHMT_H
#define CORE_BASIS_PRISMCUBICHMT_H 1

#include <Core/Basis/PrismLinearLgn.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of PrismCubicHmt
class PrismCubicHmtUnitElement : public PrismLinearLgnUnitElement {
public:
  PrismCubicHmtUnitElement() {}
  virtual ~PrismCubicHmtUnitElement() {}

  /// return degrees of freedom
  static int dofs()
    { return 24; }
};


/// Class for handling of element of type prism with
/// cubic hermitian interpolation
template <class T>
class PrismCubicHmt : public BasisAddDerivatives<T>,
                      public PrismApprox,
		      public PrismGaussian2<double>,
		      public PrismSamplingSchemes,
		      public PrismCubicHmtUnitElement,
          public PrismElementWeights
{
public:
  typedef T value_type;

  PrismCubicHmt() {}
  virtual ~PrismCubicHmt() {}

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
    double w[24];
    get_cubic_weights(coords, w);

    return (T)(
         w[0]  * cd.node0() +
	       w[1]  * this->derivs_[cd.node0_index()][0] +
	       w[2]  * this->derivs_[cd.node0_index()][1] +
	       w[3]  * this->derivs_[cd.node0_index()][2] +
	       w[4]  * cd.node1()		    +
	       w[5]  * this->derivs_[cd.node1_index()][0] +
	       w[6]  * this->derivs_[cd.node1_index()][1] +
	       w[7]  * this->derivs_[cd.node1_index()][2] +
	       w[8]  * cd.node2()		    +
	       w[9]  * this->derivs_[cd.node2_index()][0] +
	       w[10] * this->derivs_[cd.node2_index()][1] +
	       w[11] * this->derivs_[cd.node2_index()][2] +
	       w[12] * cd.node3()		    +
	       w[13] * this->derivs_[cd.node3_index()][0] +
	       w[14] * this->derivs_[cd.node3_index()][1] +
	       w[15] * this->derivs_[cd.node3_index()][2] +
	       w[16] * cd.node4()		    +
	       w[17] * this->derivs_[cd.node4_index()][0] +
	       w[18] * this->derivs_[cd.node4_index()][1] +
	       w[19] * this->derivs_[cd.node4_index()][2] +
	       w[20] * cd.node5()		    +
	       w[21] * this->derivs_[cd.node5_index()][0] +
	       w[22] * this->derivs_[cd.node5_index()][1] +
	       w[23] * this->derivs_[cd.node5_index()][2]);
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[72];
    get_cubic_derivate_weights(coords, w);

    derivs.resize(3);

    derivs[0]= static_cast<typename VECTOR2::value_type>(
         w[0]  * cd.node0() +
	       w[1]  * this->derivs_[cd.node0_index()][0] +
	       w[2]  * this->derivs_[cd.node0_index()][1] +
	       w[3]  * this->derivs_[cd.node0_index()][2] +
	       w[4]  * cd.node1()		    +
	       w[5]  * this->derivs_[cd.node1_index()][0] +
	       w[6]  * this->derivs_[cd.node1_index()][1] +
	       w[7]  * this->derivs_[cd.node1_index()][2] +
	       w[8]  * cd.node2()		    +
	       w[9]  * this->derivs_[cd.node2_index()][0] +
	       w[10] * this->derivs_[cd.node2_index()][1] +
	       w[11] * this->derivs_[cd.node2_index()][2] +
	       w[12] * cd.node3()		    +
	       w[13] * this->derivs_[cd.node3_index()][0] +
	       w[14] * this->derivs_[cd.node3_index()][1] +
	       w[15] * this->derivs_[cd.node3_index()][2] +
	       w[16] * cd.node4()		    +
	       w[17] * this->derivs_[cd.node4_index()][0] +
	       w[18] * this->derivs_[cd.node4_index()][1] +
	       w[19] * this->derivs_[cd.node4_index()][2] +
	       w[20] * cd.node5()		    +
	       w[21] * this->derivs_[cd.node5_index()][0] +
	       w[22] * this->derivs_[cd.node5_index()][1] +
	       w[23] * this->derivs_[cd.node5_index()][2]);

    derivs[1]= static_cast<typename VECTOR2::value_type>(
         w[24]  * cd.node0() +
	       w[25]  * this->derivs_[cd.node0_index()][0] +
	       w[26]  * this->derivs_[cd.node0_index()][1] +
	       w[27]  * this->derivs_[cd.node0_index()][2] +
	       w[28]  * cd.node1()		    +
	       w[29]  * this->derivs_[cd.node1_index()][0] +
	       w[30]  * this->derivs_[cd.node1_index()][1] +
	       w[31]  * this->derivs_[cd.node1_index()][2] +
	       w[32]  * cd.node2()		    +
	       w[33]  * this->derivs_[cd.node2_index()][0] +
	       w[34] * this->derivs_[cd.node2_index()][1] +
	       w[35] * this->derivs_[cd.node2_index()][2] +
	       w[36] * cd.node3()		    +
	       w[37] * this->derivs_[cd.node3_index()][0] +
	       w[38] * this->derivs_[cd.node3_index()][1] +
	       w[39] * this->derivs_[cd.node3_index()][2] +
	       w[40] * cd.node4()		    +
	       w[41] * this->derivs_[cd.node4_index()][0] +
	       w[42] * this->derivs_[cd.node4_index()][1] +
	       w[43] * this->derivs_[cd.node4_index()][2] +
	       w[44] * cd.node5()		    +
	       w[45] * this->derivs_[cd.node5_index()][0] +
	       w[46] * this->derivs_[cd.node5_index()][1] +
	       w[47] * this->derivs_[cd.node5_index()][2]);

    derivs[2]= static_cast<typename VECTOR2::value_type>(
         w[48]  * cd.node0() +
	       w[49]  * this->derivs_[cd.node0_index()][0] +
	       w[50]  * this->derivs_[cd.node0_index()][1] +
	       w[51]  * this->derivs_[cd.node0_index()][2] +
	       w[52]  * cd.node1()		    +
	       w[53]  * this->derivs_[cd.node1_index()][0] +
	       w[54]  * this->derivs_[cd.node1_index()][1] +
	       w[55]  * this->derivs_[cd.node1_index()][2] +
	       w[56]  * cd.node2()		    +
	       w[57]  * this->derivs_[cd.node2_index()][0] +
	       w[58] * this->derivs_[cd.node2_index()][1] +
	       w[59] * this->derivs_[cd.node2_index()][2] +
	       w[60] * cd.node3()		    +
	       w[61] * this->derivs_[cd.node3_index()][0] +
	       w[62] * this->derivs_[cd.node3_index()][1] +
	       w[63] * this->derivs_[cd.node3_index()][2] +
	       w[64] * cd.node4()		    +
	       w[65] * this->derivs_[cd.node4_index()][0] +
	       w[66] * this->derivs_[cd.node4_index()][1] +
	       w[67] * this->derivs_[cd.node4_index()][2] +
	       w[68] * cd.node5()		    +
	       w[69] * this->derivs_[cd.node5_index()][0] +
	       w[70] * this->derivs_[cd.node5_index()][1] +
	       w[71] * this->derivs_[cd.node5_index()][2]);
  }

  /// get parametric coordinate for value within the element
  /// iterative solution...
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const

  {
    PrismLocate< PrismCubicHmt<T> > CL;
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
    if (unit_faces[face][3]==-1)
      return get_area3<TriGaussian3<double> >(this, face, cd);
    else
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
PrismCubicHmt<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("PrismCubicHmt");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}



}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::PrismCubicHmt<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("PrismCubicHmt", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int PRISMCUBICHMT_VERSION = 1;
template <class T>
void
  Core::Basis::PrismCubicHmt<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    PRISMCUBICHMT_VERSION);
  Pio(stream, this->derivs_);
  stream.end_class();
}

}

#endif

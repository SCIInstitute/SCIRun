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

   Author:          Frank B. Sachse
   Date:            November 30 2004
*/


#ifndef CORE_BASIS_TETCUBICHMT_H
#define CORE_BASIS_TETCUBICHMT_H 1

#include <Core/Basis/TetLinearLgn.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of TetCubicHmt
class TetCubicHmtUnitElement : public TetLinearLgnUnitElement {
public:
  TetCubicHmtUnitElement() {}
  virtual ~TetCubicHmtUnitElement() {}

  /// return degrees of freedom
  static int dofs()
    { return 16; }
};

/// Class for handling of element of type tetrahedron with
/// cubic hermitian interpolation
template <class T>
class TetCubicHmt : public BasisAddDerivatives<T>,
                    public TetApprox,
		    public TetGaussian3<double>,
        public TetSamplingSchemes,
		    public TetCubicHmtUnitElement,
        public TetElementWeights
{
public:
  typedef T value_type;

  TetCubicHmt() {}
  virtual ~TetCubicHmt() {}

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
    double w[16];
    get_cubic_weights(coords, w);

    return (T)(w[0]  * cd.node0()                   +
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
	       w[15] * this->derivs_[cd.node3_index()][2]);
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[48];
    get_cubic_derivate_weights(coords, w);

    derivs.resize(3);

    derivs[0]= static_cast<typename VECTOR2::value_type>(
         w[0]  * cd.node0()                         +
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
	       w[15] * this->derivs_[cd.node3_index()][2]);

    derivs[1]= static_cast<typename VECTOR2::value_type>(
         w[16]  * cd.node0()                         +
	       w[17]  * this->derivs_[cd.node0_index()][0] +
	       w[18]  * this->derivs_[cd.node0_index()][1] +
	       w[19]  * this->derivs_[cd.node0_index()][2] +
	       w[20]  * cd.node1()		    +
	       w[21]  * this->derivs_[cd.node1_index()][0] +
	       w[22]  * this->derivs_[cd.node1_index()][1] +
	       w[23]  * this->derivs_[cd.node1_index()][2] +
	       w[24]  * cd.node2()		    +
	       w[25]  * this->derivs_[cd.node2_index()][0] +
	       w[26] * this->derivs_[cd.node2_index()][1] +
	       w[27] * this->derivs_[cd.node2_index()][2] +
	       w[28] * cd.node3()		    +
	       w[29] * this->derivs_[cd.node3_index()][0] +
	       w[30] * this->derivs_[cd.node3_index()][1] +
	       w[31] * this->derivs_[cd.node3_index()][2]);

     derivs[2]= static_cast<typename VECTOR2::value_type>(
         w[32]  * cd.node0()                         +
	       w[33]  * this->derivs_[cd.node0_index()][0] +
	       w[34]  * this->derivs_[cd.node0_index()][1] +
	       w[35]  * this->derivs_[cd.node0_index()][2] +
	       w[36]  * cd.node1()		    +
	       w[37]  * this->derivs_[cd.node1_index()][0] +
	       w[38]  * this->derivs_[cd.node1_index()][1] +
	       w[39]  * this->derivs_[cd.node1_index()][2] +
	       w[40]  * cd.node2()		    +
	       w[41]  * this->derivs_[cd.node2_index()][0] +
	       w[42] * this->derivs_[cd.node2_index()][1] +
	       w[43] * this->derivs_[cd.node2_index()][2] +
	       w[44] * cd.node3()		    +
	       w[45] * this->derivs_[cd.node3_index()][0] +
	       w[46] * this->derivs_[cd.node3_index()][1] +
	       w[47] * this->derivs_[cd.node3_index()][2]);
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    TetLocate< TetCubicHmt<T> > CL;
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
    return get_area3<TriGaussian3<double> >(this, face, cd);
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
TetCubicHmt<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("TetCubicHmt");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}




}}
template <class T>
const TypeDescription* get_type_description(Core::Basis::TetCubicHmt<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("TetCubicHmt", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int TETCUBICHMT_VERSION = 1;
template <class T>
void
  Core::Basis::TetCubicHmt<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    TETCUBICHMT_VERSION);
  Pio(stream, this->derivs_);
  stream.end_class();
}
}

#endif

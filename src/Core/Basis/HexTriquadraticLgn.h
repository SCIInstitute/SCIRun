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

#ifndef CORE_BASIS_HEXTRIQUADRATICLGN_H
#define CORE_BASIS_HEXTRIQUADRATICLGN_H 1

#include <Core/Basis/HexTrilinearLgn.h>

#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of HexTriquadraticLgn
  class SCISHARE HexTriquadraticLgnUnitElement : public HexTrilinearLgnUnitElement {
public:
  static double unit_vertices[20][3]; ///< Parametric coordinates of vertices of unit edge

  HexTriquadraticLgnUnitElement() {}
  virtual ~HexTriquadraticLgnUnitElement() {}

  static int number_of_vertices()
    { return 20; } ///< return number of vertices
  static int dofs()
    { return 20; } ///< return degrees of freedom
};


/// Class for handling of element of type hexahedron with
/// triquadratic lagrangian interpolation
template <class T>
class HexTriquadraticLgn : public BasisAddNodes<T>,
                           public HexApprox,
			   public HexGaussian3<double>,
         public HexSamplingSchemes,
			   public HexTriquadraticLgnUnitElement,
         public HexElementWeights
{
public:
  typedef T value_type;

  HexTriquadraticLgn() {}
  virtual ~HexTriquadraticLgn() {}

  static int polynomial_order() { return 2; }


  template<class VECTOR>
  inline void get_weights(const VECTOR& coords, double *w) const
    { get_quadratic_weights(coords,w); }

  template<class VECTOR>
  inline void get_derivate_weights(const VECTOR& coords, double *w) const
    { get_quadratic_derivate_weights(coords,w); }

  /// get value at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR &coords, const ElemData &cd) const
  {
    double w[20];
    get_quadratic_weights(coords, w);

    return (T)(
         w[0]  * cd.node0() +
	       w[1]  * cd.node1() +
	       w[2]  * cd.node2() +
	       w[3]  * cd.node3() +
	       w[4]  * cd.node4() +
	       w[5]  * cd.node5() +
	       w[6]  * cd.node6() +
	       w[7]  * cd.node7() +
	       w[8]  * this->nodes_[cd.edge0_index()] +
	       w[9]  * this->nodes_[cd.edge1_index()] +
	       w[10] * this->nodes_[cd.edge2_index()] +
	       w[11] * this->nodes_[cd.edge3_index()] +
	       w[12] * this->nodes_[cd.edge4_index()] +
	       w[13] * this->nodes_[cd.edge5_index()] +
	       w[14] * this->nodes_[cd.edge6_index()] +
	       w[15] * this->nodes_[cd.edge7_index()] +
	       w[16] * this->nodes_[cd.edge8_index()] +
	       w[17] * this->nodes_[cd.edge9_index()] +
	       w[18] * this->nodes_[cd.edge10_index()] +
	       w[19] * this->nodes_[cd.edge11_index()]);
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[60];
    get_quadratic_derivate_weights(coords, w);

    derivs.resize(3);

    derivs[0]=static_cast<typename VECTOR2::value_type>(
         w[0]  * cd.node0() +
	       w[1]  * cd.node1() +
	       w[2]  * cd.node2() +
	       w[3]  * cd.node3() +
	       w[4]  * cd.node4() +
	       w[5]  * cd.node5() +
	       w[6]  * cd.node6() +
	       w[7]  * cd.node7() +
	       w[8]  * this->nodes_[cd.edge0_index()] +
	       w[9]  * this->nodes_[cd.edge1_index()] +
	       w[10] * this->nodes_[cd.edge2_index()] +
	       w[11] * this->nodes_[cd.edge3_index()] +
	       w[12] * this->nodes_[cd.edge4_index()] +
	       w[13] * this->nodes_[cd.edge5_index()] +
	       w[14] * this->nodes_[cd.edge6_index()] +
	       w[15] * this->nodes_[cd.edge7_index()] +
	       w[16] * this->nodes_[cd.edge8_index()] +
	       w[17] * this->nodes_[cd.edge9_index()] +
	       w[18] * this->nodes_[cd.edge10_index()] +
	       w[19] * this->nodes_[cd.edge11_index()]);

    derivs[1]=static_cast<typename VECTOR2::value_type>(
         w[20]  * cd.node0() +
	       w[21]  * cd.node1() +
	       w[22]  * cd.node2() +
	       w[23]  * cd.node3() +
	       w[24]  * cd.node4() +
	       w[25]  * cd.node5() +
	       w[26]  * cd.node6() +
	       w[27]  * cd.node7() +
	       w[28]  * this->nodes_[cd.edge0_index()] +
	       w[29]  * this->nodes_[cd.edge1_index()] +
	       w[30] * this->nodes_[cd.edge2_index()] +
	       w[31] * this->nodes_[cd.edge3_index()] +
	       w[32] * this->nodes_[cd.edge4_index()] +
	       w[33] * this->nodes_[cd.edge5_index()] +
	       w[34] * this->nodes_[cd.edge6_index()] +
	       w[35] * this->nodes_[cd.edge7_index()] +
	       w[36] * this->nodes_[cd.edge8_index()] +
	       w[37] * this->nodes_[cd.edge9_index()] +
	       w[38] * this->nodes_[cd.edge10_index()] +
	       w[39] * this->nodes_[cd.edge11_index()]);

    derivs[2]=static_cast<typename VECTOR2::value_type>(
         w[40]  * cd.node0() +
	       w[41]  * cd.node1() +
	       w[42]  * cd.node2() +
	       w[43]  * cd.node3() +
	       w[44]  * cd.node4() +
	       w[45]  * cd.node5() +
	       w[46]  * cd.node6() +
	       w[47]  * cd.node7() +
	       w[48]  * this->nodes_[cd.edge0_index()] +
	       w[49]  * this->nodes_[cd.edge1_index()] +
	       w[50] * this->nodes_[cd.edge2_index()] +
	       w[51] * this->nodes_[cd.edge3_index()] +
	       w[52] * this->nodes_[cd.edge4_index()] +
	       w[53] * this->nodes_[cd.edge5_index()] +
	       w[54] * this->nodes_[cd.edge6_index()] +
	       w[55] * this->nodes_[cd.edge7_index()] +
	       w[56] * this->nodes_[cd.edge8_index()] +
	       w[57] * this->nodes_[cd.edge9_index()] +
	       w[58] * this->nodes_[cd.edge10_index()] +
	       w[59] * this->nodes_[cd.edge11_index()]);

  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    HexLocate< HexTriquadraticLgn<T> > CL;
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
HexTriquadraticLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("HexTriquadraticLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}




}}
template <class T>
const TypeDescription* get_type_description(Core::Basis::HexTriquadraticLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("HexTriquadraticLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int HEXTRIQUADRATICLGN_VERSION = 1;
template <class T>
void
  Core::Basis::HexTriquadraticLgn<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    HEXTRIQUADRATICLGN_VERSION);
  Pio(stream, this->nodes_);
  stream.end_class();
}
}

#endif

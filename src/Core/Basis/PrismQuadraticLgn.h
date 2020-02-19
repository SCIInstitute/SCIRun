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


#ifndef CORE_BASIS_PRISMQUADRATICLGN_H
#define CORE_BASIS_PRISMQUADRATICLGN_H 1

#include <Core/Basis/PrismLinearLgn.h>
#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of PrismQuadraticLgn
  class PrismQuadraticLgnUnitElement : public PrismLinearLgnUnitElement {
public:
  static SCISHARE double unit_vertices[15][3]; ///< Parametric coordinates of vertices of unit edge

  PrismQuadraticLgnUnitElement() {}
  virtual ~PrismQuadraticLgnUnitElement() {}

  /// return number of vertices
  static int number_of_vertices()
    { return 15; }

  /// return degrees of freedom
  static int dofs()
    { return 15; }
};


/// Class for handling of element of type prism with
/// quadratic lagrangian interpolation
template <class T>
class PrismQuadraticLgn : public BasisAddNodes<T>,
                          public PrismApprox,
			  public PrismGaussian2<double>,
			  public PrismSamplingSchemes,
			  public PrismQuadraticLgnUnitElement,
        public PrismElementWeights
{
public:
  typedef T value_type;

  PrismQuadraticLgn() {}
  virtual ~PrismQuadraticLgn() {}

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
    double w[15];
    get_quadratic_weights(coords, w);

    return(T)(
        w[0]  * cd.node0() +
	      w[1]  * cd.node1() +
	      w[2]  * cd.node2() +
	      w[3]  * cd.node3() +
	      w[4]  * cd.node4() +
	      w[5]  * cd.node5() +
	      w[6]  * this->nodes_[cd.edge0_index()] +
	      w[7]  * this->nodes_[cd.edge1_index()] +
	      w[8]  * this->nodes_[cd.edge2_index()] +
	      w[9]  * this->nodes_[cd.edge3_index()] +
	      w[10] * this->nodes_[cd.edge4_index()] +
	      w[11] * this->nodes_[cd.edge5_index()] +
	      w[12] * this->nodes_[cd.edge6_index()] +
	      w[13] * this->nodes_[cd.edge7_index()] +
	      w[14] * this->nodes_[cd.edge8_index()]);
  }


  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[45];
    get_quadratic_derivate_weights(coords, w);

    derivs.resize(3);

    derivs[0]= static_cast<typename VECTOR2::value_type>(
         w[0]  * cd.node0() +
	      w[1]  * cd.node1() +
	      w[2]  * cd.node2() +
	      w[3]  * cd.node3() +
	      w[4]  * cd.node4() +
	      w[5]  * cd.node5() +
	      w[6]  * this->nodes_[cd.edge0_index()] +
	      w[7]  * this->nodes_[cd.edge1_index()] +
	      w[8]  * this->nodes_[cd.edge2_index()] +
	      w[9]  * this->nodes_[cd.edge3_index()] +
	      w[10] * this->nodes_[cd.edge4_index()] +
	      w[11] * this->nodes_[cd.edge5_index()] +
	      w[12] * this->nodes_[cd.edge6_index()] +
	      w[13] * this->nodes_[cd.edge7_index()] +
	      w[14] * this->nodes_[cd.edge8_index()]);

    derivs[1]= static_cast<typename VECTOR2::value_type>(
        w[15]  * cd.node0() +
	      w[16]  * cd.node1() +
	      w[17]  * cd.node2() +
	      w[18]  * cd.node3() +
	      w[19]  * cd.node4() +
	      w[20]  * cd.node5() +
	      w[21]  * this->nodes_[cd.edge0_index()] +
	      w[22]  * this->nodes_[cd.edge1_index()] +
	      w[23]  * this->nodes_[cd.edge2_index()] +
	      w[24]  * this->nodes_[cd.edge3_index()] +
	      w[25] * this->nodes_[cd.edge4_index()] +
	      w[26] * this->nodes_[cd.edge5_index()] +
	      w[27] * this->nodes_[cd.edge6_index()] +
	      w[28] * this->nodes_[cd.edge7_index()] +
	      w[29] * this->nodes_[cd.edge8_index()]);

    derivs[2]= static_cast<typename VECTOR2::value_type>(
        w[30]  * cd.node0() +
	      w[31]  * cd.node1() +
	      w[32]  * cd.node2() +
	      w[33]  * cd.node3() +
	      w[34]  * cd.node4() +
	      w[35]  * cd.node5() +
	      w[36]  * this->nodes_[cd.edge0_index()] +
	      w[37]  * this->nodes_[cd.edge1_index()] +
	      w[38]  * this->nodes_[cd.edge2_index()] +
	      w[39]  * this->nodes_[cd.edge3_index()] +
	      w[40] * this->nodes_[cd.edge4_index()] +
	      w[41] * this->nodes_[cd.edge5_index()] +
	      w[42] * this->nodes_[cd.edge6_index()] +
	      w[43] * this->nodes_[cd.edge7_index()] +
	      w[44] * this->nodes_[cd.edge8_index()]);

  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    PrismLocate< PrismQuadraticLgn<T> > CL;
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
PrismQuadraticLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("PrismQuadraticLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}





}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::PrismQuadraticLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("PrismQuadraticLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int PRISMQUADRATICLGN_VERSION = 1;
template <class T>
void
  Core::Basis::PrismQuadraticLgn<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    PRISMQUADRATICLGN_VERSION);
  Pio(stream, this->nodes_);
  stream.end_class();
}
}

#endif

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
   Date:            November 30 2004
*/


#ifndef BASIS_CORE_QUADBIQUADRATICLGN_H
#define BASIS_CORE_QUADBIQUADRATICLGN_H 1

#include <Core/Basis/QuadBilinearLgn.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of QuadBiquadraticLgn
class QuadBiquadraticLgnUnitElement : public QuadBilinearLgnUnitElement {

  public:
  static SCISHARE double unit_vertices[8][2]; ///< Parametric coordinates of vertices of unit edge

  QuadBiquadraticLgnUnitElement() {}
  virtual ~QuadBiquadraticLgnUnitElement() {}

  static int number_of_vertices() { return 8; } ///< return number of vertices
  static int dofs() { return 8; } ///< return degrees of freedom
};


/// Class for handling of element of type quad with
/// biquadratic lagrangian interpolation
template <class T>
class QuadBiquadraticLgn : public BasisAddNodes<T>,
                           public QuadApprox,
                           public QuadGaussian3<double>,
                           public QuadSamplingSchemes,
                           public QuadBiquadraticLgnUnitElement,
                           public QuadElementWeights
{
public:
  typedef T value_type;

  QuadBiquadraticLgn() {}
  virtual ~QuadBiquadraticLgn() {}

  static int polynomial_order() { return 2; }

  template<class VECTOR>
  inline void get_weights(const VECTOR& coords, double *w) const
    { get_quadratic_weights(coords,w); }

  template<class VECTOR>
  inline void get_derivate_weights(const VECTOR& coords, double *w) const
    { get_quadratic_derivate_weights(coords,w); }


  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR &coords, const ElemData &cd) const
  {
    double w[8];
    get_quadratic_weights(coords, w);

    return (T)(w[0] * cd.node0() +
	       w[1] * cd.node1() +
	       w[2] * cd.node2() +
	       w[3] * cd.node3() +
	       w[4] * this->nodes_[cd.edge0_index()] +
	       w[5] * this->nodes_[cd.edge1_index()] +
	       w[6] * this->nodes_[cd.edge2_index()] +
	       w[7] * this->nodes_[cd.edge3_index()]);
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[16];
    get_quadratic_derivate_weights(coords,w);

    derivs.resize(2);
    derivs[0]=
      static_cast<typename VECTOR2::value_type>(
       w[0]*cd.node0() +
       w[1]*cd.node1() +
       w[2]*cd.node2() +
	     w[3]*cd.node3() +
       w[4]*this->nodes_[cd.edge0_index()] +
	     w[5]*this->nodes_[cd.edge1_index()] +
	     w[6]*this->nodes_[cd.edge2_index()] +
	     w[7]*this->nodes_[cd.edge3_index()]);

    derivs[1]=
      static_cast<typename VECTOR2::value_type>(
      w[8]*cd.node0() +
      w[9]*cd.node1() +
	    w[10]*cd.node2() +
	    w[11]*cd.node3() +
	    w[12]*this->nodes_[cd.edge0_index()] +
	    w[13]*this->nodes_[cd.edge1_index()] +
	    w[14]*this->nodes_[cd.edge2_index()] +
	    w[15]*this->nodes_[cd.edge3_index()]);
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    QuadLocate< QuadBiquadraticLgn<T> > CL;
    return CL.get_coords(this, coords, value, cd);
  }

  /// get arc length for edge
  template <class ElemData>
  double get_arc_length(const unsigned edge, const ElemData &cd) const
  {
    return get_arc2d_length<CrvGaussian2<double> >(this, edge, cd);
  }

 /// get area
  template <class ElemData>
    double get_area(const unsigned face, const ElemData &cd) const
  {
    return get_area2<QuadGaussian2<double> >(this, face, cd);
  }

  /// get volume
  template <class ElemData>
    double get_volume(const ElemData & /* cd */) const
  {
    return 0.;
  }

  static  const std::string type_name(int n = -1);

  virtual void io (Piostream& str);

};





template <class T>
const std::string
QuadBiquadraticLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("QuadBiquadraticLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}




}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::QuadBiquadraticLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("QuadBiquadraticLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int QUADBIQUADRATICLGN_VERSION = 1;
template <class T>
void
  Core::Basis::QuadBiquadraticLgn<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    QUADBIQUADRATICLGN_VERSION );
  Pio(stream, this->nodes_);
  stream.end_class();
}

}


#endif

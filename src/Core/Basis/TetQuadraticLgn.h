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
   Date:            December 4 2004
*/

#ifndef CORE_BASIS_TETQUADRATICLGN_H
#define CORE_BASIS_TETQUADRATICLGN_H 1

#include <Core/Basis/TetLinearLgn.h>
#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of TetQuadraticLgn
class SCISHARE TetQuadraticLgnUnitElement : public TetLinearLgnUnitElement {
  public:
    static double unit_vertices[10][3]; ///< Parametric coordinates of vertices of unit edge

    TetQuadraticLgnUnitElement() {}
    virtual ~TetQuadraticLgnUnitElement() {}

    /// return number of vertices
    static int number_of_vertices()
      { return 10; }

    ///< return degrees of freedom
    static int dofs()
      { return 10; }
};


/// Class for handling of element of type tetrahedron with
/// quadratic lagrangian interpolation
template <class T>
class TetQuadraticLgn : public BasisAddNodes<T>,
                        public TetApprox,
			public TetGaussian3<double>,
      public TetSamplingSchemes,
			public TetQuadraticLgnUnitElement,
      public TetElementWeights
{
public:
  typedef T value_type;

  TetQuadraticLgn() {}
  virtual ~TetQuadraticLgn() {}

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
    double w[10];
    get_quadratic_weights(coords, w);

    return (T)(w[0] * cd.node0() +
	       w[1] * cd.node1() +
	       w[2] * cd.node2() +
	       w[3] * cd.node3() +
	       w[4] * this->nodes_[cd.edge0_index()] +
	       w[5] * this->nodes_[cd.edge1_index()] +
	       w[6] * this->nodes_[cd.edge2_index()] +
	       w[7] * this->nodes_[cd.edge3_index()] +
	       w[8] * this->nodes_[cd.edge4_index()] +
	       w[9] * this->nodes_[cd.edge5_index()]);
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[30];
    get_quadratic_derivate_weights(coords, w);

    derivs.resize(3);

    derivs[0]=static_cast<typename VECTOR2::value_type>(
    w[0]*cd.node0() +
		w[1]*cd.node1() +
		w[4]*this->nodes_[cd.edge0_index()] +
		w[5]*this->nodes_[cd.edge1_index()] +
		w[6]*this->nodes_[cd.edge2_index()] +
		w[7]*this->nodes_[cd.edge3_index()] +
		w[8]*this->nodes_[cd.edge4_index()]);

    derivs[1]=static_cast<typename VECTOR2::value_type>(
    w[10]*cd.node0() +
		w[12]*cd.node2() +
		w[14]*this->nodes_[cd.edge0_index()] +
		w[15]*this->nodes_[cd.edge1_index()] +
		w[16]*this->nodes_[cd.edge2_index()] +
		w[17]*this->nodes_[cd.edge3_index()] +
		w[19]*this->nodes_[cd.edge5_index()]);

    derivs[2]=static_cast<typename VECTOR2::value_type>(
    w[20]*cd.node0() +
		w[23]*cd.node3() +
		w[24]*this->nodes_[cd.edge0_index()] +
		w[26]*this->nodes_[cd.edge2_index()] +
		w[27]*this->nodes_[cd.edge3_index()] +
		w[28]*this->nodes_[cd.edge4_index()] +
		w[29]*this->nodes_[cd.edge5_index()]);
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    TetLocate< TetQuadraticLgn<T> > CL;
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
TetQuadraticLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("TetQuadraticLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}




}}
template <class T>
const TypeDescription* get_type_description(Core::Basis::TetQuadraticLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("TetQuadraticLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int TETQUADRATICLGN_VERSION = 1;
template <class T>
void
  Core::Basis::TetQuadraticLgn<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    TETQUADRATICLGN_VERSION);
  Pio(stream, this->nodes_);
  stream.end_class();
}

}

#endif

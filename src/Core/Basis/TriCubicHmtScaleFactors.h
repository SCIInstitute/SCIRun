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


// NOTE: THIS FILE NEEDS TO BE CHANGED: SCALEFACTORS NEED TO BE COMPUTED
// AUTOMATICALLY FROM THE MESH. THIS WILL CHANGE IN FUTURE


#ifndef CORE_BASIS_TRICUBICHMTSCALEFACTORS_H
#define CORE_BASIS_TRICUBICHMTSCALEFACTORS_H 1

#include <Core/Persistent/PersistentSTL.h>
#include <Core/Basis/TriLinearLgn.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of TetCubicHmt
class TriCubicScaleFactorsHmtUnitElement : public TriLinearLgnUnitElement {
public:
  TriCubicScaleFactorsHmtUnitElement() {}
  virtual ~TriCubicScaleFactorsHmtUnitElement() {}

  static int dofs() { return 12; } ///< return degrees of freedom
};


/// Class for handling of element of type triangle with
/// cubic hermitian interpolation with scale factors
template <class T>
class TriCubicHmtScaleFactors : public BasisAddDerivativesScaleFactors<T>,
                                public TriApprox,
                                public TriGaussian3<double>,
                                public TriSamplingSchemes,
                                public TriCubicScaleFactorsHmtUnitElement,
                                public TriElementWeights
{
public:
  typedef T value_type;

  TriCubicHmtScaleFactors() {}
  virtual ~TriCubicHmtScaleFactors() {}

  static int polynomial_order() { return 3; }

  /// Note: these are correct for Point interpolation but not for value
  /// interpolation (Scale factors are missing)
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
    double w[12];
    unsigned int elem=cd.elem_index();
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    const double x2=x*x, x3=x2*x, y2=y*y, y3=y2*y;

    const double sdx0=this->scalefactors_[elem][0];
    const double sdx1=this->scalefactors_[elem][0];
    const double sdx2=this->scalefactors_[elem][0];

    const double sdy0=this->scalefactors_[elem][1];
    const double sdy1=this->scalefactors_[elem][1];
    const double sdy2=this->scalefactors_[elem][1];

    const double sdxy0=this->scalefactors_[elem][0]*this->scalefactors_[elem][1];
    const double sdxy1=this->scalefactors_[elem][0]*this->scalefactors_[elem][1];
    const double sdxy2=this->scalefactors_[elem][0]*this->scalefactors_[elem][1];


    w[0]  = (-1 + x + y)*(-1 - x + 2*x2 - y - 2*x*y + 2*y2);
    w[1]  = x*(1 - 2*x + x2 - 3*y2 + 2*y3)*sdx0;
    w[2]  = y*(1 - 3*x2 + 2*x3 - 2*y + y2)*sdy0;
    w[3]  = x*y*(1 - 2*x + x2 - 2*y + y2)*sdxy0;
    w[4]  = -(x2*(-3 + 2*x));
    w[5]  = (-1 + x)*x2*sdx1;
    w[6]  = -(x2*(-3 + 2*x)*y)*sdy1;
    w[7]  = (-1 + x)*x2*y*sdxy1;
    w[8]  = -y2*(-3 + 2*y);
    w[9]  = -(x*y2*(-3 + 2*y))*sdx2;
    w[10] = (-1 + y)*y2*sdy2;
    w[11] = x*(-1 + y)*y2*sdxy2;

    return (T)(w[0]  * cd.node0()
	       +w[1]  * this->derivs_[cd.node0_index()][0]
	       +w[2]  * this->derivs_[cd.node0_index()][1]
	       +w[3]  * this->derivs_[cd.node0_index()][2]
	       +w[4]  * cd.node1()
	       +w[5]  * this->derivs_[cd.node1_index()][0]
	       +w[6]  * this->derivs_[cd.node1_index()][1]
	       +w[7]  * this->derivs_[cd.node1_index()][2]
	       +w[8]  * cd.node2()
	       +w[9]  * this->derivs_[cd.node2_index()][0]
	       +w[10] * this->derivs_[cd.node2_index()][1]
	       +w[11] * this->derivs_[cd.node2_index()][2]);
  }



  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[24];
    unsigned elem=cd.elem_index();
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    const double x2=x*x, x3=x2*x, y2=y*y;
    const double y12=(y-1)*(y-1);

    const double sdx0=this->scalefactors_[elem][0];
    const double sdx1=this->scalefactors_[elem][0];
    const double sdx2=this->scalefactors_[elem][0];

    const double sdy0=this->scalefactors_[elem][1];
    const double sdy1=this->scalefactors_[elem][1];
    const double sdy2=this->scalefactors_[elem][1];

    const double sdxy0=this->scalefactors_[elem][0]*this->scalefactors_[elem][1];
    const double sdxy1=this->scalefactors_[elem][0]*this->scalefactors_[elem][1];
    const double sdxy2=this->scalefactors_[elem][0]*this->scalefactors_[elem][1];

    w[0] = 6*(-1 + x)*x;
    w[1] = (-4*x + 3*x2 + y12*(1 + 2*y))*sdx0;
    w[2] = 6*(-1 + x)*x*y*sdy0;
    w[3] = (-4*x + 3*x2 + y12)*y*sdxy0;
    w[4] = -6*(-1 + x)*x;
    w[5] = x*(-2 + 3*x)*sdx1;
    w[6] = -6*(-1 + x)*x*y*sdy1;
    w[7] = x*(-2 + 3*x)*y*sdxy1;
    w[8] = 0;
    w[9] = (3 - 2*y)*y2*sdx2;
    w[10] = 0;
    w[11] = (-1 + y)*y2*sdxy2;

    w[12] = 6*(-1 + y)*y;
    w[13] = 6*x*(-1 + y)*y*sdx0;
    w[14] = (1 - 3*x2 + 2*x3 - 4*y + 3*y2)*sdy0;
    w[15] = x*(1 - 2*x + x2 - 4*y + 3*y2)*sdxy0;
    w[16] = 0;
    w[17] = 0;
    w[18] = (3 - 2*x)*x2*sdy1;
    w[19] = (-1 + x)*x2*sdxy1;
    w[20] = -6*(-1 + y)*y;
    w[21] = -6*x*(-1 + y)*sdx2;
    w[22] = y*(-2 + 3*y)*sdy2;
    w[23] = x*y*(-2 + 3*y)*sdxy2;
    derivs.resize(2);

    derivs[0]=static_cast<typename VECTOR2::value_type>(w[0]  * cd.node0()
	       +w[1]  * this->derivs_[cd.node0_index()][0]
	       +w[2]  * this->derivs_[cd.node0_index()][1]
	       +w[3]  * this->derivs_[cd.node0_index()][2]
	       +w[4]  * cd.node1()
	       +w[5]  * this->derivs_[cd.node1_index()][0]
	       +w[6]  * this->derivs_[cd.node1_index()][1]
	       +w[7]  * this->derivs_[cd.node1_index()][2]
	       +w[8]  * cd.node2()
	       +w[9]  * this->derivs_[cd.node2_index()][0]
	       +w[10] * this->derivs_[cd.node2_index()][1]
	       +w[11] * this->derivs_[cd.node2_index()][2]);

    derivs[1]=static_cast<typename VECTOR2::value_type>(w[12]  * cd.node0()
	       +w[13]  * this->derivs_[cd.node0_index()][0]
	       +w[14]  * this->derivs_[cd.node0_index()][1]
	       +w[15]  * this->derivs_[cd.node0_index()][2]
	       +w[16]  * cd.node1()
	       +w[17]  * this->derivs_[cd.node1_index()][0]
	       +w[18]  * this->derivs_[cd.node1_index()][1]
	       +w[19]  * this->derivs_[cd.node1_index()][2]
	       +w[20]  * cd.node2()
	       +w[21]  * this->derivs_[cd.node2_index()][0]
	       +w[22] * this->derivs_[cd.node2_index()][1]
	       +w[23] * this->derivs_[cd.node2_index()][2]);
  }

  /// get the parametric coordinate for value within the element.
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    TriLocate< TriCubicHmtScaleFactors<T> > CL;
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
    return get_area2<TriGaussian3<double> >(this, face, cd);
  }

  /// get volume
  template <class ElemData>
    double get_volume(const ElemData & /* cd */) const
  {
    return 0.;
  }


  static const std::string type_name(int n = -1);


  virtual void io (Piostream& str);

};





template <class T>
const std::string
TriCubicHmtScaleFactors<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("TriCubicHmtScaleFactors");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}





}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::TriCubicHmtScaleFactors<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("TriCubicHmtScaleFactors", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int TRICUBICHMTSCALEFACTORS_VERSION = 1;
template <class T>
void
  Core::Basis::TriCubicHmtScaleFactors<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    TRICUBICHMTSCALEFACTORS_VERSION);
  Pio(stream, this->derivs_);
  Pio(stream, this->scalefactors_);
  stream.end_class();
}
}

#endif

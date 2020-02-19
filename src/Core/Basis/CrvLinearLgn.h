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


#ifndef CORE_BASIS_CRVLINEARLGN_H
#define CORE_BASIS_CRVLINEARLGN_H 1

#include <cfloat>

#include <Core/Basis/Basis.h>
#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Datatypes/Legacy/Base/TypeName.h>
#include <Core/Basis/Locate.h>
#include <Core/Basis/CrvElementWeights.h>
#include <Core/Basis/CrvSamplingSchemes.h>
#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of CrvLinearLgn
class CrvLinearLgnUnitElement {
public:
  static SCISHARE double unit_vertices[2][1]; ///< Parametric coordinates of vertices
  static SCISHARE int unit_edges[1][2];    ///< References to vertices of unit edge
  static SCISHARE double unit_center[1];

  CrvLinearLgnUnitElement() {}
  virtual ~CrvLinearLgnUnitElement() {}

  /// return dimension of domain
  static int domain_dimension()
    { return 1; }

   /// return size of the domain
  static double domain_size()
  { return 1.0; }

  /// return number of vertices
  static int number_of_vertices()
    { return 2; }

  /// return number of vertices in mesh
  static int number_of_mesh_vertices()
    { return 2; }

  /// return degrees of freedom
  static int dofs()
    { return 2; }

  /// return number of edges
  static int number_of_edges()
    { return 1; }

  /// return number of vertices per face
  static int vertices_of_face()
    { return 0; }

  /// return number of faces per cell
  static int faces_of_cell()
    { return 0; }

  static double length(int /* edge */) { return 1.; } ///< return length
  static double area(int /* face */) { return 0.; } ///< return area
  static double volume() { return 0.; } ///< return volume

};


/// Class for creating geometrical approximations of Crv meshes
class CrvApprox {
public:
  CrvApprox() {}
  virtual ~CrvApprox() {}

  /// Approximate edge for element by piecewise linear segments
  /// return: coords gives parametric coordinates of the approximation.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_edge(const unsigned /*edge*/,
                   const unsigned div_per_unit,
                   VECTOR& coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;
    coords.resize(div_per_unit + 1);
    for(unsigned i = 0; i <= div_per_unit; i++)
    {
      coords[i].resize(1);
      coords[i][0] = static_cast<typename VECTOR2::value_type>(
                                          (double)i / (double)div_per_unit);
    }
  }

  /// Approximate faces for element by piecewise linear elements
  /// return: coords gives parametric coordinates at the approximation point.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_face(const unsigned /*face*/,
                   const unsigned /*div_per_unit*/,
                   VECTOR& coords) const
  {
    coords.resize(0);
  }

};


/// Class for searching of parametric coordinates related to a
/// value in Crv meshes and fields
template <class ElemBasis>
class CrvLocate : public Dim1Locate<ElemBasis> {
public:
  typedef typename ElemBasis::value_type T;

  CrvLocate() {}
  virtual ~CrvLocate() {}

  /// find coordinate in interpolation for given value
  template <class ElemData, class VECTOR>
  bool get_coords(const ElemBasis *pEB, VECTOR &coords,
		  const T& value, const ElemData &cd) const
  {
    initial_guess(pEB, value, cd, coords);
    if (this->get_iterative(pEB, coords, value, cd))
      return check_coords(coords);
    return false;
  }

  template<class VECTOR>
  inline bool check_coords(const VECTOR &x) const
  {
    if (x[0]>=-Dim3Locate<ElemBasis>::thresholdDist &&
      x[0]<=Dim3Locate<ElemBasis>::thresholdDist1)
      return true;

    return false;
  }

protected:
  /// find a reasonable initial guess for starting Newton iteration.
  /// Reasonable means near and with a derivative!=0
  template <class ElemData, class VECTOR>
  void initial_guess(const ElemBasis *pElem, const T &val,
		     const ElemData &cd, VECTOR &guess) const
  {
    double dist = DBL_MAX;

    VECTOR coord(1);
    StackVector<T,1> derivs;
    guess.resize(1);

    const int end = 3;
    for (int x = 1; x < end; x++) {
      coord[0] = x / (double) end;
      double cur_d;
      if (compare_distance(pElem->interpolate(coord, cd), val, cur_d, dist))
      {
        pElem->derivate(coord, cd, derivs);
        if (!check_zero(derivs))
        {
          dist = cur_d;
          guess = coord;
        }
      }
    }
  }
};

/// Class with weights and coordinates for 1st order Gaussian integration
template <class T>
class CrvGaussian1
{
public:
  static int GaussianNum;
  static T GaussianPoints[1][1];
  static T GaussianWeights[1];
};

template <class T>
int CrvGaussian1<T>::GaussianNum = 1;

template <class T>
T CrvGaussian1<T>::GaussianPoints[1][1] = {{0.5}};

template <class T>
T CrvGaussian1<T>::GaussianWeights[1] = {1.};

/// Class with weights and coordinates for 2nd order Gaussian integration
template <class T>
class CrvGaussian2
{
public:
  static int GaussianNum;
  static T GaussianPoints[2][1];
  static T GaussianWeights[2];
};

template <class T>
int CrvGaussian2<T>::GaussianNum = 2;

template <class T>
T CrvGaussian2<T>::GaussianPoints[2][1] = {{0.211324865405}, {0.788675134595}};

template <class T>
T CrvGaussian2<T>::GaussianWeights[2] = {.5, .5};

/// Class with weights and coordinates for 3rd order Gaussian integration
template <class T>
class CrvGaussian3
{
public:
  static int GaussianNum;
  static T GaussianPoints[3][1];
  static T GaussianWeights[3];
};

template <class T>
int CrvGaussian3<T>::GaussianNum = 3;

template <class T>
T CrvGaussian3<T>::GaussianPoints[3][1] =
  {{0.11270166537950}, {0.5}, {0.88729833462050}};

template <class T>
T CrvGaussian3<T>::GaussianWeights[3] =
  {.2777777777, .4444444444, .2777777777};



/// Class for handling of element of type curve with
/// linear lagrangian interpolation
template <class T>
class CrvLinearLgn :
          public BasisSimple<T>,
          public CrvApprox,
          public CrvGaussian1<double>,
          public CrvSamplingSchemes,
          public CrvLinearLgnUnitElement,
          public CrvElementWeights
{
public:
  typedef T value_type;

  CrvLinearLgn() : CrvApprox() {}
  virtual ~CrvLinearLgn() {}


  static int polynomial_order() { return 1; }

  template<class VECTOR>
  inline void get_weights(const VECTOR& coords, double *w) const
    { get_linear_weights(coords,w); }

  template<class VECTOR>
  inline void get_derivate_weights(const VECTOR& coords, double *w) const
    { get_linear_derivate_weights(coords,w); }


  /// get value at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR &coords, const ElemData &cd) const
  {
    double w[2];
    get_linear_weights(coords, w);
    return (T)(w[0] * cd.node0() + w[1] * cd.node1());
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1& /*coords*/, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    derivs.resize(1);
    derivs[0] = static_cast<typename VECTOR2::value_type>(cd.node1()-cd.node0());
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    CrvLocate< CrvLinearLgn<T> > CL;
    return CL.get_coords(this, coords, value, cd);
  }

  /// get arc length for edge
  template <class ElemData>
  double get_arc_length(const unsigned edge, const ElemData &cd) const
  {
    return get_arc1d_length<CrvGaussian1<double> >(this, edge, cd);
  }

  /// get area
  template <class ElemData>
    double get_area(const unsigned /* face */, const ElemData & /* cd */) const
  {
    return 0.;
  }

  /// get volume
  template <class ElemData>
    double get_volume(const ElemData & /* cd */) const
  {
    return 0.;
  }

  template <class VECTOR>
  void approx_edge(const unsigned /*edge*/,
			   const unsigned /*div_per_unit*/,
			   std::vector<VECTOR> &coords) const
  {
    coords.resize(2);
    VECTOR &tmp = coords[0];
    tmp.resize(1);
    tmp[0] = 0.0;
    VECTOR &tmp1 = coords[1];
    tmp1.resize(1);
    tmp[0] = 1.0;
  }

  static const std::string type_name(int n = -1);

  virtual void io (Piostream& str);

};



template <class T>
const std::string
CrvLinearLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("CrvLinearLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}





}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::CrvLinearLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("CrvLinearLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}
const int CRVLINEARLGN_VERSION = 1;
template <class T>
void
  Core::Basis::CrvLinearLgn<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    CRVLINEARLGN_VERSION);
  stream.end_class();
}
}

#endif

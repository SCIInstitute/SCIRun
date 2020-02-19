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


#ifndef CORE_BASIS_TRILINEARLGN_H
#define CORE_BASIS_TRILINEARLGN_H 1

#include <float.h>

#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/TriElementWeights.h>
#include <Core/Basis/TriSamplingSchemes.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of TriLinearLgn
class TriLinearLgnUnitElement {
public:
  /// Parametric coordinates of vertices of unit edge
  static SCISHARE double unit_vertices[3][2];
  /// References to vertices of unit edge
  static SCISHARE int unit_edges[3][2];
  /// References to vertices of unit face
  static SCISHARE int unit_faces[1][3];
  /// References to normal of unit face
  static SCISHARE double unit_face_normals[1][3];
  /// The center of the unit element
  static SCISHARE double unit_center[2];

  TriLinearLgnUnitElement() {}
  virtual ~TriLinearLgnUnitElement() {}

  /// return dimension of domain
  static int domain_dimension()
    { return 2; }

  /// return size of the domain
  static double domain_size()
    { return 0.5; }

  /// return number of vertices
  static int number_of_vertices()
    { return 3; }

  /// return number of vertices in mesh
  static int number_of_mesh_vertices()
    { return 3; }

  /// return degrees of freedom
  static int dofs()
    { return 3; }

  /// return number of edges
  static int number_of_edges()
    { return 3; }

  /// return number of vertices per face
  static int vertices_of_face()
    { return 3; }

  /// return number of faces per cell
  static int faces_of_cell()
    { return 1; }

  static inline double length(int edge)
  {
    const double *v0 = unit_vertices[unit_edges[edge][0]];
    const double *v1 = unit_vertices[unit_edges[edge][1]];
    const double dx = v1[0] - v0[0];
    const double dy = v1[1] - v0[1];
    return sqrt(dx*dx+dy*dy);
  }

  static double area(int /* face */) { return 0.5; } ///< return area
  static double volume() { return 0.; } ///< return volume
};


/// Class for creating geometrical approximations of Tri meshes
class TriApprox {
public:
  TriApprox() {}
  virtual ~TriApprox() {}

  /// Approximate edge for element by piecewise linear segments
  /// return: coords gives parametric coordinates of the approximation.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_edge(const unsigned edge,
                   const unsigned div_per_unit,
                   VECTOR &coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;

    coords.resize(div_per_unit+1);

    const double *v0 = TriLinearLgnUnitElement::unit_vertices[TriLinearLgnUnitElement::unit_edges[edge][0]];
    const double *v1 = TriLinearLgnUnitElement::unit_vertices[TriLinearLgnUnitElement::unit_edges[edge][1]];

    const double &p1x = v0[0];
    const double &p1y = v0[1];
    const double dx = v1[0] - p1x;
    const double dy = v1[1] - p1y;

    for(unsigned i = 0; i <= div_per_unit; i++) {
      const double d = (double)i / (double)div_per_unit;
      typename VECTOR::value_type &tmp = coords[i];
      tmp.resize(2);
      tmp[0] = static_cast<typename VECTOR2::value_type>(p1x + d * dx);
      tmp[1] = static_cast<typename VECTOR2::value_type>(p1y + d * dy);
    }
  }

   /// Approximate faces for element by piecewise linear elements
  /// return: coords gives parametric coordinates at the approximation point.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_face(const unsigned /* face */,
                   const unsigned div_per_unit,
                   VECTOR &coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;
    typedef typename VECTOR2::value_type VECTOR3;

    coords.resize(div_per_unit);
    const double d = 1. / div_per_unit;

    for(unsigned j = 0; j < div_per_unit; j++)
    {
      const double dj = (double)j / (double)div_per_unit;
      unsigned e = 0;
      coords[j].resize((div_per_unit - j) * 2 + 1);
      typename VECTOR2::value_type& tmp = coords[j][e++];
      tmp.resize(2);
      tmp[0] = 0;
      tmp[1] = dj;
      for(unsigned i = 0; i<div_per_unit - j; i++)
      {
        const double di = (double)i / (double)div_per_unit;
        typename VECTOR2::value_type& tmp1 = coords[j][e++];
        tmp1.resize(2);
        tmp1[0] = static_cast<typename VECTOR3::value_type>(di);
        tmp1[1] = static_cast<typename VECTOR3::value_type>(dj + d);
        typename VECTOR2::value_type& tmp2 = coords[j][e++];
        tmp2.resize(2);
        tmp2[0] = static_cast<typename VECTOR3::value_type>(di + d);
        tmp2[1] = static_cast<typename VECTOR3::value_type>(dj);
      }
    }
  }
};

/// Class for searching of parametric coordinates related to a
/// value in Tri meshes and fields
template <class ElemBasis>
class TriLocate : public Dim2Locate<ElemBasis> {
public:
  typedef typename ElemBasis::value_type T;

  TriLocate() {}
  virtual ~TriLocate() {}

  /// find value in interpolation for given value
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
    if (x[0]>=-Dim2Locate<ElemBasis>::thresholdDist)
      if (x[1]>=-Dim2Locate<ElemBasis>::thresholdDist)
	if (x[0]+x[1]<=Dim2Locate<ElemBasis>::thresholdDist1)
	  return true;

    return false;
  }

protected:
  /// find a reasonable initial guess
  template <class ElemData, class VECTOR>
  void initial_guess(const ElemBasis *pElem, const T &val, const ElemData &cd,
		     VECTOR & guess) const
  {
    double dist = DBL_MAX;

    VECTOR coord(2);
    StackVector<T,2> derivs;
    guess.resize(2);

    const int end = 3;
    for (int x = 1; x < end; x++)
    {
      coord[0] = x / (double) end;
      for (int y = 1; y < end; y++)
      {
        coord[1] = y / (double) end;
        if (coord[0]+coord[1]>Dim2Locate<ElemBasis>::thresholdDist1)
          break;
        double cur_d;
        if (compare_distance(pElem->interpolate(coord, cd),
			     val, cur_d, dist))
        {
          pElem->derivate(coord, cd, derivs);
          if (!check_zero(derivs)) {
            dist = cur_d;
            guess = coord;
          }
        }
      }
    }
  }
};


/// Class with weights and coordinates for 2nd order Gaussian integration
template <class T>
class TriGaussian1
{
public:
  static int GaussianNum;
  static T GaussianPoints[1][2];
  static T GaussianWeights[1];
};

template <class T>
int TriGaussian1<T>::GaussianNum = 1;

template <class T>
T TriGaussian1<T>::GaussianPoints[1][2] = { {1./3.,1./3.}};

template <class T>
T TriGaussian1<T>::GaussianWeights[1] = {1.0};

/// Class with weights and coordinates for 2nd order Gaussian integration
template <class T>
class TriGaussian2
{
public:
  static int GaussianNum;
  static T GaussianPoints[3][2];
  static T GaussianWeights[3];
};

template <class T>
int TriGaussian2<T>::GaussianNum = 3;

template <class T>
T TriGaussian2<T>::GaussianPoints[3][2] = {
  {1./6.,1./6.}, {2./3.,1./6.}, {1./6.,2./3.}};

template <class T>
T TriGaussian2<T>::GaussianWeights[3] = {1./3., 1./3., 1./3.};

/// Class with weights and coordinates for 3rd order Gaussian integration
template <class T>
class TriGaussian3
{
public:
  static int GaussianNum;
  static T GaussianPoints[7][2];
  static T GaussianWeights[7];
};

template <class T>
int TriGaussian3<T>::GaussianNum = 7;

template <class T>
T TriGaussian3<T>::GaussianPoints[7][2] = {
  {0.1012865073, 0.1012865073}, {0.7974269853, 0.1012865073}, {0.1012865073, 0.7974269853},
  {0.4701420641, 0.0597158717}, {0.4701420641, 0.4701420641}, {0.0597158717, 0.4701420641},
  {0.3333333333, 0.3333333333}};

template <class T>
T TriGaussian3<T>::GaussianWeights[7] =
  {0.1259391805, 0.1259391805, 0.1259391805, 0.1323941527, 0.1323941527, 0.1323941527, 0.225};

#ifdef _WIN32
// force the instantiation of TriGaussian3<double>
template class TriGaussian1<double>;
template class TriGaussian2<double>;
template class TriGaussian3<double>;
#endif


/// Class for handling of element of type triangle with
/// linear lagrangian interpolation
template <class T>
class TriLinearLgn :
         public BasisSimple<T>,
         public TriApprox,
		     public TriGaussian1<double>,
         public TriSamplingSchemes,
		     public TriLinearLgnUnitElement,
         public TriElementWeights
{
public:
  typedef T value_type;

  TriLinearLgn() {}
  virtual ~TriLinearLgn() {}

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
    double w[3];
    get_linear_weights(coords, w);

    return (T)(w[0] * cd.node0() +
	       w[1] * cd.node1() +
	       w[2] * cd.node2());
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1& /*coords*/, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    derivs.resize(2);

    derivs[0] = static_cast<typename VECTOR2::value_type>(-1. * cd.node0() + cd.node1());
    derivs[1] = static_cast<typename VECTOR2::value_type>(-1. * cd.node0() + cd.node2());
  }

  /// get the parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    TriLocate< TriLinearLgn<T> > CL;
    return CL.get_coords(this, coords, value, cd);
  }

  /// get arc length for edge
  template <class ElemData>
  double get_arc_length(const unsigned edge, const ElemData &cd) const
  {
    return get_arc2d_length<CrvGaussian1<double> >(this, edge, cd);
  }

  /// get area
  template <class ElemData>
    double get_area(const unsigned face, const ElemData &cd) const
  {
    return get_area2<TriGaussian2<double> >(this, face, cd);
  }

  /// get volume
  template <class ElemData>
    double get_volume(const ElemData & /* cd */) const
  {
    return 0.;
  }

  template<class VECTOR>
  void approx_edge(const unsigned int edge,
			   const unsigned int div_per_unit,
			   VECTOR &coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;

    coords.resize(div_per_unit + 1);

    const double *v0 = TriLinearLgnUnitElement::unit_vertices[TriLinearLgnUnitElement::unit_edges[edge][0]];
    const double *v1 = TriLinearLgnUnitElement::unit_vertices[TriLinearLgnUnitElement::unit_edges[edge][1]];

    const double &p1x = v0[0];
    const double &p1y = v0[1];
    const double dx = v1[0] - p1x;
    const double dy = v1[1] - p1y;

    for(unsigned int i = 0; i <= div_per_unit; i++) {
      const double d = (double)i / (double)div_per_unit;
      typename VECTOR::value_type &tmp = coords[i];
      tmp.resize(2);
      tmp[0] = static_cast<typename VECTOR2::value_type>(p1x + d * dx);
      tmp[1] = static_cast<typename VECTOR2::value_type>(p1y + d * dy);
    }
  }

  template<class VECTOR>
  void approx_face(const unsigned int /*face*/,
			   const unsigned int div_per_unit,
			   VECTOR &coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;
    typedef typename VECTOR2::value_type VECTOR3;

    const double *v0 = TriLinearLgnUnitElement::unit_vertices[TriLinearLgnUnitElement::unit_faces[0][0]];
    const double *v1 = TriLinearLgnUnitElement::unit_vertices[TriLinearLgnUnitElement::unit_faces[0][1]];
    const double *v2 = TriLinearLgnUnitElement::unit_vertices[TriLinearLgnUnitElement::unit_faces[0][2]];

    coords.resize(div_per_unit);
    const double d = 1. / div_per_unit;
    for(unsigned int j = 0; j<div_per_unit; j++)
    {
      const double dj = (double)j / (double)div_per_unit;
      unsigned int e = 0;
      coords[j].resize((div_per_unit - j) * 2 + 1);
      typename VECTOR2::value_type &tmp = coords[j][e++];
      tmp.resize(2);
      tmp[0] = static_cast<typename VECTOR3::value_type>(v0[0] + dj * (v2[0] - v0[0]));
      tmp[1] = static_cast<typename VECTOR3::value_type>(v0[1] + dj * (v2[1] - v0[1]));

      for(unsigned int i = 0; i<div_per_unit - j; i++)
      {
        const double di = (double)i / (double)div_per_unit;
        typename VECTOR2::value_type &tmp1 = coords[j][e++];
        tmp1.resize(2);
        tmp1[0] = static_cast<typename VECTOR3::value_type>(v0[0] + (dj + d) * (v2[0] - v0[0]) + di * (v1[0] - v0[0]));
        tmp1[1] = static_cast<typename VECTOR3::value_type>(v0[1] + (dj + d) * (v2[1] - v0[1]) + di * (v1[1] - v0[1]));

        typename VECTOR2::value_type &tmp2 = coords[j][e++];
        tmp2.resize(2);
        tmp2[0] = static_cast<typename VECTOR3::value_type>(v0[0] + dj * (v2[0] - v0[0]) + (di + d) * (v1[0] - v0[0]));
        tmp2[1] = static_cast<typename VECTOR3::value_type>(v0[1] + dj * (v2[1] - v0[1]) + (di + d) * (v1[1] - v0[1]));
      }
    }
  }


  static const std::string type_name(int n = -1);


  virtual void io (Piostream& str);

};



template <class T>
const std::string
TriLinearLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("TriLinearLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}

}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::TriLinearLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("TriLinearLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

  const int TRILINEARLGN_VERSION = 1;
  template <class T>
  void
  Core::Basis::TriLinearLgn<T>::io(Piostream &stream)
  {
    stream.begin_class(get_type_description(this)->get_name(),
                       TRILINEARLGN_VERSION);
    stream.end_class();
  }
}

#endif

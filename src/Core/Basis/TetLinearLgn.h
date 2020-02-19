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

#ifndef CORE_BASIS_TETLINEARLGN_H
#define CORE_BASIS_TETLINEARLGN_H 1

#include <cfloat>

#include <Core/Basis/TetElementWeights.h>
#include <Core/Basis/TetSamplingSchemes.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of TetLinearLgn
class TetLinearLgnUnitElement {
public:
  /// Parametric coordinates of vertices of unit edge
  static SCISHARE double unit_vertices[4][3];
  /// References to vertices of unit edge
  static SCISHARE int unit_edges[6][2];
  /// References to vertices of unit face
  static SCISHARE int unit_faces[4][3];
  /// Normals of unit faces
  static SCISHARE double unit_face_normals[4][3];
  /// Precalculated area of faces
  static SCISHARE double unit_face_areas[4];
  /// Center of the unit element
  static SCISHARE double unit_center[3];

  TetLinearLgnUnitElement() {}
  virtual ~TetLinearLgnUnitElement() {}

  /// return dimension of domain
  static int domain_dimension()
    { return 3; }

  /// return size of the domain
  static double domain_size()
    { return 1.0/6.0; }

  /// return number of vertices
  static int number_of_vertices()
    { return 4; }

  /// return number of vertices in mesh
  static int number_of_mesh_vertices()
    { return 4; }

  /// return number of edges
  static int number_of_edges()
    { return 6; }

  /// return degrees of freedom
  static int dofs()
    { return 4; }

  /// return number of vertices per face
  static int vertices_of_face()
    { return 3; }

  /// return number of faces per cell
  static int faces_of_cell()
    { return 4; }

  static inline double length(int edge)
  { ///< return length
    const double *v0 = unit_vertices[unit_edges[edge][0]];
    const double *v1 = unit_vertices[unit_edges[edge][1]];
    const double dx = v1[0] - v0[0];
    const double dy = v1[1] - v0[1];
    const double dz = v1[2] - v0[2];
    return sqrt(dx*dx+dy*dy+dz*dz);
  }
  static double area(int face) { return unit_face_areas[face]; } ///< return area
  static double volume() { return 1./6.; } ///< return volume
};


/// Class for creating geometrical approximations of Tet meshes
class TetApprox {
public:

  TetApprox() {}
  virtual ~TetApprox() {}

  /// Approximate edge for element by piecewise linear segments
  /// return: coords gives parametric coordinates of the approximation.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_edge(const unsigned edge,
                   const unsigned div_per_unit,
                   VECTOR &coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;

    coords.resize(div_per_unit + 1);

    const double *v0 = TetLinearLgnUnitElement::unit_vertices[TetLinearLgnUnitElement::unit_edges[edge][0]];
    const double *v1 = TetLinearLgnUnitElement::unit_vertices[TetLinearLgnUnitElement::unit_edges[edge][1]];

    const double &p1x = v0[0];
    const double &p1y = v0[1];
    const double &p1z = v0[2];
    const double dx = v1[0] - p1x;
    const double dy = v1[1] - p1y;
    const double dz = v1[2] - p1z;

    for(unsigned int i = 0; i <= div_per_unit; i++) {
      const double d = (double)i / (double)div_per_unit;
      typename VECTOR::value_type &tmp = coords[i];
      tmp.resize(3);
      tmp[0] = static_cast<typename VECTOR2::value_type>(p1x + d * dx);
      tmp[1] = static_cast<typename VECTOR2::value_type>(p1y + d * dy);
      tmp[2] = static_cast<typename VECTOR2::value_type>(p1z + d * dz);
    }
  }

  /// Approximate faces for element by piecewise linear elements
  /// return number of vertices per face
  virtual int get_approx_face_elements() const { return 3; }


  /// Approximate faces for element by piecewise linear elements
  /// return: coords gives parametric coordinates at the approximation point.
  /// Use interpolate with coordinates to get the world coordinates.
  template <class VECTOR>
  void approx_face(const unsigned face,
                   const unsigned div_per_unit,
                   VECTOR &coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;
    typedef typename VECTOR2::value_type VECTOR3;

    const double *v0 = TetLinearLgnUnitElement::unit_vertices[TetLinearLgnUnitElement::unit_faces[face][0]];
    const double *v1 = TetLinearLgnUnitElement::unit_vertices[TetLinearLgnUnitElement::unit_faces[face][1]];
    const double *v2 = TetLinearLgnUnitElement::unit_vertices[TetLinearLgnUnitElement::unit_faces[face][2]];
    coords.resize(div_per_unit);
    const double d = 1. / div_per_unit;
    for(unsigned int j = 0; j<div_per_unit; j++)
    {
      const double dj = (double)j / (double)div_per_unit;
      unsigned int e = 0;
      coords[j].resize((div_per_unit - j) * 2 + 1);
      typename VECTOR2::value_type &tmp = coords[j][e++];
      tmp.resize(3);
      tmp[0] = static_cast<typename VECTOR3::value_type>(v0[0] + dj * (v2[0] - v0[0]));
      tmp[1] = static_cast<typename VECTOR3::value_type>(v0[1] + dj * (v2[1] - v0[1]));
      tmp[2] = static_cast<typename VECTOR3::value_type>(v0[2] + dj * (v2[2] - v0[2]));

      for(unsigned int i = 0; i<div_per_unit - j; i++)
      {
        const double di = (double)i / (double)div_per_unit;
        typename VECTOR2::value_type &tmp1 = coords[j][e++];
        tmp1.resize(3);
        tmp1[0] = static_cast<typename VECTOR3::value_type>(v0[0] + (dj + d) * (v2[0] - v0[0]) + di * (v1[0] - v0[0]));
        tmp1[1] = static_cast<typename VECTOR3::value_type>(v0[1] + (dj + d) * (v2[1] - v0[1]) + di * (v1[1] - v0[1]));
        tmp1[2] = static_cast<typename VECTOR3::value_type>(v0[2] + (dj + d) * (v2[2] - v0[2]) + di * (v1[2] - v0[2]));

        typename VECTOR2::value_type &tmp2 = coords[j][e++];
        tmp2.resize(3);
        tmp2[0] = static_cast<typename VECTOR3::value_type>(v0[0] + dj * (v2[0] - v0[0]) + (di + d) * (v1[0] - v0[0]));
        tmp2[1] = static_cast<typename VECTOR3::value_type>(v0[1] + dj * (v2[1] - v0[1]) + (di + d) * (v1[1] - v0[1]));
        tmp2[2] = static_cast<typename VECTOR3::value_type>(v0[2] + dj * (v2[2] - v0[2]) + (di + d) * (v1[2] - v0[2]));
      }
    }
  }
};

/// Class for searching of parametric coordinates related to a
/// value in Tet meshes and fields
/// to do
template <class ElemBasis>
class TetLocate : public Dim3Locate<ElemBasis> {
public:
  typedef typename ElemBasis::value_type T;

  TetLocate() {}
  virtual ~TetLocate() {}

  /// find coodinate in interpolation for given value
  template <class ElemData, class VECTOR>
  bool get_coords(const ElemBasis *pEB, VECTOR &coords,
		  const T& value, const ElemData &cd) const
  {
    initial_guess(pEB, value, cd, coords);
    if (this->get_iterative(pEB, coords, value, cd))
      return check_coords(coords);
    return false;
  }

protected:
  template <class VECTOR>
  inline bool check_coords(const VECTOR &x) const
  {
    if (x[0]>=-Dim3Locate<ElemBasis>::thresholdDist)
      if (x[1]>=-Dim3Locate<ElemBasis>::thresholdDist)
        if (x[2]>=-Dim3Locate<ElemBasis>::thresholdDist)
          if (x[0]+x[1]+x[2]<=Dim3Locate<ElemBasis>::thresholdDist1)
            return true;

    return false;
  }

  /// find a reasonable initial guess
  template <class ElemData, class VECTOR>
  void initial_guess(const ElemBasis *pElem, const T &val, const ElemData &cd,
		     VECTOR & guess) const
  {
    double dist = DBL_MAX;

    VECTOR coord(3);
    StackVector<T,3> derivs;
    guess.resize(3);

    const int end = 3;
    for (int x = 1; x < end; x++)
    {
      coord[0] = x / (double) end;
      for (int y = 1; y < end; y++)
      {
        coord[1] = y / (double) end;
        if (coord[0]+coord[1]>Dim3Locate<ElemBasis>::thresholdDist1)
          break;
        for (int z = 1; z < end; z++)
        {
          coord[2] = z / (double) end;
          if (coord[0]+coord[1]+coord[2]>Dim3Locate<ElemBasis>::thresholdDist1)
            break;

          double cur_d;
          if (compare_distance(pElem->interpolate(coord, cd),
                   val, cur_d, dist))
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
    }
  }
};



/// Class with weights and coordinates for 1nd order Gaussian integration
template <class T>
class TetGaussian1
{
public:
  static int GaussianNum;
  static T GaussianPoints[1][3];
  static T GaussianWeights[1];
};

#ifdef _WIN32
// force the instantiation of TetGaussian1<double>
template class TetGaussian1<double>;
#endif

template <class T>
int TetGaussian1<T>::GaussianNum = 1;

template <class T>
T TetGaussian1<T>::GaussianPoints[][3] = {
  {1./4., 1./4., 1./4.}};

template <class T>
T TetGaussian1<T>::GaussianWeights[] = {1.0};


/// Class with weights and coordinates for 2nd order Gaussian integration
template <class T>
class TetGaussian2
{
public:
  static int GaussianNum;
  static T GaussianPoints[4][3];
  static T GaussianWeights[4];
};

#ifdef _WIN32
// force the instantiation of TetGaussian2<double>
template class TetGaussian2<double>;
#endif

template <class T>
int TetGaussian2<T>::GaussianNum = 4;

template <class T>
T TetGaussian2<T>::GaussianPoints[][3] = {
  {0.138196601125011,  0.138196601125011, 0.138196601125011},
  {0.585410196624969,  0.138196601125011, 0.138196601125011},
  {0.138196601125011,  0.585410196624969, 0.138196601125011},
  {0.138196601125011,  0.138196601125011, 0.585410196624969}};

//  {1./6., 1./6., 1./6.}, {2./3., 1./6., 1./6.},
//  {1./6., 2./3., 1./6.}, {1./6., 1./6., 2./3.}};

template <class T>
T TetGaussian2<T>::GaussianWeights[] = {.25, .25, .25, .25};


/// Class with weights and coordinates for 3rd order Gaussian integration
template <class T>
class TetGaussian3
{
public:
  static int GaussianNum;
  static T GaussianPoints[11][3];
  static T GaussianWeights[11];
};

template <class T>
T TetGaussian3<T>::GaussianPoints[][3] = {
  {0.2500000,  0.2500000, 0.2500000},
  {0.7857143, 0.07142857, 0.07142857},
  {0.07142857, 0.7857143, 0.07142857},
  {0.07142857, 0.07142857, 0.7857143},
  {0.07142857, 0.07142857, 0.07142857},
  {0.1005964, 0.1005964, 0.3994034},
  {0.1005964, 0.3994034, 0.1005964},
  {0.1005964, 0.3994034, 0.3994034},
  {0.3994034, 0.1005964, 0.1005964},
  {0.3994034, 0.1005964, 0.3994034},
  {0.3994034, 0.3994034, 0.1005964}};

template <class T>
T TetGaussian3<T>::GaussianWeights[] = {
  -0.01315556,
  0.007622222,
  0.007622222,
  0.007622222,
  0.007622222,
  0.02488889,
  0.02488889,
  0.02488889,
  0.02488889,
  0.02488889,
  0.02488889};

template <class T>
int TetGaussian3<T>::GaussianNum = 11;

/// Class for handling of element of type tetrahedron with
/// linear lagrangian interpolation
template <class T>
class TetLinearLgn :
         public BasisSimple<T>,
         public TetApprox,
		     public TetGaussian1<double>,
         public TetSamplingSchemes,
		     public TetLinearLgnUnitElement ,
         public TetElementWeights
{
public:
  typedef T value_type;

  TetLinearLgn() {}
  virtual ~TetLinearLgn() {}

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
    double w[4];
    get_linear_weights(coords, w);

    return (T)(w[0] * cd.node0() +
	       w[1] * cd.node1() +
	       w[2] * cd.node2() +
	       w[3] * cd.node3());
  }

  /// get first derivative at parametric coordinate
  template <class ElemData,class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1& /*coords*/, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    derivs.resize(3);

    derivs[0] = static_cast<typename VECTOR2::value_type>(-1. * cd.node0() + cd.node1());
    derivs[1] = static_cast<typename VECTOR2::value_type>(-1. * cd.node0() + cd.node2());
    derivs[2] = static_cast<typename VECTOR2::value_type>(-1. * cd.node0() + cd.node3());
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    TetLocate< TetLinearLgn<T> > CL;
    return CL.get_coords(this, coords, value, cd);
  }

  /// get arc length for edge
  template <class ElemData>
  double get_arc_length(const unsigned edge, const ElemData &cd) const
  {
    return get_arc3d_length<CrvGaussian1<double> >(this, edge, cd);
  }

  /// get area
  template <class ElemData>
    double get_area(const unsigned face, const ElemData &cd) const
  {
    return get_area3<TriGaussian2<double> >(this, face, cd);
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
TetLinearLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("TetLinearLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}




}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::TetLinearLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("TetLinearLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int TETLINEARLGN_VERSION = 1;
template <class T>
void
  Core::Basis::TetLinearLgn<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    TETLINEARLGN_VERSION);
  stream.end_class();
}
}

#endif

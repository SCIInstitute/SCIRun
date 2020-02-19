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


#ifndef CORE_BASIS_HEXTRILINEARLGN_H
#define CORE_BASIS_HEXTRILINEARLGN_H 1

#include <float.h>

#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Basis/HexElementWeights.h>
#include <Core/Basis/HexSamplingSchemes.h>

#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of HexTrilinearLgn
class SCISHARE HexTrilinearLgnUnitElement {
public:
  /// Parametric coordinates of vertices of unit edge
  static double unit_vertices[8][3];
  /// References to vertices of unit edge
  static int unit_edges[12][2];
  /// References to vertices of unit face
  static int unit_faces[6][4];
  /// References to normals of unit face
  static double unit_face_normals[6][3];
  /// Parametric coordinate used for the center
  static double unit_center[3];

  HexTrilinearLgnUnitElement();
  virtual ~HexTrilinearLgnUnitElement();

  /// return dimension of domain
  static int domain_dimension()
    { return 3; }

  /// return size of the domain
  static double domain_size()
    { return 1.0; }

  /// return number of vertices
  static int number_of_vertices()
    { return 8; }

  /// return number of vertices
  static int number_of_mesh_vertices()
    { return 8; }

  /// return degrees of freedom
  static int dofs()
    { return 8; }

  /// return number of edges
  static int number_of_edges()
    { return 12; }

  /// return number of vertices per face
  static int vertices_of_face()
    { return 4; }

  /// return number of faces per cell
  static int faces_of_cell()
    { return 6; }

  static inline double length(int edge)
  {
    const double *v0 = unit_vertices[unit_edges[edge][0]];
    const double *v1 = unit_vertices[unit_edges[edge][1]];
    const double dx = v1[0] - v0[0];
    const double dy = v1[1] - v0[1];
    const double dz = v1[2] - v0[2];
    return sqrt(dx*dx+dy*dy+dz*dz);
  }
  static double area(int /*face*/) { return 1.; }
  static double volume() { return 1.; }
};


/// Class for creating geometrical approximations of Hex meshes
class HexApprox {
public:

  HexApprox() {}
  virtual ~HexApprox() {}

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

    const double *v0 = HexTrilinearLgnUnitElement::unit_vertices[HexTrilinearLgnUnitElement::unit_edges[edge][0]];
    const double *v1 = HexTrilinearLgnUnitElement::unit_vertices[HexTrilinearLgnUnitElement::unit_edges[edge][1]];

    const double &p1x = v0[0];
    const double &p1y = v0[1];
    const double &p1z = v0[2];
    const double dx = v1[0] - p1x;
    const double dy = v1[1] - p1y;
    const double dz = v1[2] - p1z;

    for(unsigned i = 0; i <= div_per_unit; i++) {
      typename VECTOR::value_type &tmp = coords[i];
      tmp.resize(3);
      const double d = (double)i / (double)div_per_unit;
      tmp[0] = static_cast<typename VECTOR2::value_type>(p1x + d * dx);
      tmp[1] = static_cast<typename VECTOR2::value_type>(p1y + d * dy);
      tmp[2] = static_cast<typename VECTOR2::value_type>(p1z + d * dz);
    }
  }

  /// return number of vertices per face
  virtual int get_approx_face_elements() const { return 4; }


  /// Approximate faces for element by piecewise linear elements
  /// return: coords gives parametric coordinates at the approximation point.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_face(const unsigned face,
                   const unsigned div_per_unit,
                   VECTOR &coords) const
  {
    typedef typename VECTOR::value_type VECTOR2;
    typedef typename VECTOR2::value_type VECTOR3;

    const double *v0 = HexTrilinearLgnUnitElement::unit_vertices[HexTrilinearLgnUnitElement::unit_faces[face][0]];
    const double *v1 = HexTrilinearLgnUnitElement::unit_vertices[HexTrilinearLgnUnitElement::unit_faces[face][1]];
    const double *v3 = HexTrilinearLgnUnitElement::unit_vertices[HexTrilinearLgnUnitElement::unit_faces[face][3]];
    const double d = 1. / (double)div_per_unit;
    coords.resize(div_per_unit);
    typename VECTOR::iterator citer = coords.begin();
    for(unsigned j = 0; j < div_per_unit; j++)
    {
      const double dj = (double)j / (double)div_per_unit;
      VECTOR2& jvec = *citer++;
      jvec.resize((div_per_unit + 1) * 2, VECTOR3(3, 0.0));
      typename VECTOR2::iterator e = jvec.begin();
      for(unsigned i=0; i <= div_per_unit; i++) {
        const double di = (double) i / (double)div_per_unit;
        VECTOR3 &c0 = *e++;
        typedef typename VECTOR3::value_type VECTOR4;
        c0[0] = static_cast<VECTOR4>(v0[0] + dj * (v3[0] - v0[0]) + di * (v1[0] - v0[0]));
        c0[1] = static_cast<VECTOR4>(v0[1] + dj * (v3[1] - v0[1]) + di * (v1[1] - v0[1]));
        c0[2] = static_cast<VECTOR4>(v0[2] + dj * (v3[2] - v0[2]) + di * (v1[2] - v0[2]));
        VECTOR3 &c1 = *e++;
        c1[0] = static_cast<VECTOR4>(v0[0] + (dj + d) * (v3[0] - v0[0]) + di * (v1[0] - v0[0]));
        c1[1] = static_cast<VECTOR4>(v0[1] + (dj + d) * (v3[1] - v0[1]) + di * (v1[1] - v0[1]));
        c1[2] = static_cast<VECTOR4>(v0[2] + (dj + d) * (v3[2] - v0[2]) + di * (v1[2] - v0[2]));
      }
    }
  }
};


/// Class for searching of parametric coordinates related to a
/// value in Hex meshes and fields
template <class ElemBasis>
class HexLocate : public Dim3Locate<ElemBasis> {
public:
  typedef typename ElemBasis::value_type T;

  HexLocate() {}
  virtual ~HexLocate() {}

  /// find value in interpolation for given value
  template <class ElemData, class VECTOR>
  bool get_coords(const ElemBasis *pEB, VECTOR &coords,
		  const T& value, const ElemData &cd) const
  {
    initial_guess(pEB, value, cd, coords);

    if (this->get_iterative(pEB, coords, value, cd))
      return check_coords(coords);
    else
      return false;
  }

  template <class VECTOR>
  inline bool check_coords(const VECTOR &x) const
  {
    if (x[0]>=-Dim3Locate<ElemBasis>::thresholdDist  &&
	x[0]<= Dim3Locate<ElemBasis>::thresholdDist1 &&
	x[1]>=-Dim3Locate<ElemBasis>::thresholdDist  &&
	x[1]<= Dim3Locate<ElemBasis>::thresholdDist1 &&
        x[2]>=-Dim3Locate<ElemBasis>::thresholdDist  &&
	x[2]<= Dim3Locate<ElemBasis>::thresholdDist1)
      return true;
    else
      return false;
  }

protected:
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
        for (int z = 1; z < end; z++)
        {
          coord[2] = z / (double) end;

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
    }
  }
};

/// Class with weights and coordinates for 1st order Gaussian integration
template <class T>
class HexGaussian1
{
public:
  static int GaussianNum;
  static T GaussianPoints[1][3];
  static T GaussianWeights[1];
};

#ifdef _WIN32
// force the instantiation of TetGaussian2<double>
template class HexGaussian1<double>;
#endif

template <class T>
int HexGaussian1<T>::GaussianNum = 1;

template <class T>
T HexGaussian1<T>::GaussianPoints[1][3] = {
  {0.5, 0.5, 0.5}};

template <class T>
T HexGaussian1<T>::GaussianWeights[1] =
  {1.0};

/// Class with weights and coordinates for 2nd order Gaussian integration
template <class T>
class HexGaussian2
{
public:
  static int GaussianNum;
  static T GaussianPoints[8][3];
  static T GaussianWeights[8];
};

#ifdef _WIN32
// force the instantiation of TetGaussian2<double>
template class HexGaussian2<double>;
#endif

template <class T>
int HexGaussian2<T>::GaussianNum = 8;

template <class T>
T HexGaussian2<T>::GaussianPoints[8][3] = {
  {0.211324865405, 0.211324865405, 0.211324865405},
  {0.788675134595, 0.211324865405, 0.211324865405},
  {0.788675134595, 0.788675134595, 0.211324865405},
  {0.211324865405, 0.788675134595, 0.211324865405},
  {0.211324865405, 0.211324865405, 0.788675134595},
  {0.788675134595, 0.211324865405, 0.788675134595},
  {0.788675134595, 0.788675134595, 0.788675134595},
  {0.211324865405, 0.788675134595, 0.788675134595}};

template <class T>
T HexGaussian2<T>::GaussianWeights[8] =
  {.125, .125, .125, .125, .125, .125, .125, .125};

/// Class with weights and coordinates for 3rd order Gaussian integration
template <class T>
class HexGaussian3
{
public:
  static int GaussianNum;
  static T GaussianPoints[27][3];
  static T GaussianWeights[27];
};

template <class T>
int HexGaussian3<T>::GaussianNum = 27;

template <class T>
T HexGaussian3<T>::GaussianPoints[27][3] =
  {
    {0.11270166537950, 0.11270166537950, 0.11270166537950}, {0.5, 0.11270166537950, 0.11270166537950}, {0.88729833462050, 0.11270166537950, 0.11270166537950},
    {0.11270166537950, 0.5, 0.11270166537950}, {0.5, 0.5, 0.11270166537950}, {0.88729833462050, 0.5, 0.11270166537950},
    {0.11270166537950, 0.88729833462050, 0.11270166537950}, {0.5, 0.88729833462050, 0.11270166537950}, {0.88729833462050, 0.88729833462050, 0.11270166537950},

    {0.11270166537950, 0.11270166537950, 0.5}, {0.5, 0.11270166537950, 0.5}, {0.88729833462050, 0.11270166537950, 0.5},
    {0.11270166537950, 0.5, 0.5}, {0.5, 0.5, 0.5}, {0.88729833462050, 0.5, 0.5},
    {0.11270166537950, 0.88729833462050, 0.5}, {0.5, 0.88729833462050, 0.5}, {0.88729833462050, 0.88729833462050, 0.5},

    {0.11270166537950, 0.11270166537950, 0.88729833462050}, {0.5, 0.11270166537950, 0.88729833462050}, {0.88729833462050, 0.11270166537950, 0.88729833462050},
    {0.11270166537950, 0.5, 0.88729833462050}, {0.5, 0.5, 0.88729833462050}, {0.88729833462050, 0.5, 0.88729833462050},
    {0.11270166537950, 0.88729833462050, 0.88729833462050}, {0.5, 0.88729833462050, 0.88729833462050}, {0.88729833462050, 0.88729833462050, 0.88729833462050}
  };

template <class T>
T HexGaussian3<T>::GaussianWeights[27] =
  {
    0.03429355278944,   0.05486968447298,   0.03429355278944,
    0.05486968447298,   0.08779149517257,   0.05486968447298,
    0.03429355278944,   0.05486968447298,   0.03429355278944,

    0.03429355278944,   0.05486968447298,   0.03429355278944,
    0.05486968447298,   0.08779149517257,   0.05486968447298,
    0.03429355278944,   0.05486968447298,   0.03429355278944,

    0.03429355278944,   0.05486968447298,   0.03429355278944,
    0.05486968447298,   0.08779149517257,   0.05486968447298,
    0.03429355278944,   0.05486968447298,   0.03429355278944
  };


/// Class for handling of element of type hexahedron with
/// trilinear lagrangian interpolation
template <class T>
class HexTrilinearLgn :
      public BasisSimple<T>,
      public HexApprox,
			public HexGaussian2<double>,
			public HexSamplingSchemes,
			public HexTrilinearLgnUnitElement,
      public HexElementWeights
{
public:
  typedef T value_type;

  inline HexTrilinearLgn() {}
  inline virtual ~HexTrilinearLgn() {}

  static int polynomial_order() { return 1; }

  /// get weight factors at parametric coordinate
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
    double w[8];
    get_linear_weights(coords, w);

    return (T)(
      w[0] * cd.node0() +
      w[1] * cd.node1() +
      w[2] * cd.node2() +
      w[3] * cd.node3() +
      w[4] * cd.node4() +
      w[5] * cd.node5() +
      w[6] * cd.node6() +
      w[7] * cd.node7());
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    double w[24];
    get_linear_derivate_weights(coords, w);

    derivs.resize(3);
    derivs[0] = static_cast<typename VECTOR2::value_type>(
      w[0] * cd.node0() +
      w[1] * cd.node1() +
      w[2] * cd.node2() +
      w[3] * cd.node3() +
      w[4] * cd.node4() +
      w[5] * cd.node5() +
      w[6] * cd.node6() +
      w[7] * cd.node7());

    derivs[1] = static_cast<typename VECTOR2::value_type>(
      w[8] * cd.node0() +
      w[9] * cd.node1() +
      w[10] * cd.node2() +
      w[11] * cd.node3() +
      w[12] * cd.node4() +
      w[13] * cd.node5() +
      w[14] * cd.node6() +
      w[15] * cd.node7());

    derivs[2] = static_cast<typename VECTOR2::value_type>(
      w[16] * cd.node0() +
      w[17] * cd.node1() +
      w[18] * cd.node2() +
      w[19] * cd.node3() +
      w[20] * cd.node4() +
      w[21] * cd.node5() +
      w[22] * cd.node6() +
      w[23] * cd.node7());
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    HexLocate< HexTrilinearLgn<T> > CL;
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
HexTrilinearLgn<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("HexTrilinearLgn");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}

}}
const int HEX_TRILINEAR_LGN_VERSION = 1;

template <class T>
const TypeDescription*
  get_type_description(Core::Basis::HexTrilinearLgn<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("HexTrilinearLgn", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

  template <class T>
  void
  Core::Basis::HexTrilinearLgn<T>::io(Piostream &stream)
  {
    stream.begin_class(get_type_description(this)->get_name(),
                       HEX_TRILINEAR_LGN_VERSION);
    stream.end_class();
  }
}

#endif

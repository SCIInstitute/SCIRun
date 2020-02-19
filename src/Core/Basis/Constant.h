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
   Date:            August 23 2004
*/


#ifndef CORE_BASIS_CONSTANT_H
#define CORE_BASIS_CONSTANT_H 1

#include <Core/Basis/Basis.h>
#include <Core/Basis/NoElementWeights.h>
#include <Core/Basis/PntSamplingSchemes.h>
#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Persistent/Persistent.h>
#include <Core/Datatypes/Legacy/Base/TypeName.h>
#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of ConstantBasis
class SCISHARE ConstantBasisUnitElement {
public:
  static double unit_vertices[1][1]; ///< Parametric coordinates of vertices
  static int unit_edges[1][1];    ///< References to vertices of unit edge
  static double unit_center[3];

  ConstantBasisUnitElement() {}
  virtual ~ConstantBasisUnitElement() {}

  /// return dimension of domain
  static int domain_dimension()
    { return 0; }

  /// return size of domain
  static double domain_size()
    { return 0.0; }

  /// return number of vertices
  static int number_of_vertices()
    { return 0; }

  /// return number of vertices
  static int number_of_mesh_vertices()
    { return 0; }

  /// return number of edges
  static int number_of_edges()
    { return 0; }

  /// return degrees of freedom
  static int dofs()
    { return 1; }

  /// return number of vertices per face
  static int vertices_of_face()
    { return 0; }

  /// return number of faces per cell
  static int faces_of_cell()
    { return 0; }

  /// return volume
  static double volume()
    { return 0.; }

};


class ConstantApprox {
public:
  ConstantApprox() {}
  virtual ~ConstantApprox() {}

  /// Approximate edge for element by piecewise linear segments
  /// return: coords gives parametric coordinates of the approximation.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_edge(const unsigned edge,
                   const unsigned div_per_unit,
                   VECTOR& coords) const
  {
    coords.resize(0);
  }

  /// Approximate faces for element by piecewise linear elements
  /// return: coords gives parametric coordinates at the approximation point.
  /// Use interpolate with coordinates to get the world coordinates.
  template<class VECTOR>
  void approx_face(const unsigned face,
                   const unsigned div_per_unit,
                   VECTOR &coords) const
  {
    coords.resize(0);
  }

};


/// Class for handling of element with constant field variables
template <class T>
  class ConstantBasis : public BasisSimple<T>,
              public ConstantBasisUnitElement,
              public ConstantApprox,
              public NoElementWeights,
              public PntSamplingSchemes
{
public:
  typedef T value_type;

  ConstantBasis() {}
  virtual ~ConstantBasis() {}


  static int polynomial_order() { return 0; }

  /// get value at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR &, const ElemData &cd) const
  {
    return cd.elem();
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &,
		VECTOR2 &derivs) const
  {
    typename VECTOR1::size_type s=coords.size();
    derivs.resize(s);
    for(typename VECTOR1::size_type si=0; si<s; si++)
      derivs[si] = static_cast<typename VECTOR2::value_type>(0);
  }

  /// Get the weights for doing an interpolation
  template <class VECTOR>
  void get_weights(const VECTOR &coords, double *w) const
  {
    w[0]=1;
  }

  /// Get the weights for doing a gradient
  template <class VECTOR>
  void get_derivate_weights(const VECTOR &coords, double *w) const
  {
    std::vector<double>::size_type s=coords.size();
    for(std::vector<double>::size_type si=0; si<s; si++)
      w[si] = 0;
  }

  /// Functions for dynamic compilation and storing this object on disk
  static  const std::string type_name(int n = -1);
  virtual void io (Piostream& str);
};

template <class T>
const std::string
ConstantBasis<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("ConstantBasis");
    return nm;
  }
  else
  {
    return find_type_name((T *)0);
  }
}


#define CONSTANTBASIS_VERSION 1

}}

template <class T>
const TypeDescription* get_type_description(Core::Basis::ConstantBasis<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("ConstantBasis", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

  template <class T>
  void
  Core::Basis::ConstantBasis<T>::io(Piostream &stream)
  {
    stream.begin_class(get_type_description(this)->get_name(),
                       CONSTANTBASIS_VERSION);
    stream.end_class();
  }
}

#endif

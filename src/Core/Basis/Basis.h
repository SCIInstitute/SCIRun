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

   Author:          Frank B. Sachse
   Date:            October 21 2005
*/


#ifndef CORE_BASIS_BASIS_H
#define CORE_BASIS_BASIS_H 1

#include <vector>
#include <cfloat>
#include <boost/array.hpp>
#include <Core/Utils/Legacy/Assert.h>
#include <Core/Utils/Exception.h>
#include <Core/Persistent/Persistent.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing interfaces to basis elements
template <class T>
class BasisSimple
{
public:
  BasisSimple() {}
  virtual ~BasisSimple() {}

  /// @todo: exceptions being used to avoid virtual inheritance?
  //
  /// @todo: SCIRun 5 porting note: ASSERTFAIL replaced by
  // REPORT_NOT_IMPLEMENTED (should not need this in production code)


  /// get value at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR& /*coords*/, const ElemData &) const
  {
    REPORT_NOT_IMPLEMENTED("interpolate not supported by basis");
  }

  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1& /*coords*/, const ElemData &,
		VECTOR2& /*derivs*/) const
  {
    REPORT_NOT_IMPLEMENTED("derivate not supported by basis");
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR& /*coords*/, const T& /*value*/,
		  const ElemData &) const
  {
    REPORT_NOT_IMPLEMENTED("get_coords not supported by basis");
  }

  /// get arc length for edge of element
  template <class ElemData>
  double get_arc_length(const unsigned /*edge*/, const ElemData&) const
  {
    REPORT_NOT_IMPLEMENTED("get_arc_length not supported by basis");
  }

  /// get area for face of element
  template <class ElemData>
  double get_area(const unsigned /*face*/, const ElemData&) const
  {
    REPORT_NOT_IMPLEMENTED("get_area not supported by basis");
  }

  /// get volume for element
  template <class ElemData>
  double get_volume(const ElemData &) const
  {
    REPORT_NOT_IMPLEMENTED("get_volume not supported by basis");
  }

  /// add a node value
  void add_node_value(const T &)
  {
    REPORT_NOT_IMPLEMENTED("add_node_value not supported by basis");
  }

  void set_node_value(const T &, unsigned int)
  {
    REPORT_NOT_IMPLEMENTED("set_node_value not supported by basis");
  }

  void get_node_value(T &, unsigned int) const
  {
    REPORT_NOT_IMPLEMENTED("get_node_value not supported by basis");
  }

  void resize_node_values(size_t)
  {
  }

  /// return number of additional nodes
  inline size_t size_node_values()
  {
    return (0);
  }

  /// add a derivative value
  void add_derivatives(const std::vector<T> &)
  {
    REPORT_NOT_IMPLEMENTED("add_derivative not supported by basis");
  }

  /// add scale factors
  void add_scalefactors(const std::vector<T> &)
  {
    REPORT_NOT_IMPLEMENTED("add_scalefactors not supported by basis");
  }

  /// return number of additional derivatives
  inline int size_derivatives()
  {
    return 0;
  }

  inline std::vector<T>& get_nodes()
  {
    return (nodes_);
  }

  inline std::vector<std::vector<T> >& get_derivs()
  {
    return (derivs_);
  }

protected:
  std::vector<T>                    nodes_;
  std::vector<std::vector<T> >      derivs_;
  std::vector<std::vector<double> > scalefactors_;
  std::vector<double>               scalefactorse_;

};


/// Class for describing interfaces to basis elements with additional nodes
template <class T>
class BasisAddNodes : public BasisSimple<T>
{
public:
  BasisAddNodes() {}
  virtual ~BasisAddNodes() {}

  /// add a node value corresponding to edge
  inline void add_node_value(const T &p) { BasisSimple<T>::nodes_.push_back(p); }

  /// get and set node value
  template<class INDEX>
  inline void set_node_value(const T &p, INDEX i) { BasisSimple<T>::nodes_[i] = p; }

  template<class INDEX>
  inline void get_node_value(T &p, INDEX i) const { p = BasisSimple<T>::nodes_[i]; }

  /// set the proper size
  template<class SIZE>
  inline void resize_node_values(SIZE s) { BasisSimple<T>::nodes_.resize(s); }

  /// return node value
  template<class INDEX>
  inline T& node_values(INDEX i) { return &BasisSimple<T>::nodes_[i]; }

  inline size_t size_node_values() { return BasisSimple<T>::nodes_.size(); }
};


/// Class for describing interfaces to basis elements with
/// additional derivatives
template <class T>
class BasisAddDerivatives : public BasisSimple<T>
{
public:
  BasisAddDerivatives() {}
  virtual ~BasisAddDerivatives() {}

  /// add derivative values (dx, dy, dxy) for nodes.
  inline void add_derivatives(const std::vector<T> &p)
  {
    BasisSimple<T>::derivs_.push_back(p);
  }

  /// get and set node value
  inline void set_deriv_value(const T &p, unsigned int i,unsigned int j) { BasisSimple<T>::derivs_[i][j] = p; }
  inline void get_deriv_value(T &p, unsigned int i,unsigned int j) const { p = BasisSimple<T>::derivs_[i][j]; }

  /// set the proper size
  inline void resize_deriv_values(size_t s) { BasisSimple<T>::derivs_.resize(s); }

  /// return number of additional derivatives
  inline int size_derivatives() { return BasisSimple<T>::derivs_.size(); }

};

/// Class for describing interfaces to basis elements with
/// additional derivatives and scale factors at nodes.
template <class T>
class BasisAddDerivativesScaleFactors : public BasisAddDerivatives<T>
{
public:
  BasisAddDerivativesScaleFactors() {}
  virtual ~BasisAddDerivativesScaleFactors() {}

  /// add scale factors (sdx, sdy) for nodes.
  inline void add_scalefactors(const std::vector<double> &p)
  {
    BasisSimple<T>::scalefactors_.push_back(p);
  }

  /// return number of additional derivatives
  inline int size_scalefactors() { return BasisSimple<T>::scalefactors_.size(); }

};

/// Class for describing interfaces to basis elements with
/// additional derivatives and scale factors at edges
template <class T>
class BasisAddDerivativesScaleFactorsEdges : public BasisAddDerivatives<T>
{
public:
  BasisAddDerivativesScaleFactorsEdges() {}
  virtual ~BasisAddDerivativesScaleFactorsEdges() {}

  /// add scale factors (sdx, sdy) for nodes.
  inline void add_scalefactors(const std::vector<double> &p)
  { BasisSimple<T>::scalefactorse_.push_back(p[0]); }

  /// return number of additional derivatives
  inline int size_scalefactors() { return BasisSimple<T>::scalefactorse_.size(); }

};

}}}

#endif

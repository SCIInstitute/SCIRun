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
*/


///
///@class LinearPWI
///@brief Linear piecewise interpolation templates
///
///@author
///       Alexei Samsonov
///       Department of Computer Science
///       University of Utah
///
///@date  July 2000
///


#ifndef SCI_LINEARPWI_H__
#define SCI_LINEARPWI_H__

#include <Core/Math/PiecewiseInterp.h>

#include <Core/Containers/Array1.h>
#include <Core/GeometryPrimitives/Point.h>

namespace SCIRun {


typedef struct Pair{
  double a;
  double b;
} PAIR;


class LinearPWI: public PiecewiseInterp<double> {
public:
	LinearPWI();
	LinearPWI(const Array1<double>&, const Array1<double>&);

	bool set_data(const Array1<double>&, const Array1<double>&);
	inline bool get_value(double, double&);

private:
	Array1<PAIR> p;
};

inline bool LinearPWI::get_value(double w, double& res){
  int interv;
  if (data_valid && (interv=get_interval(w))>=0){
    res=p[interv].a+p[interv].b*w;
    return true;
  }
  else
    return false;
}

template <class T> class Linear3DPWI: public PiecewiseInterp<T> {
public:
  Linear3DPWI() {}
  Linear3DPWI(const Array1<double>&, const Array1<T>&);

  bool set_data(const Array1<double>&, const Array1<T>&);
  inline bool get_value(double, T&);

private:
  Array1<PAIR> X;
  Array1<PAIR> Y;
  Array1<PAIR> Z;
};


template <class T> Linear3DPWI<T>::Linear3DPWI(const Array1<double>& pts, const Array1<T>& vals)
{
  set_data(pts, vals);
}

template <class T> inline bool Linear3DPWI<T>::get_value(double w, T& res)
{
  int i = 0;
  if (this->data_valid && (i = this->get_interval(w)) >= 0) {
    res = T(X[i].a + X[i].b * w, Y[i].a+Y[i].b * w, Z[i].a + Z[i].b * w);
    return true;
  }
  else
    return false;
}

// takes sorted array of points
template <class T> bool Linear3DPWI<T>::set_data(const Array1<double>& pts, const Array1<T>& vals)
{
  int sz = 0;
  this->reset();
  if (this->fill_data(pts) && (sz = this->points.size()) > 1) {
    X.resize(sz);
    Y.resize(sz);
    Z.resize(sz);

    double lb, rb, delta;

    for (int i=0; i<this->points.size()-1; i++){
      lb = this->points[i];
      rb = this->points[i+1];
      delta = rb - lb;
      X[i].a = (vals[i].x() * rb-vals[i+1].x() * lb) / delta;
      X[i].b = (vals[i+1].x() - vals[i].x()) / delta;
      Y[i].a = (vals[i].y() * rb-vals[i+1].y() * lb) / delta;
      Y[i].b = (vals[i+1].y() - vals[i].y()) / delta;
      Z[i].a = (vals[i].z() * rb-vals[i+1].z() * lb) / delta;
      Z[i].b = (vals[i+1].z() - vals[i].z()) / delta;
    }
    return this->data_valid = true;
  }
  else{
    this->reset();
    return this->data_valid = false;
  }
}

} // End namespace SCIRun

#endif //SCI_LINEARPWI_H__

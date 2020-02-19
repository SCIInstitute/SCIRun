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
///@class CubicPWI
///@brief Cubic piecewise interpolation
///
///@author
///       Alexei Samsonov
///       Department of Computer Science
///       University of Utah
///
///@date  July 2000
///

#include <Core/Utils/Legacy/Assert.h>
#include <Core/Math/CubicPWI.h>
#include <Core/Containers/Array1.h>
//

namespace SCIRun {


CubicPWI::CubicPWI()
{
}

CubicPWI::CubicPWI(const Array1<double>& pts, const Array1<double>& vals) {
  set_data(pts, vals);
}

// function calculates tangents at pts[i] that is adhere to C2 continuity of splines in pts[i]
// in case of clamped ends, the function should get end tangents as two last elements of vals-array
bool set_tangents(const Array1<double>& pts, const Array1<double>& vals, Array1<double>& r, EndCondition end_conds)
{

  int psz = pts.size();
  int vsz = vals.size();

  if (end_conds == clamped_ends) {
    ASSERT(psz == (vsz - 2) && psz >= 2);
  }
  else
    ASSERT(psz == vsz && psz >= 2);

  // tridiagonal system
  r.resize(psz);

  Array1<double> lrow(psz), diag(psz), hrow(psz);

  // building A  and r; A is diagonally dominant and trilinear - could be solved
  // by simple forward and back substitution sequence
  int hind = psz - 1;
  Array1<double> dx(hind);
  Array1<double> dv(hind);

  for (int i = 0; i < hind; i++) {
    dx[i] = pts[i+1] - pts[i];
    dv[i] = vals[i+1] - vals[i];
  }

  for (int i=1; i < hind; i++) {
    hrow[i] = dx[i-1];
    lrow[i] = dx[i];
    diag[i] = 2 * (dx[i-1] + dx[i]);
    r[i] = 3 * (dx[i] * dv[i-1] / dx[i-1] + dx[i-1] * dv[i] / dx[i]);
  }

  // filling in r[0] and r[hind] and adjusting A to our end conditions
  diag[0] = diag[hind] = 1;

  switch (end_conds) {
    case natural_ends:
      r[0]=3 * dv[0] / dx[0];
      r[hind] = 3 * dv[hind-1] / dx[hind-1];
      diag[0] = diag[hind] = 2;
      hrow[0] = 1;
      lrow[hind] = 1;
      break;

    case clamped_ends:
      r[0] = vals[vsz-2];
      r[hind] = vals[vsz-1];
      break;

    case bessel_ends:
    {
      r[0] = -2 * pts[0] * (2 * dx[0] + dx[1]) / (dx[0] * diag[1]) +
               diag[1] * pts[1] / (2 * dx[0] * dx[1]) -
               2 * dx[0] * pts[2] / (dx[1] * diag[1]);
      int h1 = hind - 1;
      int h2 = hind - 2;
      r[hind] = 2 * dx[h1] * pts[h2] / (dx[h2] * diag[h1]) -
                  diag[h1] * pts[h1] / (2 * dx[h1] * dx[h2]) +
                  2 * (2 * dx[h1] + dx[h2]) * pts[hind] / (diag[h1] * dx[h1]);
    }
      break;

    case quadratic_ends:
      r[0] = 2 * dv[0] / dx[0];
      r[hind] = 2 * dv[hind-1] / dx[hind-1];
      hrow[0] = lrow[hind] = 1;
    default:
      // unknown end condition
      return false;
  }

  std::cout << "Low-diag before solving: " << lrow << std::endl;
  std::cout << "Diag-diag before solving: " << diag << std::endl;
  std::cout << "High-diag before solving: " << hrow << std::endl;
  std::cout << "Res before solving : " << r << std::endl;

  // ---------------------------------------------------------------------------
  // solving the tridiagonal system:
  for(int i = 1;i < psz; i++) {
    double factor = lrow[i] / diag[i-1];
    diag[i] -= factor * hrow[i-1];
    r[i] -= factor*r[i-1];
  }

  r[psz-1] = r[psz-1] / diag[psz-1];
  for(int i = psz - 2; i >= 0; i--) {
    r[i] = (r[i] - hrow[i] * r[i+1]) / diag[i];
  }
  std::cout << "Low-diag after solving: " << lrow << std::endl;
  std::cout << "Diag-diag after solving: " << diag << std::endl;
  std::cout << "High-diag after solving: " << hrow << std::endl;
  std::cout << "Res after solving : " << r << std::endl;

  return true;
}

// takes sorted array of points
bool CubicPWI::set_data(const Array1<double>& pts, const Array1<double>& vals)
{
  std::cout << "About to fill in data in set_data (1D)!!!" << std::endl;
  std::cout << "pts-array:" << std::endl;
  std::cout << pts;

  std::cout << "vals-array:" << std::endl;
  std::cout << pts;

  reset();
  int sz = 0;

  if (fill_data(pts) && (sz = points.size()) > 1 && sz == vals.size()) {
    std::cout << "Inside set_data!!!" << std::endl;

    p.resize(sz);
    Array1<double> ders;
    if (set_tangents(points, vals, ders, natural_ends)){

      data_valid = true;
      double a, b, c, d, delta;

      for (int i = 0; i < sz-1; i++)
        if ( (delta = points[i+1] - points[i]) > 10e-9) {
          a = vals[i];
          b = ders[i];
          c = (3 * (vals[i+1] - vals[i]) / delta-2 * ders[i] - ders[i+1]) / delta;
          d = (ders[i] + ders[i+1] - 2 * (vals[i+1] - vals[i]) / delta) / (delta * delta);

          p[i].a = a;
          p[i].b = b;
          p[i].c = c;
          p[i].d = d;
          std::cout << "Interval: " << points[i] << ", " << points[i+1] << std::endl;
          std::cout << "Coeff. are: " << p[i].a << std::endl << p[i].b << std::endl << p[i].c << std::endl << p[i].d << std::endl;
        }
        else {
          reset();
          break;
        }
    }
  }

  return data_valid;
}

} // End namespace SCIRun

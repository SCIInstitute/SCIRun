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


/// @todo Documentation Core/Basis/PrismSamplingSchemes.h
#ifndef CORE_BASIS_PRISMSAMPLINGSCHEMES_H
#define CORE_BASIS_PRISMSAMPLINGSCHEMES_H 1

#include <vector>
#include <Core/Utils/Legacy/Assert.h>

#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

class SCISHARE PrismSamplingSchemes
{
  public:

  template <class ARRAY1, class ARRAY2>
  void get_gaussian_scheme(ARRAY1& coords, ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;
    if (order == 1 || order == 0)
    {
      const double gaussian_weights[1] = {1.0};
      const double gaussian_coords[1][3] = {{1./3.,1./3., 0.5}};
      const unsigned int num_coords = 3;
      const unsigned int num_points = 1;

      coords.resize(num_points);
      weights.resize(num_points);
      for (unsigned int i=0; i<num_points; i++)
      {
        coords[i].resize(num_coords);
        for (unsigned int j=0; j<num_coords; j++)
          coords[i][j] = static_cast<typename coords_type::value_type>(gaussian_coords[i][j]);
        weights[i] = static_cast<typename ARRAY2::value_type>(gaussian_weights[i]);
      }
    }
    else if (order == 2)
    {
      const double gaussian_weights[6] = {1./6., 1./6., 1./6.,
                                          1./6., 1./6., 1./6.};
      const double gaussian_coords[6][3] = {
        {1./6.,1./6., 0.211324865405}, {2./3.,1./6., 0.211324865405},
        {1./6.,2./3., 0.211324865405}, {1./6.,1./6., 0.788675134595},
        {2./3.,1./6., 0.788675134595}, {1./6.,2./3., 0.788675134595}};
      const unsigned int num_coords = 3;
      const unsigned int num_points = 6;

      coords.resize(num_points);
      weights.resize(num_points);
      for (unsigned int i=0; i<num_points; i++)
      {
        coords[i].resize(num_coords);
        for (unsigned int j=0; j<num_coords; j++)
          coords[i][j] = static_cast<typename coords_type::value_type>(gaussian_coords[i][j]);
        weights[i] = static_cast<typename ARRAY2::value_type>(gaussian_weights[i]);
      }
    }
    else if (order == 3)
    {
      REPORT_NOT_IMPLEMENTED("Quadrature scheme 3 has not been implemented");
    }
    else
    {
      REPORT_NOT_IMPLEMENTED("Only Gaussian scheme 1, and 2 are implemented");
    }
  }

  template <class ARRAY1, class ARRAY2>
  void get_regular_scheme(ARRAY1& coords, ARRAY2& weights, int /*order*/)
  {
    coords.resize(0);
    weights.resize(0);
  }
};

}}}

#endif

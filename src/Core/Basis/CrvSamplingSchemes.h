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


#ifndef CORE_BASIS_CRVSAMPLINGSCHEMES_H
#define CORE_BASIS_CRVSAMPLINGSCHEMES_H 1

#include <Core/Utils/Legacy/Assert.h>
#include <vector>

#include <Core/Basis/share.h>
#include <Core/Utils/Exception.h>

namespace SCIRun {
namespace Core {
namespace Basis {


class SCISHARE CrvSamplingSchemes
{
  public:

  template <class ARRAY1, class ARRAY2>
  void get_gaussian_scheme(ARRAY1& coords, ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;
    if (order == 1 || order == 0)
    {
      const double gaussian_weights[1] = { 1.0};
      const double gaussian_coords[1][1] = { {.5}};
      const unsigned int num_coords = 1;
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
      const double gaussian_weights[2] = {.5, .5};
      const double gaussian_coords[2][1] = {{0.211324865405}, {0.788675134595}};
      const unsigned int num_coords = 1;
      const unsigned int num_points = 2;

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
      const double gaussian_weights[3] =   {.2777777777, .4444444444, .2777777777};
      const double gaussian_coords[3][1] = {{0.11270166537950}, {0.5}, {0.88729833462050}};
      const unsigned int num_coords = 1;
      const unsigned int num_points = 3;

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
    else
    {
      REPORT_NOT_IMPLEMENTED("Only Gaussian scheme 1, 2, and 3 are implemented");
    }
  }

  template <class ARRAY1, class ARRAY2>
  void get_regular_scheme(ARRAY1& coords, ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;

    coords.resize(order);
    weights.resize(order);
    for (int i=0; i< order; i++)
    {
      coords[i].resize(1);
      coords[i][0] = static_cast<typename coords_type::value_type>(static_cast<double>(i+1)/static_cast<double>(order+1));
      weights[i] = static_cast<typename ARRAY2::value_type>(1.0/static_cast<double>(order));
    }
  }
};

}}}

#endif

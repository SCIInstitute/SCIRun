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


/// @todo Documentation Core/Basis/TriSamplingSchemes.h

#ifndef CORE_BASIS_TRISAMPLINGSCHEMES_H
#define CORE_BASIS_TRISAMPLINGSCHEMES_H 1

#include <vector>
#include <Core/Utils/Legacy/Assert.h>

#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

class SCISHARE TriSamplingSchemes
{
  public:

  template <class ARRAY1, class ARRAY2>
  void get_gaussian_scheme(ARRAY1& coords, ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;

    if (order == 1 || order == 0)
    {
      const double gaussian_weights[1] = { 1.0};
      const double gaussian_coords[1][2] = {{1./3.,1./3.}};
      const unsigned int num_coords = 2;
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
      const double gaussian_weights[3] =
        {1./3., 1./3., 1./3.};
      const double gaussian_coords[3][2] =
        {{1./6.,1./6.}, {2./3.,1./6.}, {1./6.,2./3.}};
      const unsigned int num_coords = 2;
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
    else if (order == 3)
    {
      const double gaussian_weights[7] =
        { 0.1259391805, 0.1259391805, 0.1259391805, 0.1323941527,
          0.1323941527, 0.1323941527, 0.225};
      const double gaussian_coords[7][2] = {
        {0.1012865073, 0.1012865073}, {0.7974269853, 0.1012865073},
        {0.1012865073, 0.7974269853}, {0.4701420641, 0.0597158717},
        {0.4701420641, 0.4701420641}, {0.0597158717, 0.4701420641},
        {0.3333333333, 0.3333333333}};
      const unsigned int num_coords = 2;
      const unsigned int num_points = 7;

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
  void get_regular_scheme(ARRAY1& coords,ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;

    int m = 0;
    for (int j=0; j<order;j++) m += (2*j+1);
    coords.resize(m);

    weights.resize(m);
    for (int p=0; p<m;p++) weights[p] = static_cast<typename ARRAY2::value_type>(1.0/static_cast<double>(m));

    int k= 0;
    for (int p=0;p<order;p++)
    {
      int r=0;
      for (; r<p; r++)
      {
        coords[k].resize(2);
        coords[k][1] = static_cast<typename coords_type::value_type>((static_cast<double>(order-1-p)+1.0/3.0)/static_cast<double>(order));
        coords[k][0] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/3.0)/static_cast<double>(order));
        k++;
        coords[k].resize(2);
        coords[k][1] = static_cast<typename coords_type::value_type>((static_cast<double>(order-1-p)+2.0/3.0)/static_cast<double>(order));
        coords[k][0] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+2.0/3.0)/static_cast<double>(order));
        k++;
      }
      coords[k].resize(2);
      coords[k][1] = static_cast<typename coords_type::value_type>((static_cast<double>(order-1-p)+1.0/3.0)/static_cast<double>(order));
      coords[k][0] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/3.0)/static_cast<double>(order));
      k++;
    }
  }
};

}}}

#endif

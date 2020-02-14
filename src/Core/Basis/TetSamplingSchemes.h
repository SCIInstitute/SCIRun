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


#ifndef CORE_BASIS_TETSAMPLINGSCHEMES_H
#define CORE_BASIS_TETSAMPLINGSCHEMES_H 1

#include <vector>
#include <Core/Utils/Legacy/Assert.h>
#include <Core/Utils/Exception.h>

#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

class SCISHARE TetSamplingSchemes
{
  public:

  template <class ARRAY1, class ARRAY2>
  void get_gaussian_scheme(ARRAY1& coords, ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;
    if (order == 1 || order == 0)
    {
      const double gaussian_weights[1] = { 1.0};
      const double gaussian_coords[1][3] = { {1./4., 1./4., 1./4.}};
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
      const double gaussian_weights[4] = {.25, .25, .25, .25};
      const double gaussian_coords[4][3] = {
        {0.138196601125011,  0.138196601125011, 0.138196601125011},
        {0.585410196624969,  0.138196601125011, 0.138196601125011},
        {0.138196601125011,  0.585410196624969, 0.138196601125011},
        {0.138196601125011,  0.138196601125011, 0.585410196624969}};
      const unsigned int num_coords = 3;
      // Not sure why this is here? It seems to truncate the size of the weights
      // vector and coords array.  - PRJ June, 2013.
      //   const unsigned int num_points = 3;
      const unsigned int num_points = 4;

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
      const double gaussian_weights[11] =   {
        // Note: weights here sum to 1/6 (which equals volume of unit element),
        //	whereas, above weights sum to 1. This might cause a few scaling
        //	problems depending on the order of integration chosen - PRJ June, 2013.
        -0.01315556,   // Should check this one Gaussian schemes should not have negative weights
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
      const double gaussian_coords[11][3] = {
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
      const unsigned int num_coords = 3;
      // Not sure why this is here? It seems to truncate the size of the weights
      // vector and coords array. I have set it back to full size go get the
      // routine BuildMassMatrix to work correctly. The same comment applies
      // to order = 2 (above) as well - PRJ June, 2013.
      //   const unsigned int num_points = 7;
      const unsigned int num_points = 11;

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
    coords.resize(0);

    int m = 0;

    for (int p=0; p<order;p++)
      for (int q=0; q<order;q++)
        for (int r=0; r<order;r++)
        {
          if (p+q+r == (order-1))
          {
            coords_type c(3);
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+1.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+1.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
          }
          if (p+q+r == (order-2))
          {
            coords_type c(3);
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+2.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+1.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+1.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+2.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+1.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+1.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+2.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+1.0/3.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+1.0/3.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/3.0)/static_cast<double>(order));
            coords.push_back(c); m++;
          }
          if (p+q+r < (order-2))
          {
            coords_type c(3);
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+2.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+1.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+1.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+2.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+1.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+1.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+2.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+1.0/3.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+1.0/3.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+1.0/3.0)/static_cast<double>(order));
            coords.push_back(c); m++;
            c[0] = static_cast<typename coords_type::value_type>((static_cast<double>(p)+2.0/4.0)/static_cast<double>(order));
            c[1] = static_cast<typename coords_type::value_type>((static_cast<double>(q)+2.0/4.0)/static_cast<double>(order));
            c[2] = static_cast<typename coords_type::value_type>((static_cast<double>(r)+2.0/4.0)/static_cast<double>(order));
            coords.push_back(c); m++;
          }
        }

    weights.resize(m);
    for (int p=0; p<m;p++) weights[p] = static_cast<typename ARRAY2::value_type>(1.0/static_cast<double>(m));
  }
};

}}}

#endif

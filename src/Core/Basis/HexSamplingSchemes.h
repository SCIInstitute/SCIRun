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


/// @todo Documentation Core/Basis/HexSamplingSchemes.h

#ifndef CORE_BASIS_HEXSAMPLINGSCHEMES_H
#define CORE_BASIS_HEXSAMPLINGSCHEMES_H 1

#include <vector>
#include <Core/Utils/Legacy/Assert.h>

#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

class SCISHARE HexSamplingSchemes
{
  public:

  template <class ARRAY1, class ARRAY2>
  void get_gaussian_scheme(ARRAY1& coords, ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;

    if (order == 0 || order == 1)
    {
      const double gaussian_weights[1] = { 1.0};
      const double gaussian_coords[1][3] = {{0.5, 0.5, 0.5}};
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
      const double gaussian_weights[8] = {.125, .125, .125, .125,
                                          .125, .125, .125, .125};
      const double gaussian_coords[8][3] = {
        {0.211324865405, 0.211324865405, 0.211324865405},
        {0.788675134595, 0.211324865405, 0.211324865405},
        {0.788675134595, 0.788675134595, 0.211324865405},
        {0.211324865405, 0.788675134595, 0.211324865405},
        {0.211324865405, 0.211324865405, 0.788675134595},
        {0.788675134595, 0.211324865405, 0.788675134595},
        {0.788675134595, 0.788675134595, 0.788675134595},
        {0.211324865405, 0.788675134595, 0.788675134595}};
      const unsigned int num_coords = 3;
      const unsigned int num_points = 8;

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
      const double gaussian_weights[27] =   {
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
      const double gaussian_coords[27][3] = {
        {0.11270166537950, 0.11270166537950, 0.11270166537950},
        {0.5, 0.11270166537950, 0.11270166537950},
        {0.88729833462050, 0.11270166537950, 0.11270166537950},
        {0.11270166537950, 0.5, 0.11270166537950},
        {0.5, 0.5, 0.11270166537950},
        {0.88729833462050, 0.5, 0.11270166537950},
        {0.11270166537950, 0.88729833462050, 0.11270166537950},
        {0.5, 0.88729833462050, 0.11270166537950},
        {0.88729833462050, 0.88729833462050, 0.11270166537950},

        {0.11270166537950, 0.11270166537950, 0.5},
        {0.5, 0.11270166537950, 0.5},
        {0.88729833462050, 0.11270166537950, 0.5},
        {0.11270166537950, 0.5, 0.5},
        {0.5, 0.5, 0.5},
        {0.88729833462050, 0.5, 0.5},
        {0.11270166537950, 0.88729833462050, 0.5},
        {0.5, 0.88729833462050, 0.5},
        {0.88729833462050, 0.88729833462050, 0.5},

        {0.11270166537950, 0.11270166537950, 0.88729833462050},
        {0.5, 0.11270166537950, 0.88729833462050},
        {0.88729833462050, 0.11270166537950, 0.88729833462050},
        {0.11270166537950, 0.5, 0.88729833462050},
        {0.5, 0.5, 0.88729833462050},
        {0.88729833462050, 0.5, 0.88729833462050},
        {0.11270166537950, 0.88729833462050, 0.88729833462050},
        {0.5, 0.88729833462050, 0.88729833462050},
        {0.88729833462050, 0.88729833462050, 0.88729833462050}
      };
      const unsigned int num_coords = 3;
      const unsigned int num_points = 27;

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

  template <class  ARRAY1, class ARRAY2>
  void get_regular_scheme(ARRAY1& coords, ARRAY2& weights, int order)
  {
    typedef typename ARRAY1::value_type coords_type;

    coords.resize(order*order*order);
    weights.resize(order*order*order);
    for (int i=0; i< order; i++)
    {
      for (int j=0; j< order; j++)
      {
        for (int k=0; k< order; k++)
        {
          coords[i+order*j+order*order*k].resize(3);
          coords[i+order*j+order*order*k][0] =
              static_cast<typename coords_type::value_type>(static_cast<double>(i+1)/static_cast<double>(order+1));
          coords[i+order*j+order*order*k][1] =
              static_cast<typename coords_type::value_type>(static_cast<double>(j+1)/static_cast<double>(order+1));
          coords[i+order*j+order*order*k][2] =
              static_cast<typename coords_type::value_type>(static_cast<double>(k+1)/static_cast<double>(order+1));
          weights[i+order*j+order*order*k] =
              static_cast<typename ARRAY2::value_type>(1.0/static_cast<double>(order*order*order));
        }
      }
    }
  }
};

}}}

#endif

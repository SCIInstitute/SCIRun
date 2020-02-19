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


/// @todo Documentation Core/Basis/PntSamplingSchemes.h
#ifndef CORE_BASIS_PNTSAMPLINGSCHEMES_H
#define CORE_BASIS_PNTSAMPLINGSCHEMES_H 1

#include <vector>
#include <Core/Utils/Legacy/Assert.h>

#include <Core/Basis/share.h>

namespace SCIRun {
namespace Core {
namespace Basis {

class SCISHARE PntSamplingSchemes
{
  public:

  template <class ARRAY1, class ARRAY2>
  void get_gaussian_scheme(ARRAY1& coords, ARRAY2& weights, int /*order*/)
  {
    typedef typename ARRAY1::value_type coords_type;

    weights.resize(1);
    weights[0] = static_cast<typename ARRAY2::value_type>(1.0);
    coords.resize(1);
    coords[0].resize(1);
    coords[0][0] = static_cast<typename coords_type::value_type>(0.0);
  }

  template <class ARRAY1, class ARRAY2>
  void get_regular_scheme(ARRAY1& coords, ARRAY2& weights, int /*order*/)
  {
    typedef typename ARRAY1::value_type coords_type;

    weights.resize(1);
    weights[0] = static_cast<typename ARRAY2::value_type>(1.0);
    coords.resize(1);
    coords[0].resize(1);
    coords[0][0] = static_cast<typename coords_type::value_type>(0.0);
  }
};

}}}

#endif

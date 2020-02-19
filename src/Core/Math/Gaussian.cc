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
///@class Gaussian
///@brief Support choosing a random value from a 1D Gaussian
///       distribution (rand), as well as evaluate the probability
///       of a particular value occuring.
///
///@author
///       David Weinstein
///       Department of Computer Science
///       University of Utah
///@date  July 2000
///

#include <Core/Math/Gaussian.h>
#include <Core/Math/MusilRNG.h>

namespace SCIRun {

Gaussian::Gaussian(double mean, double sigma, int seed)
  : mean_(mean), sigma_(sigma), mr_(new MusilRNG(seed))
{
}

Gaussian::~Gaussian() {
  delete mr_;
}

} // End namespace SCIRun

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
///@brief Support for Gaussian distributions
///
///@author
///       David Weinstein
///       Department of Computer Science
///       University of Utah
///
///@date
///       April 2002
///


#ifndef SCI_GAUSSIAN_H__
#define SCI_GAUSSIAN_H__

#include <Core/Math/MusilRNG.h>
#include <Core/Math/MiscMath.h> // for M_PI
#include <memory>
#include <cmath>

#include <Core/Math/share.h>

// Currently only used in Packages/BioPSE/Dataflow/Modules/Inverse/OptimizeConductivities.cc

namespace SCIRun {

//   http://mathworld.wolfram.com/GaussianDistribution.html
class SCISHARE Gaussian {
public:
  double mean_;
  double sigma_;
  std::unique_ptr<MusilRNG> mr_;
  Gaussian(double mean=0, double sigma=1, int seed=0);

  //   pick a random value from this Gaussian distribution
  //      - implemented using the Box-Muller transformation
  inline double rand() {return sqrt(-2*log((*mr_)()))*cos(2*M_PI*(*mr_)())*sigma_+mean_;}

  //   probablility that x was picked from this Gaussian distribution
  double prob(double x) {return exp(-(x-mean_)*(x-mean_)/(2*sigma_*sigma_))/(sigma_*sqrt(2*M_PI));}
};

} // End namespace SCIRun

#endif //SCI_GAUSSIAN_H__

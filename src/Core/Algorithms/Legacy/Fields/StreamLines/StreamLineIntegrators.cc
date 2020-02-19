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

	 Author: 				Allen R. Sanderson
	 Date: 					July 2006
*/


#include <Core/Algorithms/Legacy/Fields/StreamLines/StreamLineIntegrators.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

/// interpolate using the generic linear interpolator
bool
StreamLineIntegrators::interpolate( const Point &p,
				    Vector &v)
{
  // This has been in the code base for a long time. Not sure why
  // someone added the normalization but it is not correct.
  //  vfield_->interpolate(v, p);
  //  return (v.safe_normalize() > 0.0);

  return vfield_->interpolate(v, p);
}


// LUTs for the RK-fehlberg algorithm
static const double rkf_a[] =
  {16.0/135, 0, 6656.0/12825, 28561.0/56430, -9.0/50, 2.0/55};
static const double rkf_ab[] =
  {1.0/360, 0, -128.0/4275, -2197.0/75240, 1.0/50, 2.0/55};
//static const double rkf_c[] =
//  {0, 1.0/4, 3.0/8, 12.0/13, 1.0, 1.0/2}; // Not used, keep for reference.
static const double rkf_d[][5]=
  {{0, 0, 0, 0, 0},
   {1.0/4, 0, 0, 0, 0},
   {3.0/32, 9.0/32, 0, 0, 0},
   {1932.0/2197, -7200.0/2197, 7296.0/2197, 0, 0},
   {439.0/216, -8.0, 3680.0/513, -845.0/4104, 0},
   {-8.0/27, 2.0, -3544.0/2565, 1859.0/4104, -11.0/40}};


void
StreamLineIntegrators::FindAdamsBashforth()
{
  // Initialize Adams-Bashforth with five steps from the forth-order
  // Runga-Kutta.

  int n = max_steps_;
  max_steps_ = max_steps_ < 5 ? max_steps_ : 5;
  FindRK4();
  max_steps_ = n;

  if (nodes_.size() < 5) {
    return;
  }

  Vector f[5];

  for (unsigned int i=0; i<5; i++) {
    interpolate(nodes_[nodes_.size() - 1 - i], f[i]);
  }

  seed_ = nodes_[nodes_.size() - 1];

  for (unsigned int i=5; i<max_steps_; i++) {
    seed_ += (step_size_/720.) * (1901.0 * f[0] - 2774.0 * f[1] +
		       2616.0 * f[2] - 1274.0 * f[3] +
		       251.0 * f[4]);

    f[4] = f[3];
    f[3] = f[2];
    f[2] = f[1];
    f[1] = f[0];

    if (!interpolate(seed_, f[0])) {
      break;
    }

    nodes_.push_back(seed_);
  }
}

void
StreamLineIntegrators::FindHeun()
{
  Vector v0, v1;

  if (!interpolate(seed_, v0))
    return;

  for (unsigned int i=0; i<max_steps_; i++) {
    v0 *= step_size_;
    if (!interpolate(seed_ + v0, v1))
      break;

    v1 *= step_size_;
    seed_ += 0.5 * (v0 + v1);

    if (!interpolate(seed_, v0))
      break;

    nodes_.push_back(seed_);
  }
}


void
StreamLineIntegrators::FindRK4()
{
  Vector f[4];

  if (!interpolate(seed_, f[0]))
    return;

  for (unsigned int i=0; i<max_steps_; i++) {
    f[0] *= step_size_;
    if (!interpolate(seed_ + f[0] * 0.5, f[1]))
      break;

    f[1] *= step_size_;
    if (!interpolate(seed_ + f[1] * 0.5, f[2]))
      break;

    f[2] *= step_size_;
    if (!interpolate(seed_ + f[2], f[3]))
      break;

    f[3] *= step_size_;

    seed_ += (f[0] + 2.0 * f[1] + 2.0 * f[2] + f[3]) * (1.0 / 6.0);

    // If the new point is inside the field, add it.  Otherwise stop.
    if (!interpolate(seed_, f[0]))
      break;

    nodes_.push_back(seed_);
  }
}


void
StreamLineIntegrators::FindRKF()
{
  Vector terms[6];

  if (!interpolate(seed_, terms[0]))
    return;

  for (unsigned int i=0; i<max_steps_; i++) {
    // Compute the next set of terms.
    if (ComputeRKFTerms(terms, seed_, step_size_) < 5) {
      step_size_ /= 1.5;
      continue;
    }

    // Compute the approximate local truncation error.
    const Vector err = (terms[0]*rkf_ab[0] + terms[1]*rkf_ab[1] +
			terms[2]*rkf_ab[2] + terms[3]*rkf_ab[3] +
			terms[4]*rkf_ab[4] + terms[5]*rkf_ab[5]);
    const double err2 = err.length2();

    // Is the error tolerable?  Adjust the step size accordingly.  Too
    // small?  Grow it for next time but keep small-error result.  Too
    // big?  Recompute with smaller size.
    if (err2 * 16384.0 < tolerance2_) {
      step_size_ *= 2.0;

    } else if (err2 > tolerance2_) {
      step_size_ /= 2.0;
      continue;
    }

    // Compute and add the point to the list of points found.
    seed_ += (terms[0]*rkf_a[0] + terms[1]*rkf_a[1] + terms[2]*rkf_a[2] +
	      terms[3]*rkf_a[3] + terms[4]*rkf_a[4] + terms[5]*rkf_a[5]);

    // If the new point is inside the field, add it.  Otherwise stop.
    if (!interpolate(seed_, terms[0]))
      break;

    nodes_.push_back(seed_);
  }
}


int
StreamLineIntegrators::ComputeRKFTerms(Vector v[6],       // storage for terms
				  const Point &p,    // previous point
				  double s)
{
  v[0] *= s;

  if (!interpolate(p + v[0]*rkf_d[1][0], v[1]))
    return 0;

  v[1] *= s;

  if (!interpolate(p + v[0]*rkf_d[2][0] + v[1]*rkf_d[2][1], v[2]))
    return 1;

  v[2] *= s;

  if (!interpolate(p +
		   v[0]*rkf_d[3][0] + v[1]*rkf_d[3][1] +
		   v[2]*rkf_d[3][2], v[3]))
    return 2;

  v[3] *= s;

  if (!interpolate(p +
		   v[0]*rkf_d[4][0] + v[1]*rkf_d[4][1] +
		   v[2]*rkf_d[4][2] + v[3]*rkf_d[4][3], v[4]))
    return 3;

  v[4] *= s;

  if (!interpolate(p +
		   v[0]*rkf_d[5][0] + v[1]*rkf_d[5][1] +
		   v[2]*rkf_d[5][2] + v[3]*rkf_d[5][3] +
		   v[4]*rkf_d[5][4], v[5]))
    return 4;

  v[5] *= s;

  return 5;
}


void
StreamLineIntegrators::integrate(IntegrationMethod method)
{
  switch ( method )
  {
  case IntegrationMethod::AdamsBashforth:
    FindAdamsBashforth();
    break;

  case IntegrationMethod::Heun:
    FindHeun();
    break;

  case IntegrationMethod::RungeKutta:
    FindRK4();
    break;

  case IntegrationMethod::RungeKuttaFehlberg:
    FindRKF();
    break;
  default:
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Unknown stream line integration method (Cell walk uses different function)"));
  }
}

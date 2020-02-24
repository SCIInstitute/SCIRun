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

   Author:            David Weinstein
   Date:              December 5 2001
*/

#include <Core/Algorithms/Geometry/CoregPts.h>
#include <Core/Geometry/BBox.h>
#include <Core/Math/MiscMath.h>
#include <iostream>

using std::cerr;

namespace SCIRun {

CoregPts::CoregPts(int allowScale, int allowRotate, int allowTranslate) :
  allowScale_(allowScale), allowRotate_(allowRotate),
  allowTranslate_(allowTranslate)
{
}


CoregPts::~CoregPts()
{
}


void
CoregPts::setOrigPtsA(const std::vector<Point> &a)
{
  origPtsA_ = a;
  invalidate();
}


void
CoregPts::setOrigPtsP(const std::vector<Point> &p)
{
  origPtsP_ = p;
  invalidate();
}


int
CoregPts::getTransPtsA(std::vector<Point> &p)
{
  if (computeTransPtsA())
  {
    p = transPtsA_ ;
    return 1;
  }
  else
  {
    return 0;
  }
}


int
CoregPts::getTrans(Transform &t)
{
  if (computeTrans())
  {
    t = transform_;
    return 1;
  }
  else
  {
    return 0;
  }
}


void
CoregPts::invalidate()
{
  validTransPtsA_ = 0;
  validTrans_ = 0;
}


int
CoregPts::computeTransPtsA()
{
  if (validTransPtsA_) return 1;
  if (!validTrans_)
    if (!computeTrans()) return 0;
  transPtsA_.resize(0);
  for (unsigned int i=0; i<origPtsA_.size(); i++)
    transPtsA_.push_back(transform_.project(origPtsA_[i]));
  validTransPtsA_ = 1;
  return 1;
}


int
CoregPts::getMisfit(double &misfit)
{
  if (!computeTransPtsA())
    return 0;
  misfit=0;
  int npts = origPtsP_.size();
  for (int i=0; i<npts; i++)
    misfit += (origPtsP_[i]-transPtsA_[i]).length2()/npts;
  misfit = Sqrt(misfit);
  return 1;
}


CoregPtsAnalytic::~CoregPtsAnalytic()
{
}


CoregPtsAnalytic::CoregPtsAnalytic(int allowScale, int allowRotate,
				   int allowTranslate) :
  CoregPts(allowScale, allowRotate, allowTranslate)
{
}


int
CoregPtsAnalytic::computeTrans()
{
  unsigned int i;
  if (validTrans_) return 1;

  // make sure we have the right number of points
  unsigned int npts = Min((unsigned int)origPtsA_.size(),
                          (unsigned int)origPtsP_.size());
  if (npts < 3) return 0;

  // make sure the three A points aren't colinear
  Vector a01 = origPtsA_[1] - origPtsA_[0];
  a01.normalize();
  Vector a21 = origPtsA_[1] - origPtsA_[2];
  a21.normalize();
  if (Abs(Dot(a01,a21)) > 0.999) return 0;

  // make sure the three P points aren't colinear
  Vector p01 = origPtsP_[1] - origPtsP_[0];
  p01.normalize();
  Vector p21 = origPtsP_[1] - origPtsP_[2];
  p21.normalize();
  if (Abs(Dot(p01,p21)) > 0.999) return 0;

  // input is valid, let's compute the optimal transform according
  // to Weinstein's Univ of Utah Tech Report UUCS-98-005:
  // "The Analytic 3-D Transform for the Least-Squared Fit of Three
  // Pairs of Corresponding Points" (March, 1998).

  // To transform the points "orig_a" to "trans_a" such that they best align
  //  with their corresponding points "orig_p", we use this equation:

  // trans_a = (TCp * Bpt * S * Theta * Ba * TC_a) * orig_a

  // Here are the definitions of the intermediate matrices:
  //  TC_a  : Translate vertices to be centered about the origin
  //  Ba    : Rotate the vertices into the xy plane
  //  Theta : Rotate the vertices within the xy plane
  //  S     : Scale the distance from the vertices to the origin
  //  Bpt   : Rotate the vertices from the xy plane to the coordinate from of p
  //  TCp   : Translate the vertices to have the same centroid as p

  Transform TCp, Bpt, S, Theta, Ba, TC_a;

  // first, compute the mid-points of a and p
  Point Cp(AffineCombination(origPtsP_[0], 1./3,
			     origPtsP_[1], 1./3,
			     origPtsP_[2], 1./3));
  Point Ca(AffineCombination(origPtsA_[0], 1./3,
			     origPtsA_[1], 1./3,
			     origPtsA_[2], 1./3));
  TC_a.pre_translate(-(Ca.asVector()));

  if (allowTranslate_)
    TCp.pre_translate(Cp.asVector());
  else
    TCp.pre_translate(Ca.asVector());

  Point a[3], p[3];
  if (allowRotate_) {
    // find the normal and tangents for triangle a and for triangle p
    Vector a20=Cross(a01, a21);
    a20.normalize();
    a20.find_orthogonal(a01, a21);

    Vector p20=Cross(p01, p21);
    p20.normalize();
    p20.find_orthogonal(p01, p21);

    Transform temp;
    double d[16];
    temp.load_frame(a01, a21, a20);
    temp.get_trans(&(d[0]));
    Ba.set(d);

    Bpt.load_frame(p01, p21, p20);

    //  Bpt.load_identity();
    //  Ba.load_identity();

    // find optimal rotation theta
    // this is easier if we transform the points through the above transform
    // into their "canonical" position -- triangles centered at the origin,
    // and lying in the xy plane.

    double ra[3], rp[3], theta[3];

    for (i=0; i<3; i++) {
      // build the canonically-posed vertices
      a[i]=Ba.project(TC_a.project(origPtsA_[i]));
      p[i]=Bpt.unproject(TCp.unproject(origPtsP_[i]));

      // compute their distance from the origin
      ra[i] = a[i].asVector().length();
      rp[i] = p[i].asVector().length();

      // find the angular distance (in radians) between corresponding points
      Vector avn(a[i].asVector());
      avn.normalize();
      Vector pvn(p[i].asVector());
      pvn.normalize();
      theta[i] = acos(Dot(avn,pvn));

      // make sure we have the sign right
      if (Cross(avn,pvn).z() < 0) theta[i]*=-1;
    }

    double theta_best = -theta[0] + atan((ra[1]*rp[1]*sin(theta[0]-theta[1])+
					  ra[2]*rp[2]*sin(theta[0]-theta[2]))/
					 (ra[0]*rp[0]+
					  ra[1]*rp[1]*cos(theta[0]-theta[1])+
					  ra[2]*rp[2]*cos(theta[0]-theta[2])));
    Theta.pre_rotate(-theta_best, Vector(0,0,1));

    // lastly, rotate the a points into position and solve for scale
  }

  if (allowScale_) {
    Vector av[3];
    double scale_num=0, scale_denom=0;
    for (i=0; i<3; i++) {
      av[i] = Theta.project(a[i]).asVector();
      scale_num += Dot(av[i], p[i].asVector());
      scale_denom += av[i].length2();
    }

    double scale = scale_num/scale_denom;
    S.pre_scale(Vector(scale, scale, scale));
  }

  transform_.load_identity();
  transform_.pre_trans(TC_a);
  transform_.pre_trans(Ba);
  transform_.pre_trans(Theta);
  transform_.pre_trans(S);
  transform_.pre_trans(Bpt);
  transform_.pre_trans(TCp);

  validTrans_ = 1;

  return 1;
}


CoregPtsProcrustes::~CoregPtsProcrustes()
{
}


CoregPtsProcrustes::CoregPtsProcrustes(int allowScale, int allowRotate,
				       int allowTranslate) :
  CoregPts(allowScale, allowRotate, allowTranslate)
{
}


int
CoregPtsProcrustes::computeTrans()
{
  cerr << "ERROR - CoregPtsProcrustes::computeTrans() not yet implemented.\n";
  return 0;
}


CoregPtsSimplexSearch::~CoregPtsSimplexSearch()
{
}


CoregPtsSimplexSearch::CoregPtsSimplexSearch(int maxIters, double misfitTol,
					     int &abort,
					     VField* dField,
					     MusilRNG &mr,
					     int allowScale, int allowRotate,
					     int allowTranslate) :
  CoregPts(allowScale, allowRotate, allowTranslate),
  maxIters_(maxIters), misfitTol_(misfitTol), abort_(abort), dField_(dField),
  mr_(mr)
{
  NDIM_ = 7;
  NSEEDS_ = 8;
  params_.resize(NSEEDS_+1, NDIM_);
  misfit_.resize(NSEEDS_+1);
  dField_->vmesh()->synchronize(Mesh::LOCATE_E);
}


int
CoregPtsSimplexSearch::getMisfit(double &misfit)
{
  if (!computeTransPtsA())
    return 0;
  misfit=0;
  double m;
  int npts = origPtsP_.size();
  for (int i=0; i<npts; i++)
  {
    dField_->interpolate(m, origPtsP_[i]);
    misfit += m;
  }
  misfit = Sqrt(misfit/npts);
  return 1;
}


void
CoregPtsSimplexSearch::compute_misfit(int idx)
{
  // set up the transform_ matrix based on params_(idx, xx)
  // theta, phi, rot, transx, transy, transz, scale;
  // order: translate to the origin, rotate, scale, translate, translate back
  double theta=params_(idx,0);
  double phi=params_(idx,1);
  Vector axis(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
  double rot=params_(idx,2);
  Vector trans(params_(idx,3), params_(idx,4), params_(idx,5));
  double scale=params_(idx,6);
  transform_.load_identity();
  transform_.pre_translate(-origPtsCenter_.asVector());
  transform_.pre_rotate(rot, axis);
  transform_.pre_scale(Vector(scale, scale, scale));
  transform_.pre_translate(trans);
  transform_.pre_translate(origPtsCenter_.asVector());
  invalidate();
  getMisfit(misfit_[idx]);
}


double
CoregPtsSimplexSearch::simplex_step(std::vector<double>& sum, double factor,
                                    int worst)
{
  double factor1 = (1 - factor)/NDIM_;
  double factor2 = factor1-factor;
  int i;
  for (i=0; i<NDIM_; i++)
    params_(NSEEDS_,i) = sum[i]*factor1 - params_(worst,i)*factor2;


  // evaluate the new guess
  compute_misfit(NSEEDS_);

  // if this is better, swap it with the worst one
  if (misfit_[NSEEDS_] < misfit_[worst]) {
    misfit_[worst] = misfit_[NSEEDS_];
    for (i=0; i<NDIM_; i++) {
      sum[i] = sum[i] + params_(NSEEDS_,i)-params_(worst,i);
      params_(worst,i) = params_(NSEEDS_,i);
    }
  }

  return misfit_[NSEEDS_];
}


//! find the iterative least-squares fit between two uncorrelated point clouds
//!  using a simplex search

int
CoregPtsSimplexSearch::computeTrans()
{
  BBox bbox;
  origPtsCenter_ = origPtsA_[0];
  bbox.extend(origPtsA_[0]);
  int i,j;
  for (i=1; i< (int)origPtsA_.size(); i++) {
    origPtsCenter_+=origPtsA_[i].asVector();
    bbox.extend(origPtsA_[i]);
  }
  origPtsCenter_/=origPtsA_.size();

  double t_scale=bbox.longest_edge()/20;
  double r_scale=.03*2*M_PI;
  double s_scale=0.03;

  if (!allowScale_) s_scale=0;
  if (!allowRotate_) r_scale=0;
  if (!allowTranslate_) t_scale=0;

  // theta, phi, rot, transx, transy, transz, scale;
  // order: translate to the origin, rotate, scale, translate, translate back
  for (j=0; j<2; j++)
    for (i=0; i<NSEEDS_; i++)
      params_(i,j)=mr_()*2*M_PI;
  // j=2;
    for (i=0; i<NSEEDS_; i++)
      params_(i,j)=(mr_()-0.5)*r_scale;
  for (j=3; j<6; j++)
    for (i=0; i<NSEEDS_; i++)
      params_(i,j)=(mr_()-0.5)*t_scale;
  // j=6;
    for (i=0; i<NSEEDS_; i++)
      params_(i,j)=1+(mr_()-0.5)*s_scale;

  for (i=0; i<NSEEDS_; i++)
    compute_misfit(i);

  // as long as we haven't exceded "iters" of iterations, and our
  // error isn't below "tolerance", and we haven't been told to "abort",
  // we continue to iterate...

  std::vector<double> sum(NDIM_, 0);
  for (i=0; i<NSEEDS_; i++)
    for (j=0; j<NDIM_; j++)
      sum[j] += params_(i,j);

  double relative_tolerance = 99.9; // Give default value to remove
				    // compiler warning.
  int num_evals = 0;

  for(;;) {
    int best, worst, next_worst;
    best = 0;
    if (misfit_[0] > misfit_[1]) {
      worst = 0;
      next_worst = 1;
    } else {
      worst = 1;
      next_worst = 0;
    }
    int i;
    for (i=0; i<NSEEDS_; i++) {
      if (misfit_[i] <= misfit_[best]) best=i;
      if (misfit_[i] > misfit_[worst]) {
	next_worst = worst;
	worst = i;
      }
      else if (misfit_[i] > misfit_[next_worst] && (i != worst)) {
	next_worst=i;
      }
      relative_tolerance = 2*(misfit_[worst]-misfit_[best])/
	(misfit_[worst]+misfit_[best]);
    }

    if (relative_tolerance < misfitTol_ || num_evals > maxIters_ || abort_)
      break;

    double step_misfit = simplex_step(sum, -1, worst);
    num_evals++;
    if (step_misfit <= misfit_[best]) {
      step_misfit = simplex_step(sum, 2, worst);
      num_evals++;
    } else if (step_misfit >= misfit_[worst]) {
      double old_misfit = misfit_[worst];
      step_misfit = simplex_step(sum, 0.5, worst);
      num_evals++;
      if (step_misfit >= old_misfit) {
	for (i=0; i<NSEEDS_; i++) {
	  if (i != best) {
	    int j;
	    for (j=0; j<NDIM_; j++)
	      params_(i,j) = params_(NSEEDS_,j) =
		0.5 * (params_(i,j) + params_(best,j));
	    misfit_[i] = misfit_[NSEEDS_];
	    num_evals++;
	  }
	}
      }
      std::fill(sum.begin(), sum.end(), 0.0);
      for (i=0; i<NSEEDS_; i++) {
	for (j=0; j<NDIM_; j++)
	  sum[j]+=params_(i,j);
      }
    }
  }
  cerr << "Coregistration -- num_evals = "<<num_evals << "\n";
  return 1;
}


} // namespace SCIRun

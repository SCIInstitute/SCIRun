/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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


/*
 *  CalcTMP.cc:  class to calcuate transmenbrane potential
 *  using algorithm from ECGSim.
 *
 *  Written by:
 *   Michael Steffen
 *   Scientific Computing and Imaging Institute
 *   May 2011
 */

#include <Modules/Legacy/Forward/CalcTMPAlgo.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Forward;

bool CalcTMPAlgo::calc_single_TMP(
          const double amplitude,
          const double dep,
          const double depslope,
          const double platslope,
          const double rep,
          const double repslope,
          const double rest,
          DenseMatrix& TMP_values)
{
  /*
  Inputs: tdep: Depolarization Time
          trep: Repolarization Time
          depslope: Depolarization Slope
          platslope: Plateau Slope
          repslope: Repolarization Slope
          res:
          amplitude: Amplitude
  Output: Dense Matrix (1xsize of matrix) containing the transmembrane potential
  */

  size_type nrows = TMP_values.nrows();
  size_type ncols = TMP_values.ncols();

  if(nrows != 1){
    error("CalcTMPAlgo: TMP_values size incorrect: require rows == 1");
    return false;
  }

  if(ncols <= 0){
    error("CalcTMPAlgo: TMP_values size incorrect: need non-empty matrix");
    return false;
  }

  double tdep = -dep;
  double trep = -rep;
  double maxAmpl = 1e-6;

  for(index_type t = 0; t < ncols; ++t)
  {
    double vali = 1.0 / (1.0 + exp(-depslope * tdep)) *
    (1.0 / (1.0 + exp(platslope * trep))) *
    (1.0 / (1.0 + exp(repslope  * trep)));
    TMP_values(0,t) = vali;
    if(vali > maxAmpl)
    {
      maxAmpl = vali;
    }

    tdep += 1.0;
    trep += 1.0;
  }
  double ampl = (amplitude - rest) / maxAmpl;
  for(index_type t = 0; t < ncols; ++t)
  {
    TMP_values(0,t) = TMP_values(0,t) * ampl + rest;
  }

  return true;
}

bool CalcTMPAlgo::calc_all_TMPs(
                const DenseMatrix& amplitudes,
                const DenseMatrix& deps,
                const DenseMatrix& depslopes,
                const DenseMatrix& platslopes,
                const DenseMatrix& reps,
                const DenseMatrix& repslopes,
                const DenseMatrix& rests,
                DenseMatrix& TMP_values)
{
  if(amplitudes.ncols() != 1 ||
     deps.ncols()       != 1 ||
     depslopes.ncols()  != 1 ||
     platslopes.ncols() != 1 ||
     reps.ncols()       != 1 ||
     repslopes.ncols()  != 1 ||
     rests.ncols()      != 1)
  {
    error("CalcTMPAlgo: All inputs must be of size 1 x nodes");
    return false;
  }

  size_type nnodes = amplitudes.nrows();
  if(nnodes <= 0){
    error("CalcTMPAlgo: Size of inputs must be > 0");
    return false;
  }

  if(deps.nrows()       != nnodes ||
     depslopes.nrows()  != nnodes ||
     platslopes.nrows() != nnodes ||
     reps.nrows()       != nnodes ||
     repslopes.nrows()  != nnodes ||
     rests.nrows()      != nnodes)
  {
    error("CalcTMPAlgo: All inputs sizes must match");
    return false;
  }

  if(TMP_values.nrows() != nnodes)
  {
    error("CalcTMPAlgo: MP_values size does not match number of nodes");
    return false;
  }

  if(TMP_values.ncols() <= 0)
  {
    error("CalcTMPAlgo: Number of columns in TMP_values must be greater than 0 for output");
    return false;
  }

  DenseMatrix mat(1, TMP_values.ncols());
  for(index_type node = 0; node < nnodes; ++node)
  {
    if (!calc_single_TMP(amplitudes(node,0),
                         deps(node,0),
                         depslopes(node,0),
                         platslopes(node,0),
                         reps(node,0),
                         repslopes(node,0),
                         rests(node,0),
                         mat))
    {
      return false;
    }
    for(index_type t = 0; t < TMP_values.ncols(); ++t)
    {
      TMP_values(node, t) = mat(0, t);
    }
  }

  return true;
}

bool CalcTMPAlgo::calc_TMPs(MatrixHandle amplitudes,
                            MatrixHandle deps,
                            MatrixHandle depslopes,
                            MatrixHandle platslopes,
                            MatrixHandle reps,
                            MatrixHandle repslopes,
                            MatrixHandle rests,
                            unsigned int nsamples,
                            DenseMatrixHandle& output)
{
  if(matrix_is::sparse(amplitudes) ||
     matrix_is::sparse(deps) ||
     matrix_is::sparse(depslopes) ||
     matrix_is::sparse(platslopes) ||
     matrix_is::sparse(reps) ||
     matrix_is::sparse(repslopes) ||
     matrix_is::sparse(rests))
  {
    error("CalcTMPAlgo: Sparse matrices not supported.");
    return false;
  }

  output.reset(new DenseMatrix(amplitudes->nrows(), nsamples));

  //TODO: refactor algo to take DenseMatrix directly from module
  DenseMatrixHandle ampDense(matrix_cast::as_dense(amplitudes));
  DenseMatrixHandle depsDense(matrix_cast::as_dense(deps));
  DenseMatrixHandle depslopesDense(matrix_cast::as_dense(depslopes));
  DenseMatrixHandle platslopesDense(matrix_cast::as_dense(platslopes));
  DenseMatrixHandle repsDense(matrix_cast::as_dense(reps));
  DenseMatrixHandle repslopesDense(matrix_cast::as_dense(repslopes));
  DenseMatrixHandle restsDense(matrix_cast::as_dense(rests));

  return calc_all_TMPs(*ampDense, *depsDense, *depslopesDense, *platslopesDense, *repsDense, *repslopesDense, *restsDense,
    *output);
}

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

#include <Packages/BioPSE/Core/Algorithms/NumApproximation/CalcTMP.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace BioPSE {

using namespace SCIRun;

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
    error("CalcTMPAlgo: TMP_values size incorrect");
    return false;
  }
  
  if(ncols <= 0){
    error("CalcTMPAlgo: TMP_values size incorrect");
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
    TMP_values.put(0,t,vali);
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
    TMP_values.put(0,t,TMP_values.get(0,t)*ampl + rest);
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
    if (!calc_single_TMP(amplitudes.get(node,0),
                         deps.get(node,0),
                         depslopes.get(node,0),
                         platslopes.get(node,0),
                         reps.get(node,0),
                         repslopes.get(node,0),
                         rests.get(node,0),
                         mat))
    {
      return false;
    }
    for(index_type t = 0; t < TMP_values.ncols(); ++t)
    {
      TMP_values.put(node, t, mat.get(0, t));
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
                            MatrixHandle& output)
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
  
  output = new DenseMatrix(amplitudes->nrows(), nsamples);
  
  if(!calc_all_TMPs(*(amplitudes->dense()),
                    *(deps->dense()),
                    *(depslopes->dense()),
                    *(platslopes->dense()),
                    *(reps->dense()),
                    *(repslopes->dense()),
                    *(rests->dense()),
                    *(output->dense())))
  {
    return false;
  }
  return true;
}


} // end namespace BioPSE

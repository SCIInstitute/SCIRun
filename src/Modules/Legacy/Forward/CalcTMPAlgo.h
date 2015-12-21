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
 *  CalcTMP.h:  class to calculate transmembrane potential
 *  using algorithm from ECGSim.
 *
 *  Written by:
 *   Michael Steffen
 *   Scientific Computing and Imaging Institute, Univeristy of Utah
 *   May 2011
 */

#ifndef PACKAGES_BIOPSE_CORE_ALGORITHMS_NUMAPPROXIMATION_CALCTMP_H
#define PACKAGES_BIOPSE_CORE_ALGORITHMS_NUMAPPROXIMATION_CALCTMP_H

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Modules/Legacy/Forward/share.h>

namespace SCIRun 
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Forward
      {

        class SCISHARE CalcTMPAlgo : public AlgorithmBase
        {
        public:

          // assumes TMP_values is sized to 1 x nsamples
          bool calc_single_TMP(const double amplitude,
            const double dep,
            const double depslope,
            const double platslope,
            const double rep,
            const double repslope,
            const double rest,
            Datatypes::DenseMatrix& TMP_values);

          // assumes TMP_values is already sized to nnodes x nsamples
          bool calc_all_TMPs(const Datatypes::DenseMatrix& amplitudes,
            const Datatypes::DenseMatrix& deps,
            const Datatypes::DenseMatrix& depslopes,
            const Datatypes::DenseMatrix& platslopes,
            const Datatypes::DenseMatrix& reps,
            const Datatypes::DenseMatrix& repslopes,
            const Datatypes::DenseMatrix& rests,
            Datatypes::DenseMatrix& TMP_values);


          // normal entry case
          bool calc_TMPs(Datatypes::MatrixHandle amplitudes,
            Datatypes::MatrixHandle deps,
            Datatypes::MatrixHandle depslopes,
            Datatypes::MatrixHandle platslopes,
            Datatypes::MatrixHandle reps,
            Datatypes::MatrixHandle repslopes,
            Datatypes::MatrixHandle rests,
            unsigned int nsamples,
            Datatypes::DenseMatrixHandle& output);

          virtual AlgorithmOutput run_generic(const AlgorithmInput&) const override { throw "todo next lunch"; }
        };


      }
    }
  }
}

#endif

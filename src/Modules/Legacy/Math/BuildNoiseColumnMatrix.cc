/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  June 1999

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Math/MiscMath.h>
#include <Core/Math/MusilRNG.h>
#include <iostream>
#include <sstream>

namespace SCIRun {

///@class BuildNoiseColumnMatrix
///@brief This module creates a noise vector. 

class BuildNoiseColumnMatrix : public Module {
  MusilRNG musil;
  GuiDouble snr_;
public:
  BuildNoiseColumnMatrix(GuiContext* ctx);
  virtual ~BuildNoiseColumnMatrix();
  virtual void execute();
};

DECLARE_MAKER(BuildNoiseColumnMatrix)
BuildNoiseColumnMatrix::BuildNoiseColumnMatrix(GuiContext* ctx)
: Module("BuildNoiseColumnMatrix", ctx, Filter,"Math", "SCIRun"),
  snr_(get_ctx()->subVar("snr"), 10.0)
{
}


BuildNoiseColumnMatrix::~BuildNoiseColumnMatrix()
{
}


void
BuildNoiseColumnMatrix::execute()
{
  update_state(NeedData);

  MatrixHandle matH;
  if (!get_input_handle("Signal", matH)) return;

  // gotta make sure we have a Dense or Column matrix...
  // ...if it's Sparse, change it to Dense

  SparseRowMatrix *sm = dynamic_cast<SparseRowMatrix *>(matH.get_rep());
  if (sm) matH = matH->dense();
  else matH.detach();

  double mean, power, sigma;
  mean=power=sigma=0;
  int r, c;
  int nr = matH->nrows();
  int nc = matH->ncols();
  double curr;
  double snr = snr_.get();
  for (r=0; r<nr; r++)
    for (c=0; c<nc; c++) {
      curr = matH->get(r, c);
      mean += curr;
    }
  mean /= nr*nc;
  for (r=0; r<nr; r++)
    for (c=0; c<nc; c++) {
      curr = matH->get(r, c);
      power += (curr-mean)*(curr-mean);
    }
  power /= nr*nc;
  
  sigma = sqrt(power)/(snr*Sqrt(2*M_PI));
  
  for (r=0; r<nr; r++)
  {
    for (c=0; c<nc; c++)
    {
      // Gaussian distribution about this percentage
      const double rnd = 2.0 * musil() - 1.0;
      double perturb = rnd * sigma * sqrt((-2.0 * log(rnd*rnd)) / (rnd*rnd));
      matH->put(r, c, perturb);
    }
  }

  send_output_handle("Noise", matH);
}

} // End namespace SCIRun

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


/*
 *  SolveMinNormLeastSqSystem: Select a row or column of a matrix
 *
 *  Written by:
 *   David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   June 1999
 *
 *
 * This module computes the minimal norm, least squared solution to a
 *  nx3 linear system.
 * Given four input ColumnMatrices (v0,v1,v2,b),
 *  find the three coefficients (w0,w1,w2) that minimize:
 *  | (w0v0 + w1v1 + w2v2) - b |.
 * If more than one minimum exisits (the system is under-determined),
 *  choose the coefficients such that (w0,w1,w2) has minimum norm.
 * We output the vector (w0,w1,w2) as a row-matrix,
 *  and we ouput the ColumnMatrix (called x), which is: | w0v0 + w1v1 + w2v2 |.
 *
 */

#include <Core/Math/Mat.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/GuiInterface/GuiVar.h>
#include <iostream>
#include <sstream>
#include <math.h>

namespace SCIRun {

class SolveMinNormLeastSqSystem : public Module {
public:
  SolveMinNormLeastSqSystem(GuiContext* ctx);
  virtual ~SolveMinNormLeastSqSystem();
  virtual void execute();
};

DECLARE_MAKER(SolveMinNormLeastSqSystem)
SolveMinNormLeastSqSystem::SolveMinNormLeastSqSystem(GuiContext* ctx)
: Module("SolveMinNormLeastSqSystem", ctx, Filter, "Math", "SCIRun")
{
}

SolveMinNormLeastSqSystem::~SolveMinNormLeastSqSystem()
{
}

void
SolveMinNormLeastSqSystem::execute()
{
  int i;
  std::vector<MatrixHandle> in(4);
  if (!get_input_handle("BasisVec1(Col)", in[0])) return;
  if (!get_input_handle("BasisVec2(Col)", in[1])) return;
  if (!get_input_handle("BasisVec3(Col)", in[2])) return;
  if (!get_input_handle("TargetVec(Col)", in[3])) return;

  MatrixHandle tmp;
  for (i = 0; i < 4; i++) { tmp = in[i]->column(); in[i] = tmp; }  

  std::vector<ColumnMatrix *> Ac(4);
  for (i = 0; i < 4; i++) {
    Ac[i] = in[i]->column();
  }
  int size = Ac[0]->nrows();
  for (i = 1; i < 4; i++) {
    if ( Ac[i]->nrows() != size ) {
      error("ColumnMatrices are different sizes");
      return;
    }
  }
  double *A[3];
  for (i=0; i<3; i++) {
    A[i]=Ac[i]->get_data_pointer();
  }
  double *b = Ac[3]->get_data_pointer();
  double *bprime = new double[size];
  double *x = new double[3];

  min_norm_least_sq_3(A, b, x, bprime, size);
   
  ColumnMatrix* w_vec = new ColumnMatrix;
  w_vec->set_data(x);   
  MatrixHandle w_vecH(w_vec);
  send_output_handle("WeightVec(Col)", w_vecH);

  ColumnMatrix* bprime_vec = new ColumnMatrix;
  bprime_vec->set_data(bprime);
  MatrixHandle bprime_vecH(bprime_vec);
  send_output_handle("ResultVec(Col)", bprime_vecH);
}    

} // End namespace SCIRun

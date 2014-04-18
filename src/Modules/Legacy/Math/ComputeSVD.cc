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

///
///@file  ComputeSVD.cc
///
///@author
///   Burak Erem
///@date  November 27, 2011

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Util/StringUtil.h>
#include <iostream>
#include <sstream>

namespace SCIRun {

using namespace SCIRun;

class ComputeSVD : public Module {
  public:
    ComputeSVD(GuiContext*);

    virtual ~ComputeSVD();
    virtual void execute();
};


DECLARE_MAKER(ComputeSVD)

ComputeSVD::ComputeSVD(GuiContext* ctx) :
  Module("ComputeSVD", ctx, Source, "Math", "SCIRun")
{
}

ComputeSVD::~ComputeSVD()
{
}


void
ComputeSVD::execute()
{
  update_state(NeedData);

  // Get the input matrix
  MatrixHandle imH;
  get_input_handle("Input", imH);
  
  // The input matrix is an m-by-n matrix, find and store those sizes
  int m = imH->nrows();
  int n = imH->ncols();
  int numsingularvals = std::min(m,n);
  
  // We don't care what type of matrix the input is, just obtain a dense matrix
  // (because that's the only type that has a built-in svd member function at the moment)
  DenseMatrix *denseinput = imH->dense();
  
  // Create the matrices for the singular value decomposition
  MatrixHandle SingularVals = new ColumnMatrix(numsingularvals);
  ColumnMatrix *S = SingularVals->column();

  MatrixHandle LeftSingularMat = new DenseMatrix(m,m);
  DenseMatrix *U = LeftSingularMat->dense();

  MatrixHandle RightSingularMat = new DenseMatrix(n,n);
  DenseMatrix *V = RightSingularMat->dense();

  // Compute the SVD of the input matrix
  try
  {
    LinearAlgebra::svd(*denseinput, *U, *S, *V);
  }
  catch (const SCIRun::Exception& exception)
  {
	  std::ostringstream oss;
	  oss << "Caught exception: " << exception.type() << " " << exception.message();
	  error(oss.str());
    return;
  }
   
  // Pass the singular value decomposition matrices as output
  send_output_handle("LeftSingularMat", LeftSingularMat);
  send_output_handle("SingularVals", SingularVals);
  send_output_handle("RightSingularMat", RightSingularMat);
  
}

} // End namespace SCIRun



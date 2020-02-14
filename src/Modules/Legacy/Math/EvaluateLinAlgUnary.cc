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


/// @author
///    David Weinstein
///    Department of Computer Science
///    University of Utah
/// @date  June 1999

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Util/StringUtil.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <sci_hash_map.h>
#include <iostream>
#include <sstream>
#include <math.h>

namespace SCIRun {

/// @class EvaluateLinAlgUnary
/// @brief Performs one of a number of selectable unary matrix operations to the input matrix.

class EvaluateLinAlgUnary : public Module {
  GuiString op_;
  GuiString function_;
  void insertion_sort(double *x, int n);
  void subtract_mean(double *x, int n);
  void normalize(double *x, int n);
  void round(double *x, int n);
  void Floor(double *x, int n);
  void Ceil(double *x, int n);
public:
  EvaluateLinAlgUnary(GuiContext* ctx);
  virtual ~EvaluateLinAlgUnary();
  virtual void execute();
};

DECLARE_MAKER(EvaluateLinAlgUnary)
EvaluateLinAlgUnary::EvaluateLinAlgUnary(GuiContext* ctx)
: Module("EvaluateLinAlgUnary", ctx, Filter,"Math", "SCIRun"),
  op_(get_ctx()->subVar("op"), "Function"),
  function_(get_ctx()->subVar("function"), "x+10")
{
}

EvaluateLinAlgUnary::~EvaluateLinAlgUnary()
{
}

void EvaluateLinAlgUnary::insertion_sort(double *x, int n) {
  double tmp;
  for (int i=0; i<n-1; i++)
    for (int j=i+1; j<n; j++)
      if (x[i] > x[j]) {
	tmp = x[i]; x[i]=x[j]; x[j]=tmp;
      }
}

void EvaluateLinAlgUnary::subtract_mean(double *x, int n) {
  double sum = 0.0;
  for (int i=0; i<n; i++) {
    sum = sum + x[i];
  }
  double avg = sum / (double)n;
  for (int i=0; i<n; i++) {
    x[i] = x[i] - avg;
  }
}

void EvaluateLinAlgUnary::normalize(double *x, int n) {
  double min =  1.0e36;
  double max = -1.0e36;
  for (int i=0; i<n; i++) {
    if( min > x[i] ) min = x[i];
    else if( max < x[i] ) max = x[i];
  }
  double mult = 1.0 / (max-min);
  for (int i=0; i<n; i++) {
    x[i] = (int) ((x[i]-min) * mult);
  }
}

void EvaluateLinAlgUnary::round(double *x, int n) {
  for (int i=0; i<n; i++) {
    x[i] = (int) (x[i] + (x[i] < 0 ?  -0.5 : +0.5) );
  }
}

void EvaluateLinAlgUnary::Floor(double *x, int n) {
  for (int i=0; i<n; i++) {
    x[i] = floor(x[i]);
  }
}

void EvaluateLinAlgUnary::Ceil(double *x, int n) {
  for (int i=0; i<n; i++) {
    x[i] = ceil(x[i]);
  }
}

void EvaluateLinAlgUnary::execute()
{
  MatrixHandle mh;

  get_input_handle("Input",mh,true);

  if (inputs_changed_ || op_.changed() || function_.changed() || !oport_cached("Output"))
  {

    std::string op = op_.get();
    MatrixHandle m;

    if (op == "Transpose")
    {
      Matrix<double>* mat = mh->make_transpose();
      m = mat;
    }
    else if (op == "Invert")
    {
      m = mh->dense();
      if (m.get_rep() == 0) { error("Could not convert matrix to dense matrix"); return; }
      m.detach();

      DenseMatrix *dm = dynamic_cast<DenseMatrix *>(m.get_rep());
      if (! dm->invert()) {
        error("Input Matrix not invertible.");
        return;
      }
    }
    else if (op == "Sort")
    {
      m = mh->dense();
      if (m.get_rep() == 0) { error("Could not convert matrix to dense matrix"); return; }
      m.detach();
      insertion_sort(m->get_data_pointer(), m->get_data_size());
      if (matrix_is::sparse(mh)) { m = m->sparse(); }
    }
    else if (op == "Subtract_Mean")
    {
      m = mh->dense();
      if (m.get_rep() == 0) { error("Could not convert matrix to dense matrix"); return; }
      m.detach();
      subtract_mean(m->get_data_pointer(), m->get_data_size());
    }
    else if (op == "Normalize")
    {
      m = mh->clone();
      normalize(m->get_data_pointer(), m->get_data_size());
    }
    else if (op == "Round")
    {
      m = mh->clone();
      round(m->get_data_pointer(), m->get_data_size());
    }
    else if (op == "Floor")
    {
      m = mh->clone();
      Floor(m->get_data_pointer(), m->get_data_size());
    }
    else if (op == "Ceil")
    {
      m = mh->clone();
      Ceil(m->get_data_pointer(), m->get_data_size());
    }
    else if (op == "Function")
    {
      NewArrayMathEngine engine;
      engine.set_progress_reporter(this);

      if (!(engine.add_input_fullmatrix("x",mh))) return;

      std::string func = function_.get();
      func = "RESULT="+func;
      engine.add_expressions(func);

      MatrixHandle omatrix = mh->clone();
      if(!(engine.add_output_fullmatrix("RESULT",omatrix))) return;

      // Actual engine call, which does the dynamic compilation, the creation of the
      // code for all the objects, as well as inserting the function and looping
      // over every data point

      if (!(engine.run())) return;

      send_output_handle("Output", omatrix);
      return;
    }
    else
    {
      warning("Don't know operation "+op);
      return;
    }

    send_output_handle("Output", m);
  }
}


} // End namespace SCIRun

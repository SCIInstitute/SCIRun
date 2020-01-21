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
///    David Weinstein,
///    Department of Computer Science,
///    University of Utah
/// @date June 1999

#include <Core/Util/StringUtil.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixOperations.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <sci_hash_map.h>
#include <iostream>
#include <sstream>

namespace SCIRun {

/// @class EvaluateLinAlgBinary
/// @brief This module performs one of a number of selectable matrix operations
/// using the two input matrices.
///
/// Binary matrix operations -- add, multiply

class EvaluateLinAlgBinary : public Module {
  GuiString op_;
  GuiString function_;
public:
  EvaluateLinAlgBinary(GuiContext* ctx);
  virtual ~EvaluateLinAlgBinary() {}
  virtual void execute();
};

DECLARE_MAKER(EvaluateLinAlgBinary)
EvaluateLinAlgBinary::EvaluateLinAlgBinary(GuiContext* ctx)
: Module("EvaluateLinAlgBinary", ctx, Filter,"Math", "SCIRun"),
  op_(get_ctx()->subVar("op"), "Mult"),
  function_(get_ctx()->subVar("function"), "x+y")
{
}


void
EvaluateLinAlgBinary::execute()
{
  update_state(NeedData);

  MatrixHandle aH, bH;
  get_input_handle("A", aH,true);
  get_input_handle("B", bH,true);

  if (inputs_changed_ || op_.changed() || function_.changed() ||
    !oport_cached("Output"))
  {
    const std::string op = op_.get();
    if (op == "Add")
    {
      if (aH->ncols() != bH->ncols() || aH->nrows() != bH->nrows())
      {
        error("Addition requires A and B must be the same size.");
        return;
      }
      MatrixHandle mtmp(aH + bH);
      send_output_handle("Output", mtmp);
      return;
    }
    else if (op == "Mult")
    {
      if (aH->ncols() != bH->nrows())
      {
        error("Matrix multiply requires the number of columns in A to be the same as the number of rows in B.");
        error("A contains " + to_string(aH->ncols()) +
              " columns, B contains " + to_string(bH->nrows()) + " rows.");
        return;
      }
      MatrixHandle mtmp;
      mtmp = aH * bH;
      send_output_handle("Output", mtmp);
      return;
    }
    else if (op == "Function")
    {
      NewArrayMathEngine engine;
      engine.set_progress_reporter(this);

      if (!(engine.add_input_fullmatrix("x",aH))) return;
      if (!(engine.add_input_fullmatrix("y",bH))) return;

      std::string func = function_.get();
      func = "RESULT="+func;
      engine.add_expressions(func);

      MatrixHandle omatrix = aH->clone();
      if(!(engine.add_output_fullmatrix("RESULT",omatrix))) return;

      // Actual engine call, which does the dynamic compilation, the creation of the
      // code for all the objects, as well as inserting the function and looping
      // over every data point

      if (!(engine.run())) return;

      send_output_handle("Output", omatrix);
      return;
    }
    else if (op == "SelectColumns")
    {
      ColumnMatrix *bc = matrix_cast::as_column(bH);
      if (!bc)
      {
        error("Second input to SelectColumns must be a ColumnMatrix.");
        return;
      }
      DenseMatrix *cd = new DenseMatrix(aH->nrows(), bc->nrows());
      for (int i=0; i<cd->ncols(); i++)
      {
        int idx = (int)(*bc)[i];
        if (idx == -1) continue;
        if (idx > aH->ncols())
        {
          error("Tried to select column (" + to_string(idx) +
          ") that was out of range (" + to_string(aH->ncols()) + ").");
          return;
        }
        for (int j=0; j<aH->nrows(); j++)
        {
          (*cd)[j][i]=aH->get(j,idx);
        }
      }
      if (matrix_is::dense(aH))
      {
        MatrixHandle mtmp(cd);
        send_output_handle("Output", mtmp);
      }
      else if (matrix_is::column(aH))
      {
        MatrixHandle mtmp(cd->column());
        delete cd;
        send_output_handle("Output", mtmp);
      } else {
        MatrixHandle mtmp(cd->sparse());
        delete cd;
        send_output_handle("Output", mtmp);
      }
      return;
    }
    else if (op == "SelectRows")
    {
      ColumnMatrix *bc = matrix_cast::as_column(bH);
      if (!bc) {
        error("Second input must be a ColumnMatrix for SelectRows.");
        return;
      }
      DenseMatrix *cd = new DenseMatrix(bc->nrows(), aH->ncols());
      for (int i=0; i<cd->nrows(); i++) {
        int idx = (int)(*bc)[i];
        if (idx == -1) continue;
        if (idx > aH->nrows())
        {
          error("Tried to select a row (" + to_string(idx) +
          ") that was out of range (" + to_string(aH->nrows()) +").");
          return;
        }
        for (int j=0; j<aH->ncols(); j++)
        {
          (*cd)[i][j]=aH->get(idx,j);
        }
      }
      if (matrix_is::dense(aH))
      {
        MatrixHandle mtmp(cd);
        send_output_handle("Output", mtmp);
      }
      else if (matrix_is::column(aH)) {
        MatrixHandle mtmp(cd->column());
        delete cd;
        send_output_handle("Output", mtmp);
      } else {
        MatrixHandle mtmp(cd->sparse());
        delete cd;
        send_output_handle("Output", mtmp);
      }
      return;
    }
    else if (op == "NormalizeAtoB")
    {
      if (matrix_is::sparse(aH) || matrix_is::sparse(bH))
      {
        error("NormalizeAtoB does not currently support SparseRowMatrices.");
        return;
      }

      if (aH->get_data_size() == 0 || bH->get_data_size() == 0)
      {
        error("Cannot NormalizeAtoB with empty matrices.");
        return;
      }

      double amin, amax, bmin, bmax;
      double *a = aH->get_data_pointer();
      double *b = bH->get_data_pointer();
      const int na = aH->get_data_size();
      const int nb = bH->get_data_size();
      amin = amax = a[0];
      bmin = bmax = b[0];
      int i;
      for (i=1; i<na; i++) {
        if (a[i]<amin) amin=a[i];
        else if (a[i]>amax) amax=a[i];
      }
      for (i=1; i<nb; i++) {
        if (b[i]<bmin) bmin=b[i];
        else if (b[i]>bmax) bmax=b[i];
      }

      MatrixHandle anewH = aH->clone();
      double *anew = anewH->get_data_pointer();
      const double scale = (bmax - bmin)/(amax - amin);
      for (i=0; i<na; i++)
        anew[i] = (a[i]-amin)*scale+bmin;
      send_output_handle("Output", anewH);
    }
    else
    {
      error("Don't know operation " + op);
      return;
    }
  }
}

} // End namespace SCIRun

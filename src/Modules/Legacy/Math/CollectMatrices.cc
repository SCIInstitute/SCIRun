/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

///@brief Matrix operations -- concatenate, replace
///
///@author
///   David Weinstein &
///   Chris Butson
///   Department of Computer Science
///   University of Utah
///@date  July 2002

#include <Modules/Legacy/Math/CollectMatrices.h>
//#include <Core/Datatypes/DenseMatrix.h>
//#include <Core/Datatypes/SparseRowMatrix.h>
//#include <Core/Datatypes/MatrixTypeConverter.h>
//#include <Core/Algorithms/Math/CollectMatrices/CollectMatricesAlgorithm.h>


using namespace SCIRun;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;

/// @class EvaluateLinAlgGeneral
/// @brief This module performs a user defined linear algebra operation on up to five input matrices.

const ModuleLookupInfo CollectMatrices::staticInfo_("CollectMatrices", "Math", "SCIRun");

CollectMatrices::CollectMatrices() : Module(staticInfo_)
{
  INITIALIZE_PORT(Optional_BaseMatrix);
  INITIALIZE_PORT(SubMatrix);
  INITIALIZE_PORT(CompositeMatrix);
}

void CollectMatrices::setStateDefaults()
{
  //TODO
}

void CollectMatrices::execute()
{
  //TODO
}

#if 0
namespace SCIRun {

/// @class CollectMatrices
/// @detail This module appends/replaces rows or columns of a matrix while
/// looping through a network.

class CollectMatrices : public Module
{
public:
  explicit CollectMatrices(GuiContext* ctx);
  virtual void execute();
  virtual void tcl_command(GuiArgs&, void *);
private:
  MatrixHandle matrixH_;
  GuiInt append_;   // append or replace
  GuiInt row_;      // row or column
  GuiInt front_;    // append at the beginning or the end

  Algo::CollectMatricesAlgorithmBase* create_algo(MatrixHandle aH, MatrixHandle bH) const;
};

DECLARE_MAKER(CollectMatrices)
CollectMatrices::CollectMatrices(GuiContext* ctx)
: Module("CollectMatrices", ctx, Filter,"Math", "SCIRun"),
  append_(get_ctx()->subVar("append"), 0),
  row_(get_ctx()->subVar("row"), 0),
  front_(get_ctx()->subVar("front"), 0)
{
}


/// @todo: match output matrix type with input type.
void
CollectMatrices::execute()
{
  update_state(NeedData);

  MatrixHandle aH, bH;
  get_input_handle("Optional BaseMatrix", aH, false);

  if (!get_input_handle("SubMatrix", bH))
    return;

  bool append = append_.get();
  bool row = row_.get();
  bool front = front_.get();


  MatrixHandle omatrix;

  if (!append)               // Replace -- just send B matrix
  {
    matrixH_ = bH;
    send_output_handle("CompositeMatrix", matrixH_, true);
    return;
  }
  else if (!aH.get_rep())    // No A matrix
  {
    if (!matrixH_.get_rep())
    {
      // No previous CompositeMatrix, so send B
      matrixH_ = bH;
      send_output_handle("CompositeMatrix", matrixH_, true);
      return;
    }
    else
    {
      boost::scoped_ptr<Algo::CollectMatricesAlgorithmBase> algo(create_algo(matrixH_, bH));
      // Previous CompositeMatrix exists, concatenate with B
      if (row)
      {
        if (matrixH_->ncols() != bH->ncols())
        {
          warning("SubMatrix and CompositeMatrix must have same number of columns");
          return;
        }
        else
        {
          if (front)
            omatrix = algo->concat_rows(bH,matrixH_);
          else
            omatrix = algo->concat_rows(matrixH_,bH);
        }
      }
      else
      {
        if (matrixH_->nrows() != bH->nrows())
        {
          warning("SubMatrix and CompositeMatrix must have same number of rows");
          return;
        }
        else
        {
          if (front)
            omatrix = algo->concat_cols(bH,matrixH_);
          else
            omatrix = algo->concat_cols(matrixH_,bH);
        } // columns
      } // rows - columns
    } // previous matrix exists
  }
  else
  { // A exists
    boost::scoped_ptr<Algo::CollectMatricesAlgorithmBase> algo(create_algo(aH, bH));
    if (row)
    {
      if (aH->ncols() != bH->ncols())
      {
        warning("BaseMatrix and CompositeMatrix must have same number of columns");
        return;
      }
      else
      {
        if (front)
          omatrix = algo->concat_rows(bH,aH);
        else
          omatrix = algo->concat_rows(aH,bH);
      }
    }
    else
    {
      if (aH->nrows() != bH->nrows())
      {
        warning("BaseMatrix and CompositeMatrix must have same number of rows");
        return;
      }
      else
      {
        if (front)
          omatrix = algo->concat_cols(bH,aH);
        else
          omatrix = algo->concat_cols(aH,bH);
      } // columns
    } // rows - columns
  } // A exists

  if (omatrix.get_rep())
  {
    matrixH_ = omatrix;
    send_output_handle("CompositeMatrix", matrixH_, true);
  }
}


void
CollectMatrices::tcl_command(GuiArgs& args, void* userdata)
{
  if (args[1] == "clear")
  {
    matrixH_ = 0;
    remark("Collected matrix buffer has been cleared.");
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}

Algo::CollectMatricesAlgorithmBase*
CollectMatrices::create_algo(MatrixHandle aH, MatrixHandle bH) const
{
  if (matrix_is::sparse(aH) && matrix_is::sparse(bH))
    return new Algo::CollectSparseRowMatricesAlgorithm;
  else
    return new Algo::CollectDenseMatricesAlgorithm;
}

} // End namespace SCIRun
#endif

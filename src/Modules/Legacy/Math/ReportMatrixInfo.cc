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


#include <Core/Datatypes/Matrix.h>
#include <Core/Util/StringUtil.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Converter/ConverterAlgo.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRun {

/// @class ReportMatrixInfo
/// @brief This module is used to view the attributes of matrices.

class ReportMatrixInfo : public Module {
private:
  GuiString gui_matrixname_;
  GuiString gui_generation_;
  GuiString gui_typename_;
  GuiString gui_rows_;
  GuiString gui_cols_;
  GuiString gui_min_;
  GuiString gui_max_;
  GuiString gui_elements_;

  void clear_vals();
  void update_input_attributes(MatrixHandle);

public:
  ReportMatrixInfo(GuiContext* ctx);
  virtual void execute();
};


DECLARE_MAKER(ReportMatrixInfo)

ReportMatrixInfo::ReportMatrixInfo(GuiContext* ctx)
  : Module("ReportMatrixInfo", ctx, Sink, "Math", "SCIRun"),
    gui_matrixname_(get_ctx()->subVar("matrixname", false),"---"),
    gui_generation_(get_ctx()->subVar("generation", false),"---"),
    gui_typename_(get_ctx()->subVar("typename", false),"---"),
    gui_rows_(get_ctx()->subVar("rows", false),"---"),
    gui_cols_(get_ctx()->subVar("cols", false),"---"),
    gui_min_(get_ctx()->subVar("min", false),"---"),
    gui_max_(get_ctx()->subVar("max", false),"---"),
    gui_elements_(get_ctx()->subVar("elements", false),"---")
{
}


void ReportMatrixInfo::clear_vals()
{
  gui_matrixname_.set("---");
  gui_generation_.set("---");
  gui_typename_.set("---");
  gui_rows_.set("---");
  gui_cols_.set("---");
  gui_elements_.set("---");
}


void
ReportMatrixInfo::update_input_attributes(MatrixHandle m)
{
  std::string matrixname;
  if (m->get_property("name", matrixname))
  {
    gui_matrixname_.set(matrixname);
  }
  else
  {
    gui_matrixname_.set("--- Name Not Assigned ---");
  }

  gui_generation_.set(to_string(m->generation));

  // Set the typename.
  if (matrix_is::sparse(m))
  {
    gui_typename_.set("SparseRowMatrix");
    gui_elements_.set(to_string(m->sparse()->get_nnz()));
  }
  else if (matrix_is::column(m))
  {
    gui_typename_.set("ColumnMatrix");
    gui_elements_.set(to_string(m->ncols() * m->nrows()));
  }
  else if (matrix_is::dense_col_maj(m))
  {
    gui_typename_.set("DenseColMajMatrix");
    gui_elements_.set(to_string(m->ncols() * m->nrows()));
  }
  else if (matrix_is::dense(m))
  {
    gui_typename_.set("DenseMatrix");
    gui_elements_.set(to_string(m->ncols() * m->nrows()));
  }
  else
  {
    gui_typename_.set("Unknown");
    gui_elements_.set("*");
  }

  gui_rows_.set(to_string(m->nrows()));
  gui_cols_.set(to_string(m->ncols()));

  gui_min_.set(to_string(m->min()));
  gui_max_.set(to_string(m->max()));
}


void ReportMatrixInfo::execute()
{
  MatrixHandle mh;
  SCIRunAlgo::ConverterAlgo calgo(this);

  if(!(get_input_handle("Input",mh,true)))
  {
    clear_vals();
    return;
  }

  if (inputs_changed_ || !oport_cached("NumRows") || !oport_cached("NumCols") || !oport_cached("NumElements"))
  {
    update_input_attributes(mh);

    MatrixHandle NumRows, NumCols, NumElements;
    if (!(calgo.IntToMatrix(mh->nrows(),NumRows))) return;
    if (!(calgo.IntToMatrix(mh->ncols(),NumCols))) return;

    int numelems;
    if (matrix_is::sparse(mh))
    {
      numelems = static_cast<int>(mh->sparse()->get_nnz());
    }
    else
    {
      numelems = static_cast<int>(mh->nrows()*mh->ncols());
    }
    if (!(calgo.IntToMatrix(numelems,NumElements))) return;

    send_output_handle("NumRows", NumRows);
    send_output_handle("NumCols", NumCols);
    send_output_handle("NumElements", NumElements);
  }
}


} // end SCIRun namespace

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


#include <Modules/Legacy/Math/SelectSubMatrix.h>
#include <Core/Algorithms/Math/SelectSubMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;

SelectSubMatrix::SelectSubMatrix() : Module(ModuleLookupInfo("SelectSubMatrix", "Math", "SCIRun"))
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(RowIndicies);
  INITIALIZE_PORT(ColumnIndicies);
  INITIALIZE_PORT(ResultMatrix);
}

void SelectSubMatrix::setStateDefaults()
{
 setStateBoolFromAlgo(SelectSubMatrixAlgorithm::rowCheckBox());
 setStateBoolFromAlgo(SelectSubMatrixAlgorithm::columnCheckBox());
 setStateIntFromAlgo(SelectSubMatrixAlgorithm::rowStartSpinBox());
 setStateIntFromAlgo(SelectSubMatrixAlgorithm::columnStartSpinBox());
 setStateIntFromAlgo(SelectSubMatrixAlgorithm::columnEndSpinBox());
 setStateIntFromAlgo(SelectSubMatrixAlgorithm::rowEndSpinBox());
}

void SelectSubMatrix::execute()
{
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  MatrixHandle matrix, row_indices, column_indices, sub_matrix;

  get_input_handle("Matrix",matrix,true);
  get_input_handle("RowIndices",row_indices,false);
  get_input_handle("ColumnIndices",column_indices,false);

  if (inputs_changed_ ||
      row_select_.changed() ||
      row_start_.changed() ||
      row_end_.changed() ||
      col_select_.changed() ||
      col_start_.changed() ||
      col_end_.changed() ||
      !oport_cached("SubMatrix"))
  {
    bool row_select = row_select_.get();
    bool col_select = col_select_.get();
    index_type row_start = row_start_.get();
    index_type row_end = row_end_.get();
    index_type col_start = col_start_.get();
    index_type col_end = col_end_.get();

    if (row_indices.get_rep() || column_indices.get_rep())
    {
      if (row_select || col_select)
      {
        remark("Index matrices detected on inputs, ignoring UI settings");
      }
    }

    if (row_select)
    {
      if (row_start < 0) row_start = 0;
      if (row_end < 0) row_end = matrix->nrows()-1;
    }

    if (col_select)
    {
      if (col_start < 0) col_start = 0;
      if (col_end < 0) col_end = matrix->ncols()-1;
    }

    if (row_indices.get_rep() && column_indices.get_rep())
    {
      if(!(sub_algo_.run(matrix,sub_matrix,row_indices,column_indices))) return;
    }
    else if (row_indices.get_rep() == 0 && column_indices.get_rep())
    {
      if(!(columns_algo_.run(matrix,sub_matrix,column_indices))) return;
    }
    else if (row_indices.get_rep()  && column_indices.get_rep() == 0)
    {
      if(!(rows_algo_.run(matrix,sub_matrix,row_indices))) return;
    }
    else
    {
      std::vector<index_type> rows;
      std::vector<index_type> cols;

      if (row_select)
      {
        rows.resize(row_end-row_start+1);
        for (index_type r = row_start; r <= row_end; r++) rows[r-row_start] = r;
      }

      if (col_select)
      {
        cols.resize(col_end-col_start+1);
        for (index_type r = col_start; r <= col_end; r++) cols[r-col_start] = r;
      }

      if (row_select && col_select)
      {
        if(!(sub_algo_.run(matrix,sub_matrix,rows,cols))) return;
      }
      else if (row_select && !col_select)
      {
        if(!(rows_algo_.run(matrix,sub_matrix,rows))) return;
      }
      else if (!row_select && col_select)
      {
        if(!(columns_algo_.run(matrix,sub_matrix,cols))) return;
      }
      else
      {
        remark("This module needs or row indices, or column indices or both");
        remark("Copying matrix to output");
        sub_matrix = matrix;
      }
    }

    send_output_handle("SubMatrix",sub_matrix,true);
  }
  #endif

  auto input_matrix = getRequiredInput(InputMatrix);
  auto rowindicies = getOptionalInput(RowIndicies);
  auto columnindicies = getOptionalInput(ColumnIndicies);

  if (needToExecute())
  {
   algo().set(SelectSubMatrixAlgorithm::rowCheckBox(), get_state()->getValue(SelectSubMatrixAlgorithm::rowCheckBox()).toBool());
   algo().set(SelectSubMatrixAlgorithm::columnCheckBox(), get_state()->getValue(SelectSubMatrixAlgorithm::columnCheckBox()).toBool());
   algo().set(SelectSubMatrixAlgorithm::rowStartSpinBox(), get_state()->getValue(SelectSubMatrixAlgorithm::rowStartSpinBox()).toInt());
   algo().set(SelectSubMatrixAlgorithm::rowEndSpinBox(), get_state()->getValue(SelectSubMatrixAlgorithm::rowEndSpinBox()).toInt());
   algo().set(SelectSubMatrixAlgorithm::columnStartSpinBox(), get_state()->getValue(SelectSubMatrixAlgorithm::columnStartSpinBox()).toInt());
   algo().set(SelectSubMatrixAlgorithm::columnEndSpinBox(), get_state()->getValue(SelectSubMatrixAlgorithm::columnEndSpinBox()).toInt());

   auto output = algo().run(withInputData((InputMatrix, input_matrix)(RowIndicies, optionalAlgoInput(rowindicies))(ColumnIndicies, optionalAlgoInput(columnindicies))));

   sendOutputFromAlgorithm(ResultMatrix, output);
  }
}

/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Core/Algorithms/Math/SelectSubMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

SelectSubMatrixAlgorithm::SelectSubMatrixAlgorithm()
{
  addParameter(rowCheckBox, false);
  addParameter(columnCheckBox, false);
  addParameter(rowStartSpinBox, 0);
  addParameter(columnStartSpinBox, 0);
  addParameter(columnEndSpinBox, 0);
  addParameter(rowEndSpinBox, 0);
}


DenseMatrixHandle SelectSubMatrixAlgorithm::get_sub_matrix(MatrixHandle& input_matrix, MatrixHandle rows, MatrixHandle columns) const
{

 return DenseMatrixHandle();
}

DenseMatrixHandle SelectSubMatrixAlgorithm::run(MatrixHandle input_matrix, MatrixHandle row_indices, MatrixHandle column_indices) const
{  
  MatrixHandle sub_matrix;
  bool row_select = get(rowCheckBox).getBool();
  bool col_select = get(columnCheckBox).getBool();
  index_type row_start = get(rowStartSpinBox).getInt();
  index_type row_end = get(columnStartSpinBox).getInt();
  index_type col_start = get(columnEndSpinBox).getInt();
  index_type col_end = get(rowEndSpinBox).getInt();
  
  if (row_indices || column_indices)
    {
      if (row_select || col_select)
      {
        remark("Index matrices detected on inputs, ignoring UI settings");
      }
    }
    
  if (row_select)
    {
      if (row_start < 0) row_start = 0;
      if (row_end < 0) row_end = input_matrix->nrows()-1;
    }

  if (col_select)
    {
      if (col_start < 0) col_start = 0;
      if (col_end < 0) col_end = input_matrix->ncols()-1;
    } 
  
  if (row_indices || column_indices)  
    sub_matrix=get_sub_matrix(input_matrix,row_indices,column_indices);
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
     sub_matrix=get_sub_matrix(input_matrix,rows,cols); 
    }  else
    {
        remark("This module needs or row indices, or column indices or both");
        remark("Copying matrix to output");
        sub_matrix=input_matrix;       
    }
  } 
     

/*  size_type num_sel_rows = rows->get_data_size();
  double* sel_rows_ptr = rows->get_data_pointer();

  size_type num_sel_columns = columns->get_data_size();
  double* sel_columns_ptr = columns->get_data_pointer();
  
  std::vector<index_type> sel_rows;
  std::vector<index_type> sel_columns;
  try
  {
    sel_rows.resize(num_sel_rows);
    for (int p=0; p< num_sel_rows; p++) 
      sel_rows[p] = static_cast<index_type>(sel_rows_ptr[p]);

    sel_columns.resize(num_sel_columns);
    for (int p=0; p< num_sel_columns; p++) 
      sel_columns[p] = static_cast<index_type>(sel_columns_ptr[p]);
  }
  catch (...)
  {
    error("Could not allocate enough memory");
    algo_end(); return (false); 
  }
  
 //bool ret = run(input,output,sel_rows,sel_columns);*/
  
 return DenseMatrixHandle();
}

DenseMatrixHandle SelectSubMatrixAlgorithm::get_sub_matrix(MatrixHandle& input_matrix, std::vector<SCIRun::index_type>& rows, std::vector<SCIRun::index_type>& columns) const
{
/*
  if (input.get_rep() == 0)
  {
    error("No input matrix");
    algo_end(); return (false);
  }

  if (rows.size() == 0)
  {
    error("No row indices given");
    algo_end(); return (false);  
  }

  if (columns.size() == 0)
  {
    error("No column indices given");
    algo_end(); return (false);  
  }

  size_type m = input->nrows();
  size_type n = input->ncols();
    
  for (size_t r=0; r<rows.size(); r++)
  {
    if (rows[r] >= static_cast<index_type>(m))
    {
      error("Selected row exceeds matrix dimensions");
      algo_end(); return (false);
    }
  }

  for (size_t r=0; r<columns.size(); r++)
  {
    if (columns[r] >= static_cast<index_type>(n))
    {
      error("Selected column exceeds matrix dimensions");
      algo_end(); return (false);
    }
  }

  SparseRowMatrix* sparse = matrix_cast::as_sparse(input);
  if (sparse)
  {
    if (!sparse->is_valid())
    {
      error("Sparse matrix is invalid");
      algo_end(); return (false);      
    }

    index_type *rr = sparse->get_rows();
    index_type *cc = sparse->get_cols();
    double *vv = sparse->get_vals();
    std::vector<index_type> s(n,n);
    for (index_type r=0;
              r< static_cast<index_type>(columns.size()); r++) 
      s[columns[r]] = r;
  
    index_type k =0;
    for (index_type r=0; r<static_cast<index_type>(rows.size()); r++)
    {
      for (index_type q=rr[rows[r]]; q<rr[rows[r]+1]; q++)
      {
        if (s[cc[q]] < n) k++;
      }
    }

    SparseRowMatrix::Data outputData(rows.size()+1, k);

    if (!outputData.allocated())
    {
      error("Could not allocate output matrix");
      algo_end(); return (false);      
    }

    const SparseRowMatrix::Rows& nrr = outputData.rows();
    const SparseRowMatrix::Columns& ncc = outputData.columns();
    const SparseRowMatrix::Storage& nvv = outputData.data();
    k =0;
    for (index_type r=0; r<static_cast<index_type>(rows.size()); r++)
    {
      nrr[r] = k;
      for (index_type q=rr[rows[r]]; q<rr[rows[r]+1]; q++)
      {
        if (s[cc[q]] < n) 
        {
          ncc[k] = s[cc[q]];
          nvv[k] = vv[q];
          k++;
        }
      }
    }
    nrr[rows.size()] = k;

    output = new SparseRowMatrix(rows.size(), columns.size(), outputData, k);
    if (output.get_rep() == 0)
    {
      error("Could not allocate output matrix");
      algo_end(); return (false);          
    }
    
    algo_end(); return (true);
  }
  else
  {
    MatrixHandle mat = input->dense();
    
    if (mat.get_rep() == 0)
    {
      error("Could not convert matrix into dense matrix");
      algo_end(); return (false);    
    }
    
    
    output = new DenseMatrix(rows.size(),columns.size());
    if (output.get_rep() == 0)
    {
      error("Could not allocate output matrix");
      algo_end(); return (false);
    }
    
    double* src = mat->get_data_pointer();
    double* dst = output->get_data_pointer(); 
    
    if (dst==0 || src == 0)
    {
      error("Could not allocate output matrix");
      algo_end(); return (false);
    }
    
    size_type nr = static_cast<size_type>(rows.size());
    size_type nc = static_cast<size_type>(columns.size());
    
    for (index_type p=0;p<nr;p++)
    {
      for (index_type q=0;q<nc;q++)
      {
        dst[p*nc + q] = src[rows[p]*n +columns[q]];
      }
    }
    algo_end(); return (true);
  }
  */
  return DenseMatrixHandle();
}

AlgorithmInputName SelectSubMatrixAlgorithm::InputMatrix("InputMatrix");
AlgorithmOutputName SelectSubMatrixAlgorithm::RowIndicies("RowIndicies");
AlgorithmOutputName SelectSubMatrixAlgorithm::ColumnIndicies("ColumnIndicies");
AlgorithmParameterName SelectSubMatrixAlgorithm::rowCheckBox("rowCheckBox");
AlgorithmParameterName SelectSubMatrixAlgorithm::columnCheckBox("columnCheckBox");
AlgorithmParameterName SelectSubMatrixAlgorithm::rowStartSpinBox("rowStartSpinBox");
AlgorithmParameterName SelectSubMatrixAlgorithm::columnStartSpinBox("columnStartSpinBox");
AlgorithmParameterName SelectSubMatrixAlgorithm::columnEndSpinBox("columnEndSpinBox");
AlgorithmParameterName SelectSubMatrixAlgorithm::rowEndSpinBox("rowEndSpinBox");
    
AlgorithmOutput SelectSubMatrixAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto input_matrix = input.get<DenseMatrix>(InputMatrix);
  auto rowindicies = input.get<DenseMatrix>(RowIndicies);
  auto columnindicies = input.get<DenseMatrix>(ColumnIndicies);

  DenseMatrixHandle output_matrix;
  output_matrix = run(input_matrix, rowindicies, columnindicies);
  
  AlgorithmOutput output;
  output[Variables::ResultMatrix] = output_matrix;
  return output;
}

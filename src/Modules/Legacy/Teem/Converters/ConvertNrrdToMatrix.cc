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


/*
 *  ConvertNrrdToMatrix.cc: Converts Nrrd(s) to a SCIRun Matrix.  It may convert
 *                   it to a ColumnMatrix, DenseMatrix, or SparseMatrix
 *                   depending on which ports are connected.
 *
 *  Written by:
 *   Darby Van Uitert
 *   April 2004
 *
 */

#include <Modules/Legacy/Teem/Converters/ConvertNrrdToMatrix.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Teem;
using namespace SCIRun::Dataflow::Networks;
using namespace Core::Algorithms::Teem;
using namespace Core::Datatypes;

ALGORITHM_PARAMETER_DEF(Teem, MatrixType);
ALGORITHM_PARAMETER_DEF(Teem, SparseColumns);

const ModuleLookupInfo ConvertNrrdToMatrix::staticInfo_("ConvertNrrdToMatrix", "Converters", "Teem");

ConvertNrrdToMatrix::ConvertNrrdToMatrix() : Module(staticInfo_)
{
  INITIALIZE_PORT(Data);
  INITIALIZE_PORT(Rows);
  INITIALIZE_PORT(Columns);
  INITIALIZE_PORT(OutputMatrix);
}

void ConvertNrrdToMatrix::setStateDefaults()
{
  get_state()->setValue(Parameters::SparseColumns, 3);
}

void
ConvertNrrdToMatrix::execute()
{
  auto data = getOptionalInput(Data);
  auto rows = getOptionalInput(Rows);
  auto cols = getOptionalInput(Columns);

  if (needToExecute())
  {
    auto state = get_state();
    int colsForSparse = state->getValue(Parameters::SparseColumns).toInt();
    auto matrix = create_matrix_from_nrrds(data, rows, cols, colsForSparse);
    sendOutput(OutputMatrix, matrix);
  }
}

MatrixHandle
ConvertNrrdToMatrix::create_matrix_from_nrrds(boost::optional<NrrdDataHandle> dataHOpt,
boost::optional<NrrdDataHandle> rowsHOpt,
boost::optional<NrrdDataHandle> colsHOpt, int cols)
{
  // Determine if we have data, rows, columns to indicate whether it is
  // a dense or sparse matrix
  bool has_data = false, has_rows = false, has_cols = false;
  bool has_error = false;

  if (dataHOpt && *dataHOpt)
    has_data = true;
  if (rowsHOpt && *rowsHOpt)
    has_rows = true;
  if (colsHOpt && *colsHOpt)
    has_cols = true;

  MatrixHandle matrix;
  if (has_data && (!has_rows || !has_cols))
  {
    auto dataH = *dataHOpt;
    if (dataH->getNrrd()->dim == 1)
    {
      // column matrix
      switch (dataH->getNrrd()->type)
      {
      case nrrdTypeChar:
        matrix = create_column_matrix<char>(dataH);
        break;
      case nrrdTypeUChar:
        matrix = create_column_matrix<unsigned char>(dataH);
        break;
      case nrrdTypeShort:
        matrix = create_column_matrix<short>(dataH);
        break;
      case nrrdTypeUShort:
        matrix = create_column_matrix<unsigned short>(dataH);
        break;
      case nrrdTypeInt:
        matrix = create_column_matrix<int>(dataH);
        break;
      case nrrdTypeUInt:
        matrix = create_column_matrix<unsigned int>(dataH);
        break;
      case nrrdTypeFloat:
        matrix = create_column_matrix<float>(dataH);
        break;
      case nrrdTypeDouble:
        matrix = create_column_matrix<double>(dataH);
        break;
      default:
        error("Unknown nrrd type.");
        has_error = true;
        return nullptr;
      }
    }
    else if (dataH->getNrrd()->dim == 2) {
      // dense matrix
      switch (dataH->getNrrd()->type) {
      case nrrdTypeChar:
        matrix = create_dense_matrix<char>(dataH);
        break;
      case nrrdTypeUChar:
        matrix = create_dense_matrix<unsigned char>(dataH);
        break;
      case nrrdTypeShort:
        matrix = create_dense_matrix<short>(dataH);
        break;
      case nrrdTypeUShort:
        matrix = create_dense_matrix<unsigned short>(dataH);
        break;
      case nrrdTypeInt:
        matrix = create_dense_matrix<int>(dataH);
        break;
      case nrrdTypeUInt:
        matrix = create_dense_matrix<unsigned int>(dataH);
        break;
      case nrrdTypeFloat:
        matrix = create_dense_matrix<float>(dataH);
        break;
      case nrrdTypeDouble:
        matrix = create_dense_matrix<double>(dataH);
        break;
      default:
        error("Unknown nrrd type.");
        has_error = true;
        return nullptr;
      }
    }
    else {
      error("Can only convert data nrrds of 1 or 2D (Column or Dense Matrix).");
      has_error = true;
      return nullptr;
    }
  }
  else if (has_data && has_rows && has_cols) {
    // sparse matrix
    auto dataH = *dataHOpt;
    auto rowsH = *rowsHOpt;
    auto colsH = *colsHOpt;
    // rows and cols should be of type nrrdTypeInt
    if (rowsH->getNrrd()->type != nrrdTypeInt || colsH->getNrrd()->type != nrrdTypeInt) {
      error("Rows and Columns nrrds must both be of type nrrdTypeInt");
      has_error = true;
      return nullptr;
    }

    if (dataH->getNrrd()->dim != 1 || rowsH->getNrrd()->dim != 1 || colsH->getNrrd()->dim != 1) {
      error("All nrrds must be 1 dimension for a SparseRowMatrix.");
      has_error = true;
      return nullptr;
    }
    switch (dataH->getNrrd()->type) {
    case nrrdTypeChar:
      matrix = create_sparse_matrix<char>(dataH, rowsH, colsH, cols);
      break;
    case nrrdTypeUChar:
      matrix = create_sparse_matrix<unsigned char>(dataH, rowsH, colsH, cols);
      break;
    case nrrdTypeShort:
      matrix = create_sparse_matrix<short>(dataH, rowsH, colsH, cols);
      break;
    case nrrdTypeUShort:
      matrix = create_sparse_matrix<unsigned short>(dataH, rowsH, colsH, cols);
      break;
    case nrrdTypeInt:
      matrix = create_sparse_matrix<int>(dataH, rowsH, colsH, cols);
      break;
    case nrrdTypeUInt:
      matrix = create_sparse_matrix<unsigned int>(dataH, rowsH, colsH, cols);
      break;
    case nrrdTypeFloat:
      matrix = create_sparse_matrix<float>(dataH, rowsH, colsH, cols);
      break;
    case nrrdTypeDouble:
      matrix = create_sparse_matrix<double>(dataH, rowsH, colsH, cols);
      break;
    default:
      error("Unknown nrrd type.");
      has_error = true;
      return nullptr;
    }
  }
  else {
    error("Must have data to convert to any type of Matrix.  Must have rows and columns for a SparseRowMatrix.");
    has_error = true;
    return nullptr;
  }

  return matrix;
}

template<class PTYPE>
MatrixHandle
ConvertNrrdToMatrix::create_column_matrix(NrrdDataHandle dataH)
{
  remark("Creating column matrix");
  unsigned int rows = dataH->getNrrd()->axis[0].size;

  DenseMatrixHandle matrix(new DenseMatrix(rows, 1));

  PTYPE *val = reinterpret_cast<PTYPE*>(dataH->getNrrd()->data);
  double *data = matrix->data();

  std::copy(val, val + dataH->getNrrd()->axis[0].size, data);

  return matrix;
}

template<class PTYPE>
MatrixHandle
ConvertNrrdToMatrix::create_dense_matrix(NrrdDataHandle dataH)
{
  remark("Creating dense matrix");
  unsigned int rows = dataH->getNrrd()->axis[1].size;
  unsigned int cols = dataH->getNrrd()->axis[0].size;

  DenseMatrixHandle matrix(new DenseMatrix(rows, cols));

  PTYPE *val = reinterpret_cast<PTYPE*>(dataH->getNrrd()->data);
  double *data = matrix->data();

  for (unsigned int r = 0; r < rows; r++) {
    for (unsigned int c = 0; c < cols; c++) {
      *data = *val;
      ++data;
      ++val;
    }
  }
  return matrix;
}

template<class PTYPE>
MatrixHandle
ConvertNrrdToMatrix::create_sparse_matrix(NrrdDataHandle dataH, NrrdDataHandle rowsH,
NrrdDataHandle colsH, int cols)
{
  error("Nrrd to Sparse matrix not supported yet.");
  return nullptr;
#ifdef SCIRUN4_CODE_TO_BE_CONVERTED_LATER
  // TO DO: NEED TO FIX SCIRUN INDEX_TYPE TO NRRDTYPE CASTING AND CONVERTING
  remark("Creating sparse row matrix");
  Nrrd *data_n = dataH->getNrrd();
  Nrrd *rows_n = rowsH->getNrrd();
  Nrrd *cols_n = colsH->getNrrd();

  // pointers to nnrds
  PTYPE *data_d = (PTYPE*)data_n->data;
  index_type *rows_d = (index_type*)rows_n->data;
  index_type *cols_d = (index_type*)cols_n->data;

  if (cols == -1) {
    // Auto selected...attempt to determine number of columns
    for (index_type i = 0; i < static_cast<size_type>(cols_n->axis[0].size); i++)
    {
      if (cols_d[i] > cols) cols = cols_d[i];
    }
    cols += 1;
  }

  size_type rows = static_cast<size_type>(rows_n->axis[0].size - 1);
  size_type offset = 0;
  if (rows_d[0] != 0)
  {
    warning("First entry of rows nrrd must be a 0. Inserting 0 in first position.");
    offset = 1;
    rows++;
  }

  size_type nnz = static_cast<size_type>(data_n->axis[0].size);

  // error checking...

  // cols_n and dn should be of size nnz
  if (static_cast<size_type>(cols_n->axis[0].size) != nnz)
  {
    error("The Data and Columns nrrds should be the same size.");
    has_error_ = true;
    return 0;
  }

  // rows values must be in increasing order
  for (size_type i = 0; i < static_cast<size_type>(rows_n->axis[0].size - 1); i++)
  {
    if (rows_d[i] > rows_d[i + 1] || rows_d[i] < 0) {
      error("Rows nrrd must contain values in increasing order and positive.");
      has_error_ = true;
      return 0;
    }
  }

  // last rows value should be less than nnz
  if (rows_d[rows_n->axis[0].size - 1] > nnz) {
    error("The last entry in the rows array must be less than the number of non zeros.");
    has_error_ = true;
    return 0;
  }

  for (size_type i = 0; i < nnz; i++)
  {
    if (cols_d[i] < 0)
    {
      error("Columns nrrd must have positive values");
      has_error_ = true;
      return 0;
    }
  }

  // for each rows[N+1] - rows[N] sections of the cols array,
  // those values must be in increasing order
  for (index_type i = 0; i < static_cast<size_type>(rows_n->axis[0].size - 1); i++)
  {
    size_type span = rows_d[i + 1] - rows_d[i];
    for (index_type j = i; j < (i + span - 1); j++)
    {
      if (cols_d[j] > cols_d[j + 1])
      {
        error("Columns nrrd ordered incorrectly.");
        has_error_ = true;
        return 0;
      }
    }
  }

  SparseRowMatrix::Data sparseData(rows_n->axis[0].size + offset, nnz);
  const SparseRowMatrix::Rows& rr = sparseData.rows();
  const SparseRowMatrix::Columns& cc = sparseData.columns();
  const SparseRowMatrix::Storage& d = sparseData.data();

  // copy rest of rows
  if (offset == 1)
  {
    rr[0] = 0;
  }
  for (index_type i = 0; i < static_cast<size_type>(rows_n->axis[0].size); i++)
  {
    rr[i + offset] = rows_d[i];
  }

  // copy data and cols
  for (index_type i = 0; i < nnz; i++)
  {
    cc[i] = cols_d[i];
    d[i] = data_d[i];
  }

  return new SparseRowMatrix(rows, cols, sparseData, nnz);
#endif
}

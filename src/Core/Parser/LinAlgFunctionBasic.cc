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


//#include <Core/Algorithms/Math/SelectSubMatrix.h>
//#include <Core/Algorithms/Legacy/Math/SetSubMatrix/SetSubMatrix.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixMathVisitors.h>
#include <Core/Parser/LinAlgInterpreter.h>

#include <Core/Parser/LinAlgFunctionCatalog.h>

namespace LinAlgFunctions {

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Datatypes::MatrixMath;

//--------------------------------------------------------------------------
// Add functions

// Add scalar + scalar
bool add_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";
  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  // Special cases

  // Scalar + Matrix
  if (((*data1)->nrows() == 1) && ((*data1)->ncols() == 1) && ((*data1)->get_dense_size() == 1))
  {
    // scalar addition
    double val = (*data1)->get(0,0);

    auto data2PlusVal = convertMatrix::toDense(*data2)->array() + val;
    data0->reset(new DenseMatrix(data2PlusVal.matrix()));

    return (true);
  }


  // Matrix + Scalar
  if (((*data2)->nrows() == 1)&&((*data2)->ncols() == 1)&&((*data2)->get_dense_size() == 1))
  {
    // scalar addition
    double val = (*data2)->get(0,0);

    auto data1PlusVal = convertMatrix::toDense(*data1)->array() + val;
    data0->reset(new DenseMatrix(data1PlusVal.matrix()));

    return (true);
  }


  if ((*data1)->ncols() != (*data2)->ncols())
    { err = "Number of columns is not equal."; return (false); }
  if ((*data1)->nrows() != (*data2)->nrows())
    { err = "Number of rows is not equal."; return (false); }

  data0->reset((*data2)->clone());
  AddMatrices add(*data1);
  (*data0)->accept(add);
  *data0 = add.sum_;

  return *data0 != nullptr;
}

//--------------------------------------------------------------------------
// Sub functions

bool sub_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  // Special cases

  // Scalar - Matrix
  if (((*data1)->nrows() == 1)&&((*data1)->ncols() == 1)&&((*data1)->get_dense_size() == 1))
  {
    double val = (*data1)->get(0,0);

    auto valMinusData2 = val - convertMatrix::toDense(*data2)->array();
    data0->reset(new DenseMatrix(valMinusData2.matrix()));

    return (true);
  }


  // Matrix - Scalar
  if (((*data2)->nrows() == 1)&&((*data2)->ncols() == 1)&&((*data2)->get_dense_size() == 1))
  {
    double val = (*data2)->get(0, 0);

    auto data1MinusVal = convertMatrix::toDense(*data1)->array() - val;
    data0->reset(new DenseMatrix(data1MinusVal.matrix()));

    return (true);
  }


  if ((*data1)->ncols() != (*data2)->ncols())
    { err = "Number of columns is not equal."; return (false); }
  if ((*data1)->nrows() != (*data2)->nrows())
    { err = "Number of rows is not equal."; return (false); }

  data0->reset((*data2)->clone());
  NegateMatrix neg;
  (*data0)->accept(neg);
  AddMatrices add(*data1);
  (*data0)->accept(add);
  *data0 = add.sum_;

  return *data0 != nullptr;
}

//--------------------------------------------------------------------------
// Neg functions

bool neg_s(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);

  if (!(*data1)) return (false);
  if ((*data1)->empty()) return (false);

  data0->reset((*data1)->clone());
  NegateMatrix neg;
  (*data0)->accept(neg);

  return *data0 != nullptr;
}


//--------------------------------------------------------------------------
// Mult functions


bool mult_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  // Special cases

  // Scalar * Matrix
  if (((*data1)->nrows() == 1)&&((*data1)->ncols() == 1)&&((*data1)->get_dense_size() == 1))
  {
    double val = (*data1)->get(0, 0);

    auto data2TimesVal = convertMatrix::toDense(*data2)->array() * val;
    data0->reset(new DenseMatrix(data2TimesVal.matrix()));

    return (true);
  }


  // Matrix * Scalar
  if (((*data2)->nrows() == 1)&&((*data2)->ncols() == 1)&&((*data2)->get_dense_size() == 1))
  {
    double val = (*data2)->get(0, 0);

    auto data1TimesVal = convertMatrix::toDense(*data1)->array() * val;
    data0->reset(new DenseMatrix(data1TimesVal.matrix()));

    return (true);
  }


  if ((*data1)->ncols() != (*data2)->nrows())
  {
    err = "Number of columns is not equal to number of rows.";
    return (false);
  }

  data0->reset((*data2)->clone());
  MultiplyMatrices mult(*data1);
  (*data0)->accept(mult);
  *data0 = mult.getProduct();

  return *data0 != nullptr;
}


bool mmult_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  // Special cases

  if (((*data1)->nrows() != (*data2)->nrows())||((*data1)->ncols() != (*data2)->ncols()))
  {
    err = "Matrix sizes are not equal for .* operator";
    return (false);
  }

  if ((matrixIs::dense(*data1)||matrixIs::column(*data1)) &&
      (matrixIs::dense(*data2)||matrixIs::column(*data2)))
  {
    auto cwiseMult = convertMatrix::toDense(*data1)->cwiseProduct(*convertMatrix::toDense(*data2));
    data0->reset(new DenseMatrix(cwiseMult.matrix()));
  }
  else
  {
    err = ".* operator has not yet been implemented for sparse matrices";
    return (false);
  }

  return *data0 != nullptr;
}


//--------------------------------------------------------------------------
// Div functions


bool div_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  if (((*data1)->nrows() == 1)&&((*data1)->ncols() == 1)&&((*data1)->get_dense_size() == 1)&&
      ((*data2)->nrows() == 1)&&((*data2)->ncols() == 1)&&((*data2)->get_dense_size() == 1))
  {
    double val1 = (*data1)->get(0,0);
    double val2 = (*data2)->get(0,0);
    data0->reset(new DenseMatrix(1, 1, val1/val2));
    return (true);
  }

  err = "Division is currently only implemented for scalars.";
  return (false);
}

bool mdiv_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  // Special cases

  if (((*data1)->nrows() != (*data2)->nrows())||((*data1)->ncols() != (*data2)->ncols()))
  {
    err = "Matrix sizes are not equal for ./ operator";
    return (false);
  }

  data0->reset((*data1)->clone());
  if (!(*data0)) return (false);

  if ((matrixIs::dense(*data1)||matrixIs::column(*data1)) &&
      (matrixIs::dense(*data2)||matrixIs::column(*data2)))
  {
    auto cwiseDiv = convertMatrix::toDense(*data1)->cwiseQuotient(*convertMatrix::toDense(*data2));
    data0->reset(new DenseMatrix(cwiseDiv.matrix()));
  }
  else
  {
    err = "./ operator has not yet been implemented for sparse matrices";
    return (false);
  }

  return *data0 != nullptr;
}



//--------------------------------------------------------------------------
// Rem functions

bool rem_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  // Special cases

  // Matrix * Scalar
  if (((*data1)->nrows() == 1)&&((*data1)->ncols() == 1)&&((*data1)->get_dense_size() == 1)&&
      ((*data2)->nrows() == 1)&&((*data2)->ncols() == 1)&&((*data2)->get_dense_size() == 1))
  {
    // scalar addition
    double val1 = (*data1)->get(0,0);
    double val2 = (*data2)->get(0,0);
    data0->reset(new DenseMatrix(1, 1, fmod(val1,val2)));
    return (true);
  }

  err = "Remainder is currently only implemented for scalars.";
  return (false);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool mrem_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  // Special cases

  if (((*data1)->nrows() != (*data2)->nrows())||((*data1)->ncols() != (*data2)->ncols()))
  {
    err = "Matrix sizes are not equal for .% operator";
    return (false);
  }

  data0->reset((*data1)->clone());
  if (!(*data0)) return (false);

  if ((matrixIs::dense(*data1)||matrixIs::column(*data1)) &&
      (matrixIs::dense(*data2)||matrixIs::column(*data2)))
  {
    double* data = (*data0)->get_data_pointer();
    double* ptr1 = (*data1)->get_data_pointer();
    double* ptr2 = (*data2)->get_data_pointer();
    size_type size = (*data0)->get_dense_size();
    double* data_end = data+size;

    while (data != data_end)
    {
      *data = fmod(*ptr1,*ptr2);
      data++; ptr1++; ptr2++;
    }

    auto cwiseMod = convertMatrix::toDense(*data1)->array() % convertMatrix::toDense(*data2)->array();
    data0->reset(new DenseMatrix(cwiseMod.matrix()));
  }
  else
  {
    err = ".% operator has not yet been implemented for sparse matrices";
    return (false);
  }

  return *data0 != nullptr;
}
#endif

bool densematrix_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  if ((*data1)->get_dense_size() != 1)
  {
    err = "Create Dense matrix: number of rows needs to be a scalar.";
    return (false);
  }
  double d1 = (*data1)->get(0, 0);

  if ((*data2)->get_dense_size() != 1)
  {
    err = "Create Dense matrix: number of columns needs to be a scalar.";
    return (false);
  }
  double d2 = (*data1)->get(0, 0);

  data0->reset(new DenseMatrix(static_cast<size_type>(d1),static_cast<size_type>(d2)));

  return (true);
}

bool ones_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  if ((*data1)->get_dense_size() != 1)
  {
    err = "Create matrix: number of rows needs to be a scalar.";
    return (false);
  }
  double d1 = (*data1)->get(0, 0);

  if ((*data2)->get_dense_size() != 1)
  {
    err = "Create matrix: number of columns needs to be a scalar.";
    return (false);
  }
  double d2 = (*data1)->get(0, 0);

  data0->reset(new DenseMatrix(static_cast<size_type>(d1),static_cast<size_type>(d2), 1.0));

  return (true);
}

bool zeros_ss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);

  if ((*data1)->get_dense_size() != 1)
  {
    err = "Create matrix: number of rows needs to be a scalar.";
    return (false);
  }
  double d1 = (*data1)->get(0, 0);

  if ((*data2)->get_dense_size() != 1)
  {
    err = "Create matrix: number of columns needs to be a scalar.";
    return (false);
  }
  double d2 = (*data1)->get(0, 0);

  data0->reset(new DenseMatrix(static_cast<size_type>(d1),static_cast<size_type>(d2), 0.0));

  return (true);
}



//--------------------------------------------------------------------------
// Select functions
bool select_sss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);
  MatrixHandle* data3 = pc.get_handle(3);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);
  if (!(*data3)) return (false);

  if ((*data1)->empty()) return (false);

  if ((*data1)->get(0,0))
  {
    *data0 = (*data2);
  }
  else
  {
    *data0 = (*data3);
  }

  return *data0 != nullptr;
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

//--------------------------------------------------------------------------
// Sub functions
bool subs_sss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);
  MatrixHandle* data3 = pc.get_handle(3);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);
  if (!(*data3)) return (false);

  SelectSubMatrixAlgorithm algo;
  if(!(algo.run(*data1,*data0,*data2,*data3)))
  {
    err = "Invalid matrix indices for selecting submatrix.";
    return (false);
  }

  return *data0 != nullptr;
}


bool subs_range_sssss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);
  MatrixHandle* data3 = pc.get_handle(3);
  MatrixHandle* data4 = pc.get_handle(4);
  MatrixHandle* data5 = pc.get_handle(5);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);
  if (!(*data3)) return (false);
  if (!(*data4)) return (false);
  if (!(*data5)) return (false);

  if ((*data2)->get_dense_size() != 1)
  {
    err = "Row index needs to be a scalar.";
    return (false);
  }

  if ((*data3)->get_dense_size() != 1)
  {
    err = "Row index needs to be a scalar.";
    return (false);
  }

  if ((*data4)->get_dense_size() != 1)
  {
    err = "Column index needs to be a scalar.";
    return (false);
  }

  if ((*data5)->get_dense_size() != 1)
  {
    err = "Column index needs to be a scalar.";
    return (false);
  }

  index_type rstart = static_cast<index_type>((*data2)->get(0,0));
  index_type rend = static_cast<index_type>((*data3)->get(0,0));

  index_type cstart = static_cast<index_type>((*data4)->get(0,0));
  index_type cend = static_cast<index_type>((*data5)->get(0,0));

  std::vector<index_type> rows;
  std::vector<index_type> columns;

  if (rstart <= rend)
  {
    rows.resize(rend-rstart+1);
    for (size_t j=0;j<rows.size();j++) rows[j] = rstart+j;
  }
  else
  {
    rows.resize(rstart-rend+1);
    for (size_t j=0;j<rows.size();j++) rows[j] = rstart-j;
  }

  if (cstart <= cend)
  {
    columns.resize(cend-cstart+1);
    for (size_t j=0;j<columns.size();j++) columns[j] = cstart+j;
  }
  else
  {
    columns.resize(cstart-cend+1);
    for (size_t j=0;j<columns.size();j++) columns[j] = cstart-j;
  }

  SelectSubMatrixAlgorithm algo;
  if(!(algo.run(*data1,*data0,rows,columns)))
  {
    err = "Invalid matrix indices for selecting submatrix.";
    return (false);
  }

  return *data0 != nullptr;
}

//--------------------------------------------------------------------------
// Sub functions
bool assign_ssss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);
  MatrixHandle* data3 = pc.get_handle(3);
  MatrixHandle* data4 = pc.get_handle(4);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);
  if (!(*data3)) return (false);
  if (!(*data4)) return (false);

  SCIRunAlgo::SetSubMatrixAlgo algo;
  if(!(algo.run(*data1,*data2,*data0,*data3,*data4)))
  {
    err = "Invalid matrix indices for setting submatrix.";
    return (false);
  }

  return *data0 != nullptr;
}


bool assign_range_ssssss(SCIRun::LinAlgProgramCode& pc, std::string& err)
{
  err = "";

  MatrixHandle* data0 = pc.get_handle(0);
  MatrixHandle* data1 = pc.get_handle(1);
  MatrixHandle* data2 = pc.get_handle(2);
  MatrixHandle* data3 = pc.get_handle(3);
  MatrixHandle* data4 = pc.get_handle(4);
  MatrixHandle* data5 = pc.get_handle(5);
  MatrixHandle* data6 = pc.get_handle(6);

  if (!(*data1)) return (false);
  if (!(*data2)) return (false);
  if (!(*data3)) return (false);
  if (!(*data4)) return (false);
  if (!(*data5)) return (false);
  if (!(*data6)) return (false);

  if ((*data3)->get_dense_size() != 1)
  {
    err = "Row index needs to be a scalar.";
    return (false);
  }

  if ((*data4)->get_dense_size() != 1)
  {
    err = "Row index needs to be a scalar.";
    return (false);
  }

  if ((*data5)->get_dense_size() != 1)
  {
    err = "Column index needs to be a scalar.";
    return (false);
  }

  if ((*data6)->get_dense_size() != 1)
  {
    err = "Column index needs to be a scalar.";
    return (false);
  }

  index_type rstart = static_cast<index_type>((*data3)->get(0,0));
  index_type rend = static_cast<index_type>((*data4)->get(0,0));

  index_type cstart = static_cast<index_type>((*data5)->get(0,0));
  index_type cend = static_cast<index_type>((*data6)->get(0,0));

  std::vector<index_type> rows;
  std::vector<index_type> columns;

  if (rstart <= rend)
  {
    rows.resize(rend-rstart+1);
    for (size_t j=0;j<rows.size();j++) rows[j] = rstart+j;
  }
  else
  {
    rows.resize(rstart-rend+1);
    for (size_t j=0;j<rows.size();j++) rows[j] = rstart-j;
  }

  if (cstart <= cend)
  {
    columns.resize(cend-cstart+1);
    for (size_t j=0;j<columns.size();j++) columns[j] = cstart+j;
  }
  else
  {
    columns.resize(cstart-cend+1);
    for (size_t j=0;j<columns.size();j++) columns[j] = cstart-j;
  }

  SCIRunAlgo::SetSubMatrixAlgo algo;
  if(!(algo.run(*data1,*data2, *data0,rows,columns)))
  {
    err = "Invalid matrix indices for selecting submatrix.";
    return (false);
  }

  return *data0 != nullptr;
}
#endif







} // end namsespace

namespace SCIRun {

void
InsertBasicLinAlgFunctionCatalog(LinAlgFunctionCatalogHandle& catalog)
{
  // Add add functions to database
  catalog->add_sym_function(LinAlgFunctions::add_ss,"add$S:S","S");

  // Add sub functions to database
  catalog->add_function(LinAlgFunctions::sub_ss,"sub$S:S","S");

  // Add neg function to database
  catalog->add_function(LinAlgFunctions::neg_s,"neg$S","S");

  // Add mult functions to database
  catalog->add_function(LinAlgFunctions::mult_ss,"mult$S:S","S");
  catalog->add_sym_function(LinAlgFunctions::mmult_ss,"mmult$S:S","S");

  // Add div functions to database
  catalog->add_function(LinAlgFunctions::div_ss,"div$S:S","S");
  catalog->add_function(LinAlgFunctions::mdiv_ss,"mdiv$S:S","S");

  // Add rem functions to database
  catalog->add_function(LinAlgFunctions::rem_ss,"rem$S:S","S");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  catalog->add_function(LinAlgFunctions::mrem_ss,"mrem$S:S","S");
#endif

  catalog->add_function(LinAlgFunctions::select_sss,"select$S:S:S","S");

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  // Get sub matrix
  catalog->add_function(LinAlgFunctions::subs_sss,"subs$S:S:S","S");
  catalog->add_function(LinAlgFunctions::subs_range_sssss,"subs_range$S:S:S:S:S","S");
  catalog->add_function(LinAlgFunctions::assign_ssss,"assign$S:S:S:S","S");
  catalog->add_function(LinAlgFunctions::assign_range_ssssss,"assign_range$S:S:S:S:S:S","S");
#endif

  // For creating empty matrices
  catalog->add_function(LinAlgFunctions::densematrix_ss,"matrix$S:S","S");
  catalog->add_function(LinAlgFunctions::densematrix_ss,"Matrix$S:S","S");
  catalog->add_function(LinAlgFunctions::densematrix_ss,"densematrix$S:S","S");
  catalog->add_function(LinAlgFunctions::densematrix_ss,"DenseMatrix$S:S","S");

  catalog->add_function(LinAlgFunctions::ones_ss,"ones$S:S","S");
  catalog->add_function(LinAlgFunctions::zeros_ss,"zeros$S:S","S");

}

} // end namespace

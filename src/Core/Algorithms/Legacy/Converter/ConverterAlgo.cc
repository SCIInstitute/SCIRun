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


#include <Core/Algorithms/Legacy/Converter/ConverterAlgo.h>


#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

//#include <Core/Datatypes/MatrixTypeConverter.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Legacy/Converter/NrrdToField.h>
#include <Core/Algorithms/Legacy/Converter/FieldToNrrd.h>
//#include <Core/Algorithms/Converter/MatrixToField.h>
//#include <Core/Algorithms/Converter/MatricesToDipoleField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;

ConverterAlgo::ConverterAlgo(LoggerHandle pr) :
pr_(pr)
{
}

#ifdef SCIRUN4_CODE_TO_BE_CONVERTED_LATER
bool ConverterAlgo::MatrixToDoubleVector(MatrixHandle matrix, std::vector<double> &val)
{
  if (matrix.get_rep() == 0)
  {
    val.resize(0);
    return false;
  }

  val.resize(matrix->nrows() * matrix->ncols());

  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToDoubleVector: Matrix could not be translated into a dense matrix");
    return false;
  }
  double* data = mat->get_data_pointer();

  for (size_t r = 0; r < val.size(); r++)
    val[r] = static_cast<double>(data[r]);

  return true;
}


bool ConverterAlgo::MatrixToIntVector(MatrixHandle matrix, std::vector<int> &val)
{
  if (matrix.get_rep() == 0)
  {
    val.resize(0);
    return false;
  }

  val.resize(matrix->nrows() * matrix->ncols());

  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToIntVector: Matrix could not be translated into a dense matrix");
    return false;
  }
  double* data = mat->get_data_pointer();

  for (size_t r = 0; r < val.size(); r++)
    val[r] = static_cast<int>(data[r]);

  return true;
}


bool ConverterAlgo::MatrixToIndexVector(MatrixHandle matrix, std::vector<index_type> &val)
{
  if (matrix.get_rep() == 0)
  {
    val.resize(0);
    return false;
  }

  val.resize(matrix->nrows() * matrix->ncols());

  MatrixHandle mat = (matrix->dense());
  if (mat.get_rep() == 0)
  {
    error("MatrixToIntVector: Matrix could not be translated into a dense matrix");
    return false;
  }
  double* data = mat->get_data_pointer();

  for (size_t r = 0; r < val.size(); r++)
    val[r] = static_cast<index_type>(data[r]);

  return true;
}



bool ConverterAlgo::MatrixToUnsignedIntVector(MatrixHandle matrix, std::vector<unsigned int> &val)
{
  if (matrix.get_rep() == 0)
  {
    val.resize(0);
    return false;
  }

  val.resize(matrix->nrows() * matrix->ncols());

  MatrixHandle mat = (matrix->dense());
  if (mat.get_rep() == 0)
  {
    error("MatrixToIntVector: Matrix could not be translated into a dense matrix");
    return false;
  }
  double* data = mat->get_data_pointer();

  for (size_t r = 0; r < val.size(); r++)
    val[r] = static_cast<unsigned int>(data[r]);

  return true;
}



bool ConverterAlgo::MatrixToDouble(MatrixHandle matrix, double &val)
{
  if (matrix.get_rep() == 0)
    return false;

  if ((matrix->nrows() * matrix->ncols()) != 1)
  {
    error("MatrixToDouble: Matrix has not dimensions 1 x 1");
    return false;
  }

  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToDouble: Matrix could not be translated into a dense matrix");
    return false;
  }

  val = mat->get(0, 0);
  return true;
}


bool ConverterAlgo::MatrixToInt(MatrixHandle matrix, int &val)
{
  if (matrix.get_rep() == 0)
    return false;

  if ((matrix->nrows() * matrix->ncols()) != 1)
  {
    error("MatrixToInt: Matrix has not dimensions 1 x 1");
    return false;
  }

  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToInt: Matrix could not be translated into a dense matrix");
    return false;
  }

  double temp = mat->get(0, 0);
  val = static_cast<int>(temp);

  if ((temp - static_cast<double>(val)) != 0.0)
  {
    warning("MatrixToInt: Value in matrix is not of integer value, rounding value to nearest integer value");
  }
  return true;
}

bool ConverterAlgo::MatrixToIndex(MatrixHandle matrix, index_type &idx)
{
  if (matrix.get_rep() == 0)
    return false;

  if ((matrix->nrows() * matrix->ncols()) != 1)
  {
    error("MatrixToInt: Matrix has not dimensions 1 x 1");
    return false;
  }

  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToInt: Matrix could not be translated into a dense matrix");
    return false;
  }

  double temp = mat->get(0, 0);
  idx = static_cast<index_type>(temp);

  return true;
}



bool ConverterAlgo::MatrixToUnsignedInt(MatrixHandle matrix, unsigned int &val)
{
  if (matrix.get_rep() == 0)
    return false;

  if ((matrix->nrows() * matrix->ncols()) != 1)
  {
    error("MatrixToUnsignedInt: Matrix has not dimensions 1 x 1");
    return false;
  }

  MatrixHandle mat = (matrix->dense());
  if (mat.get_rep() == 0)
  {
    error("MatrixToUnsignedInt: Matrix could not be translated into a dense matrix");
    return false;
  }

  double temp = mat->get(0, 0);
  val = static_cast<unsigned int>(temp);

  if ((temp - static_cast<double>(val)) != 0.0)
  {
    warning("MatrixToUnsignedInt: Value in matrix is not of integer value, rounding value to nearest integer value");
  }
  return true;
}

bool ConverterAlgo::MatrixToVector(MatrixHandle matrix, Vector& vec)
{
  if (matrix.get_rep() == 0)
    return false;

  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToVector: Matrix could not be translated into a dense matrix");
    return false;
  }

  double* data = mat->get_data_pointer();
  if (data == 0)
  {
    error("MatrixToVector: Could not access the matrix data");
    return false;
  }

  if ((mat->nrows() * mat->ncols()) == 1)
  {
    vec = Vector(data[0], data[0], data[0]);
    return true;
  }

  if ((mat->nrows() * mat->ncols()) == 2)
  {
    vec = Vector(data[0], data[1], data[1]);
    return true;
  }

  if ((mat->nrows() * mat->ncols()) == 3)
  {
    vec = Vector(data[0], data[1], data[2]);
    return true;
  }

  error("MatrixToVector: Improper matrix dimensions");
  return false;
}



bool ConverterAlgo::MatrixToPoint(MatrixHandle matrix, Point& point)
{
  if (matrix.get_rep() == 0)
    return false;

  MatrixHandle mat = (matrix->dense());
  if (mat.get_rep() == 0)
  {
    error("MatrixToVector: Matrix could not be translated into a dense matrix");
    return false;
  }

  double* data = mat->get_data_pointer();
  if (data == 0)
  {
    error("MatrixToVector: Could not access the matrix data");
    return false;
  }

  if ((mat->nrows() * mat->ncols()) == 1)
  {
    point = Point(data[0], data[0], data[0]);
    return true;
  }

  if ((mat->nrows() * mat->ncols()) == 2)
  {
    point = Point(data[0], data[1], data[1]);
    return true;
  }

  if ((mat->nrows() * mat->ncols()) == 3)
  {
    point = Point(data[0], data[1], data[2]);
    return true;
  }

  error("MatrixToVector: Improper matrix dimensions");
  return false;
}



bool ConverterAlgo::MatrixToTensor(MatrixHandle matrix, Tensor& ten)
{
  if (matrix.get_rep() == 0)
    return false;

  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToTensor: Matrix could not be translated into a dense matrix");
    return false;
  }

  double* data = mat->get_data_pointer();
  if (data == 0)
  {
    error("MatrixToTensor: Could not access the matrix data");
    return false;
  }

  if ((mat->nrows() * mat->ncols()) == 1)
  {
    ten = data[0];
    return true;
  }

  if (((mat->nrows() == 1) && (mat->ncols() == 6)) ||
    ((mat->nrows() == 6) && (mat->ncols() == 1)))
  {
    ten = Tensor(data);
    return true;
  }

  if (((mat->nrows() == 1) && (mat->ncols() == 9)) ||
    ((mat->nrows() == 9) && (mat->ncols() == 1)) ||
    ((mat->nrows() == 3) && (mat->ncols() == 3)))
  {
    double tdata[6];
    tdata[0] = data[0]; tdata[1] = data[1]; tdata[2] = data[2];
    tdata[3] = data[4];  tdata[4] = data[5]; tdata[5] = data[8];
    ten = Tensor(tdata);
  }

  error("MatrixToTensor: Improper matrix dimensions");
  return false;
}

bool ConverterAlgo::MatrixToTransform(MatrixHandle matrix, Transform& trans)
{
  MatrixHandle mat = matrix->dense();
  if (mat.get_rep() == 0)
  {
    error("MatrixToTransform: Matrix could not be translated into a dense matrix");
    return false;
  }

  double* data = mat->get_data_pointer();
  if (data == 0)
  {
    error("MatrixToTransform: Could not access the matrix data");
    return false;
  }

  if ((mat->nrows() * mat->ncols()) == 1)
  {
    trans.load_identity();
    trans.post_scale(Vector(data[0], data[0], data[0]));
    return true;
  }

  if (((mat->nrows() == 1) && (mat->ncols() == 16)) ||
    ((mat->nrows() == 16) && (mat->ncols() == 1)) ||
    ((mat->nrows() == 4) && (mat->ncols() == 4)))
  {
    trans.set(data);
    return true;
  }

  error("MatrixToTransform: Improper matrix dimensions");
  return false;
}


bool ConverterAlgo::DoubleVectorToMatrix(std::vector<double> val, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(val.size(), 1);
  if (matrix.get_rep() == 0)
  {
    error("DoubleToMatrix: Could not allocate memory");
    return false;
  }
  double* data = matrix->get_data_pointer();
  for (size_t r = 0; r < val.size(); r++)
    data[r] = static_cast<double>(val[r]);

  return true;
}

bool ConverterAlgo::IntVectorToMatrix(std::vector<int> val, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(val.size(), 1);
  if (matrix.get_rep() == 0)
  {
    error("DoubleToMatrix: Could not allocate memory");
    return false;
  }
  double* data = matrix->get_data_pointer();
  for (size_t r = 0; r < val.size(); r++)
    data[r] = static_cast<double>(val[r]);

  return true;
}

bool ConverterAlgo::UnsignedIntVectorToMatrix(std::vector<unsigned int> val, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(val.size(), 1);
  if (matrix.get_rep() == 0)
  {
    error("DoubleToMatrix: Could not allocate memory");
    return false;
  }
  double* data = matrix->get_data_pointer();
  for (size_t r = 0; r < val.size(); r++)
    data[r] = static_cast<double>(val[r]);

  return true;
}


bool ConverterAlgo::IndexVectorToMatrix(std::vector<index_type> val, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(val.size(), 1);
  if (matrix.get_rep() == 0)
  {
    error("DoubleToMatrix: Could not allocate memory");
    return false;
  }
  double* data = matrix->get_data_pointer();
  for (size_t r = 0; r < val.size(); r++)
    data[r] = static_cast<double>(val[r]);

  return true;
}


bool ConverterAlgo::DoubleToMatrix(double val, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(1, 1);
  if (matrix.get_rep() == 0)
  {
    error("DoubleToMatrix: Could not allocate memory");
    return false;
  }
  matrix->put(0, 0, val);
  return true;
}

bool ConverterAlgo::IntToMatrix(int val, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(1, 1);
  if (matrix.get_rep() == 0)
  {
    error("IntToMatrix: Could not allocate memory");
    return false;
  }
  matrix->put(0, 0, static_cast<double>(val));
  return true;
}

bool ConverterAlgo::IndexToMatrix(index_type idx, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(1, 1);
  if (matrix.get_rep() == 0)
  {
    error("IntToMatrix: Could not allocate memory");
    return false;
  }
  matrix->put(0, 0, static_cast<double>(idx));
  return true;
}

bool ConverterAlgo::UnsignedIntToMatrix(unsigned int val, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(1, 1);
  if (matrix.get_rep() == 0)
  {
    error("IntToMatrix: Could not allocate memory");
    return false;
  }
  matrix->put(0, 0, static_cast<double>(val));
  return true;
}

bool ConverterAlgo::VectorToMatrix(Vector& vec, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(3, 1);
  if (matrix.get_rep() == 0)
  {
    error("VectorToMatrix: Could not allocate memory");
    return false;
  }
  matrix->put(0, 0, vec.x());
  matrix->put(1, 0, vec.y());
  matrix->put(2, 0, vec.z());
  return true;
}


bool ConverterAlgo::PointToMatrix(Point& point, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(3, 1);
  if (matrix.get_rep() == 0)
  {
    error("VectorToMatrix: Could not allocate memory");
    return false;
  }
  matrix->put(0, 0, point.x());
  matrix->put(1, 0, point.y());
  matrix->put(2, 0, point.z());
  return true;
}

bool ConverterAlgo::TensorToMatrix(Tensor& ten, MatrixHandle matrix)
{
  matrix = new DenseMatrix(3, 3);
  if (matrix.get_rep() == 0)
  {
    error("TensorToMatrix: Could not allocate memory");
    return false;
  }
  matrix->put(0, 0, ten.mat_[0][0]);
  matrix->put(1, 0, ten.mat_[1][0]);
  matrix->put(2, 0, ten.mat_[2][0]);
  matrix->put(0, 1, ten.mat_[0][1]);
  matrix->put(1, 1, ten.mat_[1][1]);
  matrix->put(2, 1, ten.mat_[2][1]);
  matrix->put(0, 2, ten.mat_[0][2]);
  matrix->put(1, 2, ten.mat_[1][2]);
  matrix->put(2, 2, ten.mat_[2][2]);

  return true;
}

bool ConverterAlgo::TransformToMatrix(Transform& trans, MatrixHandle& matrix)
{
  matrix = new DenseMatrix(4, 4);
  if (matrix.get_rep() == 0)
  {
    error("DoubleToMatrix: Could not allocate memory");
    return false;
  }

  double *dptr;
  double sptr[16];

  trans.get(sptr);
  dptr = matrix->get_data_pointer();
  for (int p = 0; p < 16; p++)
    dptr[p] = sptr[p];

  return true;
}


bool ConverterAlgo::MatricesToDipoleField(MatrixHandle locations, MatrixHandle strengths, FieldHandle& Dipoles)
{
  MatricesToDipoleFieldAlgo algo;
  return(algo.MatricesToDipoleField(pr_, locations, strengths, Dipoles));
}


bool ConverterAlgo::MatrixToField(MatrixHandle input, FieldHandle& output, const std::string& datalocation)
{
  MatrixToFieldAlgo algo;
  return(algo.MatrixToField(pr_, input, output, datalocation));
}
#endif

bool ConverterAlgo::nrrdToField(NrrdDataHandle input, FieldHandle& output, const std::string& datalocation, const std::string& fieldtype, const std::string& convertparity)
{
  NrrdToFieldAlgo algo;
  return(algo.nrrdToField(pr_, input, output, datalocation, fieldtype, convertparity));
}

bool ConverterAlgo::fieldToNrrd(FieldHandle input, NrrdDataHandle& output)
{
  FieldToNrrdAlgo algo;
  return(algo.fieldToNrrd(pr_, input, output));
}

#ifdef SCIRUN4_CODE_TO_BE_CONVERTED_LATER
bool ConverterAlgo::NrrdToMatrix(NrrdDataHandle input, MatrixHandle& output)
{
  if (!(input.get_rep()))
  {
    error("NrrdToMatrix: No input Nrrd was given");
    return false;
  }

  Nrrd* nrrd = input->nrrd_;

  if (nrrd == 0)
  {
    error("NrrdToMatrix: NrrdData does not contain Nrrd");
    return false;
  }

  if (nrrd->dim > 2)
  {
    error("NrrdToMatrix: Nrrd has a dimension larger than 2 which cannot be stored into a matrix");
    return false;
  }

  if (nrrd->dim < 1)
  {
    error("NrrdToMatrix: Nrrd dimension is zero");
    return false;
  }

  int m = 1;
  int n = 1;
  if (nrrd->dim > 0)
    m = nrrd->axis[0].size;

  if (nrrd->dim > 1)
    n = nrrd->axis[1].size;

  output = new DenseMatrix(n, m);

  if (output.get_rep() == 0)
  {
    error("NrrdToMatrix: Could not allocate the output Matrix");
    return false;
  }
  double* data = output->get_data_pointer();

  int size = n*m;

  switch (nrrd->type)
  {
  case nrrdTypeChar:
  {
    char *ptr = reinterpret_cast<char *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  case nrrdTypeUChar:
  {
    unsigned char *ptr = reinterpret_cast<unsigned char *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  case nrrdTypeShort:
  {
    short *ptr = reinterpret_cast<short *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  case nrrdTypeUShort:
  {
    unsigned short *ptr = reinterpret_cast<unsigned short *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  case nrrdTypeInt:
  {
    int *ptr = reinterpret_cast<int *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  case nrrdTypeUInt:
  {
    unsigned int *ptr = reinterpret_cast<unsigned int *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  case nrrdTypeFloat:
  {
    float *ptr = reinterpret_cast<float *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  case nrrdTypeDouble:
  {
    double *ptr = reinterpret_cast<double *>(nrrd->data);
    for (int p = 0; p < size; p++) { data[p] = static_cast<double>(ptr[p]); }
  }
  break;
  default:
  {
    error("NrrdToMatrix: Unknown Nrrd type");
    return false;
  }
  }

  return true;
}
#endif

bool ConverterAlgo::MatrixToString(Datatypes::MatrixHandle input, Datatypes::StringHandle& output)
{
  std::ostringstream oss;

  if (matrixIs::sparse(input))
  {
    SparseRowMatrixHandle sparse = convertMatrix::toSparse(input);
    size_type numRows = sparse->nrows();
    size_type numCols = sparse->ncols();

    oss << "Sparse Matrix (" << numRows << "x" << numCols << "):\n";
    for (index_type r = 0; r < numRows; r++)
    {
      for (index_type c = 0; c < numCols; c++)
      {
        oss << "[" << r << "," << c << "] = " << sparse->get(r, c) << "\n";
      }
    }
  }

  else
  {
    input = convertMatrix::toDense(input);
    size_type numRows = input->nrows();
    size_type numCols = input->ncols();
    oss << "Dense Matrix (" << numRows << "x" << numCols << "):\n";

    for (index_type r = 0; r < numRows; r++)
    {
      for (index_type c = 0; c < numCols; c++)
      {
        oss << input->get(r, c) << " ";
      }
      oss << "\n";
    }
  }

  output = boost::make_shared<String>(oss.str());

  return true;
}


AlgorithmOutput ConverterAlgo::run(const AlgorithmInput& input) const
{
  throw "not implemented";
}

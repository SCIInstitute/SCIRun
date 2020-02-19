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
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

#include <Core/Parser/ArrayMathInterpreter.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>

#include <Core/Thread/Mutex.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;

namespace ArrayMathFunctions {

//--------------------------------------------------------------------------
// Source functions

// Field data to buffer
bool get_scalar_fd(ArrayMathProgramCode& pc)
{
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0);
  // Get the field where the data is coming from
  VField* data1 = pc.get_vfield(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check (this one is inline, hence it should be fast)
  if (!(data1->is_scalar())) return (false);

  double* data0_end = data0 + pc.get_size();
  VMesh::index_type idx = pc.get_index();

  double val;
  while(data0 != data0_end)
  {
    data1->get_value(val,idx); idx++;
    *data0 = val; data0++;
  }

  return (true);
}

bool get_vector_fd(ArrayMathProgramCode& pc)
{
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0);
  // Get the field where the data is coming from
  VField* data1 = pc.get_vfield(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check (this one is inline, hence it should be fast)
  if (!(data1->is_vector())) return (false);


  double* data0_end = data0 + 3*(pc.get_size());
  VMesh::index_type idx = pc.get_index();

  Vector val;
  while (data0 != data0_end)
  {
    data1->get_value(val,idx); idx++;
    *data0 = val.x(); data0++;
    *data0 = val.y(); data0++;
    *data0 = val.z(); data0++;
  }

  return (true);
}

bool get_tensor_fd(ArrayMathProgramCode& pc)
{
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0);
  // Get the field where the data is coming from
  VField* data1 = pc.get_vfield(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check (this one is inline, hence it should be fast)
  if (!(data1->is_tensor())) return (false);

  double* data0_end = data0 + 6*(pc.get_size());
  VMesh::index_type idx = pc.get_index();

  Tensor val;
  while (data0 != data0_end)
  {
    data1->get_value(val,idx); idx++;
    *data0 = val.xx(); data0++;
    *data0 = val.xy(); data0++;
    *data0 = val.xz(); data0++;
    *data0 = val.yy(); data0++;
    *data0 = val.yz(); data0++;
    *data0 = val.zz(); data0++;
  }

  return (true);
}

bool get_node_location_fm(ArrayMathProgramCode& pc)
{
  // Get where to store the data
  double* data0 = pc.get_variable(0);
  // Get where the data needs to be stored
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + 3*pc.get_size();
  VMesh::Node::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.x(); data0++;
    *data0 = val.y(); data0++;
    *data0 = val.z(); data0++;
  }

  return (true);
}

bool get_node_x_fm(ArrayMathProgramCode& pc)
{
  // Get where to store the data
  double* data0 = pc.get_variable(0);
  // Get where the data needs to be stored
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();
  VMesh::Node::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.x(); data0++;
  }

  return (true);
}

bool get_node_y_fm(ArrayMathProgramCode& pc)
{
  // Get where to store the data
  double* data0 = pc.get_variable(0);
  // Get where the data needs to be stored
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();
  VMesh::Node::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.y(); data0++;
  }

  return (true);
}

bool get_node_z_fm(ArrayMathProgramCode& pc)
{
  // Get where to store the data
  double* data0 = pc.get_variable(0);
  // Get where the data needs to be stored
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();
  VMesh::Node::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.z(); data0++;
  }

  return (true);
}

bool get_element_location_fm(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + 3*pc.get_size();
  VMesh::Elem::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.x(); data0++;
    *data0 = val.y(); data0++;
    *data0 = val.z(); data0++;
  }

  return (true);
}

bool get_element_x_fm(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();
  VMesh::Elem::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.x(); data0++;
  }

  return (true);
}

bool get_element_y_fm(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();
  VMesh::Elem::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.y(); data0++;
  }

  return (true);
}

bool get_element_z_fm(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  VMesh*  data1 = pc.get_vmesh(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();
  VMesh::Elem::index_type idx = pc.get_index();

  Point val;
  while (data0 != data0_end)
  {
    data1->get_center(val,idx); idx++;
    *data0 = val.z(); data0++;
  }

  return (true);
}

bool get_scalar_m(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);

  // We store the matrix pointer so we can get other properties as well
  auto matrix = pc.get_matrix(1);
  if (!matrixIs::dense(matrix))
    return false;

  // One virtual call to get the data
  double* data1 = castMatrix::toDense(matrix)->data();

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check
  if (matrix->ncols() != 1) return (false);

  double* data0_end = data0 + pc.get_size();
  index_type idx = pc.get_index();

  data1 += idx;

  while (data0 != data0_end)
  {
    *data0 = *data1; data0++; data1++;
  }

  return (true);
}

bool get_scalar_ab(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  auto data1 = pc.get_bool_array(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // We store the matrix pointer so we can get other properties as well
  std::vector<bool>& array(*data1);
  // One virtual call to get the data

  double* data0_end = data0 + pc.get_size();
  index_type idx = pc.get_index();

  while (data0 != data0_end)
  {
    *data0 = static_cast<double>(array[idx]); data0++; idx++;
  }

  return (true);
}

bool get_scalar_ai(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  auto data1 = pc.get_int_array(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // We store the matrix pointer so we can get other properties as well
  std::vector<int>& array(*data1);
  // One virtual call to get the data

  double* data0_end = data0 + pc.get_size();
  index_type idx = pc.get_index();

  while (data0 != data0_end)
  {
    *data0 = static_cast<double>(array[idx]); data0++; idx++;
  }

  return (true);
}
bool get_scalar_ad(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  auto data1 = pc.get_double_array(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // We store the matrix pointer so we can get other properties as well
  std::vector<double>& array(*data1);
  // One virtual call to get the data

  double* data0_end = data0 + pc.get_size();
  index_type idx = pc.get_index();

  while (data0 != data0_end)
  {
    *data0 = array[idx]; data0++; idx++;
  }

  return (true);
}


bool get_vector_m(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);

  // We store the matrix pointer so we can get other properties as well
  auto matrix = pc.get_matrix(1);
  if (!matrixIs::dense(matrix))
    return false;
  // One virtual call to get the data
  double* data1 = castMatrix::toDense(matrix)->data();

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check
  if (matrix->ncols() != 3) return (false);

  double* data0_end = data0 + 3*(pc.get_size());
  index_type idx = pc.get_index();

  data1 += 3*idx;

  while (data0 != data0_end)
  {
    *data0 = *data1; data0++; data1++;
    *data0 = *data1; data0++; data1++;
    *data0 = *data1; data0++; data1++;
  }

  return (true);
}


bool get_tensor_m(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  // We store the matrix pointer so we can get other properties as well
  auto matrix = pc.get_matrix(1);
  if (!matrixIs::dense(matrix))
    return false;
  // One virtual call to get the data
  double* data1 = castMatrix::toDense(matrix)->data();

  if (!data0 || !data1)
  {
    return false;
  }

  // The different tensor storages invoke a different piece of code
  if (matrix->ncols() == 6)
  {
    double* data0_end = data0 + 6*(pc.get_size());
    index_type idx = pc.get_index();

    data1 += 6*idx;

    while (data0 != data0_end)
    {
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
    }
  }
  else if (matrix->ncols() == 9)
  {
    double* data0_end = data0 + 6*(pc.get_size());
    index_type idx = pc.get_index();

    data1 += 9*idx;

    while (data0 != data0_end)
    {
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++; data1++; data1++;
      *data0 = *data1; data0++; data1++;
    }
  }
  else
  {
    return (false);
  }
  return (true);
}


//--------------------------------------------------------------------------
// Sink functions

bool to_fielddata_s(ArrayMathProgramCode& pc)
{
  VField* data0 = pc.get_vfield(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check to see whether the output format is OK
  if (!(data0->is_scalar())) return (false);

  double* data1_end = data1 + (pc.get_size());
  index_type idx = pc.get_index();

  while (data1 != data1_end)
  {
    data0->set_value(*data1,idx); idx++; data1++;
  }

  return (true);
}

bool to_fielddata_v(ArrayMathProgramCode& pc)
{
  VField* data0 = pc.get_vfield(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check to see whether the output format is OK
  if (!(data0->is_vector())) return (false);

  double* data1_end = data1 + 3*(pc.get_size());
  index_type idx = pc.get_index();

  while (data1 != data1_end)
  {
    Vector vec(data1[0],data1[1],data1[2]);
    data0->set_value(vec,idx); idx++; data1+=3;
  }

  return (true);
}

bool to_fielddata_t(ArrayMathProgramCode& pc)
{
  VField* data0 = pc.get_vfield(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  // Safety check to see whether the output format is OK
  if (!(data0->is_tensor())) return (false);

  double* data1_end = data1 + 6*(pc.get_size());
  index_type idx = pc.get_index();

  while (data1 != data1_end)
  {
    Tensor ten(data1[0],data1[1],data1[2],data1[3],data1[4],data1[5]);
    data0->set_value(ten,idx); idx++; data1+=6;
  }

  return (true);
}

bool to_fieldnode_v(ArrayMathProgramCode& pc)
{
  VMesh* data0 = pc.get_vmesh(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data1_end = data1 + 3*(pc.get_size());
  VMesh::Node::index_type idx = pc.get_index();

  while (data1 != data1_end)
  {
    Point pnt(data1[0],data1[1],data1[2]);
    data0->set_point(pnt,idx); idx++; data1+=3;
  }

  return (true);
}


bool to_matrix_s(ArrayMathProgramCode& pc)
{
  // Get the pointer to the matrix object where we need to store the data
  auto matrix = pc.get_matrix(0);
  if (!matrixIs::dense(matrix))
    return false;

  double* data0 = castMatrix::toDense(matrix)->data();
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  if (matrix->ncols() != 1) return (false);

  double* data1_end = data1 + (pc.get_size());
  index_type idx = pc.get_index();

  data0 += idx;

  while (data1 != data1_end)
  {
    *data0 = *data1; data0++; data1++;
  }

  return (true);
}

bool to_matrix_v(ArrayMathProgramCode& pc)
{
  // Get the pointer to the matrix object where we need to store the data
  auto matrix = pc.get_matrix(0);
  if (!matrixIs::dense(matrix))
    return false;
  double* data0 = castMatrix::toDense(matrix)->data();
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  if (matrix->ncols() != 3) return (false);

  double* data1_end = data1 + 3*(pc.get_size());
  index_type idx = pc.get_index();

  data0 += 3*idx;

  while (data1 != data1_end)
  {
    *data0 = *data1; data0++; data1++;
    *data0 = *data1; data0++; data1++;
    *data0 = *data1; data0++; data1++;
  }

  return (true);
}

bool to_matrix_t(ArrayMathProgramCode& pc)
{
  // Get the pointer to the matrix object where we need to store the data
  auto matrix = pc.get_matrix(0);
  if (!matrixIs::dense(matrix))
    return false;
  double* data0 = castMatrix::toDense(matrix)->data();
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  if (matrix->ncols() == 6)
  {
    double* data1_end = data1 + 6*(pc.get_size());
    index_type idx = pc.get_index();

    data1 += 6*idx;

    while (data1 != data1_end)
    {
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
      *data0 = *data1; data0++; data1++;
    }
  }
  else if (matrix->ncols() == 9)
  {
    double* data1_end = data1 + 6*(pc.get_size());
    index_type idx = pc.get_index();

    data0 += 9*idx;

    while (data1 != data1_end)
    {
      data0[0] = data1[0];
      data0[1] = data1[1];
      data0[2] = data1[2];
      data0[3] = data1[1];
      data0[4] = data1[3];
      data0[5] = data1[4];
      data0[6] = data1[2];
      data0[7] = data1[4];
      data0[8] = data1[5];
      data0 +=9; data1+=6;
    }
  }
  else
  {
    return (false);
  }
  return (true);
}

bool index_(ArrayMathProgramCode& pc)
{
  // Get the pointer to the matrix object where we need to store the data
  double* data0 = pc.get_variable(0);

  if (!data0)
  {
    return false;
  }

  double* data0_end = data0 + (pc.get_size());

  index_type idx = pc.get_index();

  while (data0 != data0_end)
  {
    *data0 = static_cast<double>(idx); data0++; idx++;
  }

  return (true);
}


bool get_matrix_element_m(ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  auto data1 = pc.get_matrix(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + (pc.get_size());

  index_type   idx = pc.get_index();
  size_type    m = data1->nrows();

  if (m > 0)
  {
    while (data0 != data0_end)
    {
      index_type j = idx/m;
      index_type i = idx - j*m;

      *data0 = data1->get(i,j); data0++; idx++;
    }
  }

  return (true);
}


bool set_matrix_element_s(ArrayMathProgramCode& pc)
{
  auto data0 = pc.get_matrix(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }
  double* data1_end = data1 + (pc.get_size());

  index_type   idx = pc.get_index();
  static Mutex sparseAccessMutex("sparseAccessMutex");
  size_type m = data0->nrows();

  if (m > 0)
  {
    std::unique_ptr<Guard> guard;
    if (matrixIs::sparse(data0))
      guard.reset(new Guard(sparseAccessMutex.get()));

    while (data1 != data1_end)
    {
      index_type j = idx/m;
      index_type i = idx - j*m;
      data0->put(i,j,*data1); data1++; idx++;
    }
  }

  return (true);
}

bool to_bool_array_s(ArrayMathProgramCode& pc)
{
  // Get the pointer to the matrix object where we need to store the data
  auto data0 = pc.get_bool_array(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data1_end = data1 + (pc.get_size());
  index_type idx = pc.get_index();
  std::vector<bool>& array(*data0);

  while (data1 != data1_end)
  {
    array[idx] = *data1 != 0;
    idx++;
    data1++;
  }

  return (true);
}

bool to_int_array_s(ArrayMathProgramCode& pc)
{
  // Get the pointer to the matrix object where we need to store the data
  auto data0 = pc.get_int_array(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data1_end = data1 + (pc.get_size());
  index_type idx = pc.get_index();
  std::vector<int>& array(*data0);

  while (data1 != data1_end)
  {
    array[idx] = static_cast<int>(*data1); idx++; data1++;
  }

  return (true);
}

bool to_double_array_s(ArrayMathProgramCode& pc)
{
  // Get the pointer to the matrix object where we need to store the data
  auto data0 = pc.get_double_array(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data1_end = data1 + (pc.get_size());
  index_type idx = pc.get_index();

  std::vector<double>& array(*data0);
  while (data1 != data1_end)
  {
    array[idx] = *data1; idx++; data1++;
  }

  return (true);
}

} //end namespace

namespace SCIRun {

void
InsertSourceSinkArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog)
{
  // Source functions
  catalog->add_function(ArrayMathFunctions::get_scalar_fd,"get_scalar$FD","S");
  catalog->add_function(ArrayMathFunctions::get_vector_fd,"get_vector$FD","V");
  catalog->add_function(ArrayMathFunctions::get_tensor_fd,"get_tensor$FD","T");

  catalog->add_function(ArrayMathFunctions::get_node_location_fm,"get_node_location$FM","V");
  catalog->add_function(ArrayMathFunctions::get_node_x_fm,"get_node_x$FM","S");
  catalog->add_function(ArrayMathFunctions::get_node_y_fm,"get_node_y$FM","S");
  catalog->add_function(ArrayMathFunctions::get_node_z_fm,"get_node_z$FM","S");

  catalog->add_function(ArrayMathFunctions::get_element_location_fm,"get_element_location$FM","V");
  catalog->add_function(ArrayMathFunctions::get_element_x_fm,"get_element_x$FM","S");
  catalog->add_function(ArrayMathFunctions::get_element_y_fm,"get_element_y$FM","S");
  catalog->add_function(ArrayMathFunctions::get_element_z_fm,"get_element_z$FM","S");

  catalog->add_function(ArrayMathFunctions::get_scalar_m,"get_scalar$M","S");
  catalog->add_function(ArrayMathFunctions::get_vector_m,"get_vector$M","V");
  catalog->add_function(ArrayMathFunctions::get_tensor_m,"get_tensor$M","T");

  catalog->add_function(ArrayMathFunctions::get_scalar_ad,"get_scalar$AD","S");
  catalog->add_function(ArrayMathFunctions::get_scalar_ai,"get_scalar$AI","S");
  catalog->add_function(ArrayMathFunctions::get_scalar_ab,"get_scalar$AB","S");

  // Sink functions
  catalog->add_function(ArrayMathFunctions::to_fielddata_s,"to_fielddata$S","FD");
  catalog->add_function(ArrayMathFunctions::to_fielddata_v,"to_fielddata$V","FD");
  catalog->add_function(ArrayMathFunctions::to_fielddata_t,"to_fielddata$T","FD");

  catalog->add_function(ArrayMathFunctions::to_fieldnode_v,"to_fieldnode$V","FM");

  catalog->add_function(ArrayMathFunctions::to_matrix_s,"to_matrix$S","M");
  catalog->add_function(ArrayMathFunctions::to_matrix_v,"to_matrix$V","M");
  catalog->add_function(ArrayMathFunctions::to_matrix_t,"to_matrix$T","M");

  catalog->add_function(ArrayMathFunctions::to_bool_array_s,"to_bool_array$S","AB");
  catalog->add_function(ArrayMathFunctions::to_int_array_s,"to_int_array$S","AI");
  catalog->add_function(ArrayMathFunctions::to_double_array_s,"to_double_array$S","AD");

  // Get indices
  catalog->add_seq_function(ArrayMathFunctions::index_,"index$","S");

  // Get matrix element
  catalog->add_function(ArrayMathFunctions::get_matrix_element_m,"get_matrix_element$M","S");
  catalog->add_function(ArrayMathFunctions::set_matrix_element_s,"set_matrix_element$S","M");

}

} // end namespace

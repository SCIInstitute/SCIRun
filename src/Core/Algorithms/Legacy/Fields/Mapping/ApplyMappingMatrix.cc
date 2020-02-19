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

   Author:          Moritz Dannhauer
   Last Modified:   April 14 2014
   TODO:            improve the pointer arithmetic (from SCIRun4) in template class
*/


#include <Core/Algorithms/Legacy/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>


using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;

/// Internal function to this algorithm: no need for this function to be
/// public. It is called from the algorithm class only.
/// This is the basic algorithm behind the mapping algorithm
template <class DATA>
bool
ApplyMappingMatrixT(const ApplyMappingMatrixAlgo* algo,
                    const VField* input, VField* output,
                    SparseRowMatrixHandle mapping);

/// This is the basic algorithm behind the mapping algorithm
template <class DATA>
bool
ApplyMappingMatrixT(const ApplyMappingMatrixAlgo* algo,
                    const VField* input, VField* output,
                    SparseRowMatrixHandle mapping)
{
  double* vals = mapping->valuePtr();
  const index_type* rows = mapping->get_rows();
  const index_type* columns = mapping->get_cols();
  const size_type m = mapping->nrows();

 index_type cnt=0;
 for (index_type idx=0; idx<m; idx++)
 {
  DATA val(0);
  index_type rr = rows[idx];
  size_type  ss = rows[idx+1]-rows[idx];
  input->get_weighted_value(val,&(columns[rr]),&(vals[rr]),ss);

  output->set_value(val,idx);
  cnt++; if (cnt==400) {algo->update_progress((double)idx/m); cnt=0;}
 }

  return true;
}


/// Actual Algorithm class
ApplyMappingMatrixAlgo::ApplyMappingMatrixAlgo()
{
}

FieldHandle ApplyMappingMatrixAlgo::run(FieldHandle& isrc, FieldHandle& idst, MatrixHandle& mapping) const
{
  FieldHandle output;

  if (!isrc)
  /// safety check
  {
    THROW_ALGORITHM_INPUT_ERROR("No input source field");
  }

  if (!isrc)
  /// safety check
  {
    THROW_ALGORITHM_INPUT_ERROR("No input destination field");
  }

  if (!isrc)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input mapping field");
  }

  auto matrix = castMatrix::toSparse(mapping);

  if (!matrix)
  {
    THROW_ALGORITHM_INPUT_ERROR("Mapping matrix needs to be sparse");
  }

  VField* ifsrc =  isrc->vfield();
  VField* ifdst =  idst->vfield();
  VMesh*  imdst =  idst->vmesh();

  /// Get information about field types
  FieldInformation fi(isrc);
  FieldInformation fo(idst);

  fo.set_data_type(fi.get_data_type());
  size_type m = mapping->nrows();
  size_type n = mapping->ncols();

  size_type dst_num_nodes = imdst->num_nodes();
  size_type dst_num_elems = imdst->num_elems();
  size_type dst_num_values = ifdst->num_values();
  size_type src_num_values = ifsrc->num_values();

  if (dst_num_values == m)
  {
    /// do nothing
  }
  if (m == dst_num_nodes)
  {
    fo.make_lineardata();
  }
  else if (m == dst_num_elems)
  {
    fo.make_constantdata();
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR("The number of columns in the matrix does not match number of nodes or elements in the destination field");
  }

  if (src_num_values != n)
  {
    std::cerr << "n="<<n<<"\n";
    std::cerr << "num_values="<<src_num_values<<"\n";
    THROW_ALGORITHM_INPUT_ERROR("The number of columns in the matrix does not match number of values in the source field");
  }

  /// Create output field
  output = CreateField(fo,idst->mesh());

  VField* ofield = output->vfield();
  ofield->resize_values();

  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not create output field");
  }

  /// Simple table to deal with the various data type formats
  /// Note that not every data type is handled, all char, shorts etc,
  /// are automatically handled by the int, and unsigned int case, by
  /// casting the data on input (these should be the less frequently
  /// used datatypes and hence have no specific algorithm in place).
  /// Similarly floats are casted to doubles.

  if (isrc->vfield()->is_char())
    if (ApplyMappingMatrixT<char>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_unsigned_char())
    if (ApplyMappingMatrixT<unsigned char>(this,ifsrc,ofield,matrix))
        return output;
  if (isrc->vfield()->is_short())
    if (ApplyMappingMatrixT<short>(this,ifsrc,ofield,matrix))
        return output;
  if (isrc->vfield()->is_unsigned_short())
    if (ApplyMappingMatrixT<unsigned short>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_int())
    if (ApplyMappingMatrixT<int>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_unsigned_int())
    if (ApplyMappingMatrixT<unsigned int>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_longlong())
    if (ApplyMappingMatrixT<long long>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_unsigned_longlong())
    if (ApplyMappingMatrixT<unsigned long long>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_float())
    if (ApplyMappingMatrixT<float>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_double())
    if (ApplyMappingMatrixT<double>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_vector())
    if (ApplyMappingMatrixT<Vector>(this,ifsrc,ofield,matrix))
       return output;
  if (isrc->vfield()->is_tensor())
    if (ApplyMappingMatrixT<Tensor>(this,ifsrc,ofield,matrix))
       return output;

  return output;
}


AlgorithmInputName ApplyMappingMatrixAlgo::Source("Source");
AlgorithmInputName ApplyMappingMatrixAlgo::Destination("Destination");
AlgorithmInputName ApplyMappingMatrixAlgo::Mapping("Mapping");
AlgorithmOutputName ApplyMappingMatrixAlgo::Output("Output");

AlgorithmOutput ApplyMappingMatrixAlgo::run(const AlgorithmInput & input) const
{
  AlgorithmOutput output;

  auto src = input.get<Field>(Source);
  auto dest = input.get<Field>(Destination);
  auto mapp = input.get<Matrix>(Mapping);

  FieldHandle output_field;
  output_field = run(src,dest,mapp);
  output[Output] = output_field;

 return output;
}

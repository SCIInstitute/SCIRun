/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
   
   author: Moritz Dannhauer
   last change: 04/14/14
   TODO: improve the pointer arithmetic (from SCIRun4) in template class
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

//namespace SCIRunAlgo {

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
<<<<<<< HEAD
//! Internal function to this algorithm: no need for this function to be
//! public. It is called from the algorithm class only.

=======
/// Internal function to this algorithm: no need for this function to be
/// public. It is called from the algorithm class only.
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
template <class DATA> 
/*bool 
ApplyMappingMatrixT(const ApplyMappingMatrixAlgo* algo,
                    VField* input, VField* output,
<<<<<<< HEAD
                    SparseRowMatrix* mapping);*/
		    
bool
ApplyMappingMatrixT(const ApplyMappingMatrixAlgo* algo,
                    const VField* input, VField* output,
                    SparseRowMatrixHandle mapping);
		    
//! This is the basic algorithm behind the mapping algorithm
=======
                    SparseRowMatrix* mapping);

/// This is the basic algorithm behind the mapping algorithm
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
template <class DATA> 
bool
ApplyMappingMatrixT(const ApplyMappingMatrixAlgo* algo,
                    const VField* input, VField* output,
                    SparseRowMatrixHandle mapping)
/*bool 
ApplyMappingMatrixT(const ApplyMappingMatrixAlgo* algo,
                    VField* input, VField* output,
                    SparseRowMatrix* mapping)*/
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
  
<<<<<<< HEAD
  output->set_value(val,idx);
  cnt++; if (cnt==400) {algo->update_progress((double)idx/m); cnt=0;}
  
 }

  return true;
=======
  /// Algorithm succeeded
  algo->algo_end(); return (true);
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
}


/// Actual Algorithm class

ApplyMappingMatrixAlgo::ApplyMappingMatrixAlgo() 
{
<<<<<<< HEAD

}

FieldHandle ApplyMappingMatrixAlgo::run(FieldHandle& isrc, FieldHandle& idst, MatrixHandle& mapping) const
{
  FieldHandle output;
 
  if (!isrc)
=======
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  algo_start("ApplyMappingMatrix");
  
  /// safety check
  if (isrc.get_rep() == 0)
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
  {
    THROW_ALGORITHM_INPUT_ERROR("No input source field");
    return FieldHandle();
  }

<<<<<<< HEAD
  if (!isrc)
=======
  /// safety check
  if (idst.get_rep() == 0)
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
  {
    THROW_ALGORITHM_INPUT_ERROR("No input destination field");
    return FieldHandle();
  }
  
  if (!isrc)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input mapping field");
    return FieldHandle();
  }
  
  auto matrix = matrix_cast::as_sparse(mapping); 
  
  if (!matrix)
  {
    THROW_ALGORITHM_INPUT_ERROR("Mapping matrix needs to be sparse");
    return FieldHandle();
  }
  
  VField* ifsrc =  isrc->vfield();
  VField* ifdst =  idst->vfield();
  VMesh*  imdst =  idst->vmesh();
<<<<<<< HEAD
  
  //! Get information about field types
=======

  /// Get information about field types
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
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
    // do nothing
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
    return FieldHandle();
  }
  
  if (src_num_values != n)
  {
    std::cerr << "n="<<n<<"\n";
    std::cerr << "num_values="<<src_num_values<<"\n";
    THROW_ALGORITHM_INPUT_ERROR("The number of columns in the matrix does not match number of values in the source field");
    return FieldHandle();
  }
<<<<<<< HEAD
  
  //! Create output field
=======

  /// Create output field
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
  output = CreateField(fo,idst->mesh());
  
  VField* ofield = output->vfield();
  ofield->resize_values();  
  
<<<<<<< HEAD
  if (!output)
=======
  /// Check whether output field was created
  if (output.get_rep() == 0)
>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not create output field");
    return FieldHandle();
  } 
  
<<<<<<< HEAD
  //! Simple table to deal with the various data type formats
  //! Note that not every data type is handled, all char, shorts etc,
  //! are automatically handled by the int, and unsigned int case, by
  //! casting the data on input (these should be the less frequently
  //! used datatypes and hence have no specific algorithm in place).
  //! Similarly floats are casted to doubles.
       
=======
  /// Simple table to deal with the various data type formats
  /// Note that not every data type is handled, all char, shorts etc,
  /// are automatically handled by the int, and unsigned int case, by
  /// casting the data on input (these should be the less frequently
  /// used datatypes and hence have no specific algorithm in place).
  /// Similarly floats are casted to doubles.

>>>>>>> c2521af3852c84caa0e1e82b864408ff78ca7429
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

AlgorithmOutput ApplyMappingMatrixAlgo::run_generic(const AlgorithmInput & input) const
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

//ApplyMappingMatrixAlgo::~ApplyMappingMatrixAlgo() {}
//} // namespace SCIRunAlgo

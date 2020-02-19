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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_3.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/TetQuadraticLgn.h>
#include <Core/Basis/HexTriquadraticLgn.h>
#include <Core/Basis/PrismLinearLgn.h>
#include <Core/Datatypes/Legacy/Field/PrismVolMesh.h>
#include <Core/Datatypes/Legacy/Field/TetVolMesh.h>
#include <Core/Datatypes/Legacy/Field/LatVolMesh.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>


using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

//NoData
typedef NoDataBasis<double>                  NDBasis;

//Constant
typedef ConstantBasis<Tensor>                CFDTensorBasis;
typedef ConstantBasis<Vector>                CFDVectorBasis;
typedef ConstantBasis<double>                CFDdoubleBasis;
typedef ConstantBasis<complex>				       CFDcomplexBasis;
typedef ConstantBasis<float>                 CFDfloatBasis;
typedef ConstantBasis<complex>               CFDcomplexBasis;
typedef ConstantBasis<int>                   CFDintBasis;
typedef ConstantBasis<long long>             CFDlonglongBasis;
typedef ConstantBasis<short>                 CFDshortBasis;
typedef ConstantBasis<char>                  CFDcharBasis;
typedef ConstantBasis<unsigned int>          CFDuintBasis;
typedef ConstantBasis<unsigned short>        CFDushortBasis;
typedef ConstantBasis<unsigned char>         CFDucharBasis;
typedef ConstantBasis<unsigned long>         CFDulongBasis;


typedef TetLinearLgn<Tensor>                TFDTensorBasis;
typedef TetLinearLgn<Vector>                TFDVectorBasis;
typedef TetLinearLgn<double>                TFDdoubleBasis;
typedef TetLinearLgn<complex>               TFDComplexdoubleBasis;
typedef TetLinearLgn<float>                 TFDfloatBasis;
typedef TetLinearLgn<int>                   TFDintBasis;
typedef TetLinearLgn<long long>             TFDlonglongBasis;
typedef TetLinearLgn<short>                 TFDshortBasis;
typedef TetLinearLgn<char>                  TFDcharBasis;
typedef TetLinearLgn<unsigned int>          TFDuintBasis;
typedef TetLinearLgn<unsigned short>        TFDushortBasis;
typedef TetLinearLgn<unsigned char>         TFDucharBasis;
typedef TetLinearLgn<unsigned long>         TFDulongBasis;

typedef TetVolMesh<TetLinearLgn<Point> > TVMesh;
PersistentTypeID backwards_compat_TVM("TetVolMesh", "Mesh",
				      TVMesh::maker, TVMesh::maker);

namespace SCIRun {


template class TetVolMesh<TetLinearLgn<Point> >;


//NoData
template class GenericField<TVMesh, NDBasis, std::vector<double> >;

//Constant
template class GenericField<TVMesh, CFDTensorBasis, std::vector<Tensor> >;
template class GenericField<TVMesh, CFDVectorBasis, std::vector<Vector> >;
template class GenericField<TVMesh, CFDdoubleBasis, std::vector<double> >;
template class GenericField<TVMesh, CFDcomplexBasis, std::vector<complex> >;
template class GenericField<TVMesh, CFDfloatBasis,  std::vector<float> >;
template class GenericField<TVMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<TVMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<TVMesh, CFDshortBasis,  std::vector<short> >;
template class GenericField<TVMesh, CFDcharBasis,   std::vector<char> >;
template class GenericField<TVMesh, CFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<TVMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<TVMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<TVMesh, CFDulongBasis,  std::vector<unsigned long> >;

//Linear
template class GenericField<TVMesh, TFDTensorBasis, std::vector<Tensor> >;
template class GenericField<TVMesh, TFDVectorBasis, std::vector<Vector> >;
template class GenericField<TVMesh, TFDdoubleBasis, std::vector<double> >;
template class GenericField<TVMesh, TFDComplexdoubleBasis, std::vector<complex> >;
template class GenericField<TVMesh, TFDfloatBasis,  std::vector<float> >;
template class GenericField<TVMesh, TFDintBasis,    std::vector<int> >;
template class GenericField<TVMesh, TFDlonglongBasis,std::vector<long long> >;
template class GenericField<TVMesh, TFDshortBasis,  std::vector<short> >;
template class GenericField<TVMesh, TFDcharBasis,   std::vector<char> >;
template class GenericField<TVMesh, TFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<TVMesh, TFDushortBasis, std::vector<unsigned short> >;
template class GenericField<TVMesh, TFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<TVMesh, TFDulongBasis,  std::vector<unsigned long> >;

}

PersistentTypeID
backwards_compat_TVFT("TetVolField<Tensor>", "Field",
		      GenericField<TVMesh, TFDTensorBasis,
		      std::vector<Tensor> >::maker,
		      GenericField<TVMesh, CFDTensorBasis,
		      std::vector<Tensor> >::maker);
PersistentTypeID
backwards_compat_TVFV("TetVolField<Vector>", "Field",
		      GenericField<TVMesh, TFDVectorBasis,
		      std::vector<Vector> >::maker,
		      GenericField<TVMesh, CFDVectorBasis,
		      std::vector<Vector> >::maker);
PersistentTypeID
backwards_compat_TVFd("TetVolField<double>", "Field",
		      GenericField<TVMesh, TFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<TVMesh, CFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<TVMesh, NDBasis,
		      std::vector<double> >::maker);
PersistentTypeID
backwards_compat_TVFcd("TetVolField<complex>", "Field",
          GenericField<TVMesh, CFDcomplexBasis,
          std::vector<complex> >::maker,
          GenericField<TVMesh, TFDComplexdoubleBasis,
          std::vector<complex> >::maker,
          GenericField<TVMesh, NDBasis,
          std::vector<complex> >::maker);
PersistentTypeID
backwards_compat_TVFf("TetVolField<float>", "Field",
		      GenericField<TVMesh, TFDfloatBasis,
		      std::vector<float> >::maker,
		      GenericField<TVMesh, CFDfloatBasis,
		      std::vector<float> >::maker);
PersistentTypeID
backwards_compat_TVFi("TetVolField<int>", "Field",
		      GenericField<TVMesh, TFDintBasis,
		      std::vector<int> >::maker,
		      GenericField<TVMesh, CFDintBasis,
		      std::vector<int> >::maker);
PersistentTypeID
backwards_compat_TVFs("TetVolField<short>", "Field",
		      GenericField<TVMesh, TFDshortBasis,
		      std::vector<short> >::maker,
		      GenericField<TVMesh, CFDshortBasis,
		      std::vector<short> >::maker);
PersistentTypeID
backwards_compat_TVFc("TetVolField<char>", "Field",
		      GenericField<TVMesh, TFDcharBasis,
		      std::vector<char> >::maker,
		      GenericField<TVMesh, CFDcharBasis,
		      std::vector<char> >::maker);
PersistentTypeID
backwards_compat_TVFui("TetVolField<unsigned_int>", "Field",
		       GenericField<TVMesh, TFDuintBasis,
		       std::vector<unsigned int> >::maker,
		       GenericField<TVMesh, CFDuintBasis,
		       std::vector<unsigned int> >::maker);
PersistentTypeID
backwards_compat_TVFus("TetVolField<unsigned_short>", "Field",
		       GenericField<TVMesh, TFDushortBasis,
		       std::vector<unsigned short> >::maker,
		       GenericField<TVMesh, CFDushortBasis,
		       std::vector<unsigned short> >::maker);
PersistentTypeID
backwards_compat_TVFuc("TetVolField<unsigned_char>", "Field",
		       GenericField<TVMesh, TFDucharBasis,
		       std::vector<unsigned char> >::maker,
		       GenericField<TVMesh, CFDucharBasis,
		       std::vector<unsigned char> >::maker);
PersistentTypeID
backwards_compat_TVFul("TetVolField<unsigned_long>", "Field",
		       GenericField<TVMesh, TFDulongBasis,
		       std::vector<unsigned long> >::maker,
		       GenericField<TVMesh, CFDulongBasis,
		       std::vector<unsigned long> >::maker);


typedef TetQuadraticLgn<Tensor>                TQFDTensorBasis;
typedef TetQuadraticLgn<Vector>                TQFDVectorBasis;
typedef TetQuadraticLgn<double>                TQFDdoubleBasis;
typedef TetQuadraticLgn<float>                 TQFDfloatBasis;
typedef TetQuadraticLgn<complex>               TQFDcomplexBasis;
typedef TetQuadraticLgn<int>                   TQFDintBasis;
typedef TetQuadraticLgn<long long>             TQFDlonglongBasis;
typedef TetQuadraticLgn<short>                 TQFDshortBasis;
typedef TetQuadraticLgn<char>                  TQFDcharBasis;
typedef TetQuadraticLgn<unsigned int>          TQFDuintBasis;
typedef TetQuadraticLgn<unsigned short>        TQFDushortBasis;
typedef TetQuadraticLgn<unsigned char>         TQFDucharBasis;
typedef TetQuadraticLgn<unsigned long>         TQFDulongBasis;

typedef TetVolMesh<TetQuadraticLgn<Point> > QTVMesh;

namespace SCIRun {

template class GenericField<QTVMesh, TQFDTensorBasis, std::vector<Tensor> >;
template class GenericField<QTVMesh, TQFDVectorBasis, std::vector<Vector> >;
template class GenericField<QTVMesh, TQFDdoubleBasis, std::vector<double> >;
template class GenericField<QTVMesh, TQFDfloatBasis,  std::vector<float> >;
template class GenericField<QTVMesh, TQFDcomplexBasis, std::vector<complex> >;
template class GenericField<QTVMesh, TQFDintBasis,    std::vector<int> >;
template class GenericField<QTVMesh, TQFDlonglongBasis,std::vector<long long> >;
template class GenericField<QTVMesh, TQFDshortBasis,  std::vector<short> >;
template class GenericField<QTVMesh, TQFDcharBasis,   std::vector<char> >;
template class GenericField<QTVMesh, TQFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<QTVMesh, TQFDushortBasis, std::vector<unsigned short> >;
template class GenericField<QTVMesh, TQFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<QTVMesh, TQFDulongBasis,  std::vector<unsigned long> >;

}

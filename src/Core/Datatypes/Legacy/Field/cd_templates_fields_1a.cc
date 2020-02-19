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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_1a.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Datatypes/Legacy/Field/ImageMesh.h>
#include <Core/Datatypes/Legacy/Field/QuadSurfMesh.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>


using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

namespace SCIRun {

  //NoData
  typedef NoDataBasis<double>               NDBasis;

  //Constant
  typedef ConstantBasis<Tensor>             CFDTensorBasis;
  typedef ConstantBasis<Vector>             CFDVectorBasis;
  typedef ConstantBasis<double>             CFDdoubleBasis;
  typedef ConstantBasis<float>              CFDfloatBasis;
  typedef ConstantBasis<complex>            CFDcomplexBasis;
  typedef ConstantBasis<int>                CFDintBasis;
  typedef ConstantBasis<long long>          CFDlonglongBasis;
  typedef ConstantBasis<short>              CFDshortBasis;
  typedef ConstantBasis<char>               CFDcharBasis;
  typedef ConstantBasis<unsigned int>       CFDuintBasis;
  typedef ConstantBasis<unsigned short>     CFDushortBasis;
  typedef ConstantBasis<unsigned char>      CFDucharBasis;
  typedef ConstantBasis<unsigned long>      CFDulongBasis;

  //Linear
  typedef QuadBilinearLgn<Tensor>             FDTensorBasis;
  typedef QuadBilinearLgn<Vector>             FDVectorBasis;
  typedef QuadBilinearLgn<double>             FDdoubleBasis;
  typedef QuadBilinearLgn<float>              FDfloatBasis;
  typedef QuadBilinearLgn<complex>            FDcomplexBasis;
  typedef QuadBilinearLgn<int>                FDintBasis;
  typedef QuadBilinearLgn<long long>          FDlonglongBasis;
  typedef QuadBilinearLgn<short>              FDshortBasis;
  typedef QuadBilinearLgn<char>               FDcharBasis;
  typedef QuadBilinearLgn<unsigned int>       FDuintBasis;
  typedef QuadBilinearLgn<unsigned short>     FDushortBasis;
  typedef QuadBilinearLgn<unsigned char>      FDucharBasis;
  typedef QuadBilinearLgn<unsigned long>      FDulongBasis;

typedef QuadSurfMesh<QuadBilinearLgn<Point> > QSMesh;

template class QuadSurfMesh<QuadBilinearLgn<Point> >;
PersistentTypeID backwards_compat_QSM("QuadSurfMesh", "Mesh", QSMesh::maker, QSMesh::maker);

//NoData
template class GenericField<QSMesh, NoDataBasis<double>, std::vector<double> >;
template class GenericField<QSMesh, NoDataBasis<int>, std::vector<int> >;

//Constant
template class GenericField<QSMesh, CFDTensorBasis, std::vector<Tensor> >;
template class GenericField<QSMesh, CFDVectorBasis, std::vector<Vector> >;
template class GenericField<QSMesh, CFDdoubleBasis, std::vector<double> >;
template class GenericField<QSMesh, CFDfloatBasis,  std::vector<float> >;
template class GenericField<QSMesh, CFDcomplexBasis, std::vector<complex> >;
template class GenericField<QSMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<QSMesh, CFDlonglongBasis, std::vector<long long> >;
template class GenericField<QSMesh, CFDshortBasis,  std::vector<short> >;
template class GenericField<QSMesh, CFDcharBasis,   std::vector<char> >;
template class GenericField<QSMesh, CFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<QSMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<QSMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<QSMesh, CFDulongBasis,  std::vector<unsigned long> >;

//Linear
template class GenericField<QSMesh, FDTensorBasis, std::vector<Tensor> >;
template class GenericField<QSMesh, FDVectorBasis, std::vector<Vector> >;
template class GenericField<QSMesh, FDdoubleBasis, std::vector<double> >;
template class GenericField<QSMesh, FDfloatBasis,  std::vector<float> >;
template class GenericField<QSMesh, FDcomplexBasis, std::vector<complex> >;
template class GenericField<QSMesh, FDintBasis,    std::vector<int> >;
template class GenericField<QSMesh, FDlonglongBasis,std::vector<long long> >;
template class GenericField<QSMesh, FDshortBasis,  std::vector<short> >;
template class GenericField<QSMesh, FDcharBasis,   std::vector<char> >;
template class GenericField<QSMesh, FDuintBasis,   std::vector<unsigned int> >;
template class GenericField<QSMesh, FDushortBasis, std::vector<unsigned short> >;
template class GenericField<QSMesh, FDucharBasis,  std::vector<unsigned char> >;
template class GenericField<QSMesh, FDulongBasis,  std::vector<unsigned long> >;
}

PersistentTypeID
backwards_compat_QSFT("QuadSurfField<Tensor>", "Field",
		      GenericField<QSMesh, FDTensorBasis,
		      std::vector<Tensor> >::maker,
		      GenericField<QSMesh, CFDTensorBasis,
		      std::vector<Tensor> >::maker);
PersistentTypeID
backwards_compat_QSFV("QuadSurfField<Vector>", "Field",
		      GenericField<QSMesh, FDVectorBasis,
		      std::vector<Vector> >::maker,
		      GenericField<QSMesh, CFDVectorBasis,
		      std::vector<Vector> >::maker);
PersistentTypeID
backwards_compat_QSFd("QuadSurfField<double>", "Field",
		      GenericField<QSMesh, FDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<QSMesh, CFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<QSMesh, NDBasis,
		      std::vector<double> >::maker);
PersistentTypeID
backwards_compat_QSFf("QuadSurfField<float>", "Field",
		      GenericField<QSMesh, FDfloatBasis,
		      std::vector<float> >::maker,
		      GenericField<QSMesh, CFDfloatBasis,
		      std::vector<float> >::maker);
PersistentTypeID
backwards_compat_QSFco("QuadSurfField<complex>", "Field",
          GenericField<QSMesh, FDcomplexBasis,
          std::vector<complex> >::maker,
          GenericField<QSMesh, CFDcomplexBasis,
          std::vector<complex> >::maker);
PersistentTypeID
backwards_compat_QSFi("QuadSurfField<int>", "Field",
		      GenericField<QSMesh, FDintBasis,
		      std::vector<int> >::maker,
		      GenericField<QSMesh, CFDintBasis,
		      std::vector<int> >::maker);
PersistentTypeID
backwards_compat_QSFs("QuadSurfField<short>", "Field",
		      GenericField<QSMesh, FDshortBasis,
		      std::vector<short> >::maker,
		      GenericField<QSMesh, CFDshortBasis,
		      std::vector<short> >::maker);
PersistentTypeID
backwards_compat_QSFc("QuadSurfField<char>", "Field",
		      GenericField<QSMesh, FDcharBasis,
		      std::vector<char> >::maker,
		      GenericField<QSMesh, CFDcharBasis,
		      std::vector<char> >::maker);
PersistentTypeID
backwards_compat_QSFui("QuadSurfField<unsigned_int>", "Field",
		       GenericField<QSMesh, FDuintBasis,
		       std::vector<unsigned int> >::maker,
		       GenericField<QSMesh, CFDuintBasis,
		       std::vector<unsigned int> >::maker);
PersistentTypeID
backwards_compat_QSFus("QuadSurfField<unsigned_short>", "Field",
		       GenericField<QSMesh, FDushortBasis,
		       std::vector<unsigned short> >::maker,
		       GenericField<QSMesh, CFDushortBasis,
		       std::vector<unsigned short> >::maker);
PersistentTypeID
backwards_compat_QSFuc("QuadSurfField<unsigned_char>", "Field",
		       GenericField<QSMesh, FDucharBasis,
		       std::vector<unsigned char> >::maker,
		       GenericField<QSMesh, CFDucharBasis,
		       std::vector<unsigned char> >::maker);
PersistentTypeID
backwards_compat_QSFul("QuadSurfField<unsigned_long>", "Field",
		       GenericField<QSMesh, FDulongBasis,
		       std::vector<unsigned long> >::maker,
		       GenericField<QSMesh, CFDulongBasis,
		       std::vector<unsigned long> >::maker);

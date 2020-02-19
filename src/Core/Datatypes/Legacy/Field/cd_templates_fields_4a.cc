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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_4.cc

#include <Core/Persistent/PersistentSTL.h>

#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Core/Datatypes/Legacy/Field/CurveMesh.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

//Constant
typedef ConstantBasis<Tensor>                CFDTensorBasis;
typedef ConstantBasis<Vector>                CFDVectorBasis;
typedef ConstantBasis<double>                CFDdoubleBasis;
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

//Linear
typedef TriLinearLgn<Tensor>                FDTensorBasis;
typedef TriLinearLgn<Vector>                FDVectorBasis;
typedef TriLinearLgn<double>                FDdoubleBasis;
typedef TriLinearLgn<float>                 FDfloatBasis;
typedef TriLinearLgn<complex>                 FDcomplexBasis;
typedef TriLinearLgn<int>                   FDintBasis;
typedef TriLinearLgn<long long>             FDlonglongBasis;
typedef TriLinearLgn<short>                 FDshortBasis;
typedef TriLinearLgn<char>                  FDcharBasis;
typedef TriLinearLgn<unsigned int>          FDuintBasis;
typedef TriLinearLgn<unsigned short>        FDushortBasis;
typedef TriLinearLgn<unsigned char>         FDucharBasis;
typedef TriLinearLgn<unsigned long>         FDulongBasis;

typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;


namespace SCIRun
{
template class TriSurfMesh<TriLinearLgn<Point> >;

//Linear
template class GenericField<TSMesh, FDTensorBasis, std::vector<Tensor> >;
template class GenericField<TSMesh, FDVectorBasis, std::vector<Vector> >;
template class GenericField<TSMesh, FDdoubleBasis, std::vector<double> >;
template class GenericField<TSMesh, FDfloatBasis,  std::vector<float> >;
template class GenericField<TSMesh, FDcomplexBasis, std::vector<complex> >;
template class GenericField<TSMesh, FDintBasis,    std::vector<int> >;
template class GenericField<TSMesh, FDlonglongBasis,std::vector<long long> >;
template class GenericField<TSMesh, FDshortBasis,  std::vector<short> >;
template class GenericField<TSMesh, FDcharBasis,   std::vector<char> >;
template class GenericField<TSMesh, FDuintBasis,   std::vector<unsigned int> >;
template class GenericField<TSMesh, FDushortBasis, std::vector<unsigned short> >;
template class GenericField<TSMesh, FDucharBasis,  std::vector<unsigned char> >;
template class GenericField<TSMesh, FDulongBasis,  std::vector<unsigned long> >;
}

PersistentTypeID
backwards_compat_TSFT("TriSurfField<Tensor>", "Field",
GenericField<TSMesh, FDTensorBasis,
std::vector<Tensor> >::maker,
GenericField<TSMesh, CFDTensorBasis,
std::vector<Tensor> >::maker);
PersistentTypeID
backwards_compat_TSFV("TriSurfField<Vector>", "Field",
GenericField<TSMesh, FDVectorBasis,
std::vector<Vector> >::maker,
GenericField<TSMesh, CFDVectorBasis,
std::vector<Vector> >::maker);
PersistentTypeID
backwards_compat_TSFd("TriSurfField<double>", "Field",
GenericField<TSMesh, FDdoubleBasis,
std::vector<double> >::maker,
GenericField<TSMesh, CFDdoubleBasis,
std::vector<double> >::maker);
PersistentTypeID
backwards_compat_TSFf("TriSurfField<float>", "Field",
GenericField<TSMesh, FDfloatBasis,
std::vector<float> >::maker,
GenericField<TSMesh, CFDfloatBasis,
std::vector<float> >::maker);
PersistentTypeID
backwards_compat_TSFco("TriSurfField<complex>", "Field",
GenericField<TSMesh, FDcomplexBasis,
std::vector<complex> >::maker,
GenericField<TSMesh, CFDcomplexBasis,
std::vector<complex> >::maker);
PersistentTypeID
backwards_compat_TSFi("TriSurfField<int>", "Field",
GenericField<TSMesh, FDintBasis,
std::vector<int> >::maker,
GenericField<TSMesh, CFDintBasis,
std::vector<int> >::maker);
PersistentTypeID
backwards_compat_TSFs("TriSurfField<short>", "Field",
GenericField<TSMesh, FDshortBasis,
std::vector<short> >::maker,
GenericField<TSMesh, CFDshortBasis,
std::vector<short> >::maker);
PersistentTypeID
backwards_compat_TSFc("TriSurfField<char>", "Field",
GenericField<TSMesh, FDcharBasis,
std::vector<char> >::maker,
GenericField<TSMesh, CFDcharBasis,
std::vector<char> >::maker);
PersistentTypeID
backwards_compat_TSFui("TriSurfField<unsigned_int>", "Field",
GenericField<TSMesh, FDuintBasis,
std::vector<unsigned int> >::maker,
GenericField<TSMesh, CFDuintBasis,
std::vector<unsigned int> >::maker);
PersistentTypeID
backwards_compat_TSFus("TriSurfField<unsigned_short>", "Field",
GenericField<TSMesh, FDushortBasis,
std::vector<unsigned short> >::maker,
GenericField<TSMesh, CFDushortBasis,
std::vector<unsigned short> >::maker);
PersistentTypeID
backwards_compat_TSFuc("TriSurfField<unsigned_char>", "Field",
GenericField<TSMesh, FDucharBasis,
std::vector<unsigned char> >::maker,
GenericField<TSMesh, CFDucharBasis,
std::vector<unsigned char> >::maker);
PersistentTypeID
backwards_compat_TSFul("TriSurfField<unsigned_long>", "Field",
GenericField<TSMesh, FDulongBasis,
std::vector<unsigned long> >::maker,
GenericField<TSMesh, CFDulongBasis,
std::vector<unsigned long> >::maker);

//Linear
typedef CrvLinearLgn<Tensor>                CrFDTensorBasis;
typedef CrvLinearLgn<Vector>                CrFDVectorBasis;
typedef CrvLinearLgn<double>                CrFDdoubleBasis;
typedef CrvLinearLgn<float>                 CrFDfloatBasis;
typedef CrvLinearLgn<complex>               CrFDcomplexBasis;
typedef CrvLinearLgn<int>                   CrFDintBasis;
typedef CrvLinearLgn<long long>             CrFDlonglongBasis;
typedef CrvLinearLgn<short>                 CrFDshortBasis;
typedef CrvLinearLgn<char>                  CrFDcharBasis;
typedef CrvLinearLgn<unsigned int>          CrFDuintBasis;
typedef CrvLinearLgn<unsigned short>        CrFDushortBasis;
typedef CrvLinearLgn<unsigned char>         CrFDucharBasis;
typedef CrvLinearLgn<unsigned long>         CrFDulongBasis;

typedef CurveMesh<CrvLinearLgn<Point> > CMesh;

namespace SCIRun {

  template class CurveMesh<CrvLinearLgn<Point> >;

//Linear
template class GenericField<CMesh, CrFDTensorBasis, std::vector<Tensor> >;
template class GenericField<CMesh, CrFDVectorBasis, std::vector<Vector> >;
template class GenericField<CMesh, CrFDdoubleBasis, std::vector<double> >;
template class GenericField<CMesh, CrFDfloatBasis,  std::vector<float> >;
template class GenericField<CMesh, CrFDcomplexBasis, std::vector<complex> >;
template class GenericField<CMesh, CrFDintBasis,    std::vector<int> >;
template class GenericField<CMesh, CrFDlonglongBasis,std::vector<long long> >;
template class GenericField<CMesh, CrFDshortBasis,  std::vector<short> >;
template class GenericField<CMesh, CrFDcharBasis,   std::vector<char> >;
template class GenericField<CMesh, CrFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<CMesh, CrFDushortBasis, std::vector<unsigned short> >;
template class GenericField<CMesh, CrFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<CMesh, CrFDulongBasis,  std::vector<unsigned long> >;
}

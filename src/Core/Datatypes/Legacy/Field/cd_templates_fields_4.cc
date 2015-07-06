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

//NoData
typedef NoDataBasis<double>                  NDBasis;

//Constant
typedef ConstantBasis<Tensor>                CFDTensorBasis;
typedef ConstantBasis<Vector>                CFDVectorBasis;
typedef ConstantBasis<double>                CFDdoubleBasis;
typedef ConstantBasis<float>                 CFDfloatBasis;
typedef ConstantBasis<int>                   CFDintBasis;
typedef ConstantBasis<long long>             CFDlonglongBasis;
typedef ConstantBasis<short>                 CFDshortBasis;
typedef ConstantBasis<char>                  CFDcharBasis;
typedef ConstantBasis<unsigned int>          CFDuintBasis;
typedef ConstantBasis<unsigned short>        CFDushortBasis;
typedef ConstantBasis<unsigned char>         CFDucharBasis;
typedef ConstantBasis<unsigned long>         CFDulongBasis;

typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;


namespace SCIRun
{
template class TriSurfMesh<TriLinearLgn<Point> >;

//noData
template class GenericField<TSMesh, NDBasis, std::vector<double> >;   

//Constant
template class GenericField<TSMesh, CFDTensorBasis, std::vector<Tensor> >;       
template class GenericField<TSMesh, CFDVectorBasis, std::vector<Vector> >;       
template class GenericField<TSMesh, CFDdoubleBasis, std::vector<double> >;       
template class GenericField<TSMesh, CFDfloatBasis,  std::vector<float> >;        
template class GenericField<TSMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<TSMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<TSMesh, CFDshortBasis,  std::vector<short> >;        
template class GenericField<TSMesh, CFDcharBasis,   std::vector<char> >;         
template class GenericField<TSMesh, CFDuintBasis,   std::vector<unsigned int> >; 
template class GenericField<TSMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<TSMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<TSMesh, CFDulongBasis,  std::vector<unsigned long> >;

}

PersistentTypeID backwards_compat_TSM("TriSurfMesh", "Mesh",
  TSMesh::maker, TSMesh::maker);

typedef CurveMesh<CrvLinearLgn<Point> > CMesh;
PersistentTypeID backwards_compat_CM("CurveMesh", "Mesh",
				      CMesh::maker, CMesh::maker);

namespace SCIRun {

  template class CurveMesh<CrvLinearLgn<Point> >;

//NoData
template class GenericField<CMesh, NDBasis,  std::vector<double> >;  

//Constant
template class GenericField<CMesh, CFDTensorBasis, std::vector<Tensor> >;       
template class GenericField<CMesh, CFDVectorBasis, std::vector<Vector> >;       
template class GenericField<CMesh, CFDdoubleBasis, std::vector<double> >;       
template class GenericField<CMesh, CFDfloatBasis,  std::vector<float> >;        
template class GenericField<CMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<CMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<CMesh, CFDshortBasis,  std::vector<short> >;        
template class GenericField<CMesh, CFDcharBasis,   std::vector<char> >;         
template class GenericField<CMesh, CFDuintBasis,   std::vector<unsigned int> >; 
template class GenericField<CMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<CMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<CMesh, CFDulongBasis,  std::vector<unsigned long> >;

}


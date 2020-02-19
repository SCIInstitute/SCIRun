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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_5a.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/HexTricubicHmt.h>
#include <Core/Basis/HexTricubicHmtScaleFactors.h>
#include <Core/Basis/HexTricubicHmtScaleFactorsEdges.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Datatypes/Legacy/Field/HexVolMesh.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {

//NoData
typedef NoDataBasis<double>                NDBasis;

//Linear
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
typedef HexTrilinearLgn<Tensor>                FDTensorBasis;
typedef HexTrilinearLgn<Vector>                FDVectorBasis;
typedef HexTrilinearLgn<double>                FDdoubleBasis;
typedef HexTrilinearLgn<float>                 FDfloatBasis;
typedef HexTrilinearLgn<complex>               FDcomplexBasis;
typedef HexTrilinearLgn<int>                   FDintBasis;
typedef HexTrilinearLgn<long long>             FDlonglongBasis;
typedef HexTrilinearLgn<short>                 FDshortBasis;
typedef HexTrilinearLgn<char>                  FDcharBasis;
typedef HexTrilinearLgn<unsigned int>          FDuintBasis;
typedef HexTrilinearLgn<unsigned short>        FDushortBasis;
typedef HexTrilinearLgn<unsigned char>         FDucharBasis;
typedef HexTrilinearLgn<unsigned long>         FDulongBasis;



typedef HexTricubicHmt<double>             HTCdoubleBasis;

typedef HexVolMesh<HexTricubicHmt<Point> > HVCubMesh;
template class HexVolMesh<HexTricubicHmt<Point> >;

template class GenericField<HVCubMesh, NDBasis, std::vector<double> >;
template class GenericField<HVCubMesh, HTCdoubleBasis, std::vector<double> >;

typedef HexTricubicHmtScaleFactors<double>             HTCSFdoubleBasis;
typedef HexTricubicHmtScaleFactors<Vector>             HTCSFVectorBasis;

typedef HexVolMesh<HexTricubicHmtScaleFactors<Point> > HVCubSFMesh;
template class HexVolMesh<HexTricubicHmtScaleFactors<Point> >;

template class GenericField<HVCubSFMesh, NDBasis, std::vector<double> >;
template class GenericField<HVCubSFMesh, HTCSFdoubleBasis, std::vector<double> >;
template class GenericField<HVCubSFMesh, HTCSFVectorBasis, std::vector<Vector> >;

typedef HexVolMesh<HexTricubicHmtScaleFactorsEdges<Point> > HVCubSFEMesh;
template class HexVolMesh<HexTricubicHmtScaleFactorsEdges<Point> >;

template class GenericField<HVCubSFEMesh, NDBasis, std::vector<double> >;

}

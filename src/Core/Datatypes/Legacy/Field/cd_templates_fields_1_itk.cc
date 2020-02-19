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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_1_itk.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/Datatypes/ITKImageField.h>

namespace SCIRun {

typedef ImageMesh<QuadBilinearLgn<Point> > IMesh;

typedef QuadBilinearLgn<Tensor>             FDTensorBasis;
typedef QuadBilinearLgn<Vector>             FDVectorBasis;
typedef QuadBilinearLgn<double>             FDdoubleBasis;
typedef QuadBilinearLgn<float>              FDfloatBasis;
typedef QuadBilinearLgn<int>                FDintBasis;
typedef QuadBilinearLgn<short>              FDshortBasis;
typedef QuadBilinearLgn<char>               FDcharBasis;
typedef QuadBilinearLgn<unsigned int>       FDuintBasis;
typedef QuadBilinearLgn<unsigned short>     FDushortBasis;
typedef QuadBilinearLgn<unsigned char>      FDucharBasis;
typedef QuadBilinearLgn<unsigned long>      FDulongBasis;

template class ITKFData2d<Tensor>;
template class ITKFData2d<Vector>;
template class ITKFData2d<double>;
template class ITKFData2d<float>;
template class ITKFData2d<int>;
template class ITKFData2d<short>;
template class ITKFData2d<char>;
template class ITKFData2d<unsigned int>;
template class ITKFData2d<unsigned short>;
template class ITKFData2d<unsigned char>;
template class ITKFData2d<unsigned long>;

template class GenericField<IMesh, FDTensorBasis, ITKFData2d<Tensor> >;
template class GenericField<IMesh, FDVectorBasis, ITKFData2d<Vector> >;
template class GenericField<IMesh, FDdoubleBasis, ITKFData2d<double> >;
template class GenericField<IMesh, FDfloatBasis,  ITKFData2d<float> >;
template class GenericField<IMesh, FDintBasis,    ITKFData2d<int> >;
template class GenericField<IMesh, FDshortBasis,  ITKFData2d<short> >;
template class GenericField<IMesh, FDcharBasis,   ITKFData2d<char> >;
template class GenericField<IMesh, FDuintBasis,   ITKFData2d<unsigned int> >;
template class GenericField<IMesh, FDushortBasis, ITKFData2d<unsigned short> >;
template class GenericField<IMesh, FDucharBasis,  ITKFData2d<unsigned char> >;
template class GenericField<IMesh, FDulongBasis,  ITKFData2d<unsigned long> >;

template class ITKImageField<Tensor>;
template class ITKImageField<Vector>;
template class ITKImageField<double>;
template class ITKImageField<float>;
template class ITKImageField<int>;
template class ITKImageField<short>;
template class ITKImageField<char>;
template class ITKImageField<unsigned int>;
template class ITKImageField<unsigned short>;
template class ITKImageField<unsigned char>;
template class ITKImageField<unsigned long>;



const TypeDescription* get_type_description(ITKImageField<Tensor> *);
const TypeDescription* get_type_description(ITKImageField<Vector> *);
const TypeDescription* get_type_description(ITKImageField<double> *);
const TypeDescription* get_type_description(ITKImageField<float> *);
const TypeDescription* get_type_description(ITKImageField<int> *);
const TypeDescription* get_type_description(ITKImageField<short> *);
const TypeDescription* get_type_description(ITKImageField<char> *);
const TypeDescription* get_type_description(ITKImageField<unsigned int> *);
const TypeDescription* get_type_description(ITKImageField<unsigned short> *);
const TypeDescription* get_type_description(ITKImageField<unsigned char> *);
const TypeDescription* get_type_description(ITKImageField<unsigned long> *);



  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<SCIRun::Tensor>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<SCIRun::Tensor>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<SCIRun::Vector>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<SCIRun::Vector>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<double>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<double>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<float>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<float>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<int>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<int>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<short>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<short>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<char>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<char>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<unsigned int>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<unsigned int>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<unsigned short>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<unsigned short>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<unsigned char>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<unsigned char>*)0);
  }

  const TypeDescription*
  get_type_description(SCIRun::ITKFData2d<unsigned long>*) {
    return
      SCIRun::get_type_description((SCIRun::ITKFData2d<unsigned long>*)0);
  }

} // end of namespace SCIRun

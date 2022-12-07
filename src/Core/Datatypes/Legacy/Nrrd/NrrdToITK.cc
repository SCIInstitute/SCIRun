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


/*
 *@file    NrrdToITK.cc
 *@author  McKay Davis
 *@date    Sun Oct 22 23:09:46 2006
 */

#include <Core/Datatypes/NrrdToITK.h>
#include <Core/Util/StringUtil.h>

#ifdef HAVE_INSIGHT

namespace SCIRun {


ITKDatatypeHandle
nrrd_to_itk_image2(NrrdDataHandle &nrrd)
{
  Nrrd *n = nrrd->nrrd_;

  itk::Object::Pointer data = 0;
  switch (n->type) {
  case nrrdTypeChar: data = nrrd_to_itk_image<signed char, 2>(n); break;
  case nrrdTypeUChar: data = nrrd_to_itk_image<unsigned char, 2>(n); break;
  case nrrdTypeShort: data = nrrd_to_itk_image<signed short, 2>(n); break;
  case nrrdTypeUShort: data = nrrd_to_itk_image<unsigned short, 2>(n); break;
  case nrrdTypeInt: data = nrrd_to_itk_image<signed int, 2>(n); break;
  case nrrdTypeUInt: data = nrrd_to_itk_image<unsigned int, 2>(n); break;
  case nrrdTypeLLong: data = nrrd_to_itk_image<signed long long, 2>(n); break;
  case nrrdTypeULLong: data =nrrd_to_itk_image<unsigned long long, 2>(n); break;
  case nrrdTypeFloat: data = nrrd_to_itk_image<float, 2>(n); break;
  case nrrdTypeDouble: data = nrrd_to_itk_image<double, 2>(n); break;
  default: throw "nrrd_to_itk_image2, cannot convert type" + to_string(n->type);
  }

  ITKDatatype *result = new SCIRun::ITKDatatype();
  result->data_ = data;
  return result;
}


ITKDatatypeHandle
nrrd_to_itk_image3(NrrdDataHandle &nrrd)
{
  Nrrd *n = nrrd->nrrd_;

  itk::Object::Pointer data = 0;
  switch (n->type) {
  case nrrdTypeChar: data = nrrd_to_itk_image<signed char, 3>(n); break;
  case nrrdTypeUChar: data = nrrd_to_itk_image<unsigned char, 3>(n); break;
  case nrrdTypeShort: data = nrrd_to_itk_image<signed short, 3>(n); break;
  case nrrdTypeUShort: data = nrrd_to_itk_image<unsigned short, 3>(n); break;
  case nrrdTypeInt: data = nrrd_to_itk_image<signed int, 3>(n); break;
  case nrrdTypeUInt: data = nrrd_to_itk_image<unsigned int, 3>(n); break;
  case nrrdTypeLLong: data = nrrd_to_itk_image<signed long long, 3>(n); break;
  case nrrdTypeULLong: data =nrrd_to_itk_image<unsigned long long, 3>(n); break;
  case nrrdTypeFloat: data = nrrd_to_itk_image<float, 3>(n); break;
  case nrrdTypeDouble: data = nrrd_to_itk_image<double, 3>(n); break;
  default: throw "nrrd_to_itk_image, cannot convert type" + to_string(n->type);
  }

  ITKDatatype *result = new SCIRun::ITKDatatype();
  result->data_ = data;
  return result;
}


ITKDatatypeHandle
nrrd_to_itk_image(NrrdDataHandle &nrrd)
{
  if (nrrd->nrrd_->dim == 4)
  {
    return nrrd_to_itk_image3(nrrd);
  }
  else if (nrrd->nrrd_->dim == 3)
  {
    return nrrd_to_itk_image2(nrrd);
  }
  else
  {
    throw "nrrd_to_itk_image, unsupported dimension.";
  }
}


}

#endif /* HAVE_INSIGHT */

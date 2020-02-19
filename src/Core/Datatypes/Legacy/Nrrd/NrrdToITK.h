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
 *@file    NrrdToITK.h
 *@author  McKay Davis
 *@date    Sun Oct 22 22:46:48 2006
 */

#ifndef SEG3D_NrrdToITK_H
#define SEG3D_NrrdToITK_H

#include <sci_defs/insight_defs.h>

#ifdef HAVE_INSIGHT

#include <Core/Datatypes/NrrdData.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <Core/Util/Assert.h>
#include <itkImageToImageFilter.h>
#include <itkImportImageFilter.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {


// Note, this is a pretty specific funtion to convert a 3D volume of scalar
// nrrds with a stub axis of 1 dimension at axis 0

SCISHARE ITKDatatypeHandle
nrrd_to_itk_image(NrrdDataHandle &nrrd);

template <class PixType, int Dim>
itk::Object::Pointer
nrrd_to_itk_image(Nrrd *n)
{
  ASSERT(n);
  ASSERT(n->type == get_nrrd_type<PixType>());
  ASSERT(n->dim == Dim+1);

  typedef itk::ImportImageFilter < PixType , Dim > ImportFilterType;
  typename ImportFilterType::Pointer importFilter = ImportFilterType::New();
  typename ImportFilterType::SizeType size;

  double origin[Dim];
  double spacing[Dim];

  unsigned int count = 1;
  for(unsigned int i=0; i < n->dim-1; i++) {
    count *= n->axis[i+1].size;
    size[i] = n->axis[i+1].size;

    if (!AIR_EXISTS(n->axis[i+1].min)) {
      origin[i] = 0;
    } else {
      origin[i] = n->axis[i+1].min;
    }

    if (!AIR_EXISTS(n->axis[i+1].spacing)) {
      spacing[i] = 1.0;
    } else {
      spacing[i] = n->axis[i+1].spacing;
    }
  }
  typename ImportFilterType::IndexType start;
  typename ImportFilterType::RegionType region;
  start.Fill(0);
  region.SetIndex(start);
  region.SetSize(size);
  importFilter->SetRegion(region);
  importFilter->SetOrigin(origin);
  importFilter->SetSpacing(spacing);
  importFilter->SetImportPointer((PixType *)n->data, count, false);
  importFilter->Update();

  return importFilter->GetOutput();
}


template <class PixType, int Dim>
NrrdDataHandle
itk_image_to_nrrd(ITKDatatypeHandle &img_handle)
{
  typedef itk::Image<PixType, Dim> ImageType;

  ImageType *img = dynamic_cast<ImageType *>(img_handle->data_.GetPointer());
  if (img == 0) {
    return 0;
  }

  NrrdData *nrrd_data = new NrrdData(img_handle.get_rep());
  Nrrd *nrrd = nrrd_data->nrrd_;

  size_t size[NRRD_DIM_MAX];
  size[0] = 1;
  size[1] = img->GetRequestedRegion().GetSize()[0];
  size[2] = img->GetRequestedRegion().GetSize()[1];
  size[3] = img->GetRequestedRegion().GetSize()[2];

  unsigned int centers[NRRD_DIM_MAX];
  centers[0] = nrrdCenterNode;
  centers[1] = nrrdCenterNode;
  centers[2] = nrrdCenterNode;
  centers[3] = nrrdCenterNode;

  nrrdWrap_nva(nrrd, (void *)img->GetBufferPointer(),
               get_nrrd_type<PixType>(), 4, size);

  nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoCenter, centers);

  nrrd->axis[0].spacing = AIR_NAN;
  nrrd->axis[0].min = 0;
  nrrd->axis[0].max = 1;
  nrrd->axis[0].kind = nrrdKindStub;

  for(unsigned int i = 0; i < Dim; i++) {
    nrrd->axis[i+1].spacing = img->GetSpacing()[i];
    nrrd->axis[i+1].min = img->GetOrigin()[i];
    nrrd->axis[i+1].max = nrrd->axis[i+1].min +
      (nrrd->axis[i+1].size-1) * nrrd->axis[i+1].spacing;
    nrrd->axis[i+1].kind = nrrdKindDomain;
  }
  return nrrd_data;
}


}
#endif
#endif

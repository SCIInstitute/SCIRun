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


#include <Core/Algorithms/Legacy/Fields/ResampleMesh/ResampleRegularMesh.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Logging/Log.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <teem/nrrd.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(Fields, ResampleMethod);
ALGORITHM_PARAMETER_DEF(Fields, ResampleGaussianSigma);
ALGORITHM_PARAMETER_DEF(Fields, ResampleGaussianExtend);
ALGORITHM_PARAMETER_DEF(Fields, ResampleXDim);
ALGORITHM_PARAMETER_DEF(Fields, ResampleYDim);
ALGORITHM_PARAMETER_DEF(Fields, ResampleZDim);
ALGORITHM_PARAMETER_DEF(Fields, ResampleXDimUseScalingFactor);
ALGORITHM_PARAMETER_DEF(Fields, ResampleYDimUseScalingFactor);
ALGORITHM_PARAMETER_DEF(Fields, ResampleZDimUseScalingFactor);

ResampleRegularMeshAlgo::ResampleRegularMeshAlgo()
{
  /// Option for selecting the resampling kernel
  addOption(Parameters::ResampleMethod, "Box","Box|Tent|Cubic (Catmull-Rom)|Cubic (B-Spline)|Gaussian");
  addParameter(Parameters::ResampleGaussianSigma, 1.0);
  addParameter(Parameters::ResampleGaussianExtend, 1.0);

  // resample none = keep as is, number= specify new number of samples
  //          factor = multiply number of samples by a factor

  addParameter(Parameters::ResampleXDim, 0.5);
  addParameter(Parameters::ResampleYDim, 0.5);
  addParameter(Parameters::ResampleZDim, 0.5);
  addParameter(Parameters::ResampleXDimUseScalingFactor, true);
  addParameter(Parameters::ResampleYDimUseScalingFactor, true);
  addParameter(Parameters::ResampleZDimUseScalingFactor, true);
}

///////////////////////////////////////////////////////
// Refine elements for a TetVol

bool
ResampleRegularMeshAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "ReSampleRegularMesh");

  if (!input)
  {
    error("No input field was given.");
    return (false);
  }

  FieldInformation fi(input);

  if ((!(fi.is_latvolmesh()))&&(!(fi.is_imagemesh()))&&(!(fi.is_scanlinemesh())))
  {
    error("This algorithm only operates on regular grids");
    return (false);
  }

  // Convert data to nrrd and run through teem resampler
  Nrrd* nin;
  Nrrd* nout;

  VField::size_type num_values = input->vfield()->num_values();

  nin = nrrdNew();
  nout = nrrdNew();

  // Determine dimensions
  size_t nrrddims[NRRD_DIM_MAX];
  int nrrddim = 0;
  bool nrrdoffset = 0;

  VMesh::dimension_type dims;

  VMesh*  vmesh  = input->vmesh();
  VField* vfield = input->vfield();

  if (vfield->is_vector())
  {
    nrrddims[0] = 3; nrrddim = 1; nrrdoffset = 1;
  }
  else if (vfield->is_tensor())
  {
    nrrddims[0] = 6; nrrddim = 1; nrrdoffset = 1;
  }

  if (fi.is_lineardata()) vmesh->get_dimensions(dims);
  else vmesh->get_elem_dimensions(dims);

  int dimsSize = static_cast<int>(dims.size());
  for (int k = nrrddim; k < nrrddim + dimsSize; k++)
    nrrddims[k] = dims[k-nrrddim];
  nrrddim += dimsSize;

  if (vfield->is_char())
  {
    nrrdAlloc_nva(nin,nrrdTypeChar,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<char *>(nin->data),num_values);
  }
  else if (vfield->is_unsigned_char())
  {
    nrrdAlloc_nva(nin,nrrdTypeUChar,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<unsigned char *>(nin->data),num_values);
  }
  else if (vfield->is_short())
  {
    nrrdAlloc_nva(nin,nrrdTypeShort,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<short *>(nin->data),num_values);
  }
  else if (vfield->is_unsigned_short())
  {
    nrrdAlloc_nva(nin,nrrdTypeUShort,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<unsigned short *>(nin->data),num_values);
  }
  else if (vfield->is_int())
  {
    nrrdAlloc_nva(nin,nrrdTypeInt,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<int *>(nin->data),num_values);
  }
  else if (vfield->is_unsigned_int())
  {
    nrrdAlloc_nva(nin,nrrdTypeUInt,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<unsigned int *>(nin->data),num_values);
  }
  else if (vfield->is_long())
  {
    nrrdAlloc_nva(nin,nrrdTypeLLong,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<long long *>(nin->data),num_values);
  }
  else if (vfield->is_unsigned_long())
  {
    nrrdAlloc_nva(nin,nrrdTypeULLong,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<unsigned long long *>(nin->data),num_values);
  }
  else if (vfield->is_longlong())
  {
    nrrdAlloc_nva(nin,nrrdTypeLLong,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<long long *>(nin->data),num_values);
  }
  else if (vfield->is_unsigned_longlong())
  {
    nrrdAlloc_nva(nin,nrrdTypeULLong,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<unsigned long long *>(nin->data),num_values);
  }
  else if (vfield->is_float())
  {
    nrrdAlloc_nva(nin,nrrdTypeFloat,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<float *>(nin->data),num_values);
  }
  else if (vfield->is_double())
  {
    nrrdAlloc_nva(nin,nrrdTypeDouble,nrrddim,nrrddims);
    vfield->get_values(reinterpret_cast<double *>(nin->data),num_values);
  }
  else if (vfield->is_vector())
  {
    nrrdAlloc_nva(nin,nrrdTypeDouble,nrrddim,nrrddims);
    double* ptr = reinterpret_cast<double *>(nin->data);
    size_t k = 0;
    for (VField::index_type idx=0; idx<num_values; idx++)
    {
      Vector v;
      vfield->get_value(v,idx);
      ptr[k] = v.x(); ptr[k+1] = v.y(); ptr[k+2] = v.z();
      k += 3;
    }
  }
  else if (vfield->is_tensor())
  {
    nrrdAlloc_nva(nin,nrrdTypeDouble,nrrddim,nrrddims);
    double* ptr = reinterpret_cast<double *>(nin->data);
    size_t k = 0;
    for (VField::index_type idx=0; idx<num_values; idx++)
    {
      Tensor v;
      vfield->get_value(v,idx);
      ptr[k  ] = v.xx(); ptr[k+1] = v.xy(); ptr[k+2] = v.xz();
      ptr[k+3] = v.yy(); ptr[k+4] = v.yz(); ptr[k+5] = v.zz();
      k += 6;
    }
  }
  else
  {
    error("Unknown datatype.");
    return (false);
  }

  NrrdKernel *kern = 0;

  double param[NRRD_KERNEL_PARMS_NUM]; param[0] =  1.0;
  for (int j=1; j<NRRD_KERNEL_PARMS_NUM; j++) param[j] = 0.0;

  if (checkOption(Parameters::ResampleMethod,"Box"))
  {
    kern = nrrdKernelBox;
  }
  else if (checkOption(Parameters::ResampleMethod,"Tent"))
  {
    kern = nrrdKernelTent;
  }
  else if (checkOption(Parameters::ResampleMethod,"Cubic (Catmull-Rom)"))
  {
    kern = nrrdKernelBCCubic;
    param[1] = 0.0;
    param[2] = 0.5;
  }
  else if (checkOption(Parameters::ResampleMethod,"Cubic (B-Spline)"))
  {
    kern = nrrdKernelBCCubic;
    param[1] = 1.0;
    param[2] = 0.0;
  }
  else if (checkOption(Parameters::ResampleMethod,"Gaussian"))
  {
    kern = nrrdKernelGaussian;
    param[0] = get(Parameters::ResampleGaussianSigma).toDouble();
    param[1] = get(Parameters::ResampleGaussianExtend).toDouble();
  }
  else
  { // default is quartic
    LOG_DEBUG("ResampleRegularMeshAlgo defaulting to Quartic kernel.");
    kern = nrrdKernelAQuartic;
    param[1] = 0.0834; // most accurate as per Teem documentation
  }

  NrrdResampleInfo *info = nrrdResampleInfoNew();

  if (nrrdoffset)
  {
    info->kernel[0] = 0;
    nin->axis[0].min = 0.0;
    nin->axis[0].max = 1.0;
    info->min[0] = 0.0;
    info->max[0] = 1.0;
  }

  Transform trans;
  vmesh->get_canonical_transform(trans);

  // Set the lengths along the axis
  info->min[nrrdoffset] = 0.0;
  nin->axis[nrrdoffset].min = 0.0;
  info->max[nrrdoffset] = trans.project(Vector(1.0,0.0,0.0)).length();
  nin->axis[nrrdoffset].max = info->max[nrrdoffset];
  info->min[nrrdoffset+1] = 0.0;
  nin->axis[nrrdoffset+1].min = 0.0;
  info->max[nrrdoffset+1] = trans.project(Vector(0.0,1.0,0.0)).length();
  nin->axis[nrrdoffset+1].max = info->max[nrrdoffset+1];
  info->min[nrrdoffset+2] = 0.0;
  nin->axis[nrrdoffset+2].min = 0.0;
  info->max[nrrdoffset+2] = trans.project(Vector(0.0,0.0,1.0)).length();
  nin->axis[nrrdoffset+2].max = info->max[nrrdoffset+2];


  for (int a = nrrdoffset; a < nrrddim; a++)
  {
    info->kernel[a] = kern;
    for (int b=0; b<NRRD_KERNEL_PARMS_NUM; b++) info->parm[a][b] = param[b];
  }

  // Set the resampling options
  if (!get(Parameters::ResampleXDimUseScalingFactor).toBool())
  {
    info->kernel[nrrdoffset] = kern;
    info->samples[nrrdoffset] = static_cast<size_t>(get(Parameters::ResampleXDim).toDouble());
  }
  else
  {
    info->kernel[nrrdoffset] = kern;
    info->samples[nrrdoffset] = static_cast<size_t>(get(Parameters::ResampleXDim).toDouble() * nrrddims[nrrdoffset]);
  }

  if (nrrddim-nrrdoffset > 1)
  {
    if (!get(Parameters::ResampleYDimUseScalingFactor).toBool())
    {
      info->kernel[nrrdoffset+1] = kern;
      info->samples[nrrdoffset+1] = static_cast<size_t>(get(Parameters::ResampleYDim).toDouble());
    }
    else
    {
      info->kernel[nrrdoffset+1] = kern;
      info->samples[nrrdoffset+1] = static_cast<size_t>(get(Parameters::ResampleYDim).toDouble()*nrrddims[nrrdoffset+1]);
    }
  }

  if (nrrddim-nrrdoffset > 2)
  {
    if (!get(Parameters::ResampleZDimUseScalingFactor).toBool())
    {
      info->kernel[nrrdoffset+2] = kern;
      info->samples[nrrdoffset+2] = static_cast<size_t>(get(Parameters::ResampleZDim).toDouble());
    }
    else
    {
      info->kernel[nrrdoffset+2] = kern;
      info->samples[nrrdoffset+2] = static_cast<size_t>(get(Parameters::ResampleZDim).toDouble()*nrrddims[nrrdoffset+2]);
    }
  }

  if (nrrdSpatialResample(nout, nin, info))
  {
    char *err = biffGetDone(NRRD);
    error(std::string("Trouble resampling: ") +  err);
    free(err);
    return (false);
  }

  nrrdNix(nin);

  MeshHandle mesh;
  if (dims.size() == 3)
  {
    if (fi.is_lineardata())
    {
      mesh = CreateMesh(fi,info->samples[nrrdoffset] ,info->samples[nrrdoffset+1],info->samples[nrrdoffset+2],Point(0.0,0.0,0.0),Point(1.0,1.0,1.0));
    }
    else
    {
      mesh = CreateMesh(fi,info->samples[nrrdoffset]+1 ,info->samples[nrrdoffset+1]+1,info->samples[nrrdoffset+2]+1,Point(0.0,0.0,0.0),Point(1.0,1.0,1.0));
    }
  }
  else if (dims.size() == 2)
  {
    if (fi.is_lineardata())
    {
      mesh = CreateMesh(fi,info->samples[nrrdoffset] ,info->samples[nrrdoffset+1],Point(0.0,0.0,0.0),Point(1.0,1.0,0.0));
    }
    else
    {
      mesh = CreateMesh(fi,info->samples[nrrdoffset]+1 ,info->samples[nrrdoffset+1]+1,Point(0.0,0.0,0.0),Point(1.0,1.0,0.0));
    }
  }
  else if (dims.size() == 1)
  {
    if (fi.is_lineardata())
    {
      mesh = CreateMesh(fi,info->samples[nrrdoffset] ,Point(0.0,0.0,0.0),Point(1.0,0.0,0.0));
    }
    else
    {
      mesh = CreateMesh(fi,info->samples[nrrdoffset]+1 ,Point(0.0,0.0,0.0),Point(1.0,0.0,0.0));
    }
  }


  if (!mesh)
  {
    error("Could not create output mesh");
    return (false);
  }
  output = CreateField(fi,mesh);
  if (!output)
  {
    error("Could not create output mesh");
    return (false);
  }
  output->vmesh()->transform(trans);

  vfield = output->vfield();
  num_values = vfield->num_values();

  if (vfield->is_char())
  {
    vfield->set_values(reinterpret_cast<char *>(nout->data),num_values);
  }
  else if (vfield->is_unsigned_char())
  {
    vfield->set_values(reinterpret_cast<unsigned char *>(nout->data),num_values);
  }
  else if (vfield->is_short())
  {
    vfield->set_values(reinterpret_cast<short *>(nout->data),num_values);
  }
  else if (vfield->is_unsigned_short())
  {
    vfield->set_values(reinterpret_cast<unsigned short *>(nout->data),num_values);
  }
  if (vfield->is_int())
  {
    vfield->set_values(reinterpret_cast<int *>(nout->data),num_values);
  }
  else if (vfield->is_unsigned_int())
  {
    vfield->set_values(reinterpret_cast<unsigned int *>(nout->data),num_values);
  }
  else if (vfield->is_long())
  {
    vfield->set_values(reinterpret_cast<long long *>(nout->data),num_values);
  }
  else if (vfield->is_unsigned_long())
  {
    vfield->set_values(reinterpret_cast<unsigned long long *>(nout->data),num_values);
  }
  else if (vfield->is_longlong())
  {
    vfield->set_values(reinterpret_cast<long long *>(nout->data),num_values);
  }
  else if (vfield->is_unsigned_longlong())
  {
    vfield->set_values(reinterpret_cast<unsigned long long *>(nout->data),num_values);
  }
  else if (vfield->is_float())
  {
    vfield->set_values(reinterpret_cast<float *>(nout->data),num_values);
  }
  else if (vfield->is_double())
  {
    vfield->set_values(reinterpret_cast<double *>(nout->data),num_values);
  }
  else if (vfield->is_vector())
  {
    double* ptr = reinterpret_cast<double *>(nout->data);
    size_t k = 0;
    for (VField::index_type idx=0; idx<num_values; idx++)
    {
      Vector v(ptr[k],ptr[k+1],ptr[k+2]); k+=3;
      vfield->set_value(v,idx);
    }
  }
  else if (vfield->is_tensor())
  {
    double* ptr = reinterpret_cast<double *>(nout->data);
    size_t k = 0;
    for (VField::index_type idx=0; idx<num_values; idx++)
    {
      Tensor v(ptr[k],ptr[k+1],ptr[k+2],ptr[k+3],ptr[k+4],ptr[k+5]);
      vfield->set_value(v,idx);
      k += 6;
    }
  }

  nrrdNix(nout);

  return (true);
}

AlgorithmOutput ResampleRegularMeshAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  if (!runImpl(field, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}

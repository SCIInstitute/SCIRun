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
*/

#include <Core/Algorithms/Legacy/Fields/TransformMesh/ScaleFieldMeshAndData.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

ALGORITHM_PARAMETER_DEF(Fields, data_scale);
ALGORITHM_PARAMETER_DEF(Fields, mesh_scale);
ALGORITHM_PARAMETER_DEF(Fields, scale_from_center);

ScaleFieldMeshAndDataAlgo::ScaleFieldMeshAndDataAlgo()
{
  addParameter(Parameters::data_scale,1.0);
  addParameter(Parameters::mesh_scale,1.0);
  addParameter(Parameters::scale_from_center,false);
}

namespace
{
  template <class T>
  void
  ScaleFieldMeshAndDataAlgoT(double scale, FieldHandle output)
  {
    std::vector<T> values;
    output->vfield()->get_values(values);
    for (size_t j=0;j<values.size();j++) values[j] = static_cast<T>(scale*values[j]);
    output->vfield()->set_values(values);
  }
}

bool 
ScaleFieldMeshAndDataAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "ScaleFieldMeshAndData");
  
  if (!input)
  {
    error("No input field");
    return (false);
  }

  bool scale_from_center = get(Parameters::scale_from_center).getBool();
  double datascale = get(Parameters::data_scale).getDouble();
  double meshscale = get(Parameters::mesh_scale).getDouble();

  // scale mesh, only when needed
  if (scale_from_center || (meshscale != 1.0))
  {
    output.reset(input->deep_clone());
    Transform tf;
    BBox box = input->vmesh()->get_bounding_box();
    Vector center = 0.5*(box.min()+box.max());
    
    tf.load_identity();
    if (scale_from_center) tf.pre_translate(-center);
    tf.pre_scale(Vector(meshscale,meshscale,meshscale));
    if (scale_from_center) tf.pre_translate(center);
    
    output->vmesh()->transform(tf);
  }
  else
  {
    output.reset(input->clone());
  }

  if (!output)
  {
    error("Could not allocate output field");
    return (false);  
  }   

  if (datascale != 1.0)
  {
    VField* ofield = output->vfield();
    if (ofield->is_tensor()) ScaleFieldMeshAndDataAlgoT<Tensor>(datascale,output);
    if (ofield->is_vector()) ScaleFieldMeshAndDataAlgoT<Vector>(datascale,output);
    if (ofield->is_double()) ScaleFieldMeshAndDataAlgoT<double>(datascale,output);
    if (ofield->is_float()) ScaleFieldMeshAndDataAlgoT<float>(datascale,output);
    if (ofield->is_char()) ScaleFieldMeshAndDataAlgoT<char>(datascale,output);
    if (ofield->is_unsigned_char()) ScaleFieldMeshAndDataAlgoT<unsigned char>(datascale,output);
    if (ofield->is_short()) ScaleFieldMeshAndDataAlgoT<short>(datascale,output);
    if (ofield->is_unsigned_short()) ScaleFieldMeshAndDataAlgoT<unsigned short>(datascale,output);
    if (ofield->is_int()) ScaleFieldMeshAndDataAlgoT<int>(datascale,output);
    if (ofield->is_unsigned_int()) ScaleFieldMeshAndDataAlgoT<unsigned int>(datascale,output);
    if (ofield->is_longlong()) ScaleFieldMeshAndDataAlgoT<long long>(datascale,output);
    if (ofield->is_unsigned_longlong()) ScaleFieldMeshAndDataAlgoT<unsigned long long>(datascale,output);
  }

  CopyProperties(*input, *output);
  return true;
}

AlgorithmOutput ScaleFieldMeshAndDataAlgo::run_generic(const AlgorithmInput& input) const
{
  throw "todo";
  //auto field = input.get<Field>(Variables::InputField);

  //FieldHandle outputField;
  //if (!runImpl(field, outputField))
  //  THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  //AlgorithmOutput output;
  //output[Variables::OutputField] = outputField;
  //return output;
}
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

   Author: Spencer Frisby, Moritz Dannhauer
   Date:   May 2014
*/


#include <iostream>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Vector.h>

#include <Core/Logging/Log.h>

#include <boost/assign.hpp>

using namespace boost::assign;
using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(BrainStimulator, Skin);
ALGORITHM_PARAMETER_DEF(BrainStimulator, SoftBone);
ALGORITHM_PARAMETER_DEF(BrainStimulator, HardBone);
ALGORITHM_PARAMETER_DEF(BrainStimulator, CSF);
ALGORITHM_PARAMETER_DEF(BrainStimulator, GM);
ALGORITHM_PARAMETER_DEF(BrainStimulator, WM);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Electrode);
ALGORITHM_PARAMETER_DEF(BrainStimulator, InternalAir);

AlgorithmInputName  SetConductivitiesToMeshAlgorithm::InputField("InputField");
AlgorithmOutputName SetConductivitiesToMeshAlgorithm::OutputField("OutputField");

SetConductivitiesToMeshAlgorithm::SetConductivitiesToMeshAlgorithm()
{
  ElemLabelLookup += 1,2,3,4,5,6,7,8;

  using namespace Parameters;
  /// electrical conductivities (isotropic) default values based on the literature
  addParameter(Skin,         0.43);
  addParameter(SoftBone,     0.02856);
  addParameter(HardBone,     0.00640);
  addParameter(CSF,          1.79);
  addParameter(GM,           0.33);
  addParameter(WM,           0.142);
  addParameter(Electrode,    1.4);
  addParameter(InternalAir,  1e-6);
}

AlgorithmOutput SetConductivitiesToMeshAlgorithm::run(const AlgorithmInput& input) const
{
  auto mesh  = input.get<Field>(InputField);

  AlgorithmOutput output;

  FieldHandle output_field = run(mesh);

  output[OutputField] = output_field;

  return output;
}

FieldHandle SetConductivitiesToMeshAlgorithm::run(FieldHandle fh) const
{
  // making sure the field is not null
  if (!fh)
    THROW_ALGORITHM_INPUT_ERROR("Field supplied is empty ");

  /// making sure the data is on the elem and not the nodes
  FieldInformation fi(fh);
  if (!fi.is_constantdata())
    THROW_ALGORITHM_INPUT_ERROR("This function requires the data to be on the elements ");

  /// making sure the field contains data
  VField* vfield = fh->vfield();
  if (vfield->is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("Field supplied contained no data ");

  /// making sure the field is not in vector or tensor format
  if (!vfield->is_scalar())
    THROW_ALGORITHM_INPUT_ERROR("Function only supports scalar labels. ");

  using namespace Parameters;

  LOG_DEBUG("SetConductivitiesToTetMeshAlgorithm parameters:\n\tSkin = {}\n\tSoftBone = {}\n\tHardBone = {}"
    "\n\tCSF = {}\n\tGM = {}\n\tWM = {}\n\tElectrode = {}\n\tInternalAir = {}",
    get(Skin).toDouble(), get(SoftBone).toDouble(), get(HardBone).toDouble(),
    get(CSF).toDouble(), get(GM).toDouble(), get(WM).toDouble(), get(Electrode).toDouble(),
    get(InternalAir).toDouble());

  /// array holding conductivities
  /// @todo: enable when VS2013 is supported
//  std::vector<double> conductivities = {get(Skin).toDouble(), get(SoftBone).toDouble(), get(HardBone).toDouble(),
//  get(CSF).toDouble(), get(GM).toDouble(), get(WM).toDouble(), get(Electrode).toDouble(), get(InternalAir).toDouble()};

  // stopgap measure until VS2013 is supported
  std::vector<double> conductivities;
  conductivities += get(Skin).toDouble(), get(SoftBone).toDouble(), get(HardBone).toDouble(),
  get(CSF).toDouble(), get(GM).toDouble(), get(WM).toDouble(), get(Electrode).toDouble(), get(InternalAir).toDouble();

  // check if defined conductivities and lookup table are consistent
  if (conductivities.size() != ElemLabelLookup.size())
    THROW_ALGORITHM_INPUT_ERROR("Defined conductivities and lookup table are inconsistent! ");

  /// replacing field value with conductivity value
  FieldHandle output = CreateField(fi, fh->mesh());
  VField* ofield = output->vfield();
  int val = 0;
  int cnt = 0;

  for (VMesh::Elem::index_type i = 0; i < vfield->vmesh()->num_elems(); i++) // loop over all tetrahedral elements
  {
    vfield->get_value(val, i);  //get the data value stored on the current element

    bool found = false; // boolean that indicates if element label was found in lookup

     // loop over lookup table and check if the current element has one of the desired labels, if not error
    for (size_t j = 0; j < ElemLabelLookup.size(); ++j)
    {
      if (val == ElemLabelLookup[j])
      {
        ofield->set_value(conductivities[j], i); // if so, set it to the isotropic conductivity value
        found = true;
        break;
      }
    }

    if (!found)
    {
      THROW_ALGORITHM_INPUT_ERROR("Tetrahedral element label could not be found in lookup table. ");
    }

    cnt++;
    if (cnt == 500)
    {
      cnt = 0;
      update_progress_max(i,vfield->vmesh()->num_elems());
    }
  }

  return output;
}

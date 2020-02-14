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
 *  SplitFieldIntoNrrdData.cc:  Convert a Nrrd to a Field
 *
 *  Written by:
 *   David Weinstein
 *   School of Computing
 *   University of Utah
 *   February 2001
 *
 */

#include <Modules/Legacy/Teem/Converters/SplitFieldIntoNrrdData.h>
#include <Core/Algorithms/Legacy/Converter/ConvertToNrrd.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Teem;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Converters;

MODULE_INFO_DEF(SplitFieldIntoNrrdData, Converters, Teem)

SplitFieldIntoNrrdData::SplitFieldIntoNrrdData():
  Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Data);
  INITIALIZE_PORT(Points);
  INITIALIZE_PORT(Connections);
}

void SplitFieldIntoNrrdData::setStateDefaults()
{
  setStateStringFromAlgo(Parameters::DataLabel);
}

void
SplitFieldIntoNrrdData::execute()
{
  auto field_handle = getRequiredInput(InputField);

  // Just data for lattices, data and points for structured, all for rest.
  bool compute_points_p = true;
  bool compute_connects_p = true;
  bool compute_data_p = true;

  if (!field_handle->vmesh()->is_editable())
  {
    remark("Not computing connections for non-editable mesh type.");
    compute_connects_p = false;
  }

  if (field_handle->vfield()->basis_order() == -1)
  {
    remark("No data in input field.");
    compute_data_p = false;
  }

  if (needToExecute())
  {
    algo().set(Parameters::BuildPoints, compute_points_p);
    algo().set(Parameters::BuildConnections, compute_connects_p);
    algo().set(Parameters::BuildData, compute_data_p);
    setAlgoStringFromState(Parameters::DataLabel);

    auto output = algo().run(withInputData((InputField, field_handle)));
    sendOutputFromAlgorithm(Points, output);
    sendOutputFromAlgorithm(Data, output);
    sendOutputFromAlgorithm(Connections, output);

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // Set the Nrrd names and send them.
    std::string property;
    std::string nrrd_name = "Unknown";
    if (field_handle->get_property( "name", property ) && property != "Unknown")
      nrrd_name = property;

    if (points_handle.get_rep())
    {
      points_handle->set_property("Name", nrrd_name + "-Points", false);
      send_output_handle("Points", points_handle);
    }

    if (connect_handle.get_rep())
    {
      connect_handle->set_property("Name", nrrd_name + "-Connectivity", false);
      send_output_handle("Connections", connect_handle);
    }

    if (data_handle.get_rep())
    {
      data_handle->set_property("Name", nrrd_name + "-Data", false);
      send_output_handle("Data", data_handle);
    }
  #endif
  }
}

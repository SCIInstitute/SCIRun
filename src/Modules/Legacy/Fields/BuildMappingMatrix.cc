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

#include <Modules/Legacy/Fields/BuildMappingMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/BuildMappingMatrixAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

ModuleLookupInfo BuildMappingMatrix::staticInfo_("BuildMappingMatrix", "MiscField", "SCIRun");
const AlgorithmParameterName BuildMappingMatrix::MapSourceToSingleDestination("MapSourceToSingleDestination");

/// @class BuildMappingMatrix
/// @brief Build a mapping matrix -- a matrix that says how to project the data
/// from one field onto the data of a second field. 

BuildMappingMatrix::BuildMappingMatrix() : Module(staticInfo_)
{
}

void BuildMappingMatrix::setStateDefaults()
{
  auto state = get_state();
  state->setValue(BuildMappingMatrixAlgo::Method, std::string("linear"));
  state->setValue(BuildMappingMatrixAlgo::MaxDistance, -1.0);
  state->setValue(MapSourceToSingleDestination, false);
}

void
BuildMappingMatrix::execute()
{
  auto source = getRequiredInput(Source);
  auto destination = getRequiredInput(Destination);
  
  //if (inputs_changed_ || interpolation_basis_.changed() ||
  //    map_source_to_single_dest_.changed() ||
  //    exhaustive_search_max_dist_.changed() || !oport_cached("Mapping"))
  if (needToExecute())
  {
    update_state(Executing);
      
    auto state = get_state();
    std::string interpolation_basis = state->getValue(BuildMappingMatrixAlgo::Method).getString();
    if (interpolation_basis == "linear") 
    {
      algo().set_option(BuildMappingMatrixAlgo::Method, "interpolateddata");
    }
    else
    {
      if (state->getValue(MapSourceToSingleDestination).getBool())
        algo().set_option(BuildMappingMatrixAlgo::Method, "singledestination");
      else                
        algo().set_option(BuildMappingMatrixAlgo::Method, "closestdata");
    }
    
    setAlgoDoubleFromState(BuildMappingMatrixAlgo::MaxDistance);

    auto output = algo().run_generic(make_input((Source, source)(Destination, destination)));
   
    sendOutputFromAlgorithm(Mapping, output);
  }
}

/*  
 *  For more information, please see: http://software.sci.utah.edu
 *  
 *  The MIT License
 *  
 *  Copyright (c) 2009 Scientific Computing and Imaging Institute,
 *  University of Utah.
 *  
 *  License for the specific language governing rights and limitations under
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

/// @todo Documentation Modules/Legacy/Fields/FairMesh.cc

#include <Modules/Legacy/Fields/FairMesh.h>

#include <Core/Algorithms/Legacy/Fields/SmoothMesh/FairMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ModuleLookupInfo FairMesh::staticInfo_("FairMesh", "NewField", "SCIRun");

FairMesh::FairMesh() : 
  Module(staticInfo_)
{
  INITIALIZE_PORT(Input_Mesh);
  INITIALIZE_PORT(Faired_Mesh);
}

void FairMesh::setStateDefaults()
{
  auto state = get_state();

  setStateStringFromAlgoOption(Parameters::FairMeshMethod);
  setStateIntFromAlgo(Parameters::NumIterations);
  setStateDoubleFromAlgo(Parameters::Lambda);
  setStateDoubleFromAlgo(Parameters::FilterCutoff);
}

void FairMesh::execute()
{
  auto input = getRequiredInput(Input_Mesh);

#if SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (inputs_changed_ || iterations_.changed() ||
      method_.changed() || lambda_.changed() ||
      mu_.changed() || !oport_cached("Faired Mesh"))
#endif
  if (needToExecute())
  {
    update_state(Executing);
    auto state = get_state();
    setAlgoIntFromState(Parameters::NumIterations);
    setAlgoDoubleFromState(Parameters::Lambda);
    setAlgoDoubleFromState(Parameters::FilterCutoff);
    setAlgoOptionFromState(Parameters::FairMeshMethod);

    auto output = algo().run_generic(make_input((Input_Mesh, input)));

    sendOutputFromAlgorithm(Faired_Mesh, output);
  }
}

/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <string>
#include <boost/range/algorithm/count.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <boost/assign.hpp>
//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
#include <iostream>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;
using namespace boost::assign;

ALGORITHM_PARAMETER_DEF(BrainStimulator, TableValues);

const AlgorithmInputName ElectrodeCoilSetupAlgorithm::SCALP_SURF("SCALP_SURF");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::ELECTRODECOILPROTOTYPES("ELECTRODECOILPROTOTYPES");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::ELECTRODE_SPONGE_LOCATION_AVR("ELECTRODE_SPONGE_LOCATION_AVR");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::COILS_FIELD("COILS_FIELD");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::LOCATIONS("LOCATIONS");

VariableHandle ElectrodeCoilSetupAlgorithm::fill_table(FieldHandle scalp, DenseMatrixHandle locations, std::vector<FieldHandle> input) const
{
  Variable::List tmp;
  Variable::List table;
  if (locations->ncols()!=3)
  {
   THROW_ALGORITHM_PROCESSING_ERROR(" LOCATIONS needs to have dimensions such as: (#CoilsOrElectrodes) x 3 "); 
  }
  
  for (int i=0;i<locations->nrows();i++)
  {  
   tmp += makeVariable("Input #", "???"), 
     makeVariable("X", boost::str(boost::format("%.3f") % (* locations)(i,0))),
     makeVariable("Y", boost::str(boost::format("%.3f") % (* locations)(i,1))),
     makeVariable("Z", boost::str(boost::format("%.3f") % (* locations)(i,2))),
     makeVariable("RX", "???"),
     makeVariable("RY", "???"),
     makeVariable("RZ", "???"),
     makeVariable("thickness", "???"),
     makeVariable("Info", "???");     
     table.push_back(makeVariable("row" + boost::lexical_cast<std::string>(i), tmp)); 
  }  
 
  VariableHandle output(new Variable(Name("Table"), table));
  
  return output;
}


AlgorithmOutput ElectrodeCoilSetupAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto scalp = input.get<Field>(SCALP_SURF);
  auto locations = input.get<DenseMatrix>(LOCATIONS);
  auto elc_coil_proto = input.getList<Field>(ELECTRODECOILPROTOTYPES);
  
  if (!scalp)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" SCALP_SURF (first input) field empty. ");
  }
  
  if (!locations)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" LOCATIONS (second input) matrix empty. ");
  }
  
  if (!(elc_coil_proto.size()>=1))
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" At least one prototypical coil (POINTMESH) or electrode (TRISURFMESH) definition as a field input must be provided.");
  }
  
  VariableHandle table = fill_table(scalp, locations, elc_coil_proto);
  
  /*auto pos_orient = input.get<Field>(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  auto tri = input.get<Field>(ELECTRODE_TRIANGULATION);
  auto tri2 = input.get<Field>(ELECTRODE_TRIANGULATION2);
  auto coil = input.get<Field>(COIL);
  auto coil2 = input.get<Field>(COIL2);
  ENSURE_ALGORITHM_INPUT_NOT_NULL(pos_orient, "ELECTRODE_COIL_POSITIONS_AND_NORMAL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri, "ELECTRODE_TRIANGULATION input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri2, "ELECTRODE_TRIANGULATION2 input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil, "COIL input field");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil2, "COIL2 input field");*/
  //old-style run call, just put algorithm code here
  //auto outputs = run(boost::make_tuple(lhs, rhs), Option(get(Variables::AppendMatrixOption).toInt()));
  // CODE HERE
  DenseMatrixHandle elc_sponge_loc_avr;
  FieldHandle coils_field;
  //Algorithm starts here:
  //VField* vfield = elc_coil_pos_and_normal->vfield();
  // VMesh*  vmesh  = pos_orient->vmesh();
 
   //std::cout << "a: " << vmesh->num_nodes() << std::endl;
   //for (int i=0;i<vmesh->num_nodes();;i++)
   //{
   
   
   //}
  //


  AlgorithmOutput output;
  output[ELECTRODE_SPONGE_LOCATION_AVR] = elc_sponge_loc_avr;
  output[COILS_FIELD] = coils_field;
  
  output.setAdditionalAlgoOutput(table);
  return output;
}
